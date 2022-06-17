#include "log/aimy-log.h"
#include "core/core-include.h"
#include "imp/logo-progress/logo-progress.h"
#include "imp/localCommandlineTool.h"
#include "third_party/json/cjson-interface.h"
#include <SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<SDL2/SDL_ttf.h>
#include<unistd.h>
using namespace aimy;
using commandLineTestTool=aimy::Local::localCommandLineTestTool;
using ExternalParamList=aimy::Local::ExternalParamList;
static std::string config_file_path="/var/logo/logo_render.json";
static std::string font_path="/var/logo/SourceHanSansCN-Normal-mini.otf";
static int green=50;
static int red=50;
static int alpha=55;
static int blue=50;
static bool is_debug=false;
static const char * const debug_check_path="/.build/debug";

static void load_env();
static void load_config(LogoProgressParam &param);
static std::string read_ip_info();
static bool check_screen(std::list<ScreenPhysicalParam>&displayList);
static int test_displayinfo();
static int test_load_img(int argc,char *argv[]);
int main(int argc,char * argv[])
{
    //return test_load_img(argc,argv);
    load_env();
    AimyLogger::Instance().register_handle();
    if(is_debug)
    {
        AimyLogger::Instance().set_log_to_std(true);
    }
    else {
        AimyLogger::Instance().set_log_to_std(false);
    }
    if(argc>1)
    {
        aimy::AimyLogger::Instance().set_minimum_log_level(LOG_WARNNING);
        commandLineTestTool tool;
        tool.handleCommandlineCmd(argc,argv);
        tool.start();
        tool.waitDone();
        aimy::AimyLogger::Instance().unregister_handle();
        return 0;
    }
    test_displayinfo();
    LogoProgressParam param;
    load_config(param);
    aimy::LogoProgress progress(param);
    progress.Start();
    bool is_exit=false;
    AimyLogger::Instance().register_exit_signal_func([&](){
        is_exit=true;
        progress.Release();
        aimy::AimyLogger::Instance().unregister_handle();
        _Exit(-1);
    });
    aimy::AimyLogger::Instance().set_minimum_log_level(LOG_DEBUG);
    commandLineTestTool tool;
    argc=2;
    char opt[]="server";
    char *argv2[]={argv[0],opt};
    tool.handleCommandlineCmd(argc,argv2);
    tool.insertCallback("release","exit logo-progress",[&](const ExternalParamList&paramlist)->std::string{
        (void)paramlist;
        is_exit=true;
        progress.Release();
        aimy::AimyLogger::Instance().unregister_handle();
        return "ok";
    },0);
    tool.insertCallback("setProgress","progress[int] total[int]",[&](const ExternalParamList&paramlist)->std::string{
        auto iter=paramlist.begin();
        std::string item=std::string(reinterpret_cast<const char *>(iter->first.get()),iter->second);
        int progress_now=std::stoi(item);
        ++iter;
        item=std::string(reinterpret_cast<const char *>(iter->first.get()),iter->second);
        int total=std::stoi(item);
        progress.SetProgress(progress_now,total);
        return "ok";
    },2);
    tool.insertCallback("setDisplayStatus","status[int] zero means clear and disable display",[&](const ExternalParamList&paramlist)->std::string{
        auto iter=paramlist.begin();
        std::string item=std::string(reinterpret_cast<const char *>(iter->first.get()),iter->second);
        int status=std::stoi(item);
        progress.setDisplayStatus(status!=0);
        return "ok";
    },1);
    tool.start();
    std::list<ScreenPhysicalParam> cache;
    while (!is_exit) {
        //check screen info
        if(check_screen(cache))
        {
            for(auto &i :cache)
            {
                progress.Reinit(i,cache.size());
            }
        }
        //read ip config
        if(is_debug)
        {
            progress.SetIpInfo(read_ip_info());
        }
        usleep(40*1000);
    }
    aimy::AimyLogger::Instance().unregister_handle();
    progress.Release();
    return 0;
}

void load_env()
{
    auto p=getenv("LOGO_CONFIG_FILE_PATH");
    if(p&&access(p,R_OK)==0)
    {
        config_file_path=std::string(p);
    }
    //
    p=getenv("LOGO_FONT_PATH");
    if(p&&access(p,R_OK)==0)
    {
        font_path=std::string(p);
    }
    //
    p=getenv("LOGO_COLOR_GREEN");
    if(p)
    {
        int data=std::stoi(p);
        if(data>=0&&data<=255)
        {
            green=data;
        }
    }
    //
    p=getenv("LOGO_COLOR_RED");
    if(p)
    {
        int data=std::stoi(p);
        if(data>=0&&data<=255)
        {
            red=data;
        }
    }
    //
    p=getenv("LOGO_COLOR_BLUE");
    if(p)
    {
        int data=std::stoi(p);
        if(data>=0&&data<=255)
        {
            blue=data;
        }
    }
    //
    p=getenv("LOGO_COLOR_ALPHA");
    if(p)
    {
        int data=std::stoi(p);
        if(data>=0&&data<=255)
        {
            alpha=data;
        }
    }
    //
    if(access(debug_check_path,F_OK)==0)
    {
        is_debug=true;
    }
    p=getenv("LOGO_DEBUG_MODE");
    if(p)
    {
        int data=std::stoi(p);
        if(data!=0)
        {
            is_debug=true;
        }
    }
}

void load_config(LogoProgressParam &param)
{
    param.red=red;
    param.blue=blue;
    param.alpha=alpha;
    param.green=green;
    param.fontPath=font_path;
    param.is_debug=is_debug;
  neb::CJsonObject *object=neb::CJsonObject::CreateInstance(config_file_path);
  if(object->IsEmpty()||!object->IsArray())
  {
      LogoProgressScreenConfig config1;
      config1.index=0;
      config1.min_avaiable_indexs=2;
      config1.flip_type=0;//1
      config1.flip_angle=0;//180
      config1.logo_path="/userdata/aimy/bootstraps/logo/logo.bmp";
      config1.default_logo_path="/var/logo/logo.bmp";
      config1.is_primary=true;
      LogoProgressScreenConfig config2;
      config2.index=1;
      config2.min_avaiable_indexs=2;
      config2.flip_type=0;
      config2.flip_angle=0;
      config2.logo_path="/userdata/aimy/bootstraps/logo/logo.bmp";
      config2.default_logo_path="/var/logo/logo.bmp";
      config2.is_primary=true;
      param.screensConfig.push_back(config1);
      param.screensConfig.push_back(config2);

      neb::CJsonObject item1;
      item1.Add("index",config1.index);
      item1.Add("min_avaiable_indexs",config1.min_avaiable_indexs);
      item1.Add("flip_type",config1.flip_type);
      item1.Add("flip_angle",config1.flip_angle);
      item1.Add("logo_path",config1.logo_path);
      item1.Add("default_logo_path",config1.default_logo_path);
      item1.Add("is_primary",config1.is_primary,config2.is_primary);
      object->Add(item1);

      neb::CJsonObject item2;
      item2.Add("index",config2.index);
      item2.Add("min_avaiable_indexs",config2.min_avaiable_indexs);
      item2.Add("flip_type",config2.flip_type);
      item2.Add("flip_angle",config2.flip_angle);
      item2.Add("logo_path",config2.logo_path);
      item2.Add("default_logo_path",config2.default_logo_path);
      item2.Add("is_primary",config2.is_primary,config2.is_primary);
      object->Add(item2);
      object->SetSavePath(config_file_path);
      object->SaveToFile();
  }
  else {
      int size=object->GetArraySize();
      for(auto i=0;i<size;++i)
      {
          LogoProgressScreenConfig config;
          config.index=i;
          config.min_avaiable_indexs=1;
          config.flip_type=0;
          config.flip_angle=0;
          config.logo_path="/userdata/aimy/bootstraps/logo/logo.bmp";
          config.default_logo_path="/var/logo/logo.bmp";
          config.is_primary=true;
          neb::CJsonObject tmp;
          object->Get(i,tmp);
          if(!tmp.Get("index",config.index))
          {
              AIMY_ERROR("logoProgress parse failed! miss index item!");
          }
          if(!tmp.Get("min_avaiable_indexs",config.min_avaiable_indexs))
          {
              AIMY_ERROR("logoProgress parse failed! miss min_avaiable_indexs item!");
          }
          if(!tmp.Get("flip_type",config.flip_type))
          {
              AIMY_ERROR("logoProgress parse failed! miss flip_type item!");
          }
          if(!tmp.Get("flip_angle",config.flip_angle))
          {
              AIMY_ERROR("logoProgress parse failed! miss flip_angle item!");
          }
          if(!tmp.Get("logo_path",config.logo_path))
          {
              AIMY_ERROR("logoProgress parse failed! miss logo_path item!");
          }
          if(!tmp.Get("default_logo_path",config.default_logo_path))
          {
              AIMY_ERROR("logoProgress parse failed! miss default_logo_path item!");
          }
          if(!tmp.Get("is_primary",config.is_primary))
          {
              AIMY_ERROR("logoProgress parse failed! miss is_primary item!");
          }
          param.screensConfig.push_back(config);
      }
      AIMY_INFO("logoProgress load config success");
  }
}

std::string read_ip_info()
{
    std::string ip;
    auto network_info=NETWORK_UTIL::readNetworkInfo();
    for(auto i :network_info)
    {
        if(!ip.empty())ip+="/";
        ip+=i.ip;
    }
    return ip;
}

bool check_screen(std::list<ScreenPhysicalParam>&displayList)
{//适配1920×1080的分辨率
    std::list<ScreenPhysicalParam> temp_list;
    int displays=SDL_GetNumVideoDisplays();
    if(displays<=0)
    {
        AIMY_ERROR("logoPreogress SDL_GetNumVideoDisplays failed [%s]!",SDL_GetError());
    }
    for(int i =0;i<displays;++i)
    {
        ScreenPhysicalParam screen_param;
        SDL_GetCurrentDisplayMode(i,&screen_param.mode);
        screen_param.mode.w=1920;
        screen_param.mode.h=1080;
        int display_modes=SDL_GetNumDisplayModes(i);
        if(display_modes<=0)
        {
            AIMY_ERROR("logoPreogress SDL_GetNumDisplayModes failed [%s]!",SDL_GetError());
        }
        for(auto j=0;j<display_modes;++j)
        {
            SDL_DisplayMode mode;
            SDL_GetDisplayMode(i, j,&mode);
            if(mode.w>1920)continue;
            if(mode.refresh_rate>60)continue;
            screen_param.mode=mode;
            break;
        }
        SDL_DisplayMode tmp;
        if(0==SDL_GetClosestDisplayMode(i,&screen_param.mode,&tmp))
        {
            screen_param.mode=tmp;
        }
        screen_param.index=i;
        screen_param.name=SDL_GetDisplayName(i);
        float &ddpi=screen_param.ddpi;
        float  &hdpi=screen_param.hdpi;
        float  &vdpi=screen_param.vdpi;
        SDL_GetDisplayDPI(i, &ddpi, & hdpi, & vdpi);
        temp_list.push_back(screen_param);
    }
    bool not_found=false;
    for(auto iter=temp_list.begin();iter!=temp_list.end();++iter)
    {
        not_found=true;
        for(auto old_iter=displayList.begin();old_iter!=displayList.end();++old_iter)
        {
            if(iter->name==old_iter->name)
            {
                not_found=false;
                break;
            }
        }
        if(not_found)break;
    }
    std::swap(temp_list,displayList);
    return not_found;
}
static void
print_mode(const char *prefix, const SDL_DisplayMode *mode)
{
    if (!mode)
        return;

    AIMY_DEBUG("%s: fmt=%s w=%d h=%d refresh=%d",
            prefix, SDL_GetPixelFormatName(mode->format),
            mode->w, mode->h, mode->refresh_rate);
}
int test_displayinfo()
{
    SDL_DisplayMode mode;
    int num_displays, dpy;

    /* Load the SDL library */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        AIMY_ERROR( "Couldn't initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    AIMY_DEBUG("Using video target '%s'.", SDL_GetCurrentVideoDriver());
    num_displays = SDL_GetNumVideoDisplays();

    AIMY_DEBUG("See %d displays.", num_displays);

    for (dpy = 0; dpy < num_displays; dpy++) {
        const int num_modes = SDL_GetNumDisplayModes(dpy);
        SDL_Rect rect = { 0, 0, 0, 0 };
        float ddpi, hdpi, vdpi;
        int m;

        SDL_GetDisplayBounds(dpy, &rect);
        AIMY_DEBUG("%d: \"%s\" (%dx%d, (%d, %d)), %d modes.", dpy, SDL_GetDisplayName(dpy), rect.w, rect.h, rect.x, rect.y, num_modes);

        if (SDL_GetDisplayDPI(dpy, &ddpi, &hdpi, &vdpi) == -1) {
            AIMY_ERROR( "    DPI: failed to query (%s)", SDL_GetError());
        } else {
            AIMY_DEBUG("    DPI: ddpi=%f; hdpi=%f; vdpi=%f", ddpi, hdpi, vdpi);
        }

        if (SDL_GetCurrentDisplayMode(dpy, &mode) == -1) {
            AIMY_ERROR( "    CURRENT: failed to query (%s)", SDL_GetError());
        } else {
            print_mode("CURRENT", &mode);
        }

        if (SDL_GetDesktopDisplayMode(dpy, &mode) == -1) {
            AIMY_ERROR( "    DESKTOP: failed to query (%s)", SDL_GetError());
        } else {
            print_mode("DESKTOP", &mode);
        }

        for (m = 0; m < num_modes; m++) {
            if (SDL_GetDisplayMode(dpy, m, &mode) == -1) {
                AIMY_ERROR( "    MODE %d: failed to query (%s)", m, SDL_GetError());
            } else {
                char prefix[64];
                SDL_snprintf(prefix, sizeof (prefix), "    MODE %d", m);
                print_mode(prefix, &mode);
            }
        }

        AIMY_DEBUG("");
    }

    SDL_Quit();
    return 0;
}
int test_load_img(int argc,char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF|IMG_INIT_WEBP);
    auto window=SDL_CreateWindow("logo",
                                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080, 0);
    auto renderer=SDL_CreateRenderer(window, -1, 0);
    auto logoImageSurf=IMG_Load(argv[1]);
    auto logoImageTexture=SDL_CreateTextureFromSurface(renderer,logoImageSurf);
    SDL_RenderCopyEx(renderer,logoImageTexture,NULL,NULL,0,NULL,static_cast<SDL_RendererFlip>(0));
    SDL_RenderPresent(renderer);
    sleep(2);
    SDL_DestroyTexture(logoImageTexture);
    SDL_FreeSurface(logoImageSurf);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}
