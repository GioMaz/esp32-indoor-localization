CC=gcc
CFLAGS=-Wall
BUILD_DIR=build
COMPONENTS_DIR=../components
INCLUDE_DIRS=-I$(COMPONENTS_DIR)/dataset -I$(COMPONENTS_DIR)/utils -I$(COMPONENTS_DIR)/inference
COMPILED_FILES=$(COMPONENTS_DIR)/dataset/dataset.c $(COMPONENTS_DIR)/inference/inference.c
TEST_OUT=test.out
WIFI_MAP_CSV=wifi_map.csv

run: $(BUILD_DIR)/$(TEST_OUT) $(BUILD_DIR)/$(WIFI_MAP_CSV)
	$(BUILD_DIR)/$(TEST_OUT)

$(BUILD_DIR)/$(TEST_OUT): test.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(COMPILED_FILES) -o $(BUILD_DIR)/$(TEST_OUT) test.c

$(BUILD_DIR)/$(WIFI_MAP_CSV): generate.py | $(BUILD_DIR)
	python3 generate.py

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
