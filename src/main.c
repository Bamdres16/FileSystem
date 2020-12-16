#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "constant.c"
#include "main.h"

int main(int argc, char *argv[])
{
    GtkBuilder *builder;
    GtkWidget *window;
    GtkTextIter iter;

    // Init username
    set_username();

    app_widgets *widgets = g_slice_new(app_widgets);

    gtk_init(&argc, &argv);

    builder = gtk_builder_new_from_file("glade/window_main.glade");
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));

    // Get pointers to widgets
    widgets->textview_main = GTK_WIDGET(gtk_builder_get_object(builder, "textview_main"));
    widgets->textbuffer_main = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "textbuffer_main"));
    widgets->textentry_main = GTK_ENTRY(gtk_builder_get_object(builder, "textentry_main"));
    widgets->test = GTK_WIDGET(gtk_builder_get_object(builder, "test"));

    // Color tags
    gtk_text_buffer_create_tag(widgets->textbuffer_main, "dir_bg",
                               "foreground", "#a042e5", NULL);
    gtk_text_buffer_create_tag(widgets->textbuffer_main, "user_fg",
                               "foreground", "#00a6ff", NULL);

    gtk_builder_connect_signals(builder, widgets);

    g_object_unref(builder);
    gtk_text_buffer_get_iter_at_offset(widgets->textbuffer_main, &iter, -1);
    gtk_text_buffer_insert_with_tags_by_name(widgets->textbuffer_main, &iter, username_c, -1, "user_fg", NULL, NULL);

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
    char *buf, hostname[128];
    hostname[127] = '\0';
    buf = (char *)malloc(10 * sizeof(char));
    buf = getlogin();
    gethostname(hostname, 127);
    strcpy(username_c, buf);
    strcat(username_c, "@");
    strcat(username_c, hostname);
    strcat(username_c, ": ");
}

void print_console(app_widgets *app_wdgts, char *text)
{
    gtk_text_buffer_insert_at_cursor(app_wdgts->textbuffer_main, text, -1);
}

void print_console_color(app_widgets *app_wdgts, char *text, char *color)
{
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_offset(app_wdgts->textbuffer_main, &iter, -1);
    gtk_text_buffer_insert_with_tags_by_name(app_wdgts->textbuffer_main, &iter,
                                             text, -1, color, NULL, NULL);
}

void print_username(app_widgets *app_wdgts)
{
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_offset(app_wdgts->textbuffer_main, &iter, -1);
    gtk_text_buffer_insert_with_tags_by_name(app_wdgts->textbuffer_main, &iter, username_c, -1, "user_fg", NULL, NULL);
}

void reset_entry(app_widgets *app_wdgts)
{
    gtk_entry_set_text(app_wdgts->textentry_main, "");
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

            for (int i = 0; i < MAX_COMMANDS; i++)
            {
                char *token = strtok(command, " ");
                if (strcmp(token, commands[i]) == 0)
                {
                    if (strcmp(txt, "exit") == 0)
                    {
                        gtk_main_quit();
                    }
                    found = 1;
                    add_text(command, "\n");
                    add_text(complete, "\n");
                    print_console_color(app_wdgts, complete, "dir_bg");
                    reset_entry(app_wdgts);
                    print_username(app_wdgts);
                }
            }
            if (!found)
            {
                add_text(command, " ");
                print_console(app_wdgts, command);
                print_console(app_wdgts, notFound);
                reset_entry(app_wdgts);
                print_username(app_wdgts);
            }
            else
            {
                found = 0;
            }
        }
    }
}