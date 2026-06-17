#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "tag_c.h"
#include "track-context.h"

void pullString(char *field, size_t fieldSize, const char *src) {
    if (fieldSize == 0) {
        return;
    }

    snprintf(field, fieldSize, "%s", src ? src : "");
}

// helper that allows us to calculate metadata quality
static unsigned int isFieldPopulated(const char *field) {
    return field != NULL && field[0] != '\0';
}

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

// local urls often contain %20 or other escape characters that break when passed into extractMetadata()
// this function decodes those escapes into real characters to prevent runtime problems
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

void printContext(const TrackContext *context) {
    if (context == NULL) {
        printf("TrackContext: NULL\n");
        return;
    }

    printf("TrackContext {\n");
    printf("  title:              [%s]\n", context->title);
    printf("  artist:             [%s]\n", context->artist);
    printf("  album:              [%s]\n", context->album);
    printf("  genre:              [%s]\n", context->genre);
    printf("  year:               [%s]\n", context->year);
    printf("  filePath:           [%s]\n", context->filePath);
    printf("  sourcePlayer:       [%s]\n", context->sourcePlayer);
    printf("  metadataCompleteness: %d\n", context->metadataQuality);
    printf("}\n");
}

void extractMetadata(TrackContext *context, const char *trackURL, const char *sourcePlayer) {
    char localPath[512];

    if (context == NULL) {
        fprintf(stderr, "extractMetadata: context is NULL\n");
        return;
    }

    memset(context, 0, sizeof(*context));
    context->metadataQuality = 0;

    // populating the fields that we can't obtain through taglib
    pullString(context->filePath, sizeof(context->filePath), trackURL);
    pullString(context->sourcePlayer, sizeof(context->sourcePlayer), sourcePlayer);

    if (trackURL == NULL || trackURL[0] == '\0') {
        return;
    }

    localPathFromTrackURL(localPath, sizeof(localPath), trackURL);

    TagLib_File *file = taglib_file_new(localPath);
    if (!file || !taglib_file_is_valid(file)) {
        if (file) {
            taglib_file_free(file);
        }
        return;
    }

    TagLib_Tag *tag = taglib_file_tag(file);
    if (!tag) {
        taglib_file_free(file);
        return;
    }

    char *title = taglib_tag_title(tag);
    char *artist = taglib_tag_artist(tag);
    char *album = taglib_tag_album(tag);
    char *genre = taglib_tag_genre(tag);
    unsigned int year = taglib_tag_year(tag);

    pullString(context->title, sizeof(context->title), title);
    pullString(context->artist, sizeof(context->artist), artist);
    pullString(context->album, sizeof(context->album), album);
    pullString(context->genre, sizeof(context->genre), genre);

    if (year != 0) {
        snprintf(context->year, sizeof(context->year), "%u", year);
    }

    taglib_tag_free_strings();
    taglib_file_free(file);

    unsigned int count = 0;
    count += isFieldPopulated(context->title);
    count += isFieldPopulated(context->artist);
    count += isFieldPopulated(context->album);
    count += isFieldPopulated(context->genre);
    count += isFieldPopulated(context->year);
    count += isFieldPopulated(context->filePath);
    count += isFieldPopulated(context->sourcePlayer);
    context->metadataQuality = count;
    //printf("# of filled fields: %d", context->metadataQuality);

    return;
}
