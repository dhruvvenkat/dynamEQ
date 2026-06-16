#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "eq-engine.h"
#include "eq-profiles.h"

bool applyEQ(const EqProfile *profile) {
    printf("applying profile: %s\n", profile->name);
    char *appEQCmd = NULL;
    asprintf(&appEQCmd, "easyeffects -l %s", profile->presetName);
    int executed = system(appEQCmd);

    free(appEQCmd);

    if (executed == -1) {
        return false;
    }

    return true;
}
