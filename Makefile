CFLAGS += $(shell pkg-config sdl3 --cflags)
LDFLAGS += $(shell pkg-config sdl3 --libs)


.PHONY: default run clean

default:
	g++ src/*.cpp src/imgui/*.cpp src/formats/*.cpp src/tinyfiledialogs/*.cpp -Ilib -Ilib/formats -Ilib/imgui -Ilib/tinyfiledialogs $(CFLAGS) $(LDFLAGS) -ldl -lconfig -g -o LIVe

