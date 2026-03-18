#ifndef MUSIC_LOADER_H
#define MUSIC_LOADER_H

#include "stdint.h"
#include "stddef.h"
#include "music.h"

/**
 * Binary music format (.zuzm):
 *   - Magic: "ZUZM" (4 bytes)
 *   - Version: uint8 (1)
 *   - BPM: uint16 (little-endian)
 *   - Duration in milliseconds: uint32 (little-endian)
 *   - Note count: uint16 (little-endian)
 *   - Notes: array of {freq: uint16, duration: uint8, velocity: uint8} × count
 */

typedef struct {
    uint16_t freq;           // frequency in Hz (0 = rest)
    uint8_t duration;        // duration in beat units
    uint8_t velocity;        // MIDI velocity (0-127)
} zuzm_note_t;

/**
 * Loaded song data (heap-allocated)
 */
typedef struct {
    uint8_t version;         // format version
    uint16_t bpm;            // beats per minute
    uint32_t duration_ms;    // total duration in milliseconds
    uint16_t note_count;
    zuzm_note_t *notes;      // heap-allocated array
} zuzm_song_t;

/**
 * Load a .zuzm binary music file
 * @param data Pointer to raw .zuzm file data
 * @param size Size of data in bytes
 * @return Pointer to allocated song, or NULL on error
 */
zuzm_song_t* zuzm_load(const void *data, size_t size);

/**
 * Free a loaded song
 */
void zuzm_free(zuzm_song_t *song);

/**
 * Play a loaded song
 */
void zuzm_play(zuzm_song_t *song);

#endif
