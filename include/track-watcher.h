#include <stdbool.h>
#include "eq-profiles.h"
#include "track-context.h"

#ifndef TRACK_WATCHER_H
#define TRACK_WATCHER_H

typedef struct {
    char preset[64];
    char URL[512];
} CurrTrackInfo;

bool findPlayer();
void buildTrackContext(CurrTrackInfo *info);

#endif
