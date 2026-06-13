#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "genre-mapping.h"

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
    
    //ssize_t nread = ;
    char *currGenre = genre;

    while (getline(&genre, &capacity, fpipe) != -1) {
        printf("%s", genre);
    }
    

    perror("getline");
    free(genre);
    pclose(fpipe);
}