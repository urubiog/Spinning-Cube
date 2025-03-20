BIN_DIR = ./bin

CFLAGS = -lm

SRC_DIRS = . ./other_polyhedra

SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))

EXECS = $(patsubst %.c, $(BIN_DIR)/%, $(notdir $(SRCS)))

all: $(EXECS)

$(BIN_DIR)/%: %.c
	@mkdir -p $(BIN_DIR)
	gcc $< -o $@ $(CFLAGS)

$(BIN_DIR)/%: ./other_polyhedra/%.c
	@mkdir -p $(BIN_DIR)
	gcc $< -o $@ $(CFLAGS)

clean:
	rm -rf $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)
