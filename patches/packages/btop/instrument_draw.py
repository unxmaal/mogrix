#!/usr/bin/env python3
"""Instrument btop_draw.cpp Proc::draw with fprintf markers every ~10 lines."""
import sys

def instrument(filepath):
    with open(filepath) as f:
        lines = f.readlines()

    # Find Proc::draw function boundaries
    # The function starts after 'namespace Proc {' with 'string draw('
    # and ends at the '}' that closes the namespace (line with just '}' at col 0)
    proc_draw_start = None
    proc_draw_end = None
    in_proc_ns = False

    for i, line in enumerate(lines):
        if 'namespace Proc {' in line:
            in_proc_ns = True
            continue
        if in_proc_ns and proc_draw_start is None:
            if 'string draw(' in line:
                proc_draw_start = i
                continue
        # The real end of the function is the '}' closing namespace Proc
        # which appears as a line starting with '}' followed by empty line + 'namespace Draw'
        if in_proc_ns and proc_draw_start is not None:
            if line.strip() == '}' and i + 2 < len(lines) and 'namespace Draw' in lines[i + 2]:
                proc_draw_end = i
                break

    if proc_draw_start is None:
        print("ERROR: Could not find Proc::draw", file=sys.stderr)
        sys.exit(1)

    print(f"Proc::draw: lines {proc_draw_start+1} to {proc_draw_end+1}")

    # Insert markers at statement boundaries (~every 15 lines) inside the function
    marker_count = 0
    lines_since_marker = 0
    output = []
    for i, line in enumerate(lines):
        output.append(line)
        if proc_draw_start < i < proc_draw_end:
            lines_since_marker += 1
            stripped = line.rstrip()
            # Only insert after complete statements (ending with ; or {)
            # and not inside string literals or comments
            if lines_since_marker >= 15 and stripped and (stripped.endswith(';') or stripped.endswith('{')):
                # Don't insert inside for/if/else continuation
                next_line = lines[i+1].strip() if i+1 < len(lines) else ''
                if not next_line.startswith('else') and not next_line.startswith('catch'):
                    marker_count += 1
                    indent = '\t\t'
                    marker = f'{indent}fprintf(stderr, "IRIX_DBG: Proc::draw L{i+1}\\n");\n'
                    output.append(marker)
                    lines_since_marker = 0

    with open(filepath, 'w') as f:
        f.writelines(output)

    print(f"Inserted {marker_count} markers in Proc::draw")

if __name__ == '__main__':
    instrument(sys.argv[1] if len(sys.argv) > 1 else 'src/btop_draw.cpp')
