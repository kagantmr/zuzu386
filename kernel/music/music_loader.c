#include "music_loader.h"
#include "sound/speaker.h"
#include "timer/timer.h"
#include "mm/heap.h"
#include "string.h"

zuzm_song_t* zuzm_load(const void *data, size_t size) {
    if (!data || size < 11) {
        return NULL;  // Too small (minimum header size)
    }
    
    const uint8_t *buf = (const uint8_t *)data;
    
    // Check magic
    if (buf[0] != 'Z' || buf[1] != 'U' || buf[2] != 'Z' || buf[3] != 'M') {
        return NULL;
    }
    
    // Parse little-endian fields (new format)
    uint8_t version = buf[4];
    uint16_t bpm = buf[5] | (buf[6] << 8);
    uint32_t duration_ms = buf[7] | (buf[8] << 8) | (buf[9] << 16) | (buf[10] << 24);
    uint16_t note_count = buf[11] | (buf[12] << 8);
    
    // Check bounds: 13 bytes header + 4 bytes per note
    size_t expected_size = 13 + (note_count * 4);
    if (size < expected_size) {
        return NULL;
    }
    
    // Allocate song struct and note array
    zuzm_song_t *song = (zuzm_song_t *)kmalloc(sizeof(zuzm_song_t));
    if (!song) {
        return NULL;
    }
    
    zuzm_note_t *notes = (zuzm_note_t *)kmalloc(note_count * sizeof(zuzm_note_t));
    if (!notes) {
        kfree(song);
        return NULL;
    }
    
    // Parse notes
    const uint8_t *note_data = buf + 13;
    for (uint16_t i = 0; i < note_count; i++) {
        uint16_t freq = note_data[0] | (note_data[1] << 8);
        uint8_t duration = note_data[2];
        uint8_t velocity = note_data[3];
        
        notes[i].freq = freq;
        notes[i].duration = duration;
        notes[i].velocity = velocity;
        
        note_data += 4;
    }
    
    song->version = version;
    song->bpm = bpm;
    song->duration_ms = duration_ms;
    song->note_count = note_count;
    song->notes = notes;
    
    return song;
}


void zuzm_free(zuzm_song_t *song) {
    if (song) {
        if (song->notes) {
            kfree(song->notes);
        }
        kfree(song);
    }
}


void zuzm_play(zuzm_song_t *song) {
    if (!song || !song->notes) {
        return;
    }

    if (song->bpm == 0) {
        return;
    }

    uint32_t beat_ms = 60000 / song->bpm;
    
    for (uint16_t i = 0; i < song->note_count; i++) {
        zuzm_note_t *note = &song->notes[i];
        
        // Calculate duration in milliseconds
        uint32_t duration_ms = beat_ms * note->duration / 8;
        
        if (note->freq == 0) {
            // REST: silence for note duration.
            speaker_stop();
            timer_sleep_ms(duration_ms);
        } else {
            // Keep speaker enabled across notes to avoid audible cut between notes.
            speaker_beep(note->freq, duration_ms);
        }  
    }

    speaker_stop();
}
