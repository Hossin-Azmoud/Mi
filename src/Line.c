#include <mi.h>

Line *Alloc_line_node(int row)
{
    Line *line = (Line *)malloc(sizeof(Line));

    line->next = NULL;
    line->prev = NULL;
    line->x    = 0;
    line->size = 0;
    line->cap  = LINE_SZ;
    line->y    = row;
    line->content = malloc(LINE_SZ);
    memset(line->content, 0, LINE_SZ);

    return (line);
}

void lines_shift(Line *head, int num) {

    Line *curr = head;
    for (;curr;) {
        curr->y += num;
        curr = curr->next;
    }
}

// Line *line_at(Line *current, int y) {
//     Line *it =   current;
//     if (current->y < y + )
// }
//
//
//                     if (line_ren->current->y < event.y + line_ren->start->y) {
//                         while (line_ren->current->next) {
//                             if (line_ren->current->y == event.y + line_ren->start->y) {
//                                 break;
//                             }
//                             line_ren->current = line_ren->current->next;
//                         }
//                     } else if (line_ren->current->y > event.y + line_ren->start->y) {
//                         while (line_ren->current->prev) {
//                             if (line_ren->current->y == event.y + line_ren->start->y) {
//                                 break;
//                             }
//                             line_ren->current = line_ren->current->prev;
//                         }
//                     }
//












