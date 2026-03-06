"""Scan all source files for high bytes (>=0x80) inside string/char literals."""
import os, glob

def check_file(filepath):
    with open(filepath, 'rb') as f:
        data = f.read()
    lines = data.split(b'\n')
    hits = []
    for i, line in enumerate(lines):
        if not any(b >= 0x80 for b in line):
            continue
        # Skip lines that are pure comments
        stripped = line.lstrip()
        if stripped.startswith(b'//'):
            continue
        # Check if any high bytes are inside quote marks
        in_str = False
        in_chr = False
        high_in_literal = False
        for b in line:
            if not in_str and not in_chr:
                if b == ord('"'): in_str = True
                elif b == ord("'"): in_chr = True
            elif in_str:
                if b == ord('"'): in_str = False
                elif b >= 0x80: high_in_literal = True
            elif in_chr:
                if b == ord("'"): in_chr = False
                elif b >= 0x80: high_in_literal = True
        if high_in_literal:
            hits.append(i + 1)
    return hits

for pat in ['src/**/*.c', 'src/**/*.h', 'src/**/*.cpp']:
    for f in glob.glob(pat, recursive=True):
        fp = f.replace(os.sep, '/')
        hits = check_file(f)
        if hits:
            with open(f, 'rb') as fh:
                data = fh.read()
            lines = data.split(b'\n')
            print(f'{fp}: {len(hits)} lines with high bytes in literals')
            for lnum in hits:
                line = lines[lnum - 1]
                # Show with hex for high bytes
                display = ''
                for b in line.rstrip()[:120]:
                    if b >= 0x80:
                        display += '[%02x]' % b
                    elif 0x20 <= b < 0x7f:
                        display += chr(b)
                    else:
                        display += '.'
                print(f'  L{lnum}: {display}')
