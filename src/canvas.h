#ifndef IMP_CANVAS_H
#define IMP_CANVAS_H
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint32_t u32;

typedef enum ImpCursorMode {
    IMP_CURSOR,
    IMP_PENCIL,
} ImpCursorMode;
                                                                         
typedef struct ImpCursor {
    int x, y;
    ImpCursorMode mode;
    u32 color;
    bool scroll_locked;
    bool pencil_locked;
} ImpCursor;

typedef struct  {
    int x, y, w, h;
    SDL_Texture *texture;
    SDL_Texture *bg;
    SDL_Rect bg_rect;
} ImpCanvas;

ImpCanvas *create_imp_canvas(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *layer0_text);
void imp_canvas_event(ImpCanvas *c, SDL_Event *e, ImpCursor *cursor);
void imp_canvas_render(SDL_Renderer *renderer, ImpCanvas *c);
#endif