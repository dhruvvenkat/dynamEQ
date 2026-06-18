#include <stdbool.h>
#include "eq-profiles.h"
#include "track-context.h"

#ifndef GENRE_MAPPING_H
#define GENRE_MAPPING_H

const EqProfile *genreToPreset(const char *genre);
bool recommendFromTrackContext(const TrackContext *context, EqRecommendation *recommendation);
bool validateRecommendation(EqRecommendation *recommendation);
void neutralFallbackRecommendation(EqRecommendation *recommendation);

#endif
