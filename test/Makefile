CC=gcc
COMPONENTS_DIR=../components

run: main
	./main

main: main.c
	$(CC) -I $(COMPONENTS_DIR)/utils -L $(COMPONENTS_DIR)/dataset -o main main.c $(COMPONENTS_DIR)/dataset/dataset.c

clean:
	rm main
