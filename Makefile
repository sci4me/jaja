ifndef LIBJIT_DIR
$(error LIBJIT_DIR is not set)
endif

LIBJIT_INCLUDE_PATH=$(LIBJIT_DIR)/include
LIBJIT_AR=$(LIBJIT_DIR)/jit/.libs/libjit.a

SRC_DIR=src
TESTS_DIR=tests
CC=g++

SOURCES=$(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp) $(wildcard $(SRC_DIR)/*/*/*.cpp) $(wildcard $(SRC_DIR)/*/*/*/*.cpp) $(wildcard $(SRC_DIR)/*/*/*/*/*.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(filter %.cpp,$(SOURCES)))
TEST_SOURCES=$(wildcard $(TESTS_DIR)/*.cpp)
TEST_OBJECTS=$(patsubst %.cpp,%.o,$(filter %.cpp,$(TEST_SOURCES)))

TEST_EXECUTABLE=jaja-tests

EXECUTABLE=jaja
LDFLAGS=-lpthread
CXXFLAGS=

.PHONY: all clean run

all: release

release: clean
release: CXXFLAGS+=-O2 -DNDEBUG
release: $(EXECUTABLE)

debug: clean
debug: LDFLAGS+=-g
debug: CXXFLAGS+=-g -O0
debug: $(EXECUTABLE)

tests/test_setup.cpp:
	sh test_setup

test: LDFLAGS+=-g
test: CXXFLAGS+=-g -O0 -DTESTING
test: clean $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBJIT_AR)

$(TEST_EXECUTABLE): tests/test_setup.cpp $(OBJECTS) $(TEST_OBJECTS) tests/test_setup.o
	$(CC) $(LDFLAGS) -o $@ $(LIBJIT_AR) $(filter-out $(SRC_DIR)/main.o,$(OBJECTS)) $(TEST_OBJECTS)

$(TESTS_DIR)/%.o: $(TESTS_DIR)/%.cpp
	$(CC) $(CXXFLAGS) -I $(TESTS_DIR) -I $(LIBJIT_INCLUDE_PATH) -c -o "$@" "$<"

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CXXFLAGS) -I $(SRC_DIR) -I $(LIBJIT_INCLUDE_PATH) -c -o "$@" "$<"

clean:
	rm -f $(OBJECTS)
	rm -f $(TEST_OBJECTS)
	rm -f $(EXECUTABLE)
	rm -f $(TEST_EXECUTABLE)
	rm -f $(TESTS_DIR)/test_setup.cpp