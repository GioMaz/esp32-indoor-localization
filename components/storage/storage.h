#pragma once

#define BASE_PATH "/littlefs"
#define PATH_LEN 522

#include "dataset.h"

void mount_storage(void);
void unmount_storage(void);
int read_dataset_from_storage(Dataset *dataset);
