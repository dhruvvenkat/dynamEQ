#include <stdbool.h>

#ifndef TRACK_WATCHER_H
#define TRACK_WATCHER_H

typedef struct {
    const char *preset;
    char URL[512];
} CurrTrackInfo;

bool findPlayer();
void buildTrackContext(CurrTrackInfo *info);

#endif
