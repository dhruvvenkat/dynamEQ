#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "track-watcher.h"
#include "genre-mapping.h"
#include "eq-profiles.h"
#include "eq-engine.h"

bool findPlayer() {
    FILE *fpipe;
    char *command = "playerctl --list-all";
    char* player = NULL;
    char *vlc = "vlc";
    size_t capacity = 0;

    fpipe = (FILE*)popen(command, "r");
    if (fpipe == 0) {
        perror("findPlayer() failed");
        exit(EXIT_FAILURE);
    }

    while (getline(&player, &capacity, fpipe) != -1) {
        player[strcspn(player, "\n")] = '\0';
        if (strcmp(player, vlc) == 0) {
            //printf("vlc player found!\n");
            //printf("-----------------\n");
            return true;
        }
    }

    //printf("couldn't find vlc in player list\n");
    return false;
}

const char *pullGenre(const char *currentPreset) {
    FILE *fpipe;
    // TODO: playerctl follow doesn't work if the player is not running - need to figure out a solution
    //char *command = "playerctl --player=vlc --follow metadata xesam:genre";
    char *command = "playerctl --player=vlc metadata xesam:genre";
    char* genre = NULL;
    size_t capacity = 0;

    fpipe = (FILE*)popen(command, "r");

    if (fpipe == 0) {
        perror("popen() failed");
        return currentPreset;
    }

    // TODO: this needs to continuously run (maybe switch to async architecture?)
    ssize_t nread = getline(&genre, &capacity, fpipe);

    if (nread == -1) {
        perror("getline");
        free(genre);
        pclose(fpipe);
        //return 1;
    }

    genre[strcspn(genre, "\n")] = '\0'; // cutting newline character out of genre


    //printf("genre: %s", genre);
    const EqProfile *profileToApply = genreToPreset(genre);

    if (strcmp(profileToApply->name, currentPreset) == 0) {
        //printf("preset doesn't need to change...skipping\n");
            pclose(fpipe);
        return currentPreset;
    }

    printf("genre: %s\n", genre);
    bool eqApplied = applyEQ(profileToApply);
    if (eqApplied == true) {
        pclose(fpipe);
        return profileToApply->name;
    } else {
        printf("eq application failed :(\n");
    }

    pclose(fpipe);
    return currentPreset;
}
