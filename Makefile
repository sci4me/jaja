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

src/test_setup.cpp:
	sh test_setup

test: LDFLAGS+=-g
test: CXXFLAGS+=-g -O0 -DTESTING
test: OBJECTS+=src/test_setup.o
test: clean $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(EXTRA_OBJECTS)

define uniq =
  $(eval seen :=)
  $(foreach _,$1,$(if $(filter $_,${seen}),,$(eval seen += $_)))
  ${seen}
endef

$(TEST_EXECUTABLE): src/test_setup.cpp $(OBJECTS) src/test_setup.o
	$(CC) $(LDFLAGS) -o $@ $(filter-out src/main.o,$(call uniq,$(OBJECTS))) $(EXTRA_OBJECTS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CXXFLAGS) -I $(SRC_DIR) -I $(LIB_CODE_DIR) -c -o "$@" "$<"

clean:
	rm -f $(OBJECTS)
	rm -f $(EXECUTABLE)
	rm -f $(TEST_EXECUTABLE)
	rm -f src/test_setup.cpp