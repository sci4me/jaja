SRC_DIR=src
LIB_CODE_DIR=lib_code
LIB_OBJS_DIR=lib_objs
CC=g++

SOURCES=$(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp) $(wildcard $(SRC_DIR)/*/*/*.cpp) $(wildcard $(SRC_DIR)/*/*/*/*.cpp) $(wildcard $(SRC_DIR)/*/*/*/*/*.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(filter %.cpp,$(SOURCES)))
EXTRA_OBJECTS=$(wildcard $(LIB_OBJS_DIR)/*.o)

TEST_EXECUTABLE=se-tests

EXECUTABLE=se
LDFLAGS=
CXXFLAGS=

.PHONY: all clean run

all: release

release: clean
release: CXXFLAGS+=-O2
release: $(EXECUTABLE)

debug: clean
debug: LDFLAGS+=-g
debug: CXXFLAGS+=-g -O0
debug: $(EXECUTABLE)

test: clean
test: LDFLAGS+=-g
test: CXXFLAGS+=-g -O0 -DTESTING
test: ./test_setup
test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(EXTRA_OBJECTS)

$(TEST_EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(filter-out src/main.o,$(OBJECTS)) $(EXTRA_OBJECTS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CXXFLAGS) -I $(SRC_DIR) -I $(LIB_CODE_DIR) -c -o "$@" "$<"

clean:
	rm -f $(OBJECTS)
	rm -f $(EXECUTABLE)
