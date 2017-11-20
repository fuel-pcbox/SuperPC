#include <cstdio>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include <thread>

#include "attotime.h"
#include "interface.h"
#include "misc.h"

enum
{
    update_frame = 1,
    update_scanline = 2,
    update_pixel = 4,
    update_clock = 8
};

#include "cpu.h"
#include "mda.h"
#include "cga.h"
#include "savestate.h"

int main(int ac, char** av)
{
    if(ac < 2)
    {
        printf("Usage:\n\tsuperpc cfgfile\n");
        return 1;
    }

    attotime cpuclock;
    attotime pitclock;
    attotime current_time;
    attotime frame_time;

    PIC::pic[0].init1 = false;
    PIC::pic[0].init2 = false;
    PIC::pic[0].enabled = false;
    DMA_XT::chan[0].access_flip_flop = false;
    CPU::cr0 = 0;

    char* isa1 = new char[10];
    char* machine = new char[256];
    char* flop1 = new char[256];
    u32 cpuclock_cfg = 4772727;

    FILE* config = fopen(av[1],"r");
    fscanf(config,"isa1=%s\n",isa1);
    fscanf(config,"machine=%s\n",machine);
    fscanf(config,"cpuclock=%d\n",cpuclock_cfg);
    fscanf(config,"flop1=%s\n",flop1);

    cpuclock = attotime::from_hz(cpuclock_cfg);
    
    FILE* mda = fopen("roms/video/mda/mda.rom","rb");
    fread(MDA::ROM,1,0x2000,mda);
    fseek(mda,0,SEEK_SET);
    fread(CGA::ROM,1,0x2000,mda);
    fclose(mda);

    std::string isa1slot = isa1;
    delete[] isa1;

    std::string machinetype = machine;
    delete[] machine;

    INTERFACE::init();
    
    INTERFACE::load_floppy(flop1);

    if(isa1slot == "mda")
    {
        MDA::init();
        IO_XT::handlers.push_back(MDA::mdacrtc);
    }
    if(isa1slot == "cga")
    {
        IO_XT::handlers.push_back(CGA::cgacrtc);
    }

    if(machinetype == "ibm5150")
    {
        CPU::type = CPU::intel8088;
        pitclock = attotime::from_hz(157500000/11/12);
        FILE* biosfp = fopen("roms/machines/ibmpc/pc102782.bin", "rb");
        fread(RAM::BIOS + 0x1e000,0x2000,1,biosfp);
        fclose(biosfp);
        biosfp = fopen("roms/machines/ibmpc/ibm-basic-1.10.rom", "rb");
        fread(RAM::BIOS + 0x16000,0x8000,1,biosfp);
        fclose(biosfp);

        RAM::handlers.push_back(RAM::bios_handler);
        IO_XT::handlers.push_back(DMA_XT::handler);
        IO_XT::handlers.push_back(DMA_XT::handler2);
        IO_XT::handlers.push_back(PPI::handler);
        IO_XT::handlers.push_back(PIT::pit);
        IO_XT::handlers.push_back(PIC::pic1);
        IO_XT::handlers.push_back(FDC::handler);
    }

    if(machinetype == "test")
    {
        CPU::type = CPU::intel8088;
        pitclock = attotime::from_hz(157500000/11/12);
        char* testrom = new char[256];
        fscanf(config,"testrom=%s\n",testrom);
        FILE* testromfp = fopen(testrom, "rb");
        fread(RAM::BIOS + 0x1e000,0x2000,1,testromfp);
        fclose(testromfp);

        RAM::handlers.push_back(RAM::bios_handler);
        IO_XT::handlers.push_back(DMA_XT::handler);
        IO_XT::handlers.push_back(DMA_XT::handler2);
        IO_XT::handlers.push_back(PPI::handler);
        IO_XT::handlers.push_back(PIT::pit);
        IO_XT::handlers.push_back(PIC::pic1);
        IO_XT::handlers.push_back(FDC::handler);
    }

    fclose(config);

    bool quit = false;
    int i = 0;

    FILE* fp = fopen("save/mem.dump","rb");
    if(fp != NULL)
    {
        fread(RAM::RAM,0x100000,1,fp);
        fclose(fp);
    }
    fp = fopen("save/reg.dump","rb");
    if(fp != NULL)
    {
        fread(&CPU::ax,2,1,fp);
        fread(&CPU::bx,2,1,fp);
        fread(&CPU::cx,2,1,fp);
        fread(&CPU::dx,2,1,fp);
        fread(&CPU::si,2,1,fp);
        fread(&CPU::di,2,1,fp);
        fread(&CPU::sp,2,1,fp);
        fread(&CPU::bp,2,1,fp);
        fread(&CPU::ip,2,1,fp);
        fread(&CPU::cs,2,1,fp);
        fread(&CPU::ds,2,1,fp);
        fread(&CPU::es,2,1,fp);
        fread(&CPU::ss,2,1,fp);
        fread(&CPU::flags,2,1,fp);
        fclose(fp);
    }
    
    fp = fopen("save/mda.dump","rb");
    if(fp != NULL)
    {
        fread(&MDA::hdisp,1,1,fp);
        fread(&MDA::vdisp,1,1,fp);
        fread(&MDA::maxscan,1,1,fp);
        fread(&MDA::dispmode,1,1,fp); 
        fclose(fp);
    }
    
    fp = fopen("save/cga.dump","rb");
    if(fp != NULL)
    {
        fread(&CGA::hdisp,1,1,fp);
        fread(&CGA::vdisp,1,1,fp);
        fread(&CGA::maxscan,1,1,fp);
        fread(&CGA::dispmode,1,1,fp); 
        fclose(fp);
    }
    
    fp = fopen("save/pic.dump","rb");
    if(fp != NULL)
    {
        fread(&PIC::pic[0].intrmask,1,1,fp);
        fread(&PIC::pic[0].offset,1,1,fp);
        fread(&PIC::pic[0].enabled,sizeof(bool),1,fp);
        fclose(fp);
    }
    
    bool debugsaved = false;

    while(INTERFACE::quitflag == false)
    {
        /*if(i==100)
        {
            i = 0;
            if(isa1slot == "mda") MDA::tick_frame();
            if(isa1slot == "cga") CGA::tick_frame();
            INTERFACE::update_screen();
        }*/
        
        fflush(stdout);

        //TODO: remove SDL_* prefix
        //INTERFACE::handle_events();
        
        CPU::tick();
        
        if(CPU::hint == true) CPU::hint = false;
        
        current_time += cpuclock;
        frame_time += cpuclock;
        if(current_time >= pitclock)
        {
            PIT::tick();
            current_time -= pitclock;
        }
        if(frame_time >= attotime::from_hz(50) && isa1slot == "mda")
        {
            MDA::tick_frame();
            frame_time -= attotime::from_hz(50);
            INTERFACE::update_screen();
        }
        else if(frame_time >= attotime::from_hz(60) && isa1slot == "cga")
        {
            CGA::tick_frame();
            frame_time -= attotime::from_hz(60);
            INTERFACE::update_screen();
        }
    }

    INTERFACE::quit();

    return 0;
}
