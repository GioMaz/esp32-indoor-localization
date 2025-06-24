CC=gcc
COMPONENTS_DIR=../components

run: test
	./test

test: test.c
	$(CC) -I $(COMPONENTS_DIR)/utils -L $(COMPONENTS_DIR)/dataset -o test test.c $(COMPONENTS_DIR)/dataset/dataset.c

clean:
	rm test
