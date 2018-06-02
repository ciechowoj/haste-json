
CXX = g++-7
CXXFLAGS = -march=native -g -O0 -Wall -Wextra -ansi -pedantic -std=c++17

INCLUDE_DIRS = -Iinclude -Ibackward-cpp -I../haste-test/include

CXXFLAGS := $(CXXFLAGS) $(INCLUDE_DIRS)

SOURCE_FILES = $(wildcard *.cpp)
HEADER_FILES = $(wildcard include/haste/*)
ALL_OBJECT_FILES = $(SOURCE_FILES:%.cpp=build/%.o)
OBJECT_FILES := $(filter-out build/test.o, $(ALL_OBJECT_FILES))

DEPENDENCY_FLAGS = -MT $@ -MMD -MP -MF build/$*.Td
DEPENDENCY_POST = mv -f build/$*.Td build/$*.d

.PHONY: all test clean distclean

all: build/libhaste.a

test: build/test.bin
	@cd "./unit-tests"; "./../build/test.bin"

clean:
	rm -rf build

distclean:
	rm -rf build

build:
	mkdir build

build/%.d: ;

build/%.o: %.cpp build/%.d | build
	$(CXX) -c $(DEPENDENCY_FLAGS) $(CXXFLAGS) $< -o $@
	$(DEPENDENCY_POST)

-include $(ALL_OBJECT_FILES:build/%.o=build/%.d)

build/libhaste-json.a: $(OBJECT_FILES) Makefile
	ar rcs build/libhaste-json.a $(OBJECT_FILES)

build/test.bin: build/libhaste-json.a build/test.o Makefile
	$(CXX) -g build/test.o -L../haste-test/build -Lbuild -lhaste-test -lhaste-json -ldw -lpng -o build/test.bin








