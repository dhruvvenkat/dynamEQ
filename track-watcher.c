#include <stdlib.h>
#include <stdio.h>
#include "track-watcher.h"

void pullGenre() {
    FILE *fpipe;
    char *command = "playerctl --player=vlc --follow metadata xesam:genre";
    char* genre = NULL;
    size_t capacity = 0;  
    
    fpipe = (FILE*)popen(command, "r");
    

    if (fpipe == 0) {
        perror("popen() failed");
        exit(EXIT_FAILURE);
    }
    

    while (getline(&genre, &capacity, fpipe) != -1) {
        if ("genre" == "\0") {
            printf("no song detected");
            return;
        }
        printf("%s", genre);
    }
    

    perror("getline");
    free(genre);
    pclose(fpipe);
}