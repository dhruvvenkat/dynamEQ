#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "track-watcher.h"
#include "genre-mapping.h"
#include "eq-profiles.h"
#include "eq-engine.h"

void pullGenre() {
    FILE *fpipe;
    // TODO: playerctl follow doesn't work if the player is not running - need to figure out a solution
    //char *command = "playerctl --player=vlc --follow metadata xesam:genre";
    char *command = "playerctl --player=vlc metadata xesam:genre";
    char* genre = NULL;
    size_t capacity = 0;

    fpipe = (FILE*)popen(command, "r");


    if (fpipe == 0) {
        perror("popen() failed");
        exit(EXIT_FAILURE);
    }

    // TODO: this needs to continuously run (maybe switch to async architecture?)
    ssize_t nread = getline(&genre, &capacity, fpipe);
    genre[strcspn(genre, "\n")] = '\0'; // cutting newline character out of genre

    if (nread == -1) {
        perror("getline");
        free(genre);
        pclose(fpipe);
        //return 1;
    }
    //printf("genre: %s", genre);
    EqProfile *profileToApply = genreToPreset(genre);
    applyEQ(profileToApply);

    pclose(fpipe);
}
