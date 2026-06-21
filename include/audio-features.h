#include <stdbool.h>

#ifndef AUDIO_FEATURES_H
#define AUDIO_FEAUTURES_H

typedef struct {
    float rms; // average power of the audio signal
    float peak; // highest power level in the signal
    float dynamicRangeProxy;

    // energies of each sound range
    float bassEnergy;
    float lowMidEnergy;
    float midEnergy;
    float presenceEnergy;
    float trebleEnergy;

    float brightness; // treble as compared to the rest of the audio
    float bassRatio; // bass as compared to the rest of the audio
    float harshnessRatio; // unpredictability in upper midrange frequencies

    int samplingRate; // relative quality
    int analyzedSeconds; // keeps track of the # of seconds we analyzed for (length of a frame)
} AudioFeatures;

bool extractAudioFeatures(const char *normalizedFilePath, AudioFeatures *features);
void printAudioFeatures(const AudioFeatures *features);

#endif
