#include <stdio.h>
#include <stdlib.h>
#include "fat.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <endian.h>

char boot_code[420] = "\x0e"  /* push cs */
    "\x1f"          /* pop ds */
    "\xbe\x5b\x7c"      /* mov si, offset message_txt */
    /* write_msg: */
    "\xac"          /* lodsb */
    "\x22\xc0"          /* and al, al */
    "\x74\x0b"          /* jz key_press */
    "\x56"          /* push si */
    "\xb4\x0e"          /* mov ah, 0eh */
    "\xbb\x07\x00"      /* mov bx, 0007h */
    "\xcd\x10"          /* int 10h */
    "\x5e"          /* pop si */
    "\xeb\xf0"          /* jmp write_msg */
    /* tecla pressionada: */
    "\x32\xe4"          /* xor ah, ah */
    "\xcd\x16"          /* int 16h */
    "\xcd\x19"          /* int 19h */
    "\xeb\xfe"          /* foo: jmp foo */
    /* mensagem tentativa de boot: */
    "NAO BOOTAVEL! Drive experimental criado para o trabalho prático disciplina DCC062\r\n"
    "pressione qualquer tecla ...\r\n";

static void fill_boot_sector(struct fat_boot_sector* boot_sector);
static void fill_volume_info(struct fat_volume_info* volume_info);

//Inicializa e salva a FAT em um arquivo de nome definido pela macro FAT_FILENAME
int init(void){
	int fat_file = open(FAT_FILENAME, O_RDWR | O_CREAT, 0755); 
	if(fat_file < 0) { // TODO: abrir uma fat existente com O_EXCL
		return -1;
	}
	if(ftruncate(fat_file, TAMANHO_CLUSTER * NUM_CLUSTERS * TAMANHO_SETOR)) return -1;
	struct fat_boot_sector* boot_sector = calloc(1, sizeof(struct fat_boot_sector));
	if(boot_sector == NULL) return -1;
	fill_boot_sector(boot_sector);
	struct fat_volume_info* volume_info = calloc(1, sizeof(struct fat_volume_info));
	if(volume_info == NULL) return -1;
	fill_volume_info(volume_info);
	int rc = write(fat_file, boot_sector, TAMANHO_SETOR);
	if(rc != 512) return -1;
	rc = write(fat_file, volume_info, TAMANHO_SETOR);
	if(rc != 512) return -1;
	rc = lseek(fat_file, 6 * TAMANHO_SETOR, SEEK_SET);
	if(rc != 7 * TAMANHO_SETOR) return -1;
	rc = write(fat_file, boot_sector, TAMANHO_SETOR);
	if(rc != 512) return -1;
	rc = write(fat_file, volume_info, TAMANHO_SETOR);
	if(rc != 512) return -1;
	//TODO: Escrever duas cópias consecutivas da fat no arquivo
	allocation_table = calloc(1, (sizeof(uint32_t) * NUM_CLUSTERS) + 2);
	if(allocation_table == NULL) return -1;	
	fill_allocation_table(allocation_table);
	rc = write(fat_file, )
	

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
	boot_sector->setores_32 = htole32(NUM_CLUSTERS * TAMANHO_CLUSTER); //Tamanho filesystem = 65MiB
	boot_sector->tamanho_fat_32 = htole32(520);
	boot_sector->flags = htole16(0); //setores por FAT
	memset(boot_sector->versao, '\x00', (size_t)2);
	boot_sector->root_cluster = htole32(2); //Primeiro cluster, funciona como um root directory, segundo meus cálculos 525
	boot_sector->setor_info = htole16(1); //FSInfo setor 1
	boot_sector->backup_setor_boot = htole16(6); //setor em que se encontra uma cópia do setor boot
	memset(boot_sector->reservado, '\x00', (size_t)6);
	boot_sector->id_drive = 0x80; 
	boot_sector->boot_flags = 0x00; 
	boot_sector->assinatura_boot_extend = 0x00;
	memcpy(boot_sector->tipo_fs, "FAT32   ", strlen("FAT32   "));
	memcpy(boot_sector->boot_code, boot_code, strlen(boot_code));
	boot_sector->boot_sign = htole16(0xAA55);
}

static void fill_volume_info(struct fat_volume_info* volume_info){
	volume_info->volume_info_siginicial = htole32(0x41615252);
	memset(volume_info->reservado1, '\x00', (size_t)480);
	volume_info->volume_info_sigcomp = htole32(0x61417272);
	//volume_info->clusters_livres PRECISO CALCULAR, levando em conta FAT e BPB
	//volume_info->proximo_cluster_livre PRECISO CALCULAR, levando em conta FAT e BPB
	memset(volume_info->reservado2, '\x00', (size_t)12);
	volume_info->volume_info_sigfinal = htole32(0xaa550000);
}

static void fill_allocation_table(uint32_t *allocation_table){
	for(int i = 0; i < NUM_CLUSTERS + 2; i++){
		switch(i){
			//Entradas na FAT reservadas no padrão
			case 0:
			{ 
				allocation_table[i] = htole32(0x0ffffff8);
				break;
			}
			case 1:
			{
				allocation_table[i] = htole32(0x0c000000);
				break;
			}
			default:
			{
				//Entradas FAT ocupadas pela BPB e pela própria FAT
				if(i < 525){
					if(i == 525){
						allocation_table[i] htole32(0xffffffff);	
					}else{
						allocation_table[i] = htole32(i + 1);
					}
				}else{
				//Entradas livres
					allocation_table[i] = htole32(0x00000000);
				}
				break;
			}
		}
	}
}
