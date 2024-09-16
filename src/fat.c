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
    /* write_msg: QUEBRADO: TODO: ESCREVER MENSAGEM NA TELA CALL DE SISTEMA*/
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
static void fill_allocation_table(uint32_t *allocation_table);

//Inicializa e salva a FAT em um arquivo de nome definido pela macro FAT_FILENAME
struct fat_struct *init(char *fat_filename){
	struct fat_boot_sector* boot_sector = calloc(1, sizeof(struct fat_boot_sector));
	if(boot_sector == NULL) return NULL;
	fill_boot_sector(boot_sector);
	struct fat_volume_info* volume_info = calloc(1, sizeof(struct fat_volume_info));
	if(volume_info == NULL) return NULL;
	fill_volume_info(volume_info);
	uint32_t *allocation_table = calloc(1, (sizeof(uint32_t) * NUM_CLUSTERS) + 2);
	if(allocation_table == NULL) return NULL;	
	fill_allocation_table(allocation_table);
	struct fat_struct *f = malloc(sizeof(struct fat_struct));
	f->bpb = boot_sector;
	f->fs_info = volume_info;
	f->fat_allocation_table = allocation_table;
	int rc = write_fat(f, fat_filename);
	if(rc == -1) return NULL;
	return f;
}

//Salva a FAT para o arquivo
int write_fat(struct fat_struct *fat_struct, char *fat_filename){
	int fat_file;
	if(strlen(fat_filename) > 0) fat_file = open(fat_filename, O_RDWR | O_CREAT, 0755); 
	else fat_file = open(FAT_FILENAME, O_RDWR | O_CREAT, 0755); 
	if(fat_file < 0) { // TODO: abrir uma fat existente com O_EXCL
		return -1;
	}
	int rc = ftruncate(fat_file, TAMANHO_CLUSTER * NUM_CLUSTERS * TAMANHO_SETOR);
	if(rc != 0) return -1;
	rc = write(fat_file, fat_struct->bpb, TAMANHO_SETOR);
	if(rc != 512) return -1;
	rc = write(fat_file, fat_struct->fs_info, TAMANHO_SETOR);
	if(rc != 512) return -1;
	rc = lseek(fat_file, 6 * TAMANHO_SETOR, SEEK_SET);
	if(rc != (6 * TAMANHO_SETOR)) return -1;
	rc = write(fat_file, fat_struct->bpb, TAMANHO_SETOR);
	if(rc != 512) return -1;
	rc = write(fat_file, fat_struct->fs_info, TAMANHO_SETOR);
	if(rc != 512) return -1;
	rc = write(fat_file, fat_struct->fat_allocation_table, (sizeof(uint32_t) * NUM_CLUSTERS) + 2);
	if(rc != (sizeof(uint32_t) * NUM_CLUSTERS) + 2) return -1;
	rc = write(fat_file, fat_struct->fat_allocation_table, (sizeof(uint32_t) * NUM_CLUSTERS) + 2);
	if(rc != (sizeof(uint32_t) * NUM_CLUSTERS) + 2) return -1;
	rc = close(fat_file);
	if(rc != 0) return -1;	
	return 0;

}

static void fill_boot_sector(struct fat_boot_sector* boot_sector){
	memcpy((char *)boot_sector->boot_jump, "\xeb\x5a\x90", strlen("\xeb\x5a\x90") ); //Três bytes da instrução de branch incondicional (jmp) em assembly x86
	memcpy((char *)boot_sector->nome_oem, "FAT_MOD", strlen("FAT_MOD"));
	boot_sector->tamanho_setor = htole16(TAMANHO_SETOR);
	boot_sector->tamanho_cluster = TAMANHO_CLUSTER;
	boot_sector->setores_reservados = htole16(1048);//TODO: PROVAVELMENTE 524 * 2
	boot_sector->num_fats = 2;
	boot_sector->dir_entries = htole16(0);
	boot_sector->setores_16 = htole16(0);
	boot_sector->tipo_midia = 0xf8;
	boot_sector->tamanho_fat_16 = htole16(0);
	boot_sector->setores_trilha = htole16(0);
	boot_sector->cabecas_rw = htole16(0);
	boot_sector->setores_ocultos = htole32(0);
	boot_sector->setores_32 = htole32(NUM_CLUSTERS * TAMANHO_CLUSTER); //Tamanho filesystem = 65MiB
	boot_sector->tamanho_fat_32 = htole32(520); //setores por FAT
	boot_sector->flags = htole16(0); 
	memset(boot_sector->versao, '\x00', (size_t)2);
	boot_sector->root_cluster = htole32(525); //Primeiro cluster, funciona como um root directory, segundo meus cálculos 525
	boot_sector->setor_info = htole16(1); //FSInfo setor 1
	boot_sector->backup_setor_boot = htole16(6); //setor em que se encontra uma cópia do setor boot
	memset(boot_sector->reservado, '\x00', (size_t)6);
	boot_sector->id_drive = 0x80; 
	boot_sector->boot_flags = 0x00; 
	boot_sector->assinatura_boot_extend = 0x00;
	memcpy(boot_sector->tipo_fs, "FAT32   ", strlen("FAT32   "));
	boot_code[419] = 0;
	memcpy(boot_sector->boot_code, boot_code, 420);
	boot_sector->boot_sign = htole16(0xAA55);
}

static void fill_volume_info(struct fat_volume_info* volume_info){
	volume_info->volume_info_siginicial = htole32(0x41615252);
	memset(volume_info->reservado1, '\x00', (size_t)480);
	volume_info->volume_info_sigcomp = htole32(0x61417272);
	volume_info->clusters_livres = htole32(NUM_CLUSTERS - 526);//PRECISO CALCULAR, levando em conta FAT e BPB
	volume_info->proximo_cluster_livre = htole32(526); //PRECISO CALCULAR, levando em conta FAT e BPB
	memset(volume_info->reservado2, '\x00', (size_t)12);
	volume_info->volume_info_sigfinal = htole32(0xaa550000);
}

static void fill_allocation_table(uint32_t *allocation_table){
	for(unsigned int i = 0; i < 66562; i++){
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
					if(i == 524){
						allocation_table[i] = htole32(FAT_EOF_CLUSTER);	
					}else{
						allocation_table[i] = htole32(i + 1);
					}
				}else{
					if(i == 525){
						//Primeiro cluster Livre (Serve como root directory)
						allocation_table[i] = htole32(FAT_EOF_CLUSTER);	
					}else{
						//Entradas livres
						allocation_table[i] = htole32(FAT_FREE_CLUSTER);
					}
				}
				break;
			}
		}
	}
}

//Alocar n clusters de uma fat
uint32_t allocate_cluster(struct fat_struct *fat_struct, uint8_t n, char *fat_filename){
	if(fat_struct->fs_info->clusters_livres < n) return UINT32_MAX;
	uint32_t first_cluster = fat_struct->fs_info->proximo_cluster_livre; 
	for(int i = 0; i < n; i++){
		if(i == n - 1) fat_struct->fat_allocation_table[first_cluster + i] = htole32(FAT_EOF_CLUSTER);
		else fat_struct->fat_allocation_table[first_cluster + i] = htole32(first_cluster + i + 1);
	}
	fat_struct->fs_info->clusters_livres = htole32(fat_struct->fs_info->clusters_livres - n);
	fat_struct->fs_info->proximo_cluster_livre = htole32(fat_struct->fs_info->proximo_cluster_livre + n);
	int rc = write_cluster(first_cluster, fat_struct, 'w', fat_filename, NULL); //Write EOF to first cluster
	return first_cluster;
}

//Escrever dados em um dado cluster em uma fat, que se localiza em um determinado arquivo, o argumento op significa a operação "a" para extender o cluster e "w" para reescrever com o cursor no início do cluster
int write_cluster(uint32_t cluster_number, struct fat_struct *fat_struct, char op, char *fat_filename, void *data){
	if(cluster_number == UINT32_MAX || cluster_number < 525) return -1;
	if(fat_struct == NULL) return -1;
	if(op != 'a' && op != 'w') return -2;
	if(fat_struct->fat_allocation_table[cluster_number] == FAT_FREE_CLUSTER) return -3;

	int fat_file = open(fat_filename, O_RDWR | O_CREAT, 0755); 
	if(fat_file < 0) { 
		return -1;
	}
	int rc = 0;
	rc = lseek(fat_file, cluster_number * (TAMANHO_CLUSTER * TAMANHO_SETOR), SEEK_SET);
	if(rc != cluster_number * (TAMANHO_CLUSTER * TAMANHO_SETOR)){
		close(fat_file);
		return -1;
	}
	if(data != NULL) {
		rc = write(fat_file, data, sizeof(data));
		if(rc != sizeof(data)) return -1;
	}
	rc = write(fat_file, "\xff\xff\xff\xff", 4);
	if(rc != 1) return -1;
	close(fat_file);
	return 0;
}

//Ler dados de um cluster em uma fat, que se localiza em um determinado arquivo
void *read_cluster(uint32_t cluster_number, struct fat_struct *fat_struct, char *fat_filename){
	if(cluster_number == UINT32_MAX) return NULL;
	if(fat_struct == NULL) return NULL;
	if(fat_struct->fat_allocation_table[cluster_number] == FAT_FREE_CLUSTER) return NULL;
	int fat_file = open(fat_filename, O_RDWR | O_CREAT, 0755); 
	if(fat_file < 0) { 
		return NULL;
	}
	int rc = 0;
	rc = lseek(fat_file, cluster_number * (TAMANHO_CLUSTER * TAMANHO_SETOR), SEEK_SET);
	if(rc != cluster_number * (TAMANHO_CLUSTER * TAMANHO_SETOR)){
		close(fat_file);
		return NULL;
	}
	void *cluster_data = malloc(TAMANHO_CLUSTER * TAMANHO_SETOR);
	if(cluster_data == NULL) return NULL;
	rc = read(fat_file, cluster_data, TAMANHO_CLUSTER * TAMANHO_SETOR);
	close(fat_file);
	return cluster_data;
}
