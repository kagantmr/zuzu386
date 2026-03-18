#ifndef ZUZM_H
#define ZUZM_H

#include "stdint.h"

/**
 * ZUZM format (.zuzm files):
 * [header]
 *   magic:              4 bytes  "ZUZM"
 *   version:            1 byte   format version
 *   bpm:                2 bytes  beats per minute (uint16 LE)
 *   duration_ms:        4 bytes  total duration in milliseconds (uint32 LE)
 *   note_count:         2 bytes  number of notes (uint16 LE)
 * [notes array]
 *   for each note:
 *     frequency:        2 bytes  frequency in Hz (uint16 LE, 0 = rest)
 *     duration:         1 byte   duration in beat units (uint8)
 *     velocity:         1 byte   note velocity (0-127) (uint8)
 */

typedef struct {
    uint16_t frequency;     // frequency in Hz (0 = rest)
    uint8_t  duration;      // duration in beat units
    uint8_t  velocity;      // velocity (0-127)
} __attribute__((packed)) zuzm_note_t;

typedef struct {
    char     magic[4];      // "ZUZM"
    uint8_t  version;       // format version
    uint16_t bpm;           // beats per minute
    uint32_t duration_ms;   // total duration in milliseconds
    uint16_t note_count;    // number of notes
    zuzm_note_t *notes;     // pointer to note array (allocated by loader)
} zuzm_song_t;

/**
 * Load a ZUZM song from binary data.
 * Returns pointer to zuzm_song_t allocated with malloc.
 * Returns NULL if data is invalid or format version unsupported.
 * Must call zuzm_free() to release the struct and notes array.
 */
zuzm_song_t *zuzm_load(const void *data, uint32_t size);

/**
 * Free a ZUZM song (deallocate notes array and struct).
 */
void zuzm_free(zuzm_song_t *song);

/**
 * Play a ZUZM song (if music playback is implemented).
 * Depends on the music playback engine.
 */
void zuzm_play(zuzm_song_t *song);

#endif /* ZUZM_H */
