#include <gtk/gtk.h>
#include <string.h>
#include "constant.c"

#define ENTER_KEY 65293

typedef struct
{
    GtkWidget *textview_main; // Pointer to text view object
    GtkWidget *test;
    GtkTextBuffer *textbuffer_main; // Pointer to text buffer
    GtkEntry *textentry_main;       // Pointer to text entry
} app_widgets;

void add_text(char *dest, const char *src)
{
    char command[256];
    strcpy(command, dest);
    strcat(command, src);
    strcpy(dest, command);
}

int main(int argc, char *argv[])
{
    GtkBuilder *builder;
    GtkWidget *window;
    app_widgets *widgets = g_slice_new(app_widgets);

    gtk_init(&argc, &argv);

    builder = gtk_builder_new_from_file("glade/window_main.glade");
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));

    // Get pointers to widgets
    widgets->textview_main = GTK_WIDGET(gtk_builder_get_object(builder, "textview_main"));
    widgets->textbuffer_main = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "textbuffer_main"));
    widgets->textentry_main = GTK_ENTRY(gtk_builder_get_object(builder, "textentry_main"));
    widgets->test = GTK_WIDGET(gtk_builder_get_object(builder, "test"));

    gtk_builder_connect_signals(builder, widgets);

    g_object_unref(builder);
    gtk_text_buffer_set_text(widgets->textbuffer_main, "brayan@terminal: ", -1);

    gtk_widget_show(window);
    gtk_main();

    g_slice_free(app_widgets, widgets);

    return 0;
}

// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}

void on_textentry_main_key_release_event(GtkWidget *widget, GdkEvent *event, app_widgets *app_wdgts)
{
    GdkEventKey eventKey;
    eventKey = event->key;
    int found = 0;
    if (eventKey.keyval == ENTER_KEY)
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
                    gtk_text_buffer_insert_at_cursor(app_wdgts->textbuffer_main, complete, -1);
                    gtk_entry_set_text(app_wdgts->textentry_main, "");
                    gtk_text_buffer_insert_at_cursor(app_wdgts->textbuffer_main, "brayan@terminal: ", -1);
                }
            }
            if (!found)
            {
                add_text(command, " ");
                gtk_text_buffer_insert_at_cursor(app_wdgts->textbuffer_main, command, -1);
                gtk_text_buffer_insert_at_cursor(app_wdgts->textbuffer_main, "command not found\n", -1);
                gtk_text_buffer_insert_at_cursor(app_wdgts->textbuffer_main, "brayan@terminal: ", -1);
                gtk_entry_set_text(app_wdgts->textentry_main, "");
            }
            else
            {
                found = 0;
            }
        }
    }
}