#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H
#include "file.h"

int loop(struct fat_struct *fat, struct file_struct *root, char *fat_filename);

#endif
