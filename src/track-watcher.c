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
    const char *command = "playerctl --list-all";
    char *player = NULL;
    const char *vlc = "vlc";
    size_t capacity = 0;
    bool found = false;

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
            found = true;
            break;
        }
    }

    free(player);
    pclose(fpipe);

    //printf("couldn't find vlc in player list\n");
    return found;
}

static void printTrackInfo(const TrackContext *context, const EqRecommendation *recommendation) {
    printf("track name: %s\n", context->title);
    printf("artist: %s\n", context->artist);
    printf("genre: %s\n", context->genre);

    printf("profile selected: %s\n", recommendation->profileName);
    printf("confidence: %f\n", recommendation->confidence);
    printf("---------------\n");
}

static bool readPlayerctlValue(const char *command, char **value) {
    FILE *pipe;
    char *line = NULL;
    size_t capacity = 0;
    ssize_t nread;

    if (value == NULL) {
        return false;
    }
    *value = NULL;

    pipe = (FILE*)popen(command, "r");
    if (pipe == 0) {
        perror("popen() failed");
        return false;
    }

    nread = getline(&line, &capacity, pipe);
    if (nread == -1) {
        perror("getline");
        free(line);
        pclose(pipe);
        return false;
    }

    line[strcspn(line, "\n")] = '\0';
    pclose(pipe);
    *value = line;

    return true;
}

static bool readTrackContext(TrackContext *context) {
    // TODO: playerctl follow doesn't work if the player is not running - need to figure out a solution
    //char *command = "playerctl --player=vlc --follow metadata xesam:genre";
    const char *genreCommand = "playerctl --player=vlc metadata xesam:genre";
    const char *urlCommand = "playerctl --player=vlc metadata xesam:url";
    char *genre = NULL;
    char *url = NULL;

    if (context == NULL) {
        return false;
    }

    // TODO: this needs to continuously run (maybe switch to async architecture?)
    if (!readPlayerctlValue(genreCommand, &genre)) {
        return false;
    }

    //grab the file path to pass to taglib
    if (!readPlayerctlValue(urlCommand, &url)) {
        free(genre);
        return false;
    }

    extractMetadata(context, url, "vlc");

    free(genre);
    free(url);

    return true;
}

static void createRecommendation(const TrackContext *context, EqRecommendation *recommendation) {
    bool recommendationMade = recommendFromTrackContext(context, recommendation);
    if (recommendationMade) {
        if (!validateRecommendation(recommendation)) {
            neutralFallbackRecommendation(recommendation);
        }
    } else {
        neutralFallbackRecommendation(recommendation);
    }
}

static bool shouldSkipActiveTrack(const CurrTrackInfo *info, const TrackContext *context,
        const EqRecommendation *recommendation) {
    return info != NULL &&
            strcmp(recommendation->profileName, info->preset) == 0 &&
            strcmp(context->filePath, info->URL) == 0;
}

static void updateCurrTrackInfo(CurrTrackInfo *info, const TrackContext *context,
        const EqRecommendation *recommendation) {
    if (info == NULL) {
        return;
    }

    snprintf(info->preset, sizeof(info->preset), "%s", recommendation->profileName);
    snprintf(info->URL, sizeof(info->URL), "%s", context->filePath);
}

void buildTrackContext(CurrTrackInfo *info) {
    TrackContext context;
    EqRecommendation recommendation;

    if (!readTrackContext(&context)) {
        return;
    }

    createRecommendation(&context, &recommendation);

    //const EqProfile *profileToApply = genreToPreset(context.genre);

    if (shouldSkipActiveTrack(info, &context, &recommendation)) {
        return;
    }

    bool eqApplied = applyEQ(&recommendation);
    if (eqApplied == true) {
        updateCurrTrackInfo(info, &context, &recommendation);
        printTrackInfo(&context, &recommendation);
        return;
    } else {
        printf("eq application failed :(\n");
    }
}
