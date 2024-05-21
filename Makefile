CC = gcc

DIR = testcases

SRC := $(wildcard $(DIR)/**/*.c)
OBJ := $(patsubst $(DIR)/%.c,$(DIR)/%.o,$(SRC))

all: $(OBJ)
	mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make
	python3 autograder.py

test%: $(OBJ)
	mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make
	python3 autograder.py $@

$(DIR)/%.o: $(DIR)/%.c
	$(CC) -c -fcommon $< -o $@

# This rule override the wildcard one
$(DIR)/test1/%.o: $(DIR)/test1/%.c
	$(CC) -c -fcommon -fno-pie $< -o $@

clean:
	rm -rf $(DIR)/**/*.o $(DIR)/**/test*

submit:
	zip -r $(shell whoami | cut -d- -f1 | sed -e 's/[0-9]*/&-lab6.zip/') src/
