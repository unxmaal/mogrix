#!/usr/bin/env python3
"""Transform MiniBrowser sources into ir8 sources.

Performs three operations in one pass:
1. Strip GTK4 code (#if GTK_CHECK_VERSION(3, 98+) blocks)
2. Rename Browser* -> Ir8*, browser_* -> ir8_*, etc.
3. Remove dead code (automation, GStreamer, content filter, ITP, cmakeconfig)
"""

import re
import os
import sys

SRC = "/home/edodd/rpmbuild/BUILD/webkitgtk-2.42.5.origfedora/webkitgtk-2.42.5/Tools/MiniBrowser/gtk"
DST = "/home/edodd/projects/github/unxmaal/mogrix/patches/packages/ir8"

FILE_MAP = {
    "BrowserWindow.h": "ir8-window.h",
    "BrowserWindow.c": "ir8-window.c",
    "BrowserTab.h": "ir8-tab.h",
    "BrowserTab.c": "ir8-tab.c",
    "BrowserSearchBox.h": "ir8-searchbox.h",
    "BrowserSearchBox.c": "ir8-searchbox.c",
    "BrowserDownloadsBar.h": "ir8-downloads.h",
    "BrowserDownloadsBar.c": "ir8-downloads.c",
    "BrowserSettingsDialog.h": "ir8-settings.h",
    "BrowserSettingsDialog.c": "ir8-settings.c",
    "BrowserCellRendererVariant.h": "ir8-cellrenderer.h",
    "BrowserCellRendererVariant.c": "ir8-cellrenderer.c",
    "browser-marshal.list": "ir8-marshal.list",
    "main.c": "main.c",
}

# Ordered by longest-first to prevent partial matches
RENAMES = [
    # Include guards
    ("BrowserCellRendererVariant_h", "Ir8CellRendererVariant_h"),
    ("BrowserSettingsDialog_h", "Ir8SettingsDialog_h"),
    ("BrowserDownloadsBar_h", "Ir8DownloadsBar_h"),
    ("BrowserSearchBox_h", "Ir8SearchBox_h"),
    ("BrowserWindow_h", "Ir8Window_h"),
    ("BrowserTab_h", "Ir8Tab_h"),
    # GType macros (upper case) - longest first
    ("BROWSER_CELL_RENDERER_VARIANT_GET_CLASS", "IR8_CELL_RENDERER_VARIANT_GET_CLASS"),
    ("BROWSER_IS_CELL_RENDERER_VARIANT_CLASS", "IR8_IS_CELL_RENDERER_VARIANT_CLASS"),
    ("BROWSER_CELL_RENDERER_VARIANT_CLASS", "IR8_CELL_RENDERER_VARIANT_CLASS"),
    ("BROWSER_TYPE_CELL_RENDERER_VARIANT", "IR8_TYPE_CELL_RENDERER_VARIANT"),
    ("BROWSER_IS_CELL_RENDERER_VARIANT", "IR8_IS_CELL_RENDERER_VARIANT"),
    ("BROWSER_CELL_RENDERER_VARIANT", "IR8_CELL_RENDERER_VARIANT"),
    ("BROWSER_SETTINGS_DIALOG_GET_CLASS", "IR8_SETTINGS_DIALOG_GET_CLASS"),
    ("BROWSER_IS_SETTINGS_DIALOG_CLASS", "IR8_IS_SETTINGS_DIALOG_CLASS"),
    ("BROWSER_SETTINGS_DIALOG_CLASS", "IR8_SETTINGS_DIALOG_CLASS"),
    ("BROWSER_TYPE_SETTINGS_DIALOG", "IR8_TYPE_SETTINGS_DIALOG"),
    ("BROWSER_IS_SETTINGS_DIALOG", "IR8_IS_SETTINGS_DIALOG"),
    ("BROWSER_SETTINGS_DIALOG", "IR8_SETTINGS_DIALOG"),
    ("BROWSER_DOWNLOADS_BAR_GET_CLASS", "IR8_DOWNLOADS_BAR_GET_CLASS"),
    ("BROWSER_IS_DOWNLOADS_BAR_CLASS", "IR8_IS_DOWNLOADS_BAR_CLASS"),
    ("BROWSER_DOWNLOADS_BAR_CLASS", "IR8_DOWNLOADS_BAR_CLASS"),
    ("BROWSER_TYPE_DOWNLOADS_BAR", "IR8_TYPE_DOWNLOADS_BAR"),
    ("BROWSER_IS_DOWNLOADS_BAR", "IR8_IS_DOWNLOADS_BAR"),
    ("BROWSER_DOWNLOADS_BAR", "IR8_DOWNLOADS_BAR"),
    ("BROWSER_SEARCH_BOX_GET_CLASS", "IR8_SEARCH_BOX_GET_CLASS"),
    ("BROWSER_IS_SEARCH_BOX_CLASS", "IR8_IS_SEARCH_BOX_CLASS"),
    ("BROWSER_SEARCH_BOX_CLASS", "IR8_SEARCH_BOX_CLASS"),
    ("BROWSER_TYPE_SEARCH_BOX", "IR8_TYPE_SEARCH_BOX"),
    ("BROWSER_IS_SEARCH_BOX", "IR8_IS_SEARCH_BOX"),
    ("BROWSER_SEARCH_BOX", "IR8_SEARCH_BOX"),
    ("BROWSER_WINDOW_GET_CLASS", "IR8_WINDOW_GET_CLASS"),
    ("BROWSER_IS_WINDOW_CLASS", "IR8_IS_WINDOW_CLASS"),
    ("BROWSER_WINDOW_CLASS", "IR8_WINDOW_CLASS"),
    ("BROWSER_TYPE_WINDOW", "IR8_TYPE_WINDOW"),
    ("BROWSER_IS_WINDOW", "IR8_IS_WINDOW"),
    ("BROWSER_WINDOW", "IR8_WINDOW"),
    ("BROWSER_TAB_GET_CLASS", "IR8_TAB_GET_CLASS"),
    ("BROWSER_IS_TAB_CLASS", "IR8_IS_TAB_CLASS"),
    ("BROWSER_TAB_CLASS", "IR8_TAB_CLASS"),
    ("BROWSER_TYPE_TAB", "IR8_TYPE_TAB"),
    ("BROWSER_IS_TAB", "IR8_IS_TAB"),
    ("BROWSER_TAB", "IR8_TAB"),
    ("BROWSER_TYPE_DOWNLOAD", "IR8_TYPE_DOWNLOAD"),
    ("BROWSER_DOWNLOAD", "IR8_DOWNLOAD"),
    # Constants
    ("BROWSER_DEFAULT_URL", "IR8_DEFAULT_URL"),
    ("BROWSER_ABOUT_SCHEME", "IR8_ABOUT_SCHEME"),
    ("MINI_BROWSER_ERROR_INVALID_ABOUT_PATH", "IR8_ERROR_INVALID_ABOUT_PATH"),
    ("MINI_BROWSER_ERROR", "IR8_ERROR"),
    # Type names
    ("BrowserCellRendererVariantClass", "Ir8CellRendererVariantClass"),
    ("BrowserCellRendererVariant", "Ir8CellRendererVariant"),
    ("BrowserSettingsDialogClass", "Ir8SettingsDialogClass"),
    ("BrowserSettingsDialog", "Ir8SettingsDialog"),
    ("BrowserDownloadsBarClass", "Ir8DownloadsBarClass"),
    ("BrowserDownloadsBar", "Ir8DownloadsBar"),
    ("BrowserSearchBoxClass", "Ir8SearchBoxClass"),
    ("BrowserSearchBox", "Ir8SearchBox"),
    ("BrowserWindowClass", "Ir8WindowClass"),
    ("BrowserWindow", "Ir8Window"),
    ("BrowserTabClass", "Ir8TabClass"),
    ("BrowserTab", "Ir8Tab"),
    ("BrowserDownloadClass", "Ir8DownloadClass"),
    ("BrowserDownload", "Ir8Download"),
    # Function/variable prefixes (snake_case)
    ("browser_cell_renderer_variant", "ir8_cell_renderer_variant"),
    ("browser_settings_dialog", "ir8_settings_dialog"),
    ("browser_downloads_bar", "ir8_downloads_bar"),
    ("browser_search_box", "ir8_search_box"),
    ("browser_download", "ir8_download"),
    ("browser_window", "ir8_window"),
    ("browser_tab", "ir8_tab"),
    # camelCase internal functions
    ("browserWindow", "ir8Window"),
    ("browserTab", "ir8Tab"),
    ("browserDownloadsBar", "ir8DownloadsBar"),
    ("browserSearchBox", "ir8SearchBox"),
    ("browserSettingsDialog", "ir8SettingsDialog"),
    ("browserCellRendererVariant", "ir8CellRendererVariant"),
    ("browserDownload", "ir8Download"),
    # Marshal
    ("browser_marshal", "ir8_marshal"),
    ("BrowserMarshal", "Ir8Marshal"),
    # Include files
    ('"BrowserWindow.h"', '"ir8-window.h"'),
    ('"BrowserTab.h"', '"ir8-tab.h"'),
    ('"BrowserSearchBox.h"', '"ir8-searchbox.h"'),
    ('"BrowserDownloadsBar.h"', '"ir8-downloads.h"'),
    ('"BrowserSettingsDialog.h"', '"ir8-settings.h"'),
    ('"BrowserCellRendererVariant.h"', '"ir8-cellrenderer.h"'),
    # Error quark
    ("MiniBrowserError", "Ir8Error"),
    ("miniBrowserErrorQuark", "ir8ErrorQuark"),
    ('"minibrowser-quark"', '"ir8-quark"'),
    # Branding
    ('"WebKitGTK MiniBrowser"', '"ir8"'),
    ('"MiniBrowser"', '"ir8"'),
    ('"org.webkitgtk.MiniBrowser"', '"com.mogrix.ir8"'),
]


def strip_gtk4_blocks(text):
    """Remove GTK4 conditional blocks, keeping GTK3 code.

    Handles:
    - #if GTK_CHECK_VERSION(3, 98+, X) ... #else ... #endif → keep #else block
    - #if GTK_CHECK_VERSION(3, 98+, X) ... #endif (no else) → remove entirely
    - #if !GTK_CHECK_VERSION(3, 98, 0) ... #endif → keep the block content
    - Nested blocks handled via recursive processing
    """
    lines = text.split('\n')
    result = []
    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.strip()

        # Check for GTK version check
        m = re.match(r'\s*#\s*if\s+(!?)\s*GTK_CHECK_VERSION\s*\(\s*3\s*,\s*(\d+)\s*,\s*(\d+)\s*\)', stripped)
        if m:
            negated = m.group(1) == '!'
            minor = int(m.group(2))

            if minor >= 98:
                # This is a GTK4 version check
                # Find matching #else and #endif
                depth = 1
                gtk4_block = []
                else_block = []
                in_else = False
                j = i + 1
                while j < len(lines) and depth > 0:
                    s = lines[j].strip()
                    if s.startswith('#if ') or s.startswith('#if\t') or s == '#if' or s.startswith('#ifdef') or s.startswith('#ifndef'):
                        depth += 1
                        if in_else and depth > 1:
                            else_block.append(lines[j])
                        elif not in_else:
                            gtk4_block.append(lines[j])
                    elif s.startswith('#else') and depth == 1:
                        in_else = True
                    elif s.startswith('#endif') and depth == 1:
                        depth -= 1
                    elif s.startswith('#endif'):
                        depth -= 1
                        if in_else:
                            else_block.append(lines[j])
                        else:
                            gtk4_block.append(lines[j])
                    else:
                        if in_else:
                            else_block.append(lines[j])
                        else:
                            gtk4_block.append(lines[j])
                    j += 1

                if negated:
                    # #if !GTK_CHECK_VERSION → this IS GTK3 code, keep it
                    result.extend(gtk4_block)  # "gtk4_block" is actually the GTK3 block here
                else:
                    # #if GTK_CHECK_VERSION → GTK4 code, keep #else (GTK3) block
                    if in_else:
                        result.extend(else_block)
                    # If no #else, the whole block is GTK4-only, discard it

                i = j
                continue

        # Check for SOUP version check blocks
        m2 = re.match(r'\s*#\s*if\s+SOUP_CHECK_VERSION\s*\(\s*2\s*,\s*91\s*,\s*0\s*\)', stripped)
        if m2:
            # soup3 check - keep the #else (soup2) content
            depth = 1
            soup3_block = []
            else_block = []
            in_else = False
            j = i + 1
            while j < len(lines) and depth > 0:
                s = lines[j].strip()
                if s.startswith('#if ') or s.startswith('#if\t') or s == '#if' or s.startswith('#ifdef') or s.startswith('#ifndef'):
                    depth += 1
                    if in_else:
                        else_block.append(lines[j])
                    else:
                        soup3_block.append(lines[j])
                elif s.startswith('#else') and depth == 1:
                    in_else = True
                elif s.startswith('#endif') and depth == 1:
                    depth -= 1
                elif s.startswith('#endif'):
                    depth -= 1
                    if in_else:
                        else_block.append(lines[j])
                    else:
                        soup3_block.append(lines[j])
                else:
                    if in_else:
                        else_block.append(lines[j])
                    else:
                        soup3_block.append(lines[j])
                j += 1

            if in_else:
                result.extend(else_block)
            # If no else, discard (soup3-only code)
            i = j
            continue

        result.append(line)
        i += 1

    return '\n'.join(result)


def remove_dead_code(text, filename):
    """Remove dead code sections."""
    lines = text.split('\n')
    result = []
    skip_until_endif = 0

    for line in lines:
        stripped = line.strip()

        # Remove cmakeconfig.h include
        if stripped == '#include "cmakeconfig.h"':
            continue

        # Remove BuildRevision.h include
        if stripped == '#include "BuildRevision.h"':
            continue

        # Remove GStreamer includes
        if 'gst/gst.h' in stripped:
            continue
        if '#if !USE_GSTREAMER_FULL' in stripped or '#if USE_GSTREAMER_FULL' in stripped:
            skip_until_endif += 1
            continue

        if skip_until_endif > 0:
            if stripped.startswith('#endif'):
                skip_until_endif -= 1
            continue

        result.append(line)

    return '\n'.join(result)


def apply_renames(text):
    """Apply all rename substitutions."""
    for old, new in RENAMES:
        text = text.replace(old, new)
    return text


def update_default_url(text):
    """Update the default URL constant value."""
    text = text.replace(
        '#define IR8_DEFAULT_URL            "http://www.webkitgtk.org/"',
        '#define IR8_DEFAULT_URL            "ir8-about:home"'
    )
    return text


def update_about_scheme(text):
    """Update the about scheme constant value."""
    text = text.replace(
        '#define IR8_ABOUT_SCHEME           "minibrowser-about"',
        '#define IR8_ABOUT_SCHEME           "ir8-about"'
    )
    return text


def process_file(src_name, dst_name):
    src_path = os.path.join(SRC, src_name)
    dst_path = os.path.join(DST, dst_name)

    with open(src_path, 'r') as f:
        text = f.read()

    # Step 1: Strip GTK4 blocks
    text = strip_gtk4_blocks(text)

    # Step 2: Remove dead code
    text = remove_dead_code(text, dst_name)

    # Step 3: Apply renames
    text = apply_renames(text)

    # Step 4: Update constants
    text = update_default_url(text)
    text = update_about_scheme(text)

    # Step 5: Remove webkit/webkit.h (GTK4 header), keep webkit2/webkit2.h
    text = text.replace('#include <webkit/webkit.h>\n', '')

    with open(dst_path, 'w') as f:
        f.write(text)

    print(f"  {src_name} -> {dst_name}")


def main():
    print("Transforming MiniBrowser -> ir8...")
    for src_name, dst_name in FILE_MAP.items():
        process_file(src_name, dst_name)
    print("Done!")


if __name__ == '__main__':
    main()
