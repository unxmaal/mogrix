#!/usr/bin/env python3
"""
Simple RPM repository metadata generator.
Creates rpm-md format metadata that tdnf/libsolv can read.
"""

import os
import sys
import gzip
import hashlib
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path
from datetime import datetime

def get_rpm_info(rpm_path):
    """Extract metadata from an RPM file using rpm command."""
    # Query format for all needed fields
    qf = (
        '%{NAME}\\n'
        '%{EPOCH}\\n'
        '%{VERSION}\\n'
        '%{RELEASE}\\n'
        '%{ARCH}\\n'
        '%{SUMMARY}\\n'
        '%{DESCRIPTION}\\n'
        '%{SIZE}\\n'
        '%{LICENSE}\\n'
        '%{GROUP}\\n'
        '%{BUILDTIME}\\n'
        '%{SOURCERPM}\\n'
        '%{VENDOR}\\n'
        '%{URL}\\n'
        '%{PACKAGER}\\n'
    )

    result = subprocess.run(
        ['rpm', '-qp', '--qf', qf, rpm_path],
        capture_output=True, text=True
    )

    if result.returncode != 0:
        print(f"Error querying {rpm_path}: {result.stderr}", file=sys.stderr)
        return None

    lines = result.stdout.split('\n')

    # Get provides
    provides_result = subprocess.run(
        ['rpm', '-qp', '--provides', rpm_path],
        capture_output=True, text=True
    )
    provides = [p.strip() for p in provides_result.stdout.strip().split('\n') if p.strip()]

    # Get requires
    requires_result = subprocess.run(
        ['rpm', '-qp', '--requires', rpm_path],
        capture_output=True, text=True
    )
    requires = [r.strip() for r in requires_result.stdout.strip().split('\n') if r.strip()]

    # Get file list
    files_result = subprocess.run(
        ['rpm', '-qp', '-l', rpm_path],
        capture_output=True, text=True
    )
    files = [f.strip() for f in files_result.stdout.strip().split('\n') if f.strip()]

    # File size and checksum
    file_size = os.path.getsize(rpm_path)
    with open(rpm_path, 'rb') as f:
        sha256 = hashlib.sha256(f.read()).hexdigest()

    epoch = lines[1] if lines[1] and lines[1] != '(none)' else '0'

    return {
        'name': lines[0],
        'epoch': epoch,
        'version': lines[2],
        'release': lines[3],
        'arch': lines[4],
        'summary': lines[5],
        'description': lines[6],
        'installed_size': lines[7],
        'license': lines[8],
        'group': lines[9] if lines[9] != '(none)' else 'Unspecified',
        'buildtime': lines[10],
        'sourcerpm': lines[11] if lines[11] != '(none)' else '',
        'vendor': lines[12] if lines[12] != '(none)' else '',
        'url': lines[13] if lines[13] != '(none)' else '',
        'packager': lines[14] if lines[14] != '(none)' else '',
        'provides': provides,
        'requires': requires,
        'files': files,
        'file_size': file_size,
        'sha256': sha256,
        'filename': os.path.basename(rpm_path),
        'location': os.path.basename(rpm_path),
    }

def xml_escape(s):
    """Escape special XML characters."""
    if not s:
        return ''
    return (s.replace('&', '&amp;')
             .replace('<', '&lt;')
             .replace('>', '&gt;')
             .replace('"', '&quot;')
             .replace("'", '&apos;'))

def generate_primary_xml(packages):
    """Generate primary.xml content."""
    lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        '<metadata xmlns="http://linux.duke.edu/metadata/common" '
        'xmlns:rpm="http://linux.duke.edu/metadata/rpm" '
        f'packages="{len(packages)}">',
    ]

    for pkg in packages:
        nevra = f"{pkg['name']}-{pkg['version']}-{pkg['release']}.{pkg['arch']}"
        pkgid = pkg['sha256']

        lines.append(f'  <package type="rpm">')
        lines.append(f'    <name>{xml_escape(pkg["name"])}</name>')
        lines.append(f'    <arch>{xml_escape(pkg["arch"])}</arch>')
        lines.append(f'    <version epoch="{pkg["epoch"]}" ver="{xml_escape(pkg["version"])}" rel="{xml_escape(pkg["release"])}"/>')
        lines.append(f'    <checksum type="sha256" pkgid="YES">{pkg["sha256"]}</checksum>')
        lines.append(f'    <summary>{xml_escape(pkg["summary"])}</summary>')
        lines.append(f'    <description>{xml_escape(pkg["description"])}</description>')
        lines.append(f'    <packager>{xml_escape(pkg["packager"])}</packager>')
        lines.append(f'    <url>{xml_escape(pkg["url"])}</url>')
        lines.append(f'    <time file="{pkg["buildtime"]}" build="{pkg["buildtime"]}"/>')
        lines.append(f'    <size package="{pkg["file_size"]}" installed="{pkg["installed_size"]}" archive="{pkg["installed_size"]}"/>')
        lines.append(f'    <location href="{xml_escape(pkg["location"])}"/>')
        lines.append(f'    <format>')
        lines.append(f'      <rpm:license>{xml_escape(pkg["license"])}</rpm:license>')
        lines.append(f'      <rpm:group>{xml_escape(pkg["group"])}</rpm:group>')
        lines.append(f'      <rpm:buildhost>localhost</rpm:buildhost>')
        lines.append(f'      <rpm:sourcerpm>{xml_escape(pkg["sourcerpm"])}</rpm:sourcerpm>')

        # Provides
        if pkg['provides']:
            lines.append(f'      <rpm:provides>')
            for p in pkg['provides']:
                # Parse "name = version" or just "name"
                if ' = ' in p:
                    pname, pver = p.split(' = ', 1)
                    lines.append(f'        <rpm:entry name="{xml_escape(pname)}" flags="EQ" ver="{xml_escape(pver)}"/>')
                else:
                    lines.append(f'        <rpm:entry name="{xml_escape(p)}"/>')
            lines.append(f'      </rpm:provides>')

        # Requires
        if pkg['requires']:
            lines.append(f'      <rpm:requires>')
            for r in pkg['requires']:
                # Skip rpmlib requirements
                if r.startswith('rpmlib('):
                    continue
                # Parse version requirements
                for op, flag in [(' >= ', 'GE'), (' <= ', 'LE'), (' > ', 'GT'), (' < ', 'LT'), (' = ', 'EQ')]:
                    if op in r:
                        rname, rver = r.split(op, 1)
                        lines.append(f'        <rpm:entry name="{xml_escape(rname)}" flags="{flag}" ver="{xml_escape(rver)}"/>')
                        break
                else:
                    lines.append(f'        <rpm:entry name="{xml_escape(r)}"/>')
            lines.append(f'      </rpm:requires>')

        lines.append(f'    </format>')
        lines.append(f'  </package>')

    lines.append('</metadata>')
    return '\n'.join(lines)

def generate_filelists_xml(packages):
    """Generate filelists.xml content."""
    lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<filelists xmlns="http://linux.duke.edu/metadata/filelists" packages="{len(packages)}">',
    ]

    for pkg in packages:
        lines.append(f'  <package pkgid="{pkg["sha256"]}" name="{xml_escape(pkg["name"])}" arch="{xml_escape(pkg["arch"])}">')
        lines.append(f'    <version epoch="{pkg["epoch"]}" ver="{xml_escape(pkg["version"])}" rel="{xml_escape(pkg["release"])}"/>')
        for f in pkg['files'][:100]:  # Limit to first 100 files to keep size down
            lines.append(f'    <file>{xml_escape(f)}</file>')
        lines.append(f'  </package>')

    lines.append('</filelists>')
    return '\n'.join(lines)

def generate_other_xml(packages):
    """Generate other.xml content (changelogs - we'll keep it minimal)."""
    lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<otherdata xmlns="http://linux.duke.edu/metadata/other" packages="{len(packages)}">',
    ]

    for pkg in packages:
        lines.append(f'  <package pkgid="{pkg["sha256"]}" name="{xml_escape(pkg["name"])}" arch="{xml_escape(pkg["arch"])}">')
        lines.append(f'    <version epoch="{pkg["epoch"]}" ver="{xml_escape(pkg["version"])}" rel="{xml_escape(pkg["release"])}"/>')
        lines.append(f'  </package>')

    lines.append('</otherdata>')
    return '\n'.join(lines)

def write_gzipped(content, path):
    """Write gzipped content and return checksums."""
    data = content.encode('utf-8')

    with gzip.open(path, 'wb') as f:
        f.write(data)

    with open(path, 'rb') as f:
        gz_data = f.read()

    return {
        'size': len(data),
        'gz_size': len(gz_data),
        'checksum': hashlib.sha256(gz_data).hexdigest(),
        'open_checksum': hashlib.sha256(data).hexdigest(),
    }

def generate_repomd_xml(primary_info, filelists_info, other_info, timestamp):
    """Generate repomd.xml content."""
    lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        '<repomd xmlns="http://linux.duke.edu/metadata/repo">',
        f'  <revision>{timestamp}</revision>',
    ]

    for dtype, info, filename in [
        ('primary', primary_info, 'primary.xml.gz'),
        ('filelists', filelists_info, 'filelists.xml.gz'),
        ('other', other_info, 'other.xml.gz'),
    ]:
        lines.append(f'  <data type="{dtype}">')
        lines.append(f'    <checksum type="sha256">{info["checksum"]}</checksum>')
        lines.append(f'    <open-checksum type="sha256">{info["open_checksum"]}</open-checksum>')
        lines.append(f'    <location href="repodata/{filename}"/>')
        lines.append(f'    <timestamp>{timestamp}</timestamp>')
        lines.append(f'    <size>{info["gz_size"]}</size>')
        lines.append(f'    <open-size>{info["size"]}</open-size>')
        lines.append(f'  </data>')

    lines.append('</repomd>')
    return '\n'.join(lines)

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <repo_directory>", file=sys.stderr)
        print("Creates rpm-md metadata in <repo_directory>/repodata/", file=sys.stderr)
        sys.exit(1)

    repo_dir = Path(sys.argv[1])
    if not repo_dir.is_dir():
        print(f"Error: {repo_dir} is not a directory", file=sys.stderr)
        sys.exit(1)

    # Find all RPM files
    rpm_files = list(repo_dir.glob('*.rpm'))
    if not rpm_files:
        print(f"No RPM files found in {repo_dir}", file=sys.stderr)
        sys.exit(1)

    print(f"Found {len(rpm_files)} RPM files")

    # Extract metadata from each RPM
    packages = []
    for rpm_file in sorted(rpm_files):
        print(f"Processing: {rpm_file.name}")
        info = get_rpm_info(str(rpm_file))
        if info:
            packages.append(info)

    if not packages:
        print("No packages could be processed", file=sys.stderr)
        sys.exit(1)

    # Create repodata directory
    repodata_dir = repo_dir / 'repodata'
    repodata_dir.mkdir(exist_ok=True)

    timestamp = int(datetime.now().timestamp())

    # Generate and write primary.xml.gz
    print("Generating primary.xml.gz...")
    primary_xml = generate_primary_xml(packages)
    primary_info = write_gzipped(primary_xml, repodata_dir / 'primary.xml.gz')

    # Generate and write filelists.xml.gz
    print("Generating filelists.xml.gz...")
    filelists_xml = generate_filelists_xml(packages)
    filelists_info = write_gzipped(filelists_xml, repodata_dir / 'filelists.xml.gz')

    # Generate and write other.xml.gz
    print("Generating other.xml.gz...")
    other_xml = generate_other_xml(packages)
    other_info = write_gzipped(other_xml, repodata_dir / 'other.xml.gz')

    # Generate and write repomd.xml
    print("Generating repomd.xml...")
    repomd_xml = generate_repomd_xml(primary_info, filelists_info, other_info, timestamp)
    with open(repodata_dir / 'repomd.xml', 'w') as f:
        f.write(repomd_xml)

    print(f"\nRepository metadata created in {repodata_dir}")
    print(f"Packages: {len(packages)}")

if __name__ == '__main__':
    main()
