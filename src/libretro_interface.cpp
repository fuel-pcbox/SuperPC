#include "libretro.h"
#include <stdio.h>
#include <stdlib.h>

#include "attotime.h"
#include "interface.h"
#include "misc.h"

#include "cpu.h"
#include "mda.h"
#include "cga.h"
#include "savestate.h"

#define AUDIO_FREQ 44100
#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480

static retro_environment_t env_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

/*
 * libretro host implementation
 */
 
 namespace INTERFACE
 {
    retro_log_callback* log_cb;
    attotime cpuclock;
    attotime pitclock;
    attotime current_time;
    attotime frame_time;

 FILE* flop1 = nullptr;
 
 bool quitflag = false;
 bool emulatingflag = false;
 int vidwidth = 720;
 int vidheight = 350;

 char appdir[PATH_MAX];

 u8* screen;
 
 int init(int width, int height)
 {
     screen = (u8*)malloc(width * height * 4);
     memset(screen, 0, width * height * 4);
 }
 
 void quit()
 {
     if(flop1) fclose(flop1);
 }
 
 inline void window_caption(const char *title, const char *icon)
 {
 }
 
 inline void update_screen()
 {
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
 }
 } //namespace INTERFACE
 

extern "C"
{
 
/*
 * libretro core implementation
 */
void retro_init() {
  /* set application directory */
  const char *sysdir = NULL;
  if (env_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &sysdir))
  {
    snprintf(INTERFACE::appdir, sizeof(INTERFACE::appdir), "%s/superpc", sysdir);
  }
  env_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &INTERFACE::log_cb);

  INTERFACE::init(720,350);
}

void retro_deinit() {}

unsigned retro_api_version() {
  return RETRO_API_VERSION;
}

void retro_get_system_info(struct retro_system_info *info) {
  info->library_name = "SuperPC";
  info->library_version = "v1";
  info->valid_extensions = "";
  info->need_fullpath = false;
  info->block_extract = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
  info->geometry.base_width = 720;
  info->geometry.base_height = 350;
  info->geometry.max_width = 800;
  info->geometry.max_height = 600;
  info->geometry.aspect_ratio = (float)4 / (float)3;
  info->timing.fps = 50;
  info->timing.sample_rate = AUDIO_FREQ;
}

void retro_set_environment(retro_environment_t env) {
  env_cb = env;
}

void retro_set_video_refresh(retro_video_refresh_t video) {
  video_cb = video;
}

void retro_reset() {}

void retro_run() {

  for(;;)
  {
    CPU::tick();
    
    if(CPU::hint == true) CPU::hint = false;
    
    INTERFACE::current_time += INTERFACE::cpuclock;
    INTERFACE::frame_time += INTERFACE::cpuclock;
    if(INTERFACE::current_time >= INTERFACE::pitclock)
    {
        PIT::tick();
        INTERFACE::current_time -= INTERFACE::pitclock;
    }
    if(INTERFACE::frame_time >= attotime::from_hz(50))
    {
        MDA::tick_frame();
        INTERFACE::frame_time -= attotime::from_hz(50);
        video_cb(INTERFACE::screen, 720, 350, 720*4);
        break;
    }
  }
  /* call back into retroarch, letting it know a frame has been rendered */
}

size_t retro_serialize_size() {
  return 0;
}

bool retro_serialize(void *data, size_t size) {
  return false;
}

bool retro_unserialize(const void *data, size_t size) {
  return false;
}

void retro_cheat_reset() {}

void retro_cheat_set(unsigned index, bool enabled, const char *code) {}

bool retro_load_game(const struct retro_game_info *info)
{
    PIC::pic[0].init1 = false;
    PIC::pic[0].init2 = false;
    PIC::pic[0].enabled = false;
    DMA_XT::chan[0].access_flip_flop = false;
    CPU::cr0 = 0;

    u32 cpuclock_cfg = 4772727;

    INTERFACE::cpuclock = attotime::from_hz(cpuclock_cfg);
    
    char path[PATH_MAX];

    sprintf(path, "%s/%s", INTERFACE::appdir, "roms/video/mda/mda.rom");

    FILE* mda = fopen(path,"rb");
    fread(MDA::ROM,1,0x2000,mda);
    fseek(mda,0,SEEK_SET);
    fread(CGA::ROM,1,0x2000,mda);
    fclose(mda);

    INTERFACE::init();

    MDA::init();
    IO_XT::handlers.push_back(MDA::mdacrtc);
    INTERFACE::window_caption("SuperPC v0.0.1");

    CPU::type = CPU::intel8088;
    INTERFACE::pitclock = attotime::from_hz(157500000/11/12);

    sprintf(path, "%s/%s", INTERFACE::appdir, "tests/graphics/mda/text.bin");

    FILE* biosfp = fopen(path, "rb");
    fread(RAM::BIOS + 0x1e000,0x2000,1,biosfp);
    fclose(biosfp);

    sprintf(path, "%s/%s", INTERFACE::appdir, "roms/machines/ibmpc/ibm-basic-1.10.rom");

    biosfp = fopen(path, "rb");
    fread(RAM::BIOS + 0x16000,0x8000,1,biosfp);
    fclose(biosfp);

    RAM::handlers.push_back(RAM::bios_handler);
    IO_XT::handlers.push_back(DMA_XT::handler);
    IO_XT::handlers.push_back(DMA_XT::handler2);
    IO_XT::handlers.push_back(PPI::handler);
    IO_XT::handlers.push_back(PIT::pit);
    IO_XT::handlers.push_back(PIC::pic1);
    IO_XT::handlers.push_back(FDC::handler);

    int pixformat = RETRO_PIXEL_FORMAT_XRGB8888;
    
    if(!env_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixformat)) return false;
}

bool retro_load_game_special(unsigned game_type,
                             const struct retro_game_info *info,
                             size_t num_info)
{
  return false;
}

void retro_unload_game()
{
    INTERFACE::quit();
}
}