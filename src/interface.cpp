#include "interface.h"

#include "savestate.h"   

namespace INTERFACE
{

FILE* flop1 = nullptr;

bool quitflag = false;
bool emulatingflag = false;
int vidwidth = 720;
int vidheight = 350;

#ifdef USE_SDL
Surface* screen = NULL;
#endif

int init(int width, int height)
{
    int ret = SDL_Init(SDL_INIT_EVERYTHING);
    screen = SDL_SetVideoMode(width, height, 24, SDL_SWSURFACE);
    return ret;
}

void quit()
{
    if(flop1) fclose(flop1);
    SDL_Quit();
}

void window_caption(const char *title, const char *icon)
{
    SDL_WM_SetCaption(title, icon);
}

void update_screen()
{
    SDL_Flip(screen);
}

void load_floppy(const char* filename)
{
    flop1 = fopen(filename,"rb");
}

void read_floppy_sector(unsigned cylinder, unsigned head, unsigned sector)
{
    fseek(flop1,((((cylinder * 2) + head) * 18)+sector-1)*512,SEEK_SET);
    fread(RAM::RAM + RAM::getaddr(CPU::es,CPU::bx),1,512,flop1);
}

int handle_events()
{
    SDL_Event e;

    while(SDL_PollEvent(&e))
    {
        if(e.type == SDL_QUIT) quitflag = true;
        if(e.type == SDL_KEYDOWN)
        {
            switch(e.key.keysym.sym)
            {
            case SDLK_SCROLLOCK:
            {
                emulatingflag ^= 1; //Toggle the emulating flag.
                break;
            }
            case SDLK_s:
            {
                if(!emulatingflag) savestate_save();
                else
                {
                    CPU::hint = true;
                    CPU::hintnum = 1;
                    PPI::keyboardshift.push_back(0x1F);
                }
                break;
            }
            }
        }
        if(e.type == SDL_KEYUP)
        {
            switch(e.key.keysym.sym)
            {
                case SDLK_s:
                {
                    if(emulatingflag)
                    {
                        CPU::hint = true;
                        CPU::hintnum = 1;
                        PPI::keyboardshift.push_back(0x9F);
                    }
                    break;
                }
            }
        }
    }
}

} //namespace INTERFACE
