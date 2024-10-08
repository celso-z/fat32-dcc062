#ifndef FAT_H
#define FAT_H

#include <inttypes.h>

#define TAMANHO_SETOR 512
#define TAMANHO_CLUSTER 2
#define NUM_CLUSTERS 66560
#define FAT_FILENAME "fat.fat"

#define FAT_FREE_CLUSTER 0x00000000
#define FAT_BAD_CLUSTER 0x0FFFFFF7
#define FAT_EOF_CLUSTER 0xFFFFFFFF

struct fat_volume_info {
	uint32_t volume_info_siginicial; /* Assinatura dessa estrutura, DEVE SER 0x41615252 */
	uint8_t reservado1[480]; /* Reservado, DEVE SER 0; */
	uint32_t volume_info_sigcomp; /* Assinatura complementar para estrutura, DEVE SER 0x61417272*/
	uint32_t clusters_livres; /* Soma de todos os clusters livres no volume 
								0xFFFFFFFF significa que essa soma não é conhecida*/
	uint32_t proximo_cluster_livre; /* Número do próximo cluster livre */
	uint8_t reservado2[12]; /* Reservado, deve ser 0; */
	uint32_t volume_info_sigfinal; /* Assinatura de integridade do fim da estrutura,
									  DEVE SER 0xaa550000 */
} __attribute__ ((packed));

struct fat_boot_sector {
    uint8_t boot_jump[3];	/* Instrução Jump para a localização do bootcode */
    uint8_t nome_oem[8];	/* Nome OEM, pode ser setado para qualquer valor */
    uint16_t tamanho_setor;	/* Tamanho (em bytes) de cada setor 
   									(valores válidos = 512, 1024, 2048 e 4096)	*/
    uint8_t tamanho_cluster;	/* Tamanho da unidade de alocação (cluster) (em setores) 
   									(valores válidos = 1, 2, 4, 8, 16, 32, 64 e 128)	*/
    uint16_t setores_reservados;		/* Quantidade de setores reservados até o inicio dos dados */
    uint8_t num_fats;		/* quantidade de FATs */
    uint16_t dir_entries;	/* DEVE SER SETADO PARA 0, pois em FAT32 as entradas de diretórios ficam em cluster chains*/
    uint16_t setores_16;		/* DEVE SER SETADO PARA 0 */
    uint8_t tipo_midia;		/* Tipo de mídia (0xF8 para disco não removível, 0xF0 caso contrário) */
    uint16_t tamanho_fat_16;	/* DEVE SER SETADO PARA 0 */
    uint16_t setores_trilha;	/* Setores por trilha, utilizado para endereçamento utilizando
   									CHS no real-mode da bios (IGNORE)	*/
    uint16_t cabecas_rw;		/* Cabeças de leitura e escrita por disco, utilizado para endereçamento 
   									CHS no real-mode da bios (IGNORE)	*/
    uint32_t setores_ocultos;		/* DEVE SER SETADO PARA 0 */
    uint32_t setores_32;	/* Quantidade de setores total no disco */
	uint32_t tamanho_fat_32;	/* Tamanho da FAT (em setores) */
	uint16_t flags;		/* bit 8: fat mirroring, low 4: Caso mirroring esteja desativado
						  indica o número da fat ativa */
	uint8_t versao[2];		/* Versão do filesystem major, minor */
	uint32_t root_cluster;	/* Primeiro cluster válido do diretório root */
	uint16_t setor_info;	/* Setor em que está localizada a infraestrutura FSinfo */
	uint16_t backup_setor_boot;	/* Localização do backup do setor boot, 0 caso não tenha*/
	uint16_t reservado[6];	/* DEVE SER SETADO PARA 0 */
    uint8_t id_drive;	/* Número do driver para BIOS (0x80 ou 0x00) */
    uint8_t boot_flags;		/* bit 0: dirty, bit 1: need surface test */
    uint8_t assinatura_boot_extend;	/* 0x29 caso os próximos dois campos sejam diferentes de zero */
    uint8_t volume_id[4];	/* ID Volume */
    uint8_t volume_nome[11];	/* nome volume */
    uint8_t tipo_fs[8];		/* Deve ser = "FAT32   " */
	uint8_t boot_code[420]; /* Boot code */
    uint16_t boot_sign;     /* Deve ser = 0x55aa */
} __attribute__ ((packed));

struct fat_struct{
	struct fat_boot_sector *bpb;
	struct fat_volume_info *fs_info;
	uint32_t *fat_allocation_table;
};


struct fat_struct *init(char *fat_filename);
int write_fat(struct fat_struct *fat_struct, char *fat_filename);
uint32_t allocate_cluster(struct fat_struct *fat_struct, uint8_t n, char *fat_filename);
void disallocate_cluster(struct fat_struct *fat_struct, uint32_t first_cluster, char *fat_filename);
int write_cluster(uint32_t cluster_number, struct fat_struct *fat_struct, char op, char *fat_filename, void *data);
void *read_cluster(uint32_t cluster_number, struct fat_struct *fat_struct, char *fat_filename);

#endif 
