#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

bool audio_init(void);
void audio_beep_on(void);
void audio_beep_off(void);
void audio_destroy(void);

#endif
