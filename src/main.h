// Structures

typedef struct
{
    GtkWidget *textview_main; // Pointer to text view object
    GtkWidget *test;
    GtkTextBuffer *textbuffer_main; // Pointer to text buffer
    GtkEntry *textentry_main;       // Pointer to text entry
} app_widgets;

// Methods

void add_text(char *dest, const char *src);
void set_username();
void on_window_main_destroy();
void on_textentry_main_key_release_event(GtkWidget *widget, GdkEvent *event, app_widgets *app_wdgts);