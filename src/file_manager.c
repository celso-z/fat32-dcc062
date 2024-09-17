#include "file_manager.h"
#include "fat.h"
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include <ncurses.h>


static WINDOW *create_newwin(int height, int width, int starty, int startx);
static void destroy_win(WINDOW *local_win);
static struct file_struct **display_directory(WINDOW *win, struct fat_struct *fat,  struct file_struct *dir, char *fat_filename, char *selected_line);
static void display_help(WINDOW *win);
static void display_error(WINDOW *win, const char *str);
static void create_directory(WINDOW *win, struct file_struct *parent, struct fat_struct *fat, char *fat_filename);
static void create_file(WINDOW *win, struct file_struct *parent, struct fat_struct *fat, char *fat_filename);
static void edit_mode(WINDOW *win, struct file_struct *file, struct fat_struct *fat, char *fat_filename);
static struct file_struct *select_directory(struct file_struct **directories, char selected_line);

int loop(struct fat_struct *fat, struct file_struct *root, char *fat_filename)
{	WINDOW *my_win;
	int ch;

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);	

	refresh();
	my_win = create_newwin(24, 80, 0, 0);
	start_color();
	char selected_line = 0;
	struct file_struct **sub = display_directory(my_win, fat, root, fat_filename, &selected_line);

	while((ch = getch()) != KEY_F(1)){	
		switch(ch){	
			case KEY_UP:
				if(selected_line > 0) selected_line--;
				//destroy_win(my_win);
				//my_win = create_newwin(height, width, --starty,startx);
				break;
			case KEY_DOWN:
				if(selected_line < 78) selected_line++;
				//destroy_win(my_win);
				//my_win = create_newwin(height, width, ++starty,startx);
				break;
			case 'd':
				create_directory(my_win, root, fat, fat_filename);
				break;
			case 'D':
				struct file_struct *selected = select_directory(sub, selected_line);
				int rc = delete_file_struct(selected, root, fat, fat_filename);
				if(rc == -1) display_error(my_win, "Diretorio precisa estar vazio");
				break;
			case 'n':
				create_file(my_win, root, fat, fat_filename);
				break;
			case 'E':
				struct file_struct *selected_file = select_directory(sub, selected_line);
				if(!(selected_file->DIR_attr & ATTR_DIRECTORY)) edit_mode(my_win, selected_file, fat, fat_filename);
				break;
			case 'h':
				display_help(my_win);
				break;
			case 10:
				struct file_struct *selected_struct = select_directory(sub, selected_line);
				if(selected_struct->DIR_attr & ATTR_DIRECTORY) root = selected_struct;
				//destroy_win(my_win);
				//my_win = create_newwin(height, width, ++starty,startx);
				break;
		}
		sub = display_directory(my_win, fat, root, fat_filename, &selected_line);
	}

	endwin();		
	return 0;
}

static WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);
	mvprintw(height-1, 1,"%.19s", "Press h for (h)elp.");
					
	wrefresh(local_win);

	return local_win;
}

static void destroy_win(WINDOW *local_win)
{
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wclear(local_win);
	wrefresh(local_win);
	delwin(local_win);
}

static struct file_struct **display_directory(WINDOW *win, struct fat_struct *fat,  struct file_struct *dir, char *fat_filename, char *selected_line){

	int i = 0;
	struct file_struct **sub_directories = fetch_data(fat, dir, fat_filename);
	for(; i < INT_MAX; i++){
		if(memcmp(sub_directories[0][i].DIR_name, "\000\000\000\000\000\000\000\000\000\000\000", 11) == 0) break;	
		if(i == *selected_line){
			init_pair(1, COLOR_BLACK, COLOR_WHITE);
			attron(COLOR_PAIR(1) | A_UNDERLINE);
			mvprintw(i + 1, 1,"%.11s", sub_directories[0][i].DIR_name);
			attroff(COLOR_PAIR(1) | A_UNDERLINE);
			//mvwchgat(win, i + 1, 1, 11,A_BLINK,1,NULL);
		} 
		else mvprintw(i + 1, 1,"%.11s", sub_directories[0][i].DIR_name);
	}
	if(*selected_line > i - 1) *selected_line = i - 1;
	box(win, 0 , 0);
	mvprintw(23, 1,"%.19s", "Press h for (h)elp.");
					
	wrefresh(win);
	refresh();
	return sub_directories;
}

static void display_help(WINDOW *win){
	WINDOW *help_window = newwin(10, 30, 7, 15);
	box(help_window, 0 , 0);
	mvwprintw(help_window,0, 0,"%s", "Help");
	mvwprintw(help_window,1, 1,"%s", "d - Criar novo diretorio");
	mvwprintw(help_window,1, 1,"%s", "ENTER - Adentrar diretorio");
	mvwprintw(help_window,1, 1,"%s", "d - Deletar arquivo/diretorio");
	mvwprintw(help_window,1, 1,"%s", "n - Criar novo arquivo");
	mvwprintw(help_window,1, 1,"%s", "E - Editar arquivo");
	wrefresh(help_window);
	refresh();
	getch();
	destroy_win(help_window);
	return;
}

static void display_error(WINDOW *win, const char *str){
	WINDOW *help_window = newwin(10, 30, 7, 15);
	box(help_window, 0 , 0);
	mvwprintw(help_window,0, 0,"%s", "Erro!");
	mvwprintw(help_window,1, 1,"%s", str);
	wrefresh(help_window);
	refresh();
	getch();
	destroy_win(help_window);
	return;
}

static void create_directory(WINDOW *win, struct file_struct *parent, struct fat_struct *fat, char *fat_filename){
	char *dir_name = malloc(sizeof(char) * 12);
	WINDOW *prompt = newwin(3, 30, 7, 15);
	box(prompt, 0 , 0);
	echo();
	mvwprintw(prompt,0, 0,"%s", "Digite o nome: MAX: 11 letras");
	mvwscanw(prompt, 1, 1, "%11s", dir_name);
	dir_name[11] = '\0'; //cstring compliant para usar strlen 
	new_directory(dir_name, ' ', parent, fat, fat_filename);
	noecho();
	wrefresh(prompt);
	refresh();
	free(dir_name);
	destroy_win(prompt);
	return;

}

static void create_file(WINDOW *win, struct file_struct *parent, struct fat_struct *fat, char *fat_filename){
	char *file_name = malloc(sizeof(char) * 12);
	WINDOW *prompt = newwin(3, 30, 7, 15);
	box(prompt, 0 , 0);
	echo();
	mvwprintw(prompt,0, 0,"%s", "Digite o nome: MAX: 11 letras");
	mvwscanw(prompt, 1, 1, "%11s", file_name);
	file_name[11] = '\0'; //cstring compliant para usar strlen 
	new_file(file_name, ' ', parent, fat, fat_filename);
	noecho();
	wrefresh(prompt);
	refresh();
	free(file_name);
	destroy_win(prompt);
	return;

}

static struct file_struct *select_directory(struct file_struct **directories, char selected_line){
	return &(directories[0][selected_line]);
}

static void edit_mode(WINDOW *win, struct file_struct *file, struct fat_struct *fat, char *fat_filename){
	char ch = 0;
	char buffer[1024];
	int32_t first_cluster = 0;
    first_cluster = first_cluster | file->DIR_fstClusHI;
    first_cluster = first_cluster << 16;
    first_cluster =  first_cluster | file->DIR_fstClusLO;
	int i = 0;
	WINDOW *prompt = newwin(24, 80, 0, 0);
	box(prompt, 0 , 0);
	mvwprintw(prompt,0, 1,"%.11s", file->DIR_name);
	mvwprintw(prompt,23, 1,"%s", "F1 para salvar");
	wrefresh(prompt);
	refresh();
	//echo();
	move(1,1);
	while((ch = getch()) != KEY_F(2)){	
		addch(ch);				
	}
	memcpy(buffer + i, "\x00\x00\x00\x00", 4);	
	write_data(fat, first_cluster, buffer, 1024, fat_filename);
	destroy_win(prompt);
	refresh();
	wrefresh(win);
	//echo();
	noecho();
	return;

}
