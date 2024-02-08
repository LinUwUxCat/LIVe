CFLAGS += $(shell pkg-config sdl3 --cflags)
LDFLAGS += $(shell pkg-config sdl3 --libs)


.PHONY: default run clean

default:
	g++ src/*.cpp src/imgui/*.cpp src/formats/*.cpp -Ilib -Ilib/formats -Ilib/imgui $(CFLAGS) $(LDFLAGS) -ldl -lconfig -o LIVe

