#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

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


static bool extractAmplitudeFeatures(FILE *pcmFile, AudioFeatures *features) {
    double sumSquares = 0.0f; // stores the sum of all squares for RMS calc
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
        printf("rms: %f, peak: %f, dynamic range proxy; %f\n", features->rms, features->peak, features->dynamicRangeProxy);
        return true;
    }

    printf("no samples were found in the inputted pcm file :(\n");
    return false;
}

// using the Goertzel DSP Algorithm to calcualte the magnitude of a given frequency component
// this is how we will be calculating the strength of each frequency band in a song
static double goertzelAlgo(const float *samples, size_t sampleCount, float targetFreq, int sampleRate) {
    int k = 0.5 + (sampleCount * targetFreq / sampleRate);
    double omega = 2.0 * M_PI * k / sampleCount;
    double coeff = 2.0 * cos(omega);
    double yPrev2 = 0.0;
    double yPrev1 = 0.0;
    double y = 0.0;

    for (size_t n = 0; n < sampleCount; n++) {
        y = samples[n] + coeff * yPrev1 - yPrev2;
        yPrev2 = yPrev1;
        yPrev1 = y;
    }

    double power = yPrev1 * yPrev1 + yPrev2 * yPrev2 - coeff * yPrev1 * yPrev2;

    return power;
}

static bool extractFrequencyFeatures(FILE *pcmFile, AudioFeatures *features) {
    if (pcmFile == NULL || features == NULL) {
        return false;
    }

    int16_t rawBuffer[2048];
    float samples[2048];

    double bassEnergy = 0.0f;
    double lowMidEnergy = 0.0f;
    double midEnergy = 0.0f;
    double presenceEnergy = 0.0f;
    double trebleEnergy = 0.0f;

    size_t samplesRead = 0;

    while ((samplesRead = fread(rawBuffer, sizeof(int16_t), 2048, pcmFile)) > 0) {
        for (size_t i = 0; i < samplesRead; i++) {
            samples[i] = rawBuffer[i] / 32768.0f;
        }

        // add energies at several target frequencies (need to make more robust)
        bassEnergy += goertzelAlgo(samples, samplesRead, 60.0f, SAMPLE_RATE);
        bassEnergy += goertzelAlgo(samples, samplesRead, 120.0f, SAMPLE_RATE);
        bassEnergy += goertzelAlgo(samples, samplesRead, 200.0f, SAMPLE_RATE);

        lowMidEnergy += goertzelAlgo(samples, samplesRead, 300.0f, SAMPLE_RATE);
        lowMidEnergy += goertzelAlgo(samples, samplesRead, 400.0f, SAMPLE_RATE);

        midEnergy += goertzelAlgo(samples, samplesRead, 750.0f, SAMPLE_RATE);
        midEnergy += goertzelAlgo(samples, samplesRead, 1000.0f, SAMPLE_RATE);
        midEnergy += goertzelAlgo(samples, samplesRead, 1500.0f, SAMPLE_RATE);

        presenceEnergy += goertzelAlgo(samples, samplesRead, 2500.0f, SAMPLE_RATE);
        presenceEnergy += goertzelAlgo(samples, samplesRead, 4000.0f, SAMPLE_RATE);
        presenceEnergy += goertzelAlgo(samples, samplesRead, 5500.0f, SAMPLE_RATE);

        trebleEnergy += goertzelAlgo(samples, samplesRead, 8000.0f, SAMPLE_RATE);
        trebleEnergy += goertzelAlgo(samples, samplesRead, 12000.0f, SAMPLE_RATE);
        trebleEnergy += goertzelAlgo(samples, samplesRead, 16000.0f, SAMPLE_RATE);
    }

    if (ferror(pcmFile)) {
        return false;
    }

    double totalEnergy = bassEnergy + lowMidEnergy + midEnergy + presenceEnergy + trebleEnergy;

    if (totalEnergy <= 0.0) {
        printf("total energy is %f, invalid (< 0), canceling\n", totalEnergy);
        return false;
    }

    // apply energies to the features object
    features->bassEnergy = (double)bassEnergy;
    features->lowMidEnergy = (double)lowMidEnergy;
    features->midEnergy = (double)midEnergy;
    features->presenceEnergy = (double)presenceEnergy;
    features->trebleEnergy = (double)trebleEnergy;

    // calculate various ratios to inform new equalization
    features->bassRatio = (double)(bassEnergy / totalEnergy);
    features->brightness = (double)(trebleEnergy / totalEnergy);
    features->harshnessRatio = (double)(presenceEnergy / totalEnergy);

    printf(
        "AudioFeatures:\n"
        "  bassEnergy:      %.6f\n"
        "  lowMidEnergy:    %.6f\n"
        "  midEnergy:       %.6f\n"
        "  presenceEnergy:  %.6f\n"
        "  trebleEnergy:    %.6f\n"
        "  bassRatio:       %.6f\n"
        "  brightness:      %.6f\n"
        "  harshnessRatio:  %.6f\n",
        features->bassEnergy,
        features->lowMidEnergy,
        features->midEnergy,
        features->presenceEnergy,
        features->trebleEnergy,
        features->bassRatio,
        features->brightness,
        features->harshnessRatio
    );

    return true;
}

bool extractAudioFeatures(AudioFeatures *features) {
    // // sample the current song and kick it out to a pcm file for features to be extracted
    // char *extractFeaturesCMD = NULL;
    // asprintf(&extractFeaturesCMD, "ffmpeg -nostdin -v error -ss 30 -i \"%s\" -t 30 -ac 1 -ar 44100 -f s16le pipe:1 > /tmp/dynaeq_clip.pcm", normalizedFilePath);
    // int executed = system(extractFeaturesCMD);
    // free(extractFeaturesCMD);

    // if (executed == -1) {
    //     return false;
    // }

    char clipPath[3][64] = {"/tmp/100hz.pcm", "/tmp/1000hz.pcm", "/tmp/8000hz.pcm"};

    FILE *pcmPath;
    pcmPath = fopen(clipPath[2], "rb");

    extractAmplitudeFeatures(pcmPath, features);
    rewind(pcmPath); // move the file stream pointer back to the beginning so that frequency extraction isn't read as EOF
    extractFrequencyFeatures(pcmPath, features);

    fclose(pcmPath);

    return true;
}

int main() {
    AudioFeatures features;
    extractAudioFeatures(&features);
}
