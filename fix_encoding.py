"""Replace high bytes (>=0x80) inside string/char literals with \\xNN hex escapes."""
import sys

def hex_escape_high_bytes(line):
    result = bytearray()
    in_string = False
    in_char = False
    BACKSLASH = 0x5C
    QUOTE = ord('"')
    APOS = ord("'")
    for b in line:
        if not in_string and not in_char:
            if b == QUOTE:
                in_string = True
            elif b == APOS:
                in_char = True
            result.append(b)
        elif in_string:
            if b == QUOTE:
                in_string = False
                result.append(b)
            elif b >= 0x80:
                h = '%02x' % b
                result.extend([BACKSLASH, ord('x'), ord(h[0]), ord(h[1])])
            else:
                result.append(b)
        elif in_char:
            if b == APOS:
                in_char = False
                result.append(b)
            elif b >= 0x80:
                h = '%02x' % b
                result.extend([BACKSLASH, ord('x'), ord(h[0]), ord(h[1])])
            else:
                result.append(b)
    return bytes(result)

filename = sys.argv[1]
target_lines = [int(x) for x in sys.argv[2:]]  # 1-based line numbers

with open(filename, 'rb') as f:
    data = f.read()

lines = data.split(b'\n')

changed = 0
for lnum in target_lines:
    idx = lnum - 1
    old = lines[idx]
    new = hex_escape_high_bytes(old)
    if old != new:
        old_high = sum(1 for b in old if b >= 0x80)
        new_high = sum(1 for b in new if b >= 0x80)
        print(f'L{lnum}: {len(old)} -> {len(new)} bytes (high: {old_high} -> {new_high})')
        lines[idx] = new
        changed += 1
    else:
        print(f'L{lnum}: no change')

if changed:
    with open(filename, 'wb') as f:
        f.write(b'\n'.join(lines))
    print(f'Written {changed} changes to {filename}')
else:
    print('No changes needed.')
