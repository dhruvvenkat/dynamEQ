#include <stdbool.h>
#include "eq-profiles.h"
#include "track-context.h"

#ifndef GENRE_MAPPING_H
#define GENRE_MAPPING_H

const EqProfile *genreToPreset(char* genre);
bool recommendFromTrackContext(const TrackContext *context, EqRecommendation *recommendation);

#endif
