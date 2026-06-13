#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "genre-mapping.h"

void pullGenre() {
    FILE *fpipe;
    char *command = "playerctl -p vlc metadata xesam:genre";
    char* genre = NULL;
    size_t capacity = 0;    

    fpipe = (FILE*)popen(command, "r");

    if (fpipe == 0) {
        perror("popen() failed");
        exit(EXIT_FAILURE);
    }

    ssize_t nread = getline(&genre, &capacity, fpipe);

    if (nread == -1) {
        perror("getline");
        free(genre);
        pclose(fpipe);
        //return 1;
    }

    printf("%s", genre);

    pclose(fpipe);
}