#include <stdlib.h>
#include <stdbool.h>

#ifndef TRACK_CONTEXT_H
#define TRACK_CONTEXT_H

typedef struct {
    char title[256];
    char artist[256];
    char album[256];
    char genre[128];
    char year[16];
    char filePath[512];
    char sourcePlayer[64];
    int metadataQuality;
} TrackContext;

void extractMetadata(TrackContext *context, const char *trackURL, const char *sourcePlayer);
// static void pullString(char *field, size_t fieldSize, const char *src);
// static unsigned int isFieldPopulated(const char *field);
// static void printContext(const TrackContext *context);

#endif
