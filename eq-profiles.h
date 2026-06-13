#ifndef EQ_PROFILES_H
#define EQ_PROFILES_H

#define EQ_BAND_COUNT 10

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
    EqPresetID preset;
    const char *name;
    float gains_db[EQ_BAND_COUNT];
} EqProfile;

const EqProfile *eq_profile_get(EqPresetID id);
const char *eq_profile_name(EqPresetID id);
#endif