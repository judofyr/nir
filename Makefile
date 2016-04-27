.PHONY: all clean release debug

all: release

clean:
	rm -rf build

build/%:
	mkdir -p $@
	cd $@ && cmake -DCMAKE_BUILD_TYPE=$(@F) ../..

release: | build/Release
	cd build/Release && make

debug: | build/Debug
	cd build/Debug && make

