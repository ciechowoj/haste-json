
CXX = g++-7
CXXFLAGS = -march=native -g -O0 -Wall -Wextra -ansi -pedantic -std=c++17

INCLUDE_DIRS = -Iinclude -Ibackward-cpp -I../haste-test/include -Idouble-conversion
CXXFLAGS := $(CXXFLAGS) $(INCLUDE_DIRS)

SOURCE_FILES = $(wildcard *.cpp)
HEADER_FILES = $(wildcard include/haste/*)
ALL_OBJECT_FILES = $(SOURCE_FILES:%.cpp=build/%.o)
OBJECT_FILES := $(filter-out build/test.o, $(ALL_OBJECT_FILES))

UNITTESTS_SOURCE_FILES = $(wildcard unit-tests/*.cpp)
UNITTESTS_OBJECT_FILES = $(UNITTESTS_SOURCE_FILES:%.cpp=build/%.o)

BENCHMARK_SOURCE_FILES = $(wildcard benchmark/*.cpp)
BENCHMARK_OBJECT_FILES = $(BENCHMARK_SOURCE_FILES:%.cpp=build/%.o)

DEPENDENCY_FLAGS = -MT $@ -MMD -MP -MF build/$*.Td
DEPENDENCY_POST = mv -f build/$*.Td build/$*.d

.PHONY: all test clean distclean

all: build/libhaste.a

test: build/test.bin
	@cd "./unit-tests"; "./../build/test.bin"

benchmark: build/benchmark.bin
	@cd "./benchmark"; "./../build/benchmark.bin"

clean:
	rm -rf build

distclean:
	rm -rf build

build:
	mkdir build

build/benchmark: | build
	mkdir build/benchmark

build/unit-tests: | build
	mkdir build/unit-tests

build/%.d: ;

build/%.o: %.cpp build/%.d | build build/benchmark build/unit-tests
	$(CXX) -c $(DEPENDENCY_FLAGS) $(CXXFLAGS) $< -o $@
	$(DEPENDENCY_POST)

-include $(ALL_OBJECT_FILES:build/%.o=build/%.d)
-include $(BENCHMARK_OBJECT_FILES:build/%.o=build/%.d)
-include $(UNITTESTS_OBJECT_FILES:build/%.o=build/%.d)

build/libhaste-json.a: $(OBJECT_FILES) Makefile
	ar rcs build/libhaste-json.a $(OBJECT_FILES)

build/test.bin: build/libhaste-json.a $(UNITTESTS_OBJECT_FILES) build/test.o Makefile
	$(CXX) -g $(UNITTESTS_OBJECT_FILES) build/test.o -L../haste-test/build -Ldouble-conversion/build  -Lbuild -lhaste-test -lhaste-json -ldouble-conversion -o build/test.bin

build/benchmark.bin: build/libhaste-json.a $(BENCHMARK_OBJECT_FILES) Makefile
	$(CXX) -g $(BENCHMARK_OBJECT_FILES) -L../haste-test/build -Lbuild -Ldouble-conversion/build -lhaste-test -lhaste-json -ldouble-conversion -o build/benchmark.bin






