#ifndef LOGOPROGRESS_H
#define LOGOPROGRESS_H
#include "log/aimy-log.h"
#ifdef USE_SDL
#include <SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<SDL2/SDL_ttf.h>
#include<vector>
#include<list>
#define DEFAULT_FONT_PATH "SourceHanSansCN-Normal-mini.otf"
namespace aimy {
//1 inch = 2.54 cm
struct LogoProgressScreenConfig
{
    int index; //屏幕名
    int min_avaiable_indexs;//可用屏幕数
    bool is_primary;//是否是主屏 主屏才显示进度进ip
    std::string logo_path;//logo路径
    std::string default_logo_path;//默认logo路径
    int flip_type;//翻转类型
    double flip_angle;//翻转角度
    LogoProgressScreenConfig():index(0),min_avaiable_indexs(1),is_primary(false),logo_path("")
      ,flip_type(0),flip_angle(0.0f)
    {

    }
};

struct LogoProgressParam
{
    bool is_debug;//是否开启调试模式
    int red;//
    int green;//
    int blue;//
    int alpha;//
    std::string fontPath;
    std::vector<LogoProgressScreenConfig>screensConfig;
    LogoProgressParam():is_debug(false)
      ,red(255),green(0),blue(0),alpha(0),fontPath(DEFAULT_FONT_PATH)
    {}
};

struct ScreenPhysicalParam
{
    int index;//
    int avaiable_indexs;
    std::string name;//名称
    SDL_Rect rect;//屏幕详细信息
    float ddpi;//对角线
    float hdpi;//水平
    float vdpi;//垂直方向
    SDL_DisplayMode mode;//显示模式
    ScreenPhysicalParam():index(0),avaiable_indexs(1),name(""),ddpi(0.0f),hdpi(0.0f),vdpi(0.0f)
    {

    }
};
class LogoProgress final{
    struct screenRenderInfo{
        const LogoProgressScreenConfig config;
        ScreenPhysicalParam param;
        SDL_Window * window;
        SDL_Renderer *renderer;
        //logo
        SDL_Surface * logoImageSurf;
        SDL_Texture * logoImageTexture;
        //ip
        std::string ipInfo;//含有多个ip时，使用/做分隔符
        //显示 长：2.2英寸 宽 0.2英寸
        TTF_Font* font;//使用12号字体显示
        SDL_Surface* ipSurf;
        SDL_Texture * ipTexture;
        //progress
        //进度条宽 0.02英寸 进度条间隔0.02英寸
        int total;
        int now;

        screenRenderInfo(const LogoProgressScreenConfig &_config):config(_config),window(nullptr),renderer(nullptr)
          ,logoImageSurf(nullptr),logoImageTexture(nullptr),ipInfo(""),font(nullptr),ipSurf(nullptr),ipTexture(nullptr)
          ,total(0),now(0)
        {

        }

        ~ screenRenderInfo()
        {
            Release();
        }

        void Release(){
            ReleaseIpSrc();
            if(font)
            {
                TTF_CloseFont(font);
                font=nullptr;
            }
            if(logoImageTexture)
            {
                SDL_DestroyTexture(logoImageTexture);
                logoImageTexture=nullptr;
            }
            if(logoImageSurf)
            {
                SDL_FreeSurface(logoImageSurf);
                logoImageSurf=nullptr;
            }
            if(renderer)
            {
                SDL_DestroyRenderer(renderer);
                renderer=nullptr;
            }
            if(window)
            {
                SDL_DestroyWindow(window);
                window=nullptr;
            }

        }

        void ReleaseIpSrc(){
            if(ipTexture)
            {
                SDL_DestroyTexture(ipTexture);
                ipTexture=nullptr;
            }
            if(ipSurf)
            {
                SDL_FreeSurface(ipSurf);
                ipSurf=nullptr;
            }
        }

    };
public:
    LogoProgress(const LogoProgressParam &param);
    ~LogoProgress();
    void Start();
    void Release();
    void Reinit(const ScreenPhysicalParam &param,int display_indexs=1);
    void SetIpInfo(std::string ipInfo);
    void SetProgress(int progress,int total);
    void setDisplayStatus(bool enable);
private:
    void renderTask();
private:
    std::atomic<bool> running;
    std::atomic<bool> enableShow;
    std::condition_variable cv;
    std::mutex dataMutex;
    std::list<std::function<void()>> taskQueue;
    std::shared_ptr<std::thread>renderThread;
    const LogoProgressParam configParam;
    std::list<screenRenderInfo> renderInfo;

};
}
#endif
#endif // LOGOPROGRESS_H
