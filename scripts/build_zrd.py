#!/usr/bin/env python3
"""
build_zrd.py - Build a ZRD ramdisk image for zuzu386
Usage: python3 build_zrd.py <output.zrd> <file1> [file2] ...
   or: python3 build_zrd.py <output.zrd> --dir <directory>

ZRD Format:
  [header]
    magic:   4 bytes  "ZRD\0"
    count:   4 bytes  number of files
  [file entries] x count
    name:    32 bytes  null-terminated filename
    offset:  4 bytes   offset from start of ramdisk
    size:    4 bytes   file size in bytes
  [file data]
    raw bytes of each file, packed sequentially
"""

import sys
import os
import struct
import argparse

MAGIC = b'ZRD\x00'
MAX_NAME_LEN = 31  # 32 bytes including null terminator
HEADER_SIZE = 8    # magic(4) + count(4)
ENTRY_SIZE = 40    # name(32) + offset(4) + size(4)

def build_zrd(output_path, input_files):
    files = []

    for path in input_files:
        if not os.path.isfile(path):
            print(f"Warning: {path} is not a file, skipping")
            continue

        name = os.path.basename(path)
        if len(name) > MAX_NAME_LEN:
            print(f"Warning: filename '{name}' is too long (max {MAX_NAME_LEN} chars), truncating")
            name = name[:MAX_NAME_LEN]

        with open(path, 'rb') as f:
            data = f.read()

        files.append((name, data))
        print(f"  + {name} ({len(data)} bytes)")

    if not files:
        print("Error: no files to pack")
        sys.exit(1)

    count = len(files)
    data_offset_base = HEADER_SIZE + ENTRY_SIZE * count

    # build entries and collect data
    entries = []
    data_blocks = []
    current_offset = data_offset_base

    for name, data in files:
        entries.append((name, current_offset, len(data)))
        data_blocks.append(data)
        current_offset += len(data)

    # write output
    with open(output_path, 'wb') as out:
        # header
        out.write(MAGIC)
        out.write(struct.pack('<I', count))

        # entries
        for name, offset, size in entries:
            name_bytes = name.encode('ascii')[:MAX_NAME_LEN]
            name_padded = name_bytes + b'\x00' * (32 - len(name_bytes))
            out.write(name_padded)
            out.write(struct.pack('<I', offset))
            out.write(struct.pack('<I', size))

        # file data
        for data in data_blocks:
            out.write(data)

    total_size = current_offset
    print(f"\nBuilt {output_path}")
    print(f"  Files:      {count}")
    print(f"  Total size: {total_size} bytes ({total_size / 1024:.1f} KB)")

    # warn if approaching 8MB limit
    if total_size > 8 * 1024 * 1024:
        print(f"  WARNING: ramdisk exceeds 8MB limit!")
    elif total_size > 6 * 1024 * 1024:
        print(f"  WARNING: ramdisk is over 6MB, approaching 8MB limit")

def main():
    parser = argparse.ArgumentParser(description='Build a ZRD ramdisk for zuzu386')
    parser.add_argument('output', help='Output .zrd file')
    parser.add_argument('files', nargs='*', help='Files to include')
    parser.add_argument('--dir', help='Include all files from a directory')
    parser.add_argument('--ext', help='Filter by extension when using --dir (e.g. .mid)')

    args = parser.parse_args()

    input_files = list(args.files)

    if args.dir:
        if not os.path.isdir(args.dir):
            print(f"Error: {args.dir} is not a directory")
            sys.exit(1)
        for fname in sorted(os.listdir(args.dir)):
            fpath = os.path.join(args.dir, fname)
            if os.path.isfile(fpath):
                if args.ext is None or fname.endswith(args.ext):
                    input_files.append(fpath)

    if not input_files:
        parser.print_help()
        sys.exit(1)

    print(f"Building ZRD ramdisk -> {args.output}")
    build_zrd(args.output, input_files)

if __name__ == '__main__':
    main()