#include "music.h"
#include "../sound/speaker.h"

void play_note(const note_t *note) {
    uint32_t duration_ms = (60000 / 120) * (4 / note->duration); // Assuming tempo of 120 BPM
    speaker_beep(note->freq, duration_ms);
    speaker_stop();
}

void play_melody(const song_t *song) {
    uint32_t beat_ms = 60000 / song->tempo;
    for (size_t i = 0; i < song->length; i++) {
        uint32_t duration_ms = beat_ms * song->notes[i].duration / 8;
        speaker_beep(song->notes[i].freq, duration_ms);
        speaker_stop();
        for (volatile uint32_t j = 0; j < 100000; j++);
    }
}