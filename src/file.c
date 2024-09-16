#include "file.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

void *fetch_data(struct fat_struct *fat, struct file_struct *obj, char *fat_filename){
	uint32_t first_cluster = 0;
    first_cluster	= first_cluster | obj->DIR_fstClusHI;
	first_cluster = first_cluster << 16;
	first_cluster =  first_cluster | obj->DIR_fstClusLO;
	if(fat->fat_allocation_table[first_cluster + 2] == FAT_FREE_CLUSTER) return NULL;
	//Allocar a data
	void **data_block = malloc(sizeof(data_block));
    data_block[0] = malloc(TAMANHO_CLUSTER * TAMANHO_SETOR);
	if(data_block[0] == NULL) return NULL;
	for(int i = 0; i < INT_MAX; i++){

		//LÊ O CLUSTER
		data_block[i] = read_cluster(first_cluster, fat, fat_filename);
		//VERIFICA SE TEM MAIS
		if(fat->fat_allocation_table[first_cluster + 2] == FAT_EOF_CLUSTER){
			data_block[i+1] = NULL;
			break;
		} 
		first_cluster = fat->fat_allocation_table[first_cluster + 2];
		//ALOCA MAIS UM CHUNK DE MEMÓRIA
		data_block[i + 1] = malloc(TAMANHO_CLUSTER * TAMANHO_SETOR);
		if(data_block[i + 1] == NULL) return NULL;
	}
	return data_block;
}

//Escrever em cluster já alocado
static int write_data(struct fat_struct *fat, uint32_t first_cluster, void *data, uint32_t data_size, char *fat_filename){
	if(fat->fat_allocation_table[first_cluster + 2] == FAT_FREE_CLUSTER) return -1;
	while(fat->fat_allocation_table[first_cluster + 2] != FAT_EOF_CLUSTER){
		first_cluster = fat->fat_allocation_table[first_cluster + 2];	
	}
	uint8_t *cluster = read_cluster(first_cluster, fat, fat_filename);
	for(unsigned int i = 0; i < INT_MAX; i++){
		int current_data = 0;
	    memcpy(&current_data,&cluster[i], 4);
		if(current_data == EOF){
			uint32_t available_size = (TAMANHO_CLUSTER * TAMANHO_SETOR) -  i;
			if(available_size > data_size + 4){
				memcpy(&cluster[i], data, data_size);
				memcpy(&cluster[i+data_size], "\xff\xff\xff\xff", 4);
				int rc = write_cluster(first_cluster, fat, 'w', fat_filename, cluster);
				if(rc != 0) return rc;
			}else{
				uint32_t next_cluster = allocate_cluster(fat, 1, fat_filename);
				fat->fat_allocation_table[first_cluster + 2] = next_cluster;
				int rc = write_cluster(next_cluster, fat, 'w', fat_filename, data);
				if(rc != 0) return rc;
			}
			break;
		}	
	}
	return 0;
}

struct file_struct *new_directory(char *DIR_name, char type, struct file_struct *parent, struct fat_struct *fat, char *fat_filename){

	struct file_struct *n_dir = calloc(1, sizeof(struct file_struct));
	if(strlen(DIR_name) > 11 || strlen(DIR_name) < 3){
		//Nome de diretório maior que o permitido
		free(n_dir);
		return NULL;	
	}
	else{
		strncpy(n_dir->DIR_name, DIR_name, 11);	
	}
	n_dir->DIR_attr = ATTR_DIRECTORY | ATTR_ARCHIVE;
	if(type == 'h') n_dir->DIR_attr |= ATTR_HIDDEN; 
	n_dir->DIR_fileSize = htole32(32);
	//Alocar um cluster e adicionar seu endereço à estrutura	
	uint32_t directory_cluster = allocate_cluster(fat, 1, fat_filename);
	if(directory_cluster == UINT32_MAX) return NULL; // Completely allocated fat
	n_dir->DIR_fstClusLO = htole32(directory_cluster & 0x0000ffff);
	n_dir->DIR_fstClusHI = htole32(directory_cluster >> 16);
	//Atualizar data e hora
	
	if(parent != NULL){
		n_dir->DIR_fileSize = htole32(64);
		uint32_t parent_first_cluster = parent->DIR_fstClusHI;
		parent_first_cluster = parent_first_cluster << 16;
		parent_first_cluster = parent_first_cluster | parent->DIR_fstClusLO;
		write_data(fat, parent_first_cluster, n_dir, sizeof(struct file_struct), fat_filename);
		memcpy(parent->DIR_name, "..         ", 11);
		write_data(fat, directory_cluster, parent, sizeof(struct file_struct), fat_filename);
	}
	memcpy(n_dir->DIR_name, ".          ", 11);
	write_data(fat, directory_cluster, n_dir, sizeof(struct file_struct), fat_filename);
	return n_dir;
}

struct file_struct *new_file(char *DIR_name, char type, struct file_struct *parent, struct fat_struct *fat, char *fat_filename){

	struct file_struct *n_file = calloc(1, sizeof(struct file_struct));
	if(parent == NULL) return NULL;
	if(strlen(DIR_name) > 11 || strlen(DIR_name) < 3){
		free(n_file);
		return NULL;	
	}
	else{
		strncpy(n_file->DIR_name, DIR_name, 11);	
	}
	n_file->DIR_attr = ATTR_DIRECTORY | ATTR_ARCHIVE;
	if(type == 'h') n_file->DIR_attr |= ATTR_HIDDEN; 
	n_file->DIR_fileSize = htole32(0x00000000);
	//Alocar um cluster e adicionar seu endereço à estrutura	
	uint32_t directory_cluster = allocate_cluster(fat, 1, fat_filename);
	if(directory_cluster == UINT32_MAX) return NULL; // Completely allocated fat
	n_file->DIR_fstClusLO = htole32(directory_cluster & 0x0000ffff);
	n_file->DIR_fstClusHI = htole32(directory_cluster >> 16);
	//Atualizar data e hora
	
	uint32_t parent_first_cluster = parent->DIR_fstClusHI;
	parent_first_cluster = parent_first_cluster << 16;
	parent_first_cluster = parent_first_cluster | parent->DIR_fstClusLO;
	write_data(fat, parent_first_cluster, n_file, sizeof(struct file_struct), fat_filename);
	return n_file;
}
