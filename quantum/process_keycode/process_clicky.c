#include "audio.h"
#include "process_clicky.h"

#ifdef AUDIO_CLICKY

#    ifndef AUDIO_CLICKY_DELAY_DURATION
#        define AUDIO_CLICKY_DELAY_DURATION 1
#    endif // !AUDIO_CLICKY_DELAY_DURATION
#    ifndef AUDIO_CLICKY_FREQ_DEFAULT
#        define AUDIO_CLICKY_FREQ_DEFAULT 440.0f
#    endif // !AUDIO_CLICKY_FREQ_DEFAULT
#    ifndef AUDIO_CLICKY_FREQ_MIN
#        define AUDIO_CLICKY_FREQ_MIN 65.0f
#    endif // !AUDIO_CLICKY_FREQ_MIN
#    ifndef AUDIO_CLICKY_FREQ_MAX
#        define AUDIO_CLICKY_FREQ_MAX 1500.0f
#    endif // !AUDIO_CLICKY_FREQ_MAX
#    ifndef AUDIO_CLICKY_FREQ_FACTOR
#        define AUDIO_CLICKY_FREQ_FACTOR 1.18921f
#    endif // !AUDIO_CLICKY_FREQ_FACTOR
#    ifndef AUDIO_CLICKY_FREQ_RANDOMNESS
#        define AUDIO_CLICKY_FREQ_RANDOMNESS 0.05f
#    endif // !AUDIO_CLICKY_FREQ_RANDOMNESS

float clicky_freq = AUDIO_CLICKY_FREQ_DEFAULT;
float clicky_rand = AUDIO_CLICKY_FREQ_RANDOMNESS;

// the first "note" is an intentional delay; the 2nd and 3rd notes are the "clicky"
float clicky_song[][2] = {{AUDIO_CLICKY_FREQ_MIN, AUDIO_CLICKY_DELAY_DURATION}, {AUDIO_CLICKY_FREQ_DEFAULT, 3}, {AUDIO_CLICKY_FREQ_DEFAULT, 1}}; // 3 and 1 --> durations

extern audio_config_t audio_config;

#    ifndef NO_MUSIC_MODE
extern bool music_activated;
extern bool midi_activated;
#    endif // !NO_MUSIC_MODE

void clicky_play(uint16_t keycode) {
#    ifndef NO_MUSIC_MODE
    if (music_activated || midi_activated || !audio_config.enable) return;
#    endif // !NO_MUSIC_MODE
#    ifdef AUDIO_CLICKY_DETERMINISTIC
    uint16_t lower_bits = keycode & 0x000F;//get rid of extra bits TODO maybe we should mask more?
    //create a click by iterating symmetrically around the median frequency of MAX and MIN
    clicky_song[2][0] = AUDIO_CLICKY_FREQ_MIN + (float)lower_bits*(AUDIO_CLICKY_FREQ_MAX - AUDIO_CLICKY_FREQ_MIN)/AUDIO_CLICKY_FREQ_FACTOR;
    //now calculate the opposite side of the median based around max and min freqs
    clicky_song[1][0] = AUDIO_CLICKY_FREQ_MAX - (float)lower_bits*(AUDIO_CLICKY_FREQ_MAX - AUDIO_CLICKY_FREQ_MIN)/AUDIO_CLICKY_FREQ_FACTOR;
    PLAY_SONG(clicky_song);
#    else
    clicky_song[1][0] = 2.0f * clicky_freq * (1.0f + clicky_rand * (((float)rand()) / ((float)(RAND_MAX))));
    clicky_song[2][0] = clicky_freq * (1.0f + clicky_rand * (((float)rand()) / ((float)(RAND_MAX))));
    PLAY_SONG(clicky_song);
#    endif // !AUDIO_CLICKY_DETERMINISTIC
}

void clicky_freq_up(void) {
    float new_freq = clicky_freq * AUDIO_CLICKY_FREQ_FACTOR;
    if (new_freq < AUDIO_CLICKY_FREQ_MAX) {
        clicky_freq = new_freq;
    }
}

void clicky_freq_down(void) {
    float new_freq = clicky_freq / AUDIO_CLICKY_FREQ_FACTOR;
    if (new_freq > AUDIO_CLICKY_FREQ_MIN) {
        clicky_freq = new_freq;
    }
}

void clicky_freq_reset(void) {
    clicky_freq = AUDIO_CLICKY_FREQ_DEFAULT;
}

void clicky_toggle(void) {
    audio_config.clicky_enable ^= 1;
    eeconfig_update_audio(audio_config.raw);
}

void clicky_on(void) {
    audio_config.clicky_enable = 1;
    eeconfig_update_audio(audio_config.raw);
}

void clicky_off(void) {
    audio_config.clicky_enable = 0;
    eeconfig_update_audio(audio_config.raw);
}

bool is_clicky_on(void) {
    return (audio_config.clicky_enable != 0);
}

bool process_clicky(uint16_t keycode, keyrecord_t *record) {
    if (keycode == QK_AUDIO_CLICKY_TOGGLE && record->event.pressed) {
        clicky_toggle();
    }

    if (keycode == QK_AUDIO_CLICKY_ON && record->event.pressed) {
        clicky_on();
    }
    if (keycode == QK_AUDIO_CLICKY_OFF && record->event.pressed) {
        clicky_off();
    }

    if (keycode == QK_AUDIO_CLICKY_RESET && record->event.pressed) {
        clicky_freq_reset();
    }

    if (keycode == QK_AUDIO_CLICKY_UP && record->event.pressed) {
        clicky_freq_up();
    }
    if (keycode == QK_AUDIO_CLICKY_DOWN && record->event.pressed) {
        clicky_freq_down();
    }

    if (audio_config.enable && audio_config.clicky_enable) {
        if (record->event.pressed) {                                 // Leave this separate so it's easier to add upstroke sound
            if (keycode != QK_AUDIO_ON && keycode != QK_AUDIO_OFF) { // DO NOT PLAY if audio will be disabled, and causes issuse on ARM
                clicky_play(keycode);
            }
        }
    }
    return true;
}

#endif // AUDIO_CLICKY
