#include "logo-progress.h"
#ifdef USE_SDL
using namespace aimy;
LogoProgress::LogoProgress(const LogoProgressParam &param):running(false),enableShow(true),renderThread(nullptr),configParam(param)
{
    if(0!=SDL_Init(SDL_INIT_VIDEO))
    {
        AIMY_ERROR("logoPreogress SDL_Init failed [%s]!",SDL_GetError());
        _Exit(-1);
    }
    IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF|IMG_INIT_WEBP);
    TTF_Init();
}

LogoProgress::~LogoProgress()
{
    Release();
    TTF_Quit();
    //退出image库
    IMG_Quit();
    //退出SDL
    SDL_Quit();
}

void LogoProgress::Start()
{
    if(running)
    {
        AIMY_WARNNING("logoProgress is already started!");
        return;
    }
    running.exchange(true);
    renderThread=std::make_shared<std::thread>([this](){
        this->renderTask();
    });
}

void LogoProgress::Release()
{
    if(!running)
    {
        AIMY_WARNNING("logoProgress is not running!");
        return;
    }
    running.exchange(false);
    {
        std::lock_guard<std::mutex>locker(dataMutex);
        cv.notify_one();
    }
    if(renderThread)
    {
        if(renderThread->joinable())renderThread->join();
        renderThread.reset();
    }
}

void LogoProgress::Reinit(const ScreenPhysicalParam &param, int display_indexs)
{
    std::lock_guard<std::mutex>locker(dataMutex);
    taskQueue.push_back([=](){
        auto iter=renderInfo.begin();
        while(iter!=renderInfo.end())
        {
            if(iter->config.index==param.index)break;
            ++iter;
        }
        if(iter->config.min_avaiable_indexs>display_indexs)
        {
            AIMY_ERROR("logoPreogress need %d displays now have %d,wait!",iter->config.min_avaiable_indexs,display_indexs);
            return ;
        }
        if(iter!=renderInfo.end())
        {
            iter->Release();
            iter->param=param;
            //
            iter->window=SDL_CreateWindow("logo",
                                          SDL_WINDOWPOS_UNDEFINED_DISPLAY(iter->param.index), SDL_WINDOWPOS_UNDEFINED_DISPLAY(iter->param.index), 1920, 1080, 0);
            if(!iter->window)
            {
                AIMY_ERROR("logoPreogress SDL_CreateWindow failed [%s]!",SDL_GetError());
                _Exit(-1);
                return ;
            }
            if(0!=SDL_SetWindowDisplayMode(iter->window,&iter->param.mode))
            {
                AIMY_ERROR("logoPreogress SDL_SetWindowDisplayMode failed [%s]!",SDL_GetError());
            }
            SDL_Rect &rect=iter->param.rect;
            if(0!=SDL_GetDisplayBounds(iter->param.index,&rect))
            {
                AIMY_ERROR("logoPreogress SDL_GetDisplayBounds failed [%s]!",SDL_GetError());
            }
            SDL_SetWindowPosition( iter->window, rect.x
                                   , 0);
            float &ddpi=iter->param.ddpi;
            float  &hdpi=iter->param.hdpi;
            float  &vdpi=iter->param.vdpi;
            if(0!=SDL_GetDisplayDPI(iter->param.index, &ddpi, & hdpi, & vdpi))
            {
                iter->param.ddpi=80.0;
                iter->param.hdpi=80.0;
                iter->param.vdpi=80.0;
                AIMY_ERROR("logoPreogress SDL_GetDisplayDPI failed [%s]!",SDL_GetError());
            }
            //
            iter->renderer=SDL_CreateRenderer(iter->window, -1, 0);
            if(!iter->renderer)
            {
                AIMY_ERROR("logoPreogress SDL_CreateRenderer failed [%s]!",SDL_GetError());
                return;
            }
            //
            if(!iter->config.logo_path.empty())
            {
                iter->logoImageSurf=IMG_Load(iter->config.logo_path.c_str());
                if(!iter->logoImageSurf)
                {
                    AIMY_ERROR("logoPreogress IMG_Load failed [%s]!",SDL_GetError());
                }
                else {
                    iter->logoImageTexture=SDL_CreateTextureFromSurface(iter->renderer, iter->logoImageSurf);
                    if(!iter->logoImageTexture)
                    {
                        AIMY_ERROR("logoPreogress SDL_CreateTextureFromSurface failed [%s]!",SDL_GetError());
                    }
                }
                AIMY_DEBUG("logoProgress  %s load %s success!",param.name.c_str(),iter->config.logo_path.c_str());
            }
            else {
                AIMY_WARNNING("logoProgress doesn't set logo path for %s!",param.name.c_str());
            }
            if(!iter->logoImageTexture&&!iter->config.default_logo_path.empty())
            {
                iter->logoImageSurf=IMG_Load(iter->config.default_logo_path.c_str());
                if(!iter->logoImageSurf)
                {
                    AIMY_ERROR("logoPreogress IMG_Load failed [%s]!",SDL_GetError());
                }
                else {
                    iter->logoImageTexture=SDL_CreateTextureFromSurface(iter->renderer, iter->logoImageSurf);
                    if(!iter->logoImageTexture)
                    {
                        AIMY_ERROR("logoPreogress SDL_CreateTextureFromSurface failed [%s]!",SDL_GetError());
                    }
                }
                AIMY_DEBUG("logoProgress  %s load %s success!",param.name.c_str(),iter->config.default_logo_path.c_str());
            }
            //
            if(!configParam.fontPath.empty())
            {
                iter->font = TTF_OpenFont(configParam.fontPath.c_str(), 12);
                if(!iter->font)
                {
                    AIMY_ERROR("logoProgress open font %s failed!",configParam.fontPath.c_str());
                }
            }
            else {
                AIMY_WARNNING("logoProgress doesn't set font path for %s!",param.name.c_str());
            }
            AIMY_DEBUG("logoProgress init %d %s %d %d %d success!",iter->param.index,iter->param.name.c_str(),iter->param.rect.x,
                       iter->param.rect.w,iter->param.rect.h);
        }

    });
    cv.notify_one();
}

void LogoProgress::SetIpInfo(std::string ipInfo)
{
    std::lock_guard<std::mutex>locker(dataMutex);
    taskQueue.push_back([=](){
        if(!configParam.is_debug)return ;
        for(auto &i :renderInfo)
        {
            if(!i.config.is_primary||!i.window||!i.font)continue;
            i.ReleaseIpSrc();
            SDL_Color color = { 255, 255, 255, 0};
            i.ipSurf = TTF_RenderText_Blended(i.font, ipInfo.c_str(), color);//绘制文字到图片
            if(!i.ipSurf)
            {
                //AIMY_ERROR("logoPreogress TTF_RenderText_Blended failed [%s]!",SDL_GetError());
            }
            i.ipTexture= SDL_CreateTextureFromSurface(i.renderer, i.ipSurf);
            if(!i.ipTexture)
            {
                //AIMY_ERROR("logoPreogress SDL_CreateTextureFromSurface failed [%s]!",SDL_GetError());
            }
        }
    });
    cv.notify_one();
}

void LogoProgress::SetProgress(int progress,int total)
{
    std::lock_guard<std::mutex>locker(dataMutex);
    taskQueue.push_back([=](){
        for(auto &i :renderInfo)
        {
            if(!i.config.is_primary||!i.window)continue;
            i.total=total;
            i.now=progress;
        }
    });
    cv.notify_one();
}

void LogoProgress::setDisplayStatus(bool enable)
{
    std::lock_guard<std::mutex>locker(dataMutex);
    taskQueue.push_back([=](){
        enableShow.exchange(enable);
        if(!enable)
        {
            for(auto & i : renderInfo)
            {
                if(!i.window||!i.renderer)continue;
                if(0!=SDL_RenderClear(i.renderer))
                {
                    AIMY_ERROR("logoPreogress SDL_RenderClear failed [%s]!",SDL_GetError());
                }
                if(0!=SDL_SetRenderDrawColor(i.renderer, 0, 0, 0, 0))
                {
                    AIMY_ERROR("logoPreogress SDL_SetRenderDrawColor failed [%s]!",SDL_GetError());
                }
                int w=0,h=0;
                SDL_GetRendererOutputSize(i.renderer,&w,&h);
                printf("%d %d ------------\r\n",w,h);
                if(w<=0)w=1920;
                if(h<=0)h=1080;
                SDL_Rect sdl_rect = { 0,0,w,h };
                if(0!=SDL_RenderFillRect(i.renderer, &sdl_rect))
                {
                    AIMY_ERROR("logoPreogress SDL_RenderFillRect failed [%s]!",SDL_GetError());
                }
                SDL_RenderPresent(i.renderer);
            }
        }
    });
    cv.notify_one();
}

void LogoProgress::renderTask()
{
    AIMY_INFO("logoProgress renderTask start");
    for(auto &i: renderInfo)
    {
        i.Release();
    }
    renderInfo.clear();

    for(auto &i : configParam.screensConfig)
    {
        screenRenderInfo info(i);
        renderInfo.push_back(info);
    }
    SDL_ShowCursor(false);
    std::unique_lock<std::mutex>locker(dataMutex);
    while(running)
    {
        if(taskQueue.empty())
        {
            cv.wait_for(locker,std::chrono::milliseconds(40));
        }
        //handle task
        if(!taskQueue.empty())
        {
            auto task=taskQueue.front();
            taskQueue.pop_front();
            if(locker.owns_lock())locker.unlock();
            task();
        }
        if(locker.owns_lock())locker.unlock();
        //render
        for(auto & i : renderInfo)
        {
            if(!enableShow||!i.window||!i.renderer)continue;
            if(0!=SDL_RenderClear(i.renderer))
            {
                AIMY_ERROR("logoPreogress SDL_RenderClear failed [%s]!",SDL_GetError());
            }
            if(i.logoImageTexture)
            {
                if(0!=SDL_RenderCopyEx(i.renderer,i.logoImageTexture,NULL,NULL,i.config.flip_angle,NULL,static_cast<SDL_RendererFlip>(i.config.flip_type)))
                {
                    AIMY_ERROR("logoPreogress SDL_RenderCopyEx failed [%s]!",SDL_GetError());
                }
            }
            if(i.config.is_primary)
            {
                //render ip
                if(configParam.is_debug&&i.ipTexture)
                {
                    int render_width=std::ceil(i.param.hdpi*2.2);
                    int render_height=std::ceil(i.param.vdpi*0.2);
                    render_width=render_width<=0?100:render_width;
                    render_height=render_height<=0?30:render_height;

                    SDL_Rect sdlRect={0,0,render_width,render_height};
                    if(0!=SDL_RenderCopyEx(i.renderer,i.ipTexture,NULL,&sdlRect,i.config.flip_angle,NULL,static_cast<SDL_RendererFlip>(i.config.flip_type)))
                    {
                        AIMY_ERROR("logoPreogress SDL_RenderCopyEx failed [%s]!",SDL_GetError());
                    }
                }
                //render progress
                if(i.total>0&&i.now>0)
                {
                    if(0!=SDL_SetRenderDrawColor(i.renderer, configParam.red, configParam.green, configParam.blue, configParam.alpha))
                    {
                        AIMY_ERROR("logoPreogress SDL_SetRenderDrawColor failed [%s]!",SDL_GetError());
                    }
                    int render_height=std::ceil(i.param.vdpi*0.012);
                    int render_gap=std::ceil(i.param.hdpi*0.02);
                    render_height=render_height<=0?2:render_height;
                    render_gap=render_gap<=0?2:render_gap;

                    int area_width=i.param.rect.w/i.total;
                    if(render_gap>=area_width)
                    {
                        render_gap=2;
                        area_width=4;
                    }
                    for(int index=0;index<i.now;++index)
                    {
                        if((index+1)*area_width<=i.param.rect.w+render_gap)
                        {

                            SDL_Rect sdl_rect = { index*area_width,i.param.rect.h-render_height,area_width-render_gap,render_height };
                            if(0!=SDL_RenderFillRect(i.renderer, &sdl_rect))
                            {
                                AIMY_ERROR("logoPreogress SDL_RenderFillRect failed [%s]!",SDL_GetError());
                            }
                        }
                    }
                }
            }
            SDL_RenderPresent(i.renderer);
        }
        if(!locker.owns_lock())locker.lock();
    }
    for(auto &i: renderInfo)
    {
        i.Release();
    }
    renderInfo.clear();
    AIMY_INFO("logoProgress renderTask stop");
}
#endif
