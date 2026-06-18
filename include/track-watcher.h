#include <stdbool.h>
#include "eq-profiles.h"
#include "track-context.h"

#ifndef TRACK_WATCHER_H
#define TRACK_WATCHER_H

typedef struct {
    const char *preset;
    char URL[512];
} CurrTrackInfo;

bool findPlayer();
static void printTrackInfo(TrackContext *context, EqRecommendation *recommendation);
void buildTrackContext(CurrTrackInfo *info);

#endif
