#include "main.h"

int main(int argc, char *argv[])
{
    int err;
    char disk_img[] = "disk.img";

    GtkBuilder *builder;
    GtkWidget *window;

    // Creation / Initialization of disk file
    if(access(disk_img, F_OK) == -1 || (argc == 2 && atoi(argv[1]) == 1)) {
        err = init_fresh_disk(disk_img, BLOCK_SIZE, BLOCKS);
        disk_allocated = false;
    } else {
        err = init_disk(disk_img, BLOCK_SIZE, BLOCKS);
        disk_allocated = true;
    }

    if (err != 0) {
        printf("ERROR: There is a problem with the disk init.\n");
        exit(1);
    }

    // Init username

    widgets = g_slice_new(app_widgets);

    gtk_init(&argc, &argv);

    builder = gtk_builder_new_from_file("glade/window_main.glade");
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));

    // Get pointers to widgets
    widgets->textview_main = GTK_WIDGET(gtk_builder_get_object(builder, "textview_main"));
    widgets->textbuffer_main = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "textbuffer_main"));
    widgets->textentry_main = GTK_ENTRY(gtk_builder_get_object(builder, "textentry_main"));
    widgets->test = GTK_WIDGET(gtk_builder_get_object(builder, "test"));

    // Color tags
    gtk_text_buffer_create_tag(widgets->textbuffer_main, "dir_fg",
                               "foreground", "#a042e5", NULL);
    gtk_text_buffer_create_tag(widgets->textbuffer_main, "fil_fg",
                               "foreground", "#00ffa1", NULL);
    gtk_text_buffer_create_tag(widgets->textbuffer_main, "user_fg",
                               "foreground", "#00a6ff", NULL);

    gtk_builder_connect_signals(builder, widgets);
    set_username(widgets);

    g_object_unref(builder);

    print_username();

    gtk_widget_show(window);
    gtk_main();

    g_slice_free(app_widgets, widgets);

    return 0;
}

void add_text(char *dest, const char *src)
{
    char command[256];
    strcpy(command, dest);
    strcat(command, src);
    strcpy(dest, command);
}

// set the user and host name
void set_username()
{
    char *user, hostname[128];
    user = (char *)malloc(10 * sizeof(char));
    hostname[127] = '\0';
    user = getlogin();
    gethostname(hostname, 127);
    strcpy(username_c, user);
    printf("%s\n", user);
    strcat(username_c, "@");
    strcat(username_c, hostname);
    strcat(username_c, ": ");
}

void get_username(char *user)
{
    char line[64];
    FILE *fp;
    if ((fp = fopen("src/users.txt", "r")) == NULL)
    {
        printf("Error! opening file");
        exit(1);
    }

    // reads text until newline is encountered
    fgets(line, 63, fp);
    strcpy(user, line);
    fclose(fp);
}

void print_console(char *text)
{
    gtk_text_buffer_insert_at_cursor(widgets->textbuffer_main, text, -1);
}

void print_console_color(char *text, char *color)
{
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_offset(widgets->textbuffer_main, &iter, -1);
    gtk_text_buffer_insert_with_tags_by_name(widgets->textbuffer_main, &iter,
                                             text, -1, color, NULL, NULL);
}

void print_username()
{
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_offset(widgets->textbuffer_main, &iter, -1);
    gtk_text_buffer_insert_with_tags_by_name(widgets->textbuffer_main, &iter, username_c, -1, "user_fg", NULL, NULL);
}

void reset_entry()
{
    gtk_entry_set_text(widgets->textentry_main, "");
}

// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}

// main method, to take the commands
void on_textentry_main_key_release_event(GtkWidget *widget, GdkEvent *event, app_widgets *app_wdgts)
{
    GdkEventKey eventKey;
    eventKey = event->key;
    int found = 0;
    if (eventKey.keyval == GDK_KEY_Return || eventKey.keyval == GDK_KEY_KP_Enter)
    {

        const char *txt = gtk_entry_get_text(app_wdgts->textentry_main);
        if (strcmp(txt, "") != 0)
        {
            char command[256], complete[256];
            strcpy(command, txt);
            strcpy(complete, txt);
            for (struct action *ptr = table; ptr->cmd != NULL; ptr++)
            //for (int i = 0; i < MAX_COMMANDS; i++)
            {
                char *token = strtok(command, " ");
                if (strcmp(token, ptr->cmd) == 0)
                {
                    found = 1;
                    add_text(command, "\n");
                    add_text(complete, " ");
                    print_console(complete);
                    print_console("\n");
                    // Console process
                    fileSystem(complete, ptr);
                    reset_entry();
                    print_username();
                }
            }
            if (!found)
            {
                add_text(command, " ");
                print_console(command);
                print_console(notFound);
                reset_entry();
                print_username();
            }
            else
            {
                found = 0;
            }
        }
    }
}

//FILE SYSTEM

void fileSystem(char *in, struct action *ptr)
{
    // parse input
    parse(in, &n, a);

    cmd = (n > 0) ? a[0] : dummy;
    fnm = (n > 1) ? a[1] : dummy;
    fsz = (n > 2) ? a[2] : dummy;

    int ret = (ptr->action)(fnm, fsz);
    //every function returns -1 on failure
    if (ret == -1)
    {
        print_console(cmd);
        print_console(fnm);
        print_console(fsz);
        print_console(": failed\n");
    }
}

/*--------------------------------------------------------------------------------*/

void parse(char *buf, int *argc, char *argv[])
{
    char *delim;   // points to first space delimiter
    int count = 0; // number of args

    char whsp[] = " \t\n\v\f\r"; // whitespace characters

    while (1) // build the argv list
    {
        buf += strspn(buf, whsp);   // skip leading whitespace
        delim = strpbrk(buf, whsp); // next whitespace char or NULL
        if (delim == NULL)          // end of line, input parsed
        {
            break;
        }
        argv[count++] = buf;
        *delim = '\0';
        buf = delim + 1;
    }
    argv[count] = NULL;

    *argc = count;

    return;
}

/*--------------------------------------------------------------------------------*/

//This function initializes the disk, descriptor within the disk, as well as the root directory.
int do_root(char *name, char *size)
{
    print_console("Welcome to your file system\n");
    (void)*name;
    (void)*size;

    if (disk_allocated != true) {

        //Initialize disk
        // disk = (char *)malloc(DISK_PARTITION);
        add_descriptor("descriptor");

        //Add descriptor and root directory to disk
        add_directory("root");

        printf("New disk~\n");

        //Set up the working_directory structure
        strcpy(current.directory, "root");
        current.directory_index = 3;
        strcpy(current.parent, "");
        current.parent_index = -1;
        disk_allocated = true;
    } else {
        int index = find_block("root", true);
        strcpy(current.directory, "root");
        current.directory_index = 3;
        strcpy(current.parent, "");
        current.parent_index = -1;
    }

    return 0;
}

/*--------------------------------------------------------------------------------*/

int do_print(char *name, char *size)
{
    (void)*name;
    (void)*size;
    if (disk_allocated == false)
    {
        print_console("Error: Disk not allocated\n");
        return 0;
    }
    //Start with the root directory, which is directory type (true)
    printing("root");

    return 0;
}

/*--------------------------------------------------------------------------------*/

int do_chdir(char *name, char *size)
{
    (void)*size;
    if (disk_allocated == false)
    {
        print_console("Error: Disk not allocated\n");
        return 0;
    }

    //Case if ".." is the argument to "chdir"
    if (strcmp(name, "..") == 0)
    {

        //If we are in root directory then we can't go back
        if (strcmp(current.directory, "root") == 0)
            return 0;

        //Adjust the working_directory struct
        strcpy(current.directory, current.parent);
        strcpy(current.parent, get_directory_top_level(current.parent));
        return 0;
    }
    else
    {
        char tmp[20];

        //Check to make sure it is a subdirectory that is to be changed
        //If name is not in the current directory then returns -1, else return 0
        if ((strcmp(get_directory_subitem(current.directory, -1, name), "-1") == 0) && strcmp(current.parent, name) != 0)
        {

            print_console("chdir: ");
            print_console(name);
            print_console(": No such file or directory\n");

            return 0;
        }

        strcpy(tmp, get_directory_name(name));
        if (strcmp(tmp, "") == 0)
            return 0;

        if (strcmp(tmp, name) == 0)
        {
            //Adjust the working_directory struct
            strcpy(current.directory, tmp);

            strcpy(current.parent, get_directory_top_level(name));

            return 1;
        }
        return -1;
    }
    return 0;
}

/*--------------------------------------------------------------------------------*/

int do_mkdir(char *name, char *size)
{
    (void)*size;
    if (disk_allocated == false)
    {
        print_console("Error: Disk not allocated\n");
        return 0;
    }

    //If it returns 0, there is a subitem with that name already
    if ((strcmp(get_directory_subitem(current.directory, -1, name), "0")) == 0)
    {

        print_console("mkdir: cannot create directory ");
        print_console(name);
        print_console(": Folder exists\n");

        return 0;
    }

    //Call add directory

    if (add_directory(name) != 0)
    {

        print_console("mkdir: missing operand\n");

        return 0;
    }

    //Edit the current directory to add our new directory to the current directory's "subdirectory" member.
    //NULL ==> for just editing the subdirectory
    edit_directory(current.directory, name, NULL, false, true);

    return 0;
}

/*--------------------------------------------------------------------------------*/

int do_rmdir(char *name, char *size)
{
    (void)*size;
    if (disk_allocated == false)
    {
        print_console("Error: Disk not allocated\n");
        return 0;
    }

    if (strcmp(name, "") == 0)
    {

        print_console("rmdir: missing operand\n");

        return 0;
    }

    if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0))
    {

        print_console("rmdir: ");
        print_console(name);
        print_console(": No such file or directory\n");

        return 0;
    }

    //Check to make sure it is a subdirectory that is to be changed
    //If name is not in the current directory then returns -1, else return 0
    if (strcmp(get_directory_subitem(current.directory, -1, name), "-1") == 0)
    {
        print_console("rmdir: ");
        print_console(name);
        print_console(": No such file or directory\n");
        return 0;
    }

    //Remove directory from the parent's subitems.
    dir_type *folder = malloc(BLOCK_SIZE);
    int block_index = find_block(name, true);

    // memcpy(folder, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, folder);

    if (err < 0) {
        printf("Error: can't read folder\n");
        return -1;
    }

    dir_type *top_folder = malloc(BLOCK_SIZE);

    //The top_level is created based off the folder
    int top_block_index = find_block(folder->top_level, true);

    // memcpy(disk + block_index * BLOCK_SIZE, folder, BLOCK_SIZE);
    err = write_blocks(block_index, 1, folder);

    if (err < 0) {
        printf("Error: can't write folder\n");
        return -1;
    }

    // memcpy(top_folder, disk + top_block_index * BLOCK_SIZE, BLOCK_SIZE);
    err = read_blocks(top_block_index, 1, top_folder);

    if (err < 0) {
        printf("Error: can't read top_folder\n");
        return -1;
    }

    char subitem_name[MAX_STRING_LENGTH];         // holds the current subitem in the parent directory array
    const int subcnt = top_folder->subitem_count; // no. of subitems
    int j;
    int k = 0;

    //iterate through the subitem count
    for (j = 0; j < subcnt; j++)
    {
        strcpy(subitem_name, top_folder->subitem[j]);
        if (strcmp(subitem_name, name) != 0)
        {
            strcpy(top_folder->subitem[k], subitem_name);
            //printf("------ Subitem [%s] copied ------\n", subitem_name);
            k++;
        }
    }

    //Remove the directory subitem from the parent
    strcpy(top_folder->subitem[k], "");

    top_folder->subitem_count--;

    // memcpy(disk + top_block_index * BLOCK_SIZE, top_folder, BLOCK_SIZE);
    err = write_blocks(top_block_index, 1, top_folder);

    free(top_folder);

    if (err < 0) {
        printf("Error: can't write top_folder\n");
        return -1;
    }

    //Remove the directory with its contents

    if (remove_directory(name) == -1)
    {
        return 0;
    }
    return 0;
}

/*--------------------------------------------------------------------------------*/

int do_mvdir(char *name, char *size) //"size" is actually the new name
{
    if (disk_allocated == false)
    {
        print_console("Error: Disk not allocated\n");
        return 0;
    }

    //Rename the directory

    //if the directory "name" is not found, return -1
    if (edit_directory(name, "", size, true, true) == -1)
    {

        print_console("mvdir: cannot rename file or directory");
        print_console(name);
        print_console("\n");

        return 0;
    }

    return 0;
}

/*--------------------------------------------------------------------------------*/

int do_mkfil(char *name, char *size)
{
    if (disk_allocated == false)
    {
        print_console("Error: Disk not allocated\n");
        return 0;
    }

    //If it returns 0, there is a subitem with that name already
    if (get_directory_subitem(current.directory, -1, name) == 0)
    {

        print_console("%s: cannot create file ");
        print_console(name);
        print_console(": File exists\n");

        return 0;
    }

    if (add_file(name, atoi(size)) != 0)
        return 0;

    //Edit the current directory to add our new file to the current directory's "subdirectory" member.
    edit_directory(current.directory, name, NULL, false, false);
    return 0;
}

/*--------------------------------------------------------------------------------*/

// Remove a file
int do_rmfil(char *name, char *size)
{
    if (disk_allocated == false)
    {
        print_console("Error: Disk not allocated\n");
        return 0;
    }

    (void)*size;

    //If the file to be removed actually exists in current directory, remove it
    if (get_directory_subitem(current.directory, -1, name) == 0)
    {
        remove_file(name);
        return 0;
    }
    else
    { // If it doesn't exist, print error and return 0

        print_console("rmfil: ");
        print_console(name);
        print_console(": No such file or directory\n");

        return 0;
    }
}

/*--------------------------------------------------------------------------------*/

// Rename a file
int do_mvfil(char *name, char *size)
{
    if (disk_allocated == false)
    {
        print_console("Error: Disk not allocated\n");
        return 0;
    }

    //If it returns 0, there is a subitem with that name already
    if (get_directory_subitem(current.directory, -1, size) == 0)
    {
        print_console("mvfil: cannot rename file or directory");
        print_console(name);
        print_console("\n");

        return 0;
    }

    int er = edit_file(name, 0, size);

    if (er == -1)
        return -1;

    return 0;
}

/*--------------------------------------------------------------------------------*/

// Resize a file -- addon to be implemented
int do_szfil(char *name, char *size)
{
    if (disk_allocated == false)
    {
        print_console("Error: Disk not allocated\n");
        return 0;
    }

    //remove file; make new file with updated size
    if (remove_file(name) != -1)
        do_mkfil(name, size);

    else
    {
        print_console("szfill: cannot resize ");
        print_console(name);
        print_console(": No such file or directory\n");
    }

    return 0;
}

/*--------------------------------------------------------------------------------*/

int do_exit(char *name, char *size)
{
    (void)*name;
    (void)*size;

    close_disk();
    // Destroy main window (GTK)
    gtk_main_quit();
    exit(0);
    return 0;
}

/*--------------------------------------------------------------------------------*/

/******************************* Helper Functions Start *****************************/

//Prints the information of directories and files starting at the root
void printing(char *name)
{
    //Allocate memory to a dir_type so that we can copy the folder from memory into this variable.
    dir_type *folder = malloc(BLOCK_SIZE);
    int block_index = find_block(name, true);

    // memcpy(folder, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, folder);

    if (err < 0) {
        printf("Error: can't read descriptor\n");
        return;
    }

    printf("No poblem finding the block\n");

    if (strcmp(current.directory, folder->name) == 0)
    {
        print_console_color("> ", "fil_fg");
    }
    print_console(folder->name);
    print_console(":\n");
    //printf("%s:\n", folder->name);
    for (int i = 0; i < folder->subitem_count; i++)
    {
        print_console("\t");
        if (folder->subitem_type[i])
        {
            print_console_color(folder->subitem[i], "dir_fg");
        }
        else
        {
            print_console_color(folder->subitem[i], "fil_fg");
        }

        print_console("\n");
        //printf("\t%s\n", folder->subitem[i]);
    }

    //Go through again if there is a subdirectory
    for (int i = 0; i < folder->subitem_count; i++)
    {
        if (folder->subitem_type[i] == true)
        {
            //Recursively call the function! can be improved further ;)
            printing(folder->subitem[i]);
        }
    }
}

/*--------------------------------------------------------------------------------*/

//Displays the content of the descriptor block and free block table.
void print_descriptor()
{
    descriptor_block *descriptor = malloc(BLOCK_SIZE * 2);

    // memcpy(descriptor, disk, BLOCK_SIZE * 2);
    int err = read_blocks(0, 2, descriptor);

    if (err < 0) {
        printf("Error: can't read descriptor\n");
        return;
    }

    printf("Disk Descriptor Free Table:\n");

    for (int i = 0; i < BLOCKS; i++)
    {
        printf("\tIndex %d : %d\n", i, descriptor->free[i]);
    }

    free(descriptor);
}

/*--------------------------------------------------------------------------------*/

//finds the first free block on the disk; the free block index is returned
int allocate_block(char *name, bool directory)
{

    descriptor_block *descriptor = malloc(BLOCK_SIZE * 2);

    // memcpy(descriptor, disk, BLOCK_SIZE * 2);
    int err = read_blocks(0, 2, descriptor);

    if (err < 0) {
        printf("Error: can't read descriptor\n");
        return -1;
    }

    //Goes through every block until free one is found

    for (int i = 0; i < BLOCKS; i++)
    {
        if (descriptor->free[i])
        {
            //Once free block is found, update descriptor information
            descriptor->free[i] = false;
            descriptor->directory[i] = directory;
            strcpy(descriptor->name[i], name);

            //update descriptor back to the beginning of the disk
            // memcpy(disk, descriptor, BLOCK_SIZE * 2);
            err = write_blocks(0, 2, descriptor);

            if (err < 0) {
                printf("Error: descriptor not write\n");
                return -1;
            }

            free(descriptor);
            return i;
        }
    }
    free(descriptor);
    return -1;
}

/*--------------------------------------------------------------------------------*/

//updates the descriptor block on disk to reflect that the block is no longer in use.
void unallocate_block(int offset)
{
    descriptor_block *descriptor = malloc(BLOCK_SIZE * 2);

    // memcpy(descriptor, disk, BLOCK_SIZE * 2);
    int err = read_blocks(0, 2, descriptor);

    if (err < 0) {
        printf("Error: can't read descriptor\n");
        return;
    }

    //TODO: check if the block holds a file, and then unallocate all its sub-block
    descriptor->free[offset] = true;
    strcpy(descriptor->name[offset], "");

    // memcpy(disk, descriptor, BLOCK_SIZE * 2);
    err = write_blocks(0, 2, descriptor);

    free(descriptor);

    if (err < 0) {
        printf("Error: can't write descriptor\n");
        return;
    } 
}

/*--------------------------------------------------------------------------------*/

//Takes in a name, and searches through descriptor block to find the block that contains the item
int find_block(char *name, bool directory)
{

    descriptor_block *descriptor = malloc(BLOCK_SIZE * 2);

    // memcpy(descriptor, disk, BLOCK_SIZE * 2);
    int err = read_blocks(0, 2, descriptor);

    if (err < 0) {
        printf("Error: can't read descriptor\n");
        return -1;
    }

    for (int i = 0; i < BLOCKS; i++)
    {
        if (strcmp(descriptor->name[i], name) == 0)
        {
            //Make sure it is of the type that we are searching for
            if (descriptor->directory[i] == directory)
            {
                free(descriptor);
                //Return the block index where the item resides in memory
                return i;
            }
        }
    }

    free(descriptor);
    return -1;
}

/*--------------------------------------------------------------------------------*/

int add_descriptor(char *name)
{
    //Allocate memory to a descriptor_block type so that we start assigning values to its members.
    descriptor_block *descriptor = malloc(BLOCK_SIZE * 2);

    
    //Allocate memory to the array of strings within the descriptor block, which holds the name of each block
    descriptor->name = malloc(sizeof *name * BLOCKS);

    //initialize each block ==> that it is free
    for (int i = 0; i < BLOCKS; i++)
    {
        descriptor->free[i] = true;
        descriptor->directory[i] = false;
    }

    //descriptor occupied space on the disk
    int limit = (int)(sizeof(descriptor_block) / BLOCK_SIZE) + 1;

    for (int i = 0; i < limit; i++)
    {
        descriptor->free[i] = false; //marking space occupied by descriptor as used
    }

    strcpy(descriptor->name[0], "descriptor");

    //writing new updated descriptor to the beginning of the disk
    //may encounter error here, to be fixed

    // memcpy(disk, descriptor, (BLOCK_SIZE * (limit + 1)));
    int err = write_blocks(0, limit + 1, descriptor);

    if (err < 0) {
        printf("Error: descriptor not added\n");
        return -1;
    }
    
    return 0;
}

/*--------------------------------------------------------------------------------*/

//Allows us to directly update values in the descriptor block.
int edit_descriptor(int free_index, bool free, int name_index, char *name)
{

    descriptor_block *descriptor = malloc(BLOCK_SIZE * 2);

    //Copy descriptor on disk to our descriptor_block type,
    // memcpy(descriptor, disk, BLOCK_SIZE * 2);
    int err = read_blocks(0, 2, descriptor);

    if (err < 0) {
        printf("Error: can't read descriptor\n");
        return -1;
    }

    //Each array in the descriptor block will be updated
    if (free_index > 0)
    {
        descriptor->free[free_index] = free;
    }
    if (name_index > 0)
    {
        strcpy(descriptor->name[name_index], name);
    }

    // write the new updated descriptor back to the beginning of the disk
    // memcpy(disk, descriptor, BLOCK_SIZE * 2);
    err = write_blocks(0, 2, descriptor);

    if (err < 0) {
        printf("Error: can't write descriptor\n");
        return -1;
    }

    return 0;
}

/*--------------------------------------------------------------------------------*/

// This changes the name of a file in the descriptor; used for moving files;
int edit_descriptor_name(int index, char *new_name)
{
    descriptor_block *descriptor = malloc(BLOCK_SIZE * 2);

    // memcpy(descriptor, disk, BLOCK_SIZE * 2);
    int err = read_blocks(0, 2, descriptor);

    if (err < 0) {
        printf("Error: can't read descriptor\n");
        return -1;
    }

    // Change the name of the file at index to the new_name
    strcpy(descriptor->name[index], new_name);

    // memcpy(disk, descriptor, BLOCK_SIZE * 2);
    err = write_blocks(0, 2, descriptor);

    if (err < 0) {
        printf("Error: can't write descriptor\n");
        return -1;
    }

    free(descriptor);
    return 0;
}

/*--------------------------------------------------------------------------------*/

//Allows us to add a folder to the disk.
int add_directory(char *name)
{

    if (strcmp(name, "") == 0)
    {
        return -1;
    }

    //Allocating memory for new folder
    dir_type *folder = malloc(BLOCK_SIZE);

    //Initialize our new folder
    strcpy(folder->name, name);
    strcpy(folder->top_level, current.directory);
    folder->subitem = malloc(sizeof *(folder->subitem) * MAX_SUBDIRECTORIES);
    folder->subitem_count = 0; // Imp : Initialize subitem array to have 0 elements

    //Find free block in disk to store our folder; true => mark the block as directory
    int index = allocate_block(name, true);

    //Copy our folder to the disk
    // memcpy(disk + index * BLOCK_SIZE, folder, BLOCK_SIZE);
    int err = write_blocks(index, 1, folder);

    if (err < 0) {
        printf("Error: directory not added\n");
        return -1;
    }

    free(folder);
    return 0;
}

/*--------------------------------------------------------------------------------*/

//Allows to remove a directory folder from the disk.
int remove_directory(char *name)
{

    dir_type *folder = malloc(BLOCK_SIZE);
    int block_index = find_block(name, true);

    //If there was no subdirectory found, then return -1
    if (block_index == -1)
    {
        return -1;
    }

    // memcpy(folder, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, folder);

    if (err < 0) {
        printf("Error: can't read folder\n");
        return -1;
    }

    //Go through again if there is a subdirectory ==> as implemented in Unix
    for (int i = 0; i < folder->subitem_count; i++)
    {
        if (folder->subitem_type[i] == true)
        {
            //Recursively call the function to remove the subitem
            remove_directory(folder->subitem[i]);
        }
        else
        {
            //Remove the subitem that is a file
            remove_file(folder->subitem[i]);
        }
    }
    unallocate_block(block_index);
    free(folder);

    return 0;
}

/*--------------------------------------------------------------------------------*/

//Allows you to directly add items to a folders subitem array, or change the folder's name
int edit_directory(char *name, char *subitem_name, char *new_name, bool name_change, bool directory)
{

    if (strcmp(name, "") == 0)
    {
        return -1;
    }

    dir_type *folder = malloc(BLOCK_SIZE);

    //Find where the folder is on disk
    int block_index = find_block(name, true);
    //If the directory is not found, should return
    if (block_index == -1)
    {
        return -1;
    }

    // memcpy(folder, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, folder);

    if (err < 0) {
        printf("Error: can't read folder\n");
        return -1;
    }

    if (strcmp(subitem_name, "") != 0)
    { //Case that we are adding subitem to the descriptor block

        if (!name_change)
        { //Case adding subitem
            strcpy(folder->subitem[folder->subitem_count], subitem_name);
            folder->subitem_type[folder->subitem_count] = directory;
            folder->subitem_count++;

            //update the disk too!
            // memcpy(disk + block_index * BLOCK_SIZE, folder, BLOCK_SIZE);
            err = write_blocks(block_index, 1, folder);

            free(folder);

            if (err < 0) {
                printf("Error: can't write folder\n");
                return -1;
            }

            return 0;
        }
        else
        { //Case editing a subitem's name
            for (int i = 0; i < folder->subitem_count; i++)
            {
                if (strcmp(folder->subitem[i], subitem_name) == 0)
                {
                    strcpy(folder->subitem[i], new_name);
                    // memcpy(disk + block_index * BLOCK_SIZE, folder, BLOCK_SIZE);
                    err = write_blocks(block_index, 1, folder);

                    free(folder);

                    if (err < 0) {
                        printf("Error: can't write folder\n");
                        return -1;
                    }
                    
                    return 0;
                }
            }
            free(folder);
            return -1;
        }
    }
    else
    { //Case we are changing the folders name
        //if directory with given name already exists, don't allow
        int block_index2 = find_block(new_name, true);

        //If the directory for the new name already exists, should return -1
        if (block_index2 != -1)
        {
            return -1;
        }

        strcpy(folder->name, new_name);

        // memcpy(disk + block_index * BLOCK_SIZE, folder, BLOCK_SIZE);
        err = write_blocks(block_index, 1, folder);

        if (err < 0) {
            printf("Error: can't write folder\n");
            return -1;
        }

        //edit descriptors
        edit_descriptor(-1, false, block_index, new_name);

        //changing parents name
        edit_directory(folder->top_level, name, new_name, true, true);

        int child_index;

        //Iterates through to change the subitems' top_level name
        for (int i = 0; i < folder->subitem_count; i++)
        {
            file_type *child_file = malloc(BLOCK_SIZE);
            dir_type *child_folder = malloc(BLOCK_SIZE);

            child_index = find_block(folder->subitem[i], folder->subitem_type);
            if (folder->subitem_type[i])
            {
                //if type == folder
                // memcpy(child_folder, disk + child_index * BLOCK_SIZE, BLOCK_SIZE);
                err = read_blocks(child_index, 1, child_folder);

                if (err < 0) {
                    printf("Error: can't read child_folder\n");
                    return -1;
                }


                strcpy(child_folder->top_level, new_name);

                // memcpy(disk + child_index * BLOCK_SIZE, child_folder, BLOCK_SIZE);
                err = write_blocks(child_index, 1, child_folder);

                if (err < 0) {
                    printf("Error: can't write child_folder\n");
                    return -1;
                }

                free(child_folder);
                free(child_file);
            }
            else
            {
                //if type == file
                // memcpy(child_file, disk + child_index * BLOCK_SIZE, BLOCK_SIZE);
                err = read_blocks(child_index, 1, child_file);

                if (err < 0) {
                    printf("Error: can't read child_file\n");
                    return -1;
                }

                strcpy(child_file->top_level, new_name);

                // memcpy(disk + child_index * BLOCK_SIZE, child_file, BLOCK_SIZE);
                err = write_blocks(child_index, 1, child_file);

                if (err < 0) {
                    printf("Error: can't write child_file\n");
                    return -1;
                }

                free(child_folder);
                free(child_file);
            }
        }

        free(folder);
        return 0;
    }

    free(folder);
}

/*--------------------------------------------------------------------------------*/

//Allows us to add a file to our disk; This function will allocate this file descriptor block (holds file info), as well as data blocks
int add_file(char *name, int size)
{
    char subname[20];

    if (size < 0 || strcmp(name, "") == 0)
    {

        print_console("mkfil: missing operand\n");

        return 1;
    }

    //Allocate memory to a file_type
    file_type *file = malloc(BLOCK_SIZE);

    //Initialize all the members of our new file
    strcpy(file->name, name);
    strcpy(file->top_level, current.directory);
    file->size = size;
    file->data_block_count = 0;
    //Find free block to put this file descriptor block in memory, false ==> indicates a file
    int index = allocate_block(name, false);

    //Find free blocks to put the file data into
    for (int i = 0; i < size / BLOCK_SIZE + 1; i++)
    {
        sprintf(subname, "%s->%d", name, i);
        file->data_block_index[i] = allocate_block(subname, false);
        file->data_block_count++;
    }
    //data blocks in memory not copied to disk
    // memcpy(disk + index * BLOCK_SIZE, file, BLOCK_SIZE);
    int err = write_blocks(index, 1, file);

    if (err < 0) {
        printf("Error: can't write file\n");
        return -1;
    }

    free(file);
    return 0;
}

/*--------------------------------------------------------------------------------*/

int remove_file(char *name)
{
    if (strcmp(name, "") == 0)
    {
        print_console("rmfil: missing operand\n");
        return 1;
    }

    file_type *file = malloc(BLOCK_SIZE);
    dir_type *folder = malloc(BLOCK_SIZE);

    int file_index = find_block(name, false);

    // If the file is not found, error, return -1
    if (file_index == -1)
    {
        return -1;
    }

    // memcpy(file, disk + file_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(file_index, 1, file);

    if (err < 0) {
        printf("Error: can't read file\n");
        return -1;
    }

    //Find the top_level folder on disk
    int folder_index = find_block(file->top_level, true);

    // memcpy(folder, disk + folder_index * BLOCK_SIZE, BLOCK_SIZE);
    err = read_blocks(folder_index, 1, folder);

    if (err < 0) {
        printf("Error: can't read folder\n");
        return -1;
    }

    // Go through the parent directory's subitem array and remove our file
    char subitem_name[MAX_STRING_LENGTH];
    const int subcnt = folder->subitem_count; // no of subitems
    int j;
    int k = 0;
    for (j = 0; j < subcnt; j++)
    {
        strcpy(subitem_name, folder->subitem[j]);
        if (strcmp(subitem_name, name) != 0)
        // if this element is not the one we are removing, copy back
        {
            strcpy(folder->subitem[k], subitem_name);
            k++;
        }
    }
    strcpy(folder->subitem[k], "");
    folder->subitem_count--;

    // memcpy(disk + folder_index * BLOCK_SIZE, folder, BLOCK_SIZE); // Update the folder in memory
    err = write_blocks(folder_index, 1, folder);

    if (err < 0) {
        printf("Error: can't write folder\n");
        return -1;
    }

    //Imp :  Unallocate all of the data blocks from the file that we are deleting
    int i = 0;
    while (file->data_block_count != 0)
    {
        unallocate_block(file->data_block_index[i]);
        file->data_block_count--;
        i++;
    }

    unallocate_block(file_index); // Deallocate the file control block

    free(folder);
    free(file);
    return 0;
}

/*--------------------------------------------------------------------------------*/

//Allows you to directly edit a file and change its size or its name
int edit_file(char *name, int size, char *new_name)
{
    file_type *file = malloc(BLOCK_SIZE);

    //Find the block in memory where this file is written
    int block_index = find_block(name, false);
    if (block_index == -1)
    {
        return -1;
    }

    // memcpy(file, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, file);

    if (err < 0) {
        printf("Error: can't read file\n");
        return -1;
    }

    if (size > 0)
    {
        //If size is greater than zero, then the files size will be updated
        file->size = size;
        free(file);
        return 0;
    }
    else
    {
        //Otherwise, the file's name will be updated
        char top_level[MAX_STRING_LENGTH];
        strcpy(top_level, get_file_top_level(name));

        // Change the name of the directory's subitem
        edit_directory_subitem(top_level, name, new_name);

        // Change the name of the actual file descriptor
        edit_descriptor_name(block_index, new_name);

        strcpy(file->name, new_name);

        // memcpy(disk + block_index * BLOCK_SIZE, file, BLOCK_SIZE);
        err = write_blocks(block_index, 1, file);

        free(file);

        if (err < 0) {
            printf("Error: can't write file\n");
            return -1;
        }

        return 0;
    }
}

/*--------------------------------------------------------------------------------*/

/************************** Getter functions ************************************/
char *get_directory_name(char *name)
{
    dir_type *folder = malloc(BLOCK_SIZE);
    char *tmp = malloc(sizeof(char) * MAX_STRING_LENGTH);

    //True arguement tells the find function that we are looking
    //for a directory not a file
    int block_index = find_block(name, true);
    if (block_index == -1)
    {
        strcpy(tmp, "");
        return tmp;
    }

    // memcpy(folder, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, folder);

    if (err < 0) {
        printf("Error: can't read folder\n");
        return "-1";
    }

    strcpy(tmp, folder->name);
    free(folder);
    return tmp;
}

/*--------------------------------------------------------------------------------*/

char *get_directory_top_level(char *name)
{
    dir_type *folder = malloc(BLOCK_SIZE);
    char *tmp = malloc(sizeof(char) * MAX_STRING_LENGTH);

    //true ==> indicates a folder and not a file
    int block_index = find_block(name, true);
    if (block_index == -1)
    {
        strcpy(tmp, "");
        return tmp;
    }

    // memcpy(folder, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, folder);

    if (err < 0) {
        printf("Error: can't read folder\n");
        return "-1";
    }

    strcpy(tmp, folder->top_level);
    free(folder);
    return tmp;
}

/*--------------------------------------------------------------------------------*/

char *get_directory_subitem(char *name, int subitem_index, char *subitem_name)
{
    dir_type *folder = malloc(BLOCK_SIZE);
    char *tmp = malloc(sizeof(char) * MAX_STRING_LENGTH);

    int block_index = find_block(name, true);
    if (block_index == -1)
    {
        strcpy(tmp, "");
        return tmp;
    }

    // memcpy(folder, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, folder);

    if (err < 0) {
        printf("Error: can't read folder\n");
        return "-1";
    }

    if (subitem_index >= 0)
    {
        //Case we are changing the name of a subitem
        strcpy(tmp, folder->subitem[subitem_index]);
        free(folder);
        return tmp;
    }
    else
    {
        //Case that we Are Searching for a Sub Item
        for (int i = 0; i < folder->subitem_count; i++)
        {
            if (strcmp(folder->subitem[i], subitem_name) == 0)
            {
                return "0";
            }
        }
        free(folder);
        return "-1";
    }
    free(folder);
    return tmp;
}

/*--------------------------------------------------------------------------------*/

int edit_directory_subitem(char *name, char *sub_name, char *new_sub_name)
{
    dir_type *folder = malloc(BLOCK_SIZE);

    //True argument tells the find function that we are looking
    //for a directory not a file
    int block_index = find_block(name, true);
    if (block_index == -1)
    {
    }

    // memcpy(folder, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, folder);

    if (err < 0) {
        printf("Error: can't read folder\n");
        return -1;
    }

    const int cnt = folder->subitem_count;
    int i;
    for (i = 0; i < cnt; i++)
    {
        if (strcmp(folder->subitem[i], sub_name) == 0)
        {
            strcpy(folder->subitem[i], new_sub_name);

            // memcpy(disk + block_index * BLOCK_SIZE, folder, BLOCK_SIZE);
            err = write_blocks(block_index, 1, folder);

            free(folder);

            if (err < 0) {
                printf("Error: can't write folder\n");
                return -1;
            }
            return i;
        }
    }

    free(folder);
    return -1;
}

/*--------------------------------------------------------------------------------*/

int get_directory_subitem_count(char *name)
{

    dir_type *folder = malloc(BLOCK_SIZE);
    int tmp;

    int block_index = find_block(name, true);
    if (block_index == -1)
    {
        return -1;
    }

    // memcpy(folder, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, folder);

    if (err < 0) {
        printf("Error: can't read folder\n");
        return -1;
    }

    tmp = folder->subitem_count;
    free(folder);
    return tmp;
}

/*--------------------------------------------------------------------------------*/

char *get_file_name(char *name)
{
    file_type *file = malloc(BLOCK_SIZE);
    char *tmp = malloc(sizeof(char) * MAX_STRING_LENGTH);

    //false ==> indicates a file and not a folder
    int block_index = find_block(name, false);
    if (block_index == -1)
    {
        strcpy(tmp, "");
        return tmp;
    }

    // memcpy(file, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, file);

    if (err < 0) {
        printf("Error: can't read file\n");
        return "-1";
    }

    strcpy(tmp, file->name);
    free(file);
    return tmp;
}

/*--------------------------------------------------------------------------------*/

char *get_file_top_level(char *name)
{
    file_type *file = malloc(BLOCK_SIZE);
    char *tmp = malloc(sizeof(char) * MAX_STRING_LENGTH);

    int block_index = find_block(name, false);
    if (block_index == -1)
    {
        strcpy(tmp, "");
        return tmp;
    }

    // memcpy(file, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, file);

    if (err < 0) {
        printf("Error: can't read file\n");
        return "-1";
    }

    strcpy(tmp, file->top_level);
    free(file);
    return tmp;
}

/*--------------------------------------------------------------------------------*/

int get_file_size(char *name)
{

    file_type *file = malloc(BLOCK_SIZE);
    int tmp;

    int block_index = find_block(name, false);
    if (block_index == -1)
    {
        return -1;
    }

    // memcpy(file, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, file);

    if (err < 0) {
        printf("Error: can't read file\n");
        return -1;
    }

    tmp = file->size;
    free(file);
    return tmp;
}

/*--------------------------------------------------------------------------------*/

/********************************* Print Functions ********************************/
void print_directory(char *name)
{
    dir_type *folder = malloc(BLOCK_SIZE);
    int block_index = find_block(name, true);

    // memcpy(folder, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, folder);

    if (err < 0) {
        printf("Error: can't read folder\n");
        return;
    }

    printf("	-----------------------------\n");
    printf("	New Folder Attributes:\n\n\tname = %s\n\ttop_level = %s\n\tsubitems = ", folder->name, folder->top_level);
    for (int i = 0; i < folder->subitem_count; i++)
    {
        printf("%s ", folder->subitem[i]);
    }
    printf("\n\tsubitem_count = %d\n", folder->subitem_count);
    printf("	-----------------------------\n");

    free(folder);
}

void print_file(char *name)
{
    file_type *file = malloc(BLOCK_SIZE);
    int block_index = find_block(name, false);

    // memcpy(file, disk + block_index * BLOCK_SIZE, BLOCK_SIZE);
    int err = read_blocks(block_index, 1, file);

    if (err < 0) {
        printf("Error: can't read file\n");
        return;
    }

    printf("	-----------------------------\n");
    printf("	New File Attributes:\n\n\tname = %s\n\ttop_level = %s\n\tfile size = %d\n\tblock count = %d\n", file->name, file->top_level, file->size, file->data_block_count);
    printf("	-----------------------------\n");

    free(file);
}

/*--------------------------------------------------------------------------------*/