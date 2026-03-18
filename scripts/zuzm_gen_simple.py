#!/usr/bin/env python3
import struct
import sys

def generate_zuzm(output_path, bpm, duration_ms):
    notes = [(0, 8, 0)]
    with open(output_path, 'wb') as f:
        f.write(b'ZUZM')
        f.write(struct.pack('<B', 1))
        f.write(struct.pack('<H', bpm))
        f.write(struct.pack('<I', duration_ms))
        f.write(struct.pack('<H', len(notes)))
        for freq, duration, velocity in notes:
            f.write(struct.pack('<HBB', freq, duration, velocity))
    print(f'Generated {output_path}')

if __name__ == '__main__':
    output_path = sys.argv[1] if len(sys.argv) > 1 else 'test.zuzm'
    bpm = int(sys.argv[2]) if len(sys.argv) > 2 else 120
    duration_ms = int(sys.argv[3]) if len(sys.argv) > 3 else 1000
    generate_zuzm(output_path, bpm, duration_ms)
