#ifndef EQ_PROFILES_H
#define EQ_PROFILES_H

#define EQ_BAND_COUNT 10
#define EQ_MAX_BOOST_DB 3.0f
#define EQ_MAX_CUT_DB -6.0f

typedef enum {
    EQ_REC_SOURCE_GENRE_FALLBACK, // uses genre to recommend
    EQ_REC_SOURCE_NEUTRAL_FALLBACK, // recommends neutral in case of failure
    // inference model sources are placeholders for now
    EQ_REC_SOURCE_MODEL_METADATA, // passes metadata text into inference model
    EQ_REC_SOURCE_MODEL_AUDIO, // passes audio signature into inference model
    EQ_REC_SOURCE_MANUAL_OVERRIDE // user override
} EqRecommendationSource;

typedef struct {
    char profileName[64];
    float gainsDb[EQ_BAND_COUNT];
    float confidence;
    EqRecommendationSource source;
} EqRecommendation;

typedef enum {
    EQ_PRESET_NEUTRAL,
    EQ_PRESET_BASS_PLUS,
    EQ_PRESET_CLEAN_BASS,
    EQ_PRESET_ANTI_HARSH,
    EQ_PRESET_VOCAL_CLARITY,
    EQ_PRESET_WARMTH,
    EQ_PRESET_COUNT
} EqPresetID;

typedef struct {
    EqPresetID id;
    const char *name;
    float gains_db[EQ_BAND_COUNT];
    const char *presetName; // stores the name of the preset as held in easyeffects
} EqProfile;

extern const EqProfile profiles[EQ_PRESET_COUNT];
const EqProfile *eq_profile_get(EqPresetID id);
void printProfile(const EqProfile *profile);
#endif
