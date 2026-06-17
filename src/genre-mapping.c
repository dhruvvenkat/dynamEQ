#include <stdio.h>
#include <string.h>
#include "genre-mapping.h"
#include "eq-profiles.h"
#include "track-context.h"

typedef struct {
    const char *genre;
    EqPresetID preset;
} GenreMapping;

// v0: mapping genres directly to their EQ presets
// in future iterations, this will be changed to pipe genre directly to the LLM, which will return the appropriate preset
static const GenreMapping mappings[] = {
    {"Hip-Hop", EQ_PRESET_BASS_PLUS},
    {"Rap", EQ_PRESET_BASS_PLUS},
    {"Trap", EQ_PRESET_BASS_PLUS},
    {"Rage Rap", EQ_PRESET_BASS_PLUS},

    { "Rock",       EQ_PRESET_CLEAN_BASS },
    { "Classic Rock",       EQ_PRESET_CLEAN_BASS },
    { "Metal",      EQ_PRESET_ANTI_HARSH },

    { "Pop",        EQ_PRESET_VOCAL_CLARITY },
    { "Acoustic",   EQ_PRESET_VOCAL_CLARITY },

    { "Lo-Fi",      EQ_PRESET_WARMTH },

    { "Jazz",       EQ_PRESET_NEUTRAL },
    { "Classical",  EQ_PRESET_NEUTRAL }
};

const EqProfile *genreToPreset(char* genre) {
    size_t mappingCount = sizeof(mappings)/sizeof(mappings[0]);
    //printf("incoming genre: %s\n", genre);
    for (size_t i = 0; i < mappingCount; i++) {
        if (strcmp(genre, mappings[i].genre) == 0) {
            const EqProfile *profile = eq_profile_get(mappings[i].preset);
            //printProfile(profile);
            return profile;
        }
    }

    printf("no correlating genre found, skipping...");
    return NULL;
}

bool recommendFromTrackContext(const TrackContext *context, EqRecommendation *recommendation) {
    char *genre = context->genre;
    EqProfile *mappedProfile = genreToPreset(genre);

    // recommendation->profileName =
    pullString(recommendation->profileName, 64, mappedProfile->presetName);
    recommendation->confidence = 1.00; // confidence is 100% for now - will change once inference is added
    for (int i = 10; i > 0; i--) {
        recommendation->gainsDb[i] = mappedProfile->gains_db[i];
    }
    recommendation->source = EQ_REC_SOURCE_GENRE_FALLBACK;

    return true;
}
