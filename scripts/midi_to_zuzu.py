#!/usr/bin/env python3
"""
midi_to_zuzu.py - Convert MIDI files to zuzu386 binary music format (.zuzm)
Usage: python3 midi_to_zuzu.py <input.mid> [output.zuzm] [track_index]

Binary format (.zuzm):
  - Magic: "ZUZM" (4 bytes)
  - Version: uint8 (1)
  - BPM: uint16 (little-endian)
  - Duration in milliseconds: uint32 (little-endian)
  - Note count: uint16 (little-endian)
  - Notes: array of {freq: uint16, duration: uint8, velocity: uint8} x count

Requires: pip install mido
"""

import sys
import struct
import mido

# Frequency table (MIDI note to Hz)
FREQ_TABLE = [
    16, 17, 18, 19, 21, 22, 23, 24, 26, 28, 29, 31,  # C0-B0
    33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62,  # C1-B1
    65, 69, 73, 78, 82, 87, 93, 98, 104, 110, 117, 123,  # C2-B2
    131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247,  # C3-B3
    261, 277, 293, 311, 329, 349, 370, 392, 415, 440, 466, 493,  # C4-B4
    523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 987,  # C5-B5
    1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,  # C6-B6
    2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951,  # C7-B7
    4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902,  # C8-B8
]

MIN_SUPPORTED_MIDI = 12   # C0
MAX_SUPPORTED_MIDI = 119  # B8
ZUZM_FORMAT_VERSION = 1

def ticks_to_milliseconds(ticks, ticks_per_beat, tempo_microseconds):
    """Convert MIDI ticks to milliseconds."""
    beat_duration_ms = tempo_microseconds / 1000.0
    return (ticks / ticks_per_beat) * beat_duration_ms


def ticks_to_beat_units(ticks, ticks_per_beat):
    """Convert MIDI ticks to zuzu386 beat units (8 = one beat)"""
    return max(1, round((ticks / ticks_per_beat) * 8))


def midi_note_to_freq(note_num):
    """Map MIDI note number to frequency in Hz."""
    clamped = min(max(note_num, MIN_SUPPORTED_MIDI), MAX_SUPPORTED_MIDI)
    idx = clamped - MIN_SUPPORTED_MIDI
    return FREQ_TABLE[idx]


def extract_highest_voice(track, ticks_per_beat):
    """Extract a monophonic line by taking the highest active note at each time slice."""
    events = []
    current_tick = 0

    for msg in track:
        current_tick += msg.time
        if msg.type == 'note_on' and msg.velocity > 0:
            events.append((current_tick, msg.note, msg.velocity, True))
        elif msg.type == 'note_off' or (msg.type == 'note_on' and msg.velocity == 0):
            events.append((current_tick, msg.note, 0, False))

    if not events:
        return []

    active_notes = {}  # note_num -> velocity
    segments = []
    segment_start = 0
    current_note = None
    current_velocity = 0
    idx = 0

    while idx < len(events):
        tick = events[idx][0]

        if tick > segment_start:
            segments.append((current_note, current_velocity, segment_start, tick))
            segment_start = tick

        # Apply all note changes at this tick before selecting the voice for the next slice.
        while idx < len(events) and events[idx][0] == tick:
            _, note_num, velocity, is_on = events[idx]
            if is_on:
                active_notes[note_num] = velocity
            else:
                active_notes.pop(note_num, None)
            idx += 1

        if active_notes:
            current_note = max(active_notes)
            current_velocity = active_notes[current_note]
        else:
            current_note = None
            current_velocity = 0

    notes = []
    for note_num, velocity, start_tick, end_tick in segments:
        duration_ticks = end_tick - start_tick
        if duration_ticks <= 0:
            continue

        duration_units = ticks_to_beat_units(duration_ticks, ticks_per_beat)
        if notes and notes[-1][0] == note_num:
            prev_note, prev_duration, prev_velocity, _ = notes[-1]
            notes[-1] = (prev_note, prev_duration + duration_units, prev_velocity, end_tick)
        else:
            notes.append((note_num, duration_units, velocity, end_tick))

    return notes


def smooth_monophonic_notes(notes, max_rest_gap_units=4):
    """Make monophonic output less choppy for PC speaker playback.

    Strategy:
      - absorb tiny rests into the previous note
      - merge adjacent notes with identical pitch
    """
    if not notes:
        return notes

    smoothed = []
    for note_num, duration, velocity, end_tick in notes:
        if duration <= 0:
            continue

        # Absorb short silence gaps into the previous note for legato playback.
        if note_num is None and duration <= max_rest_gap_units and smoothed and smoothed[-1][0] is not None:
            prev_note, prev_duration, prev_velocity, _ = smoothed[-1]
            smoothed[-1] = (prev_note, prev_duration + duration, prev_velocity, end_tick)
            continue

        if smoothed and smoothed[-1][0] == note_num:
            prev_note, prev_duration, prev_velocity, _ = smoothed[-1]
            merged_velocity = max(prev_velocity, velocity)
            smoothed[-1] = (prev_note, prev_duration + duration, merged_velocity, end_tick)
        else:
            smoothed.append((note_num, duration, velocity, end_tick))

    return smoothed


def midi_to_binary(midi_path, output_path, track_index=0):
    """Convert MIDI to binary music format."""
    mid = mido.MidiFile(midi_path)
    ticks_per_beat = mid.ticks_per_beat

    # find tempo (microseconds per beat), default 500000 = 120 BPM
    tempo = 500000
    for track in mid.tracks:
        for msg in track:
            if msg.type == 'set_tempo':
                tempo = msg.tempo
                break

    bpm = round(60_000_000 / tempo)

    # pick track
    tracks_with_notes = [t for t in mid.tracks if any(m.type == 'note_on' for m in t)]
    if not tracks_with_notes:
        print("Error: no tracks with notes found")
        sys.exit(1)

    if track_index >= len(tracks_with_notes):
        print(f"Error: track {track_index} not found, {len(tracks_with_notes)} tracks available")
        sys.exit(1)

    track = tracks_with_notes[track_index]
    notes = extract_highest_voice(track, ticks_per_beat)
    notes = smooth_monophonic_notes(notes)

    if not notes:
        print("Error: no notes extracted")
        sys.exit(1)

    # Calculate total duration in milliseconds
    max_tick = max(n[3] for n in notes) if notes else 0
    total_duration_ms = int(ticks_to_milliseconds(max_tick, ticks_per_beat, tempo))

    # Write binary format
    with open(output_path, 'wb') as f:
        # Magic
        f.write(b'ZUZM')
        
        # Version (uint8)
        f.write(struct.pack('<B', ZUZM_FORMAT_VERSION))
        
        # BPM (uint16 LE)
        f.write(struct.pack('<H', bpm))
        
        # Duration in milliseconds (uint32 LE)
        f.write(struct.pack('<I', total_duration_ms))
        
        # Note count (uint16 LE)
        f.write(struct.pack('<H', len(notes)))
        
        # Notes
        for note_num, duration, velocity, _ in notes:
            if note_num is None:
                freq = 0  # REST
            else:
                freq = midi_note_to_freq(note_num)
            
            # Ensure values fit in their types
            duration = min(duration, 255)
            velocity = min(velocity, 127)  # MIDI velocity is 0-127
            
            f.write(struct.pack('<HBB', freq, duration, velocity))
    
    print(f"Wrote {output_path}")
    print(f"  Version: {ZUZM_FORMAT_VERSION}")
    print(f"  BPM: {bpm}")
    print(f"  Duration: {total_duration_ms}ms")
    print(f"  Notes: {len(notes)}")


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python3 midi_to_zuzu.py <input.mid> [output.zuzm] [track_index]")
        sys.exit(1)
    
    midi_path = sys.argv[1]
    
    # Output path: replace .mid with .zuzm if not specified
    if len(sys.argv) > 2 and not sys.argv[2].isdigit():
        output_path = sys.argv[2]
        track_index = int(sys.argv[3]) if len(sys.argv) > 3 else 0
    else:
        import os
        base = os.path.splitext(midi_path)[0]
        output_path = base + '.zuzm'
        track_index = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    
    midi_to_binary(midi_path, output_path, track_index)