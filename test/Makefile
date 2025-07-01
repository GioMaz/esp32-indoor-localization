CC=gcc
BUILD_DIR=build
STORAGE_IMAGE_DIR=../storage_image
COMPONENTS_DIR=../components

# Include dirs
INCLUDE_DATASET=-I$(COMPONENTS_DIR)/dataset
INCLUDE_UTILS=-I$(COMPONENTS_DIR)/utils
INCLUDE_INFERENCE=-I$(COMPONENTS_DIR)/inference

# File names
TEST_OUT=test.out
DATASET_PRINT_OUT=dataset_print.out
WIFI_MAP_CSV=wifi_map.csv
HEATMAP_PNG=heatmap.png
DATASET_BIN=dataset.bin

run: $(BUILD_DIR)/$(TEST_OUT) $(BUILD_DIR)/$(WIFI_MAP_CSV) $(BUILD_DIR)/$(HEATMAP_PNG)
	$(BUILD_DIR)/$(TEST_OUT) $(BUILD_DIR)/$(WIFI_MAP_CSV)

$(BUILD_DIR)/$(TEST_OUT): test.c | $(BUILD_DIR)
	$(CC) $(INCLUDE_DATASET) $(INCLUDE_UTILS) $(INCLUDE_INFERENCE) -o $(BUILD_DIR)/$(TEST_OUT) $(COMPONENTS_DIR)/dataset/dataset.c $(COMPONENTS_DIR)/inference/inference.c test.c -lm

$(BUILD_DIR)/$(WIFI_MAP_CSV) $(BUILD_DIR)/$(HEATMAP_PNG): generate.py | $(BUILD_DIR)
	python3 generate.py $(BUILD_DIR)/$(WIFI_MAP_CSV) --heatmap $(BUILD_DIR)/$(HEATMAP_PNG)

$(BUILD_DIR)/$(DATASET_PRINT_OUT) : dataset_print.c | $(BUILD_DIR)
	$(CC) $(INCLUDE_DATASET) -o $(BUILD_DIR)/$(DATASET_PRINT_OUT) $(INCLUDE_UTILS) $(COMPONENTS_DIR)/dataset/dataset.c dataset_print.c

dataset_print: $(BUILD_DIR)/$(DATASET_PRINT_OUT)
	$(BUILD_DIR)/$(DATASET_PRINT_OUT) $(STORAGE_IMAGE_DIR)/$(DATASET_BIN)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
