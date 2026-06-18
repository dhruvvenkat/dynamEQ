#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "eq-engine.h"
#include "eq-profiles.h"

bool applyEQ(const EqRecommendation *recommendation) {
    //printf("applying profile: %s\n", recommendation->profileName);
    char *appEQCmd = NULL;
    asprintf(&appEQCmd, "easyeffects -l %s", recommendation->profileName);
    int executed = system(appEQCmd);

    free(appEQCmd);

    if (executed == -1) {
        return false;
    }

    return true;
}
