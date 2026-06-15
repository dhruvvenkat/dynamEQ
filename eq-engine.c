#include <stdio.h>
#include <stdbool.h>
#include "eq-engine.h"
#include "eq-profiles.h"

bool applyEQ(EqProfile *profile) {
    printf("applying profile: %s\n", profile->name);
    return true;
}
