#include <mi.h>

static Vec2 vec2() {
    return (Vec2) {.x = 0, .y=0, ._line=NULL};
}

MiEditor *init_editor(char *path)
{
    MiEditor *E = malloc(sizeof(MiEditor));
    init_signals();

    memset(E, 0, sizeof(*E));
    char *pathBuff = NULL;
    E->ewindow = init_ncurses_window();
    E->renderer = malloc(sizeof(Lines_renderer));
    editor_load_layout(E);

    // If the caller did not supply a file then we ask him in a seperate screen.
    if (path == NULL) {
        pathBuff = editor_render_startup(E->renderer->win_w / 2, E->renderer->win_h / 2, E->renderer->win_w);
        if (pathBuff == NULL) {
            release_editor(E);
            return NULL;
        }
    } else {
        pathBuff =  string_dup(path);
    }

    E->fb = new_file_browser(pathBuff);
    free(pathBuff); 
    // Prepare for highlighting text (Copying and pasting..)
    E->highlighted_end         = vec2() ; // x=0, Y=0
    E->highlighted_start       = vec2() ; // x=0, Y=0
    E->highlighted_data_length = 0;

    // Prepare notification_buffer and other important properties..
    E->notification_buffer = malloc((1024 + 1) * sizeof(char));
    memset(E->notification_buffer, 0, 1024);
    E->exit_pressed = false;
    E->char_deleted = false;
    E->mode         = NORMAL;
    
    // Prepare the renderer.
    
    E->renderer->origin      = Alloc_line_node(0);
    E->renderer->count       = 0;
    E->renderer->start       = E->renderer->origin;
    E->renderer->end         = E->renderer->origin; 
    E->renderer->current     = E->renderer->origin; 
    E->renderer->max_padding = 2;

    // If the path that was passed was a file, or if it does not exist. we assign.
    if (E->fb->type == FILE__ || E->fb->type == NOT_EXIST) {
        E->renderer->file_type = get_file_type (E->fb->open_entry_path);
        load_file(E->fb->open_entry_path, E->renderer);
    }

    // Init binding queue.    
    E->binding_queue = (vKeyBindingQueue){0x0};
    if (E->fb->type == FILE__ || E->fb->type == NOT_EXIST) {
        editor_refresh(E); 
    } else {
        E->mode = FILEBROWSER;
        render_file_browser(E);
        editor_render_details(E->renderer, E->fb->open_entry_path, E->mode, E->notification_buffer);
    }
    
    return (E);
}

void editor_load_layout(MiEditor *E)
{
    // Set the dimentions of the edittor..
    getmaxyx(E->ewindow, 
         E->renderer->win_h, 
         E->renderer->win_w
    );
}

void release_editor(MiEditor *E)
{
    if (!E) return;
    release_fb(E->fb);
    if (E->renderer) {
        free_lines(E->renderer->origin);
        free(E->renderer);
    }
    
    free(E->notification_buffer);
    free(E);
}

void editor_refresh(MiEditor *E)
{
    erase();
    render_lines(E->renderer);
    editor_render_details(E->renderer, E->fb->open_entry_path, E->mode, E->notification_buffer);
    editor_apply_move(E->renderer);
    refresh();
}



void editor_update(int c, MiEditor *E)
{
    // Globals.
    switch (c) {
        case ESC: {
            E->mode = NORMAL;
            return;
        } break;
    }
    // Actions that depend on the mode.
    switch (E->mode) {
        case VISUAL: { editor_visual(c, E);  } break;
        case NORMAL: { editor_normal(c, E);  } break;
        case INSERT: { editor_insert(c, E);  } break;
        default: {} break;
    }
}
