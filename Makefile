CFLAGS += $(shell pkg-config sdl3 --cflags)
LDFLAGS += $(shell pkg-config sdl3 --libs)
SRCPATH := src/*.cpp src/imgui/*.cpp src/formats/*.cpp src/tinyfiledialogs/*.cpp

.PHONY: default

default:
	g++ $(SRCPATH) -Ilib -Ilib/formats -Ilib/imgui -Ilib/tinyfiledialogs $(CFLAGS) $(LDFLAGS) -ldl -Wno-write-strings -o LIVe

debug:
	g++ $(SRCPATH) -Ilib -Ilib/formats -Ilib/imgui -Ilib/tinyfiledialogs $(CFLAGS) $(LDFLAGS) -fsanitize=address -ldl -Wno-write-strings -g -o LIVe