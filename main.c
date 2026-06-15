#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "genre-mapping.h"
#include "track-watcher.h"
#include "eq-profiles.h"

int main() {
    char *currentPreset = NULL;
    while (1) {
        while (!findPlayer()) {
            sleep(2);
        }

        currentPreset = pullGenre(currentPreset);
        sleep(1);
    }
}
