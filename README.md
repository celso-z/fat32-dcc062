# fat32-dcc062
Implementação filesystem FAT 32 - Trabalho prático DCC062
Implementamos a fat e para sua manipulação criamos um pequeno gerenciador de arquivos CLI no estilo Midnight Commander

##Utilização
apenas rode o executável e estará no diretório root do sistema de arquivos, para verificar como utilizar o mesmo digite a tecla 'h'
Após o fim da utilização digite F1 e poderá verificar o estado da FAT no arquivo f.fat
**DELETE ESTE ARQUIVO ANTES DE EXECUTAR NOVAMENTE**

##build
gcc -Wall fat.h fat.c file.h file.c file_manager.h file_manager.c main.c -o fat -lncurses

##Participantes
- Celso Zacarias
- Enzo Faceroli
- Joao Victor
- Daniel Queiroz
- Davi Kirchmaier
