#include <stdio.h>
#include <stdlib.h>
#include "audio-features.h"

bool extractAudioFeatures(const char *normalizedFilePath, AudioFeatures *features) {
    char *extractFeaturesCMD = NULL;
    asprintf(&extractFeaturesCMD, "ffmpeg -nostdin -v error -ss 30 -i \"%s\" -t 30 -ac 1 -ar 44100 -f s16le pipe:1 > /tmp/dynaeq_clip.pcm", normalizedFilePath);
    int executed = system(extractFeaturesCMD);

    free(extractFeaturesCMD);

    if (executed == -1) {
        return false;
    }


    return true;
}
