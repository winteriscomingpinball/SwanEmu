#ifndef SHARED_H
#define SHARED_H

#if defined(BITTBOY)
#define SOUND_OUTPUT_FREQUENCY 22050
#define SOUND_SAMPLES_SIZE 1024
#elif defined(RS97)
#define SOUND_OUTPUT_FREQUENCY 11025
#define SOUND_SAMPLES_SIZE 512
#elif defined(ARCADEMINI)
#define SOUND_OUTPUT_FREQUENCY 22050
#define SOUND_SAMPLES_SIZE 1024
#elif defined(RETROSTONE_1)
#define SOUND_OUTPUT_FREQUENCY 48000
#define SOUND_SAMPLES_SIZE 2048
#elif defined(RETROSTONE_2)
#define SOUND_OUTPUT_FREQUENCY 48000
#define SOUND_SAMPLES_SIZE 2048
#elif defined(ZIPIT)
#define SOUND_OUTPUT_FREQUENCY 44100
#define SOUND_SAMPLES_SIZE 1024
#else
#define SOUND_OUTPUT_FREQUENCY 48000
#define SOUND_SAMPLES_SIZE 2048
#endif

#endif
