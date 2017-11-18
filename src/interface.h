#ifndef USE_SDL
#include "libretro.h"
#endif

#ifndef INTERFACE_H_INCLUDED
#define INTERFACE_H_INCLUDED

/* A thin wrapper around SDL (and potentially other libraries), that handles screen,
 * keyboard & mouse on the host computer.
 */

#include <functional>

//#define USE_SDL

#ifdef USE_SDL
    //TODO: fix this dirty workaround
    #ifdef dx
        #undef dx
        #include <SDL/SDL.h>
        #define dx dw.w
    #else
        #include <SDL/SDL.h>
    #endif //dx
#endif // USE_SDL

#include "ram.h"
#include "cpu.h"

//TODO: why not a class?
namespace INTERFACE
{

extern bool quitflag;
extern bool emulatingflag;

#ifndef USE_SDL
extern retro_log_callback* log_cb;
#endif

#ifdef USE_SDL
typedef SDL_Surface Surface;
typedef SDL_Event Event;
#endif

extern FILE* flop1;
extern int vidwidth, vidheight;

#ifdef USE_SDL
extern Surface* screen;
#else
extern u8* screen;
#endif

int init(int width = 720, int height = 350);

void quit();

void window_caption(const char *title, const char *icon = NULL);

void update_screen();

int handle_events();

void load_floppy(const char* filename);

void read_floppy_sector(unsigned cylinder, unsigned head, unsigned sector);

} //namespace INTERFACE

#endif // INTERFACE_H_INCLUDED
