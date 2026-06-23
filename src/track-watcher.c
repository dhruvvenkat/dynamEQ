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
#include "audio-features.h"

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

// brought in these helpers from track-context.c, need to remove them from track-context to avoid redundancy
static int hexValue(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return -1;
}

static void decodePercentEscapes(char *field, size_t fieldSize, const char *src) {
    size_t writeIndex = 0;

    if (fieldSize == 0) {
        return;
    }

    if (src == NULL) {
        field[0] = '\0';
        return;
    }

    while (*src != '\0' && writeIndex + 1 < fieldSize) {
        int high = hexValue(src[1]);
        int low = hexValue(src[2]);

        if (*src == '%' && high >= 0 && low >= 0) {
            field[writeIndex++] = (char)((high << 4) | low);
            src += 3;
        } else {
            field[writeIndex++] = *src++;
        }
    }

    field[writeIndex] = '\0';
}

static void localPathFromTrackURL(char *field, size_t fieldSize, const char *trackURL) {
    const char *path = trackURL;

    if (trackURL != NULL && strncmp(trackURL, "file://", 7) == 0) {
        path = trackURL + 7;
        if (strncmp(path, "localhost/", 10) == 0) {
            path += 9;
        }
    }

    decodePercentEscapes(field, fieldSize, path);
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

static void isCSVEmpty(FILE *file) {
    fseek(file, 0, SEEK_END);

    if (ftell(file) == 0) {
        fprintf(file, "title,artist,album,genre,year,profile name,confidence,applied,rms,peak,dynamic_range_proxy,bass_energy,low_mid_energy,mid_energy,presence_energy,treble_energy,bass_ratio,brightness,harshness_ratio");
    }
}

static bool logTrackEvent(const TrackContext *context, const EqRecommendation *recommendation, bool applied, AudioFeatures *features) {
    FILE *csvptr;
    csvptr = fopen("HISTORY.csv", "a"); // eventually make this an absolute path
    if (!csvptr) {
        printf("error - cannot open history csv file");
        return false;
    }

    isCSVEmpty(csvptr);

    fprintf(csvptr, "%s,%s,%s,%s,%s,%s,%f,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
        context->title,
        context->artist,
        context->album,
        context->genre,
        context->year,
        recommendation->profileName,
        recommendation->confidence,
        applied,
        features->rms,
        features->peak,
        features->dynamicRangeProxy,
        features->bassEnergy,
        features->lowMidEnergy,
        features->midEnergy,
        features->presenceEnergy,
        features->trebleEnergy,
        features->bassRatio,
        features->brightness,
        features->harshnessRatio
    );

    fclose(csvptr);

    return true;
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

    char localPath[512];
    localPathFromTrackURL(localPath, sizeof(localPath), context.filePath);

    AudioFeatures features;

    bool eqApplied = applyEQ(&recommendation);
    if (eqApplied == true) {
        updateCurrTrackInfo(info, &context, &recommendation);
        extractAudioFeatures(localPath, &features);
        logTrackEvent(&context, &recommendation, eqApplied, &features);
        printTrackInfo(&context, &recommendation);
        return;
    } else {
        printf("eq application failed :(\n");
    }
}
