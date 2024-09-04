#include "file.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

struct file_struct *new_directory(char *DIR_name, char type, struct file_struct *parent, struct fat_struct *fat){

	struct file_struct *n_dir = calloc(1, sizeof(struct file_struct));
	if(strlen(DIR_name) > 11 || strlen(DIR_name) > 3){
		//Nome de diretório maior que o permitido
		free(n_dir);
		return NULL;	
	}
	else{
		strncpy(n_dir->DIR_name, DIR_name, 11);	
	}
	n_dir->DIR_attr = ATTR_DIRECTORY | ATTR_ARCHIVE;
	if(type == 'h') n_dir->DIR_attr |= ATTR_HIDDEN; 
	n_dir->DIR_fileSize = htole32(0x00000000);
	//Alocar um cluster e adicionar seu endereço à estrutura	
	
	//Inicializar esse cluster com 0
	
	return n_dir;
}
