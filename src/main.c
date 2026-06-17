#include <stdbool.h>
#include <unistd.h>
#include "genre-mapping.h"
#include "track-watcher.h"

int main() {
    CurrTrackInfo avoidDuplicates = {"neutral", ""};
    while (1) {
        while (!findPlayer()) {
            sleep(2);
        }

        buildTrackContext(&avoidDuplicates);
        sleep(1);
    }
}
