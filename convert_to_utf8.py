"""Convert Latin-1 source files to UTF-8. Only touches files that are not already ASCII/UTF-8."""
import os
import glob

converted = 0
skipped = 0
errors = 0

for pat in ['src/**/*.c', 'src/**/*.h', 'src/**/*.cpp']:
    for filepath in glob.glob(pat, recursive=True):
        with open(filepath, 'rb') as f:
            data = f.read()

        # Skip files that are pure ASCII (valid in any encoding)
        if all(b < 0x80 for b in data):
            skipped += 1
            continue

        # Check if already valid UTF-8
        try:
            data.decode('utf-8')
            skipped += 1
            continue
        except UnicodeDecodeError:
            pass

        # It's Latin-1 — convert to UTF-8
        try:
            text = data.decode('latin-1')
            utf8_data = text.encode('utf-8')
            with open(filepath, 'wb') as f:
                f.write(utf8_data)
            fp = filepath.replace(os.sep, '/')
            print(f'  Converted: {fp} ({len(data)} -> {len(utf8_data)} bytes)')
            converted += 1
        except Exception as e:
            fp = filepath.replace(os.sep, '/')
            print(f'  ERROR: {fp}: {e}')
            errors += 1

print(f'\nDone: {converted} converted, {skipped} skipped (already ASCII/UTF-8), {errors} errors')
