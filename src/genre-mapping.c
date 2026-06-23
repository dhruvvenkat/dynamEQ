#include <stdio.h>
#include <string.h>
#include "genre-mapping.h"
#include "eq-profiles.h"
#include "track-context.h"
#include "audio-features.h"
#include "eq-profiles.h"

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

const EqProfile *genreToPreset(const char *genre) {
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
    const EqProfile *mappedProfile = genreToPreset(context->genre);

    // recommendation->profileName =
    pullString(recommendation->profileName, 64, mappedProfile->presetName);
    recommendation->confidence = 1.00; // confidence is 100% for now - will change once inference is added
    for (int i = 9; i >= 0; i--) {
        recommendation->gainsDb[i] = mappedProfile->gains_db[i];
    }
    recommendation->source = EQ_REC_SOURCE_GENRE_FALLBACK;

    return true;
}

bool validateRecommendation(EqRecommendation *recommendation) {
    if (strcmp(recommendation->profileName, "") == 0) {
        printf("no profile name found - recommendation invalid");
        return false;
    }

    if ((recommendation->confidence < 0.0f) || (recommendation->confidence > 1.0f)) {
        printf("condfidence outside of the range 0-1, incalid");
        return false;
    }

    for (int i = 9; i >= 0; i--) {
        float gain = recommendation->gainsDb[i];
        if ((gain < EQ_MAX_CUT_DB) || (gain > EQ_MAX_BOOST_DB)) {
            printf("gain outside of safe levels - invalid");
            return false;
        }
    }

    if (recommendation->source > 5) {
        printf("recommendation enum outside of proper range - invalid");
        return false;
    }

    return true;
}

void neutralFallbackRecommendation(EqRecommendation *recommendation) {
    char *neutralString = "neutral";
    pullString(recommendation->profileName, 64, neutralString);

    recommendation->confidence = 1.0f;

    for (int i = 9; i >= 0; i--) {
        recommendation->gainsDb[i] = 0;
    }

    recommendation->source = EQ_REC_SOURCE_NEUTRAL_FALLBACK;
}

// helper to directly input a recommendation ourselves when vetoing an EQ profile
static void inputRecommendation(EqRecommendation *recommendation, const EqProfile *profile) {
    pullString(recommendation->profileName, 64, profile->name);
    recommendation->confidence = 1.0f;

    for (int i = 9; i >= 0; i--) {
        recommendation->gainsDb[i] = profile->gains_db[i];
    }

    recommendation->source = EQ_REC_SOURCE_MODEL_AUDIO;
}

// function that runs after the initial recommendation and audio features are pulled
// vetos/adjusts the recommendation based on the waveform as analyzed in audio-features.c
void adjustRecommendationUsingFeatures(EqRecommendation *recommendation, AudioFeatures *features) {
    // if BassPlus is recommended but the song is already bass-heavy, do not edit the bass
    if ((strcmp(recommendation->profileName, "BassPlus") == 0) && (features->bassRatio > 0.95)) {
        inputRecommendation(recommendation, &profiles[2]);
        printf("adjusted by features: true\n");
        printf("adjustment reason: bass too high\n");
        return;
    }

    printf("no adjustments to make :)\n");
}
