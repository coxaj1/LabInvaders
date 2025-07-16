PKGCONFIG=pkg-config
PACKAGES=sdl2
CXXSOURCES=main.cpp lafuncs.cpp
LIBS=-lSDL2_image -lSDL2_ttf -lSDL2_mixer $(shell $(PKGCONFIG) --libs $(PACKAGES))
CFLAGS=$(shell $(PKGCONFIG) --cflags $(PACKAGES)) -g -std=c++1z

all: labinvaders

labinvaders: $(CXXSOURCES) lafuncs.h
	g++ $(CXXSOURCES) $(LIBS) $(CFLAGS) -o labinvaders

.PHONY: all clean

clean:
	rm labinvaders
