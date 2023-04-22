SRC := $(shell find src -name '*.c')
OBJ := $(patsubst src/%.c, bin/%.o, $(SRC))
C_FLAGS := 

EXT_INCLUDE := -Ilib
INCLUDE := -Iinclude $(EXT_INCLUDE)

EXT_LIBRARY := -Llib
LIBRARY := $(EXT_LIBRARY) -lfreetype -lm

all: bin captcha
debug: C_FLAGS += -DDEBUG
debug: bin captcha

captcha: $(OBJ)
	gcc $^ -o $@ ${LIBRARY}

bin:
	mkdir bin

bin/%.o: src/%.c
	gcc $(C_FLAGS) $(INCLUDE) -c $^ -o $@

.PHONY: clean
clean:
	rm -fr bin
	rm -f captcha