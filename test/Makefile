CC=gcc
CFLAGS=-Wall
COMPONENTS_DIR=../components/
INCLUDE_DIRS=-I$(COMPONENTS_DIR)/dataset -I$(COMPONENTS_DIR)/utils -I$(COMPONENTS_DIR)/inference
COMPILED_FILES=$(COMPONENTS_DIR)/dataset/dataset.c $(COMPONENTS_DIR)/inference/inference.c
TEST_OUT=test.out

run: $(TEST_OUT)
	./$(TEST_OUT)

test.out: test.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(COMPILED_FILES) -o $(TEST_OUT) test.c 

clean:
	rm $(TEST_OUT)
