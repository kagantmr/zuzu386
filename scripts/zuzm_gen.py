#!/usr/bin/env python3
"""Generate simple .zuzm files without mido.

Usage:
  python3 scripts/zuzm_gen.py <output.zuzm> <bpm> [duration_ms] [freq dur vel ...]

If no notes are provided, a short audible default melody is generated.
"""

import struct
import sys

ZUZM_VERSION = 1


def parse_notes(args):
    notes = []
    for i in range(0, len(args), 3):
        if i + 2 >= len(args):
            break
        freq = int(args[i])
        dur = int(args[i + 1])
        vel = int(args[i + 2])
        notes.append((max(0, min(freq, 65535)), max(1, min(dur, 255)), max(0, min(vel, 127))))
    return notes


def gen(path, bpm, duration_ms, notes):
    if not notes:
        # C5, E5, G5, C6 (all audible, medium velocity)
        notes = [
            (523, 4, 100),
            (659, 4, 100),
            (784, 4, 100),
            (1047, 8, 110),
        ]

    with open(path, "wb") as f:
        f.write(b"ZUZM")
        f.write(struct.pack("<B", ZUZM_VERSION))
        f.write(struct.pack("<H", bpm))
        f.write(struct.pack("<I", duration_ms))
        f.write(struct.pack("<H", len(notes)))
        for freq, dur, vel in notes:
            f.write(struct.pack("<HBB", freq, dur, vel))


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 scripts/zuzm_gen.py <output.zuzm> <bpm> [duration_ms] [freq dur vel ...]")
        sys.exit(1)

    output_path = sys.argv[1]
    bpm = int(sys.argv[2])
    duration_ms = int(sys.argv[3]) if len(sys.argv) > 3 and sys.argv[3].isdigit() else 1200
    note_start = 4 if len(sys.argv) > 3 and sys.argv[3].isdigit() else 3
    note_args = sys.argv[note_start:]

    gen(output_path, bpm, duration_ms, parse_notes(note_args))
