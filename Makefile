CFLAGS = -Wall -Wextra -g $(shell pkg-config --cflags taglib_c)
LDFLAGS = $(shell pkg-config --libs taglib_c) -lstdc++ -lm

make:
	cc main.c genre-mapping.c track-watcher.c eq-profiles.c eq-engine.c track-context.c $(CFLAGS) -o equalizer.out $(LDFLAGS)
