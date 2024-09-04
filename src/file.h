#ifndef FILE_H
#define FILE_H

#include <inttypes.h>
#include "fat.h"

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

struct file_struct {
	char DIR_name[11]; //Atenção! Essa string não é null-terminated
	uint8_t DIR_attr;
	uint8_t DIR_reserved;
	uint8_t DIR_crtTimeDec;
	uint16_t DIR_crtTime;
	uint16_t DIR_crtDate;
	uint16_t DIR_lstAccDate;
	uint16_t DIR_fstClusHI;
	uint16_t DIR_wrtTime;
	uint16_t DIR_wrtDate;
	uint16_t DIR_fstClusLO;
	uint32_t DIR_fileSize;
} __attribute__ ((packed));

//Retorna um novo diretório, a partir do nome, tipo diretório e diretório pai
struct file_struct *new_directory(char *DIR_name, char type, struct file_struct *parent, struct fat_struct *fat);

#endif
