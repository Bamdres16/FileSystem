#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void print_console(char *text)
{
    gtk_text_buffer_insert_at_cursor(widgets->textbuffer_main, text, -1);
}