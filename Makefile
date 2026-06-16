CFLAGS = -Wall -Wextra -g -Iinclude $(shell pkg-config --cflags taglib_c)
LDFLAGS = $(shell pkg-config --libs taglib_c) -lstdc++ -lm

make:
	cc src/main.c src/genre-mapping.c src/track-watcher.c src/eq-profiles.c src/eq-engine.c src/track-context.c $(CFLAGS) -o equalizer.out $(LDFLAGS)
