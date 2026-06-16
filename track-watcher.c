#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "track-watcher.h"
#include "genre-mapping.h"
#include "eq-profiles.h"
#include "eq-engine.h"
#include "track-context.h"

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
    FILE *genrePipe;
    FILE *urlPipe;
    // TODO: playerctl follow doesn't work if the player is not running - need to figure out a solution
    //char *command = "playerctl --player=vlc --follow metadata xesam:genre";
    char *command = "playerctl --player=vlc metadata xesam:genre";
    char *commandURL = "playerctl --player=vlc metadata xesam:url";
    char* genre = NULL;
    char *url = NULL;
    size_t capacity = 0;

    genrePipe = (FILE*)popen(command, "r");

    if (genrePipe == 0) {
        perror("popen() failed");
        return currentPreset;
    }

    // TODO: this needs to continuously run (maybe switch to async architecture?)
    ssize_t nread = getline(&genre, &capacity, genrePipe);

    if (nread == -1) {
        perror("getline");
        free(genre);
        pclose(genrePipe);
        return currentPreset;
    }
    genre[strcspn(genre, "\n")] = '\0'; // cutting newline character out of genre
    pclose(genrePipe);

    //grab the file path to pass to taglib
    capacity = 0;
    urlPipe = (FILE*)popen(commandURL, "r");
    if (urlPipe == 0) {
        perror("popen() failed");
        free(genre);
        return currentPreset;
    }

    nread = getline(&url, &capacity, urlPipe);
    if (nread == -1) {
        perror("getline");
        free(genre);
        pclose(urlPipe);
        return currentPreset;
    }
    url[strcspn(url, "\n")] = '\0';

    TrackContext context;
    extractMetadata(&context, url, "vlc");

    //printf("genre: %s", genre);
    const EqProfile *profileToApply = genreToPreset(genre);

    if (strcmp(profileToApply->name, currentPreset) == 0) {
        //printf("preset doesn't need to change...skipping\n");
            pclose(urlPipe);
        return currentPreset;
    }

    printf("genre: %s\n", genre);
    bool eqApplied = applyEQ(profileToApply);
    if (eqApplied == true) {
        return profileToApply->name;
    } else {
        printf("eq application failed :(\n");
    }

    free(genre);
    free(url);
    return currentPreset;
}
