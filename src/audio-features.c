#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "audio-features.h"

static bool extractAmplitudeFeatures(FILE *pcmFile, AudioFeatures *features) {
    float sumSquares=0.0f; // stores the sum of all squares for RMS calc
    int sampleCount = 0; // # of samples in the pcm file
    int16_t rawSample;
    size_t samplesRead;
    float peak = 0.0f;

    while ((samplesRead = fread(&rawSample, sizeof(int16_t), 1, pcmFile)) > 0) {
        float truncate = rawSample / 32768.0f; // convert sample from int16_t range to float range

        sumSquares += truncate * truncate;
        peak = fmax(peak, fabs(truncate));
        sampleCount++;
    }

    if (ferror(pcmFile)) {
        return false;
    }

    if (sampleCount > 0) {
        features->rms = sqrt(sumSquares / sampleCount);
        features->peak = peak;
        features->dynamicRangeProxy = features->peak / features->rms;
        return true;
    }

    printf("no samples were found in the inputted pcm file :(");
    return false;
}

// static void extractFrequencyFeatures(char *samplePath, AudioFeatures *features) {

// }

bool extractAudioFeatures(const char *normalizedFilePath, AudioFeatures *features) {
    // sample the current song and kick it out to a pcm file for features to be extracted
    char *extractFeaturesCMD = NULL;
    asprintf(&extractFeaturesCMD, "ffmpeg -nostdin -v error -ss 30 -i \"%s\" -t 30 -ac 1 -ar 44100 -f s16le pipe:1 > /tmp/dynaeq_clip.pcm", normalizedFilePath);
    int executed = system(extractFeaturesCMD);
    free(extractFeaturesCMD);

    if (executed == -1) {
        return false;
    }

    char *clipPath = "~/tmp/dynaeq_clip.pcm";

    FILE *pcmPath;
    pcmPath = fopen(clipPath, "r");

    extractAmplitudeFeatures(pcmPath, features);
    //extractFrequencyFeatures(pcmPath, features);
    extractAudioFeatures(clipPath, features);

    return true;
}
