#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "dataset.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <dataset.bin>\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file %s\n", filename);
        exit(1);
    }

    Dataset dataset;
    int rb = fread((char *)&dataset, 1, sizeof(Dataset), file);
    // if (rb == 0) {
    //     fprintf(stderr, "Failed to read bytes from dataset\n");
    //     exit(1);
    // }

    dataset_print(&dataset);
}
