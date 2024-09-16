#include "fat.h"
#include "file.h"
#include "file_manager.h"
#include <stdlib.h>

#define FAT_FILE "f.fat"
int main(int argc, char *argv[]){

	struct fat_struct *fat = init(FAT_FILE);
	struct file_struct *nc = new_directory("Celsao", ' ', NULL, fat, FAT_FILE);
	int rc = loop(fat, nc, FAT_FILE);
	//new_file("Celsao", ' ', nc, fat, FAT_FILE);
	write_fat(fat, FAT_FILE);

	return 0;
}
