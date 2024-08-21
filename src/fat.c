#include <stdio.h>
#include <stdlib.h>
#include "fat.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <endian.h>

static void fill_boot_sector(struct fat_boot_sector* boot_sector);

//Inicializa e salva a FAT em um arquivo de nome definido pela macro FAT_FILENAME
int init(void){
	int fat_file = open(FAT_FILENAME, O_RDWR | O_CREAT, 0755); 
	if(fat_file < 0) { // TODO: abrir uma fat existente com O_EXCL
		return -1;
	}
	if(ftruncate(fat_file, TAMANHO_CLUSTER * NUM_CLUSTERS)) return -1;
	struct fat_boot_sector* boot_sector = calloc(1, sizeof(struct fat_boot_sector));
	if(boot_sector == NULL) return -1;
	fill_boot_sector(boot_sector);

	return 0;
}

static void fill_boot_sector(struct fat_boot_sector* boot_sector){
	memcpy((char *)boot_sector->boot_jump, "\xeb\x5a\x90", strlen("\xeb\x5a\x90") ); //Três bytes da instrução de branch incondicional (jmp) em assembly x86
	memcpy((char *)boot_sector->nome_oem, "FAT_MOD", strlen("FAT_MOD"));
	boot_sector->tamanho_setor = htole16(TAMANHO_SETOR);
	boot_sector->tamanho_cluster = TAMANHO_CLUSTER;
	boot_sector->setores_reservados = htole16(32);
	boot_sector->num_fats = 2;
	boot_sector->dir_entries = htole16(0);
	boot_sector->setores_16 = htole16(0);
	boot_sector->tipo_midia = 0xf8;
	boot_sector->tamanho_fat_16 = htole16(0);
	boot_sector->setores_trilha = htole16(0);
	boot_sector->cabecas_rw = htole16(0);
	boot_sector->setores_ocultos = htole32(0);
	boot_sector->setores_32 = htole32(NUM_CLUSTERS * TAMANHO_CLUSTER); //Tamanho filesystem = 32MiB
	boot_sector->tamanho_fat_32 = htole32(1);
	boot_sector->flags = htole16(0);
	memcpy(boot_sector->versao, "\x00\x00", strlen("\x00\x00"));
	boot_sector->root_cluster = htole32(2);
	boot_sector->setor_info = htole16(1); //FSInfo setor 1
	boot_sector->backup_setor_boot = htole16(0); //setor em que se encontra uma cópia do setor boot
	memcpy(boot_sector->reservado, "\x00\x00\x00\x00\x00\x00", strlen("\x00\x00\x00\x00\x00\x00"));
	boot_sector->id_drive = 0x80; 
	boot_sector->boot_flags = 0x00; 
	boot_sector->assinatura_boot_extend = 0x00;
	memcpy(boot_sector->tipo_fs, "FAT32   ", strlen("FAT32   "));
	//bootcode
	boot_sector->boot_sign = htol16(0xAA55);

}
