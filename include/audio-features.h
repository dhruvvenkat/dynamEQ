#include <stdbool.h>

#ifndef AUDIO_FEATURES_H
#define AUDIO_FEATURES_H

#define SAMPLE_RATE 44100

typedef struct {
    // amplitude features
    double rms; // average power of the audio signal
    double peak; // highest power level in the signal
    double dynamicRangeProxy;

    // frequency features - energies of each sound range
    double bassEnergy;
    double lowMidEnergy;
    double midEnergy;
    double presenceEnergy;
    double trebleEnergy;

    double brightness; // treble as compared to the rest of the audio
    double bassRatio; // bass as compared to the rest of the audio
    double harshnessRatio; // unpredictability in upper midrange frequencies

    int samplingRate; // relative quality
    int analyzedSeconds; // keeps track of the # of seconds we analyzed for (length of a frame)
} AudioFeatures;

bool extractAudioFeatures(const char *normalizedFilePath, AudioFeatures *features);
void printAudioFeatures(const AudioFeatures *features);

#endif
