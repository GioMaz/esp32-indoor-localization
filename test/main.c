#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

#include "../components/dataset/dataset.h"

int main()
{
    FILE *file = fopen("../storage_image/dataset.bin", "r");
    if (file == NULL)
        exit(1);

    Dataset dataset;
    fread(&dataset, 1, sizeof(dataset), file);

    printf("data_count %d\n", dataset.data_count);
    dataset_print(&dataset);
}
