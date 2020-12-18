#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "constant.c"
#include <stdbool.h>
// Structures

typedef struct
{
    GtkWidget *textview_main; // Pointer to text view object
    GtkWidget *test;
    GtkTextBuffer *textbuffer_main; // Pointer to text buffer
    GtkEntry *textentry_main;       // Pointer to text entry
} app_widgets;

app_widgets *widgets;
// Methods

void add_text(char *dest, const char *src);
void set_username();
void get_username(char *user);
void on_window_main_destroy();
void on_textentry_main_key_release_event(GtkWidget *widget, GdkEvent *event, app_widgets *app_wdgts);
void reset_entry();
void print_console(char *text);
void print_console_color(char *text, char *color);
void print_username();

// Global variables

int isUserCreated = false;
char userName[64];

//FILE SYSTEM

int debug = 0; // extra output; 1 = on, 0 = off

int do_root(char *name, char *size);
int do_print(char *name, char *size);
int do_chdir(char *name, char *size);
int do_mkdir(char *name, char *size);
int do_rmdir(char *name, char *size);
int do_mvdir(char *name, char *size);
int do_mkfil(char *name, char *size);
int do_rmfil(char *name, char *size);
int do_mvfil(char *name, char *size);
int do_szfil(char *name, char *size);
int do_exit(char *name, char *size);

struct action
{
    char *cmd;                             // pointer to string
    int (*action)(char *name, char *size); // pointer to function
} table[] = {
    {"root", do_root},
    {"print", do_print},
    {"chdir", do_chdir},
    {"mkdir", do_mkdir},
    {"rmdir", do_rmdir},
    {"mvdir", do_mvdir},
    {"mkfil", do_mkfil},
    {"rmfil", do_rmfil},
    {"mvfil", do_mvfil},
    {"szfil", do_szfil},
    {"exit", do_exit},
    {NULL, NULL} // end mark, do not remove ,gives wierd errors! :(
};

/*--------------------------------------------------------------------------------*/
void printing(char *name);
void print_descriptor();
void parse(char *buf, int *argc, char *argv[]);
int allocate_block(char *name, bool directory);
void unallocate_block(int offset);
int find_block(char *name, bool directory);

int add_descriptor(char *name);
int edit_descriptor(int free_index, bool free, int name_index, char *name);
int edit_descriptor_name(int index, char *new_name);
int add_directory(char *name);
int remove_directory(char *name);
int rename_directory(char *name, char *new_name);
int edit_directory(char *name, char *subitem_name, char *new_name, bool name_change, bool directory);
int add_file(char *name, int size);
int edit_file(char *name, int size, char *new_name);
int remove_file(char *name);
int edit_directory_subitem(char *name, char *sub_name, char *new_sub_name);

void print_directory(char *name);
char *get_directory_name(char *name);
char *get_directory_top_level(char *name);
char *get_directory_subitem(char *name, int subitem_index, char *subitem);
int get_directory_subitem_count(char *name);

char *get_file_name(char *name);
char *get_file_top_level(char *name);
int get_file_size(char *name);
void print_file(char *name);

/************************** Defining Constants for fs *******************/
#define LINESIZE 128
#define DISK_PARTITION 4000000
#define BLOCK_SIZE 5000
#define BLOCKS 4000000 / 5000
#define MAX_STRING_LENGTH 20
#define MAX_FILE_DATA_BLOCKS (BLOCK_SIZE - 64 * 59) //Hard-coded as of now
#define MAX_SUBDIRECTORIES (BLOCK_SIZE - 136) / MAX_STRING_LENGTH

char *cmd, *fnm, *fsz;
char dummy[] = "";
int n;
char *a[LINESIZE];

typedef struct
{
    char directory[MAX_STRING_LENGTH];
    int directory_index;
    char parent[MAX_STRING_LENGTH];
    int parent_index;
} working_directory;

typedef struct dir_type
{
    char name[MAX_STRING_LENGTH];      //Name of file or dir
    char top_level[MAX_STRING_LENGTH]; //Name of directory one level up(immediate parent)
    char (*subitem)[MAX_STRING_LENGTH];
    bool subitem_type[MAX_SUBDIRECTORIES]; //true if directory, false if file
    int subitem_count;
    struct dir_type *next;
} dir_type;

typedef struct file_type
{
    char name[MAX_STRING_LENGTH];      //Name of file or dir
    char top_level[MAX_STRING_LENGTH]; //Name of directory one level up
    int data_block_index[MAX_FILE_DATA_BLOCKS];
    int data_block_count;
    int size;
    struct file_type *next;
} file_type;

typedef struct
{
    bool free[BLOCKS];
    bool directory[BLOCKS];
    char (*name)[MAX_STRING_LENGTH];
} descriptor_block;

char *disk;
working_directory current;
bool disk_allocated = false; // makes sure that do_root is first thing being called and only called once

/*--------------------------------------------------------------------------------*/
