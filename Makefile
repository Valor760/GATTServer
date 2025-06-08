NPROCS := $(shell grep -c ^processor /proc/cpuinfo)

all: build

build: configure
	cmake --build build/ -j $(NPROCS)

configure:
	cmake -S . -B build/

clean:
	rm -rf build/