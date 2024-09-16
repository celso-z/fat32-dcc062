#include "file_manager.h"
#include "fat.h"

#include <ncurses.h>


static WINDOW *create_newwin(int height, int width, int starty, int startx);
static void destroy_win(WINDOW *local_win);
static struct file_struct *display_directory(WINDOW *win, struct fat_struct *fat,  struct file_struct *dir, char *fat_filename);

int loop(struct fat_struct *fat, struct file_struct *root, char *fat_filename)
{	WINDOW *my_win;
	int ch;

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);	

	refresh();
	my_win = create_newwin(24, 80, 0, 0);
	struct file_struct *sub = display_directory(my_win, fat, root, fat_filename);

	while((ch = getch()) != KEY_F(1))
	{	switch(ch)
		{	
			case KEY_UP:
				//destroy_win(my_win);
				//my_win = create_newwin(height, width, --starty,startx);
				break;
			case KEY_DOWN:
				//destroy_win(my_win);
				//my_win = create_newwin(height, width, ++starty,startx);
				break;
		}
	}

	endwin();		
	return 0;
}

static WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);
					 
					
	wrefresh(local_win);

	return local_win;
}

static void destroy_win(WINDOW *local_win)
{
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wrefresh(local_win);
	delwin(local_win);
}

static struct file_struct *display_directory(WINDOW *win, struct fat_struct *fat,  struct file_struct *dir, char *fat_filename){

	struct file_struct *sub_directories = fetch_data(fat, dir, fat_filename);
	return sub_directories;
}

