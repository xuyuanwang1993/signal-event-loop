#include "log/aimy-log.h"
#include "core/core-include.h"
#include "third_party/json/cjson-interface.h"
#include "imp/utils/common_utils.h"
using namespace aimy;
static int object_test(int argc,char *argv[]);
static int loop_test(int argc,char *argv[]);
static int object_test2(int argc ,char *argv[]);
static int json_test(int argc ,char *argv[]);
int main(int argc,char *argv[])
{
    (void)argc;
    (void)argv;
    aimy::AimyLogger::Instance().register_handle();
    //aimy::AimyLogger::Instance().set_log_path("/userdata/aimy/logs/test","test");
    aimy::AimyLogger::Instance().set_log_to_std(true);
    atexit([](){
        aimy::AimyLogger::Instance().unregister_handle();
    });
    if(!AIMY_UTILS::acquireSigleInstanceLcok())
    {
        AIMY_ERROR("is running exit!");
        return -1;
    }
    while(1)
    {
        sleep(1);
    }
    return  json_test(argc,argv);
    return  object_test2(argc,argv);
    //return object_test(argc,argv);
    return loop_test(argc,argv);
}
int object_test(int argc,char *argv[])
{
    class test_loop:public Athread{
    public:
        void exec()override{
            while(isExecing)
            {
                handleEvent(0);
                auto time_out=getNextTimeOut();
                if(time_out==-1)time_out=5000000;
                std::unique_lock<std::mutex>locker(wakeup_mutex);
                wakeup_cv.wait_for(locker,std::chrono::microseconds(time_out));
            }
        }
        void echo()
        {
            AIMY_INFO("echo");
        }
        test_loop(std::string name):Athread(name){}
    protected:
        void wakeup()override
        {
            std::lock_guard<std::mutex>locker(wakeup_mutex);
            wakeup_cv.notify_one();
        }
        std::mutex wakeup_mutex;
        std::condition_variable wakeup_cv;
    };
    class test_object:public Object{
    public:
        Signal<>echo;
        test_object(Object*parent):Object(parent),echo(this){}
        void print_info()
        {
            AIMY_DEBUG("test_obj");
            echo();
        }
    };

    test_loop l1("l1");
    l1.startThread();
    auto timer=l1.addTimer(100);
    auto timer2=l1.addTimer(10);
    timer2->start();
    auto con=timer2->timeout.connectFunc([](){
        AIMY_ERROR("timer2 timeout");
    });
    auto timer3=l1.addTimer(1);
    timer3->timeout.connectFunc([](){
        AIMY_ERROR("timer3 timeout");
    });
    timer3->start();
    test_object *obj=new test_object(&l1);
    obj->echo.connect(&l1,std::bind(&test_loop::echo,&l1));
    timer2->timeout.connect(obj,std::bind(&test_object::print_info,obj));
    std::thread t([&](){
        timer->timeout.connectFunc([](){
            AIMY_INFO("timeout");
        });
        while(1)
        {
            AIMY_WARNNING("START");
            timer->start();
            sleep(2);
            AIMY_WARNNING("STOP 1");
            timer3->stop();
            AIMY_WARNNING("STOP 2");
            timer->stop();
            AIMY_WARNNING("STOP 3");
            con->disconnect();
            AIMY_WARNNING("STOP 4");
            sleep(2);
            AIMY_WARNNING("STart");
            timer->start();
            delete obj;
            sleep(2);
            AIMY_WARNNING("release");
            timer->release();
            timer2->stop();
            sleep(2);
            AIMY_WARNNING("start after release");
            timer->start();
            sleep(10);
            break;
        }
        timer3->start();
        AIMY_WARNNING("finished");
        //l1.stopThread();
    });
    t.detach();
    aimy::AimyLogger::Instance().register_exit_signal_func([&](){
        l1.stopThread();
    });
    l1.waitStop();
    return 0;
}
int loop_test(int argc,char *argv[])
{
    EventLoop loop(2);
    aimy::AimyLogger::Instance().register_exit_signal_func([&](){
        loop.stop();
    });
    auto timer=loop.addTimer(1);
    int cnt=0;
    timer->timeout.connectFunc([&](){
        AIMY_INFO("timeout cnt:%d",++cnt);
    });
    timer->start();
    loop.start();
    loop.waitStop();
    return 0;
}

int object_test2(int argc ,char *argv[])
{
    EventLoop loop(2);
    aimy::AimyLogger::Instance().register_exit_signal_func([&](){
        loop.stop();
    });
    loop.start();
    Object test(loop.getTaskScheduler().get());
    Object test2(loop.getTaskScheduler().get());
    Signal<> test_s(&test);
    test_s.connectFunc([](){
        AIMY_DEBUG("------------333333333----------");
    });
    std::thread t([&](){
        while (1) {
            usleep(1000);
            test2.addTriggerEvent([&](){
                AIMY_DEBUG("------------2222222----------");
                test_s.emit();
            });
        }
    });
    while (1) {
        usleep(1000);
        test.addTriggerEvent([](){
            AIMY_DEBUG("------------11111111----------");

        });
    }
}

int json_test(int argc ,char *argv[])
{
    neb::CJsonObject object;
//    object.Add("boost",0.1);
//    object.Add("boost_comment","-36 -> 36 db");
    neb::CJsonObject filter;
    neb::CJsonObject pipe1;
    neb::CJsonObject band_list;
    band_list.Add(1);
    band_list.Add(2);
    pipe1.Add("pipe",0);
    pipe1.Add("band_list",band_list);
    neb::CJsonObject pipe2;
    neb::CJsonObject band_list2;
    band_list2.Add(3);
    band_list2.Add(4);
    pipe2.Add("pipe",2);
    pipe2.Add("band_list",band_list2);
    filter.Add(pipe1);
    filter.Add(pipe2);
    object.Add("filter",filter);
    printf("%s\r\n",object.ToString().c_str());
    printf("%s\r\n",object.ToFormattedString().c_str());

    {
        //parser
        neb::CJsonObject object2=object;
        double boost;
        if(!object2.Get("boost",boost))
        {
            printf("get boost failed!\r\n");
        }
        printf("boost %f\r\n",boost);
        neb::CJsonObject filter;
        if(!object2.Get("filter",filter))
        {
            printf("get filter failed!\r\n");
        }
        printf("filter %s\r\n",filter.ToString().c_str());
        auto size=filter.GetArraySize();
        for (int i=0;i<size;++i)
        {
            neb::CJsonObject pipe;
            if(!filter.Get(i,pipe))
            {
                printf("get pipe failed!\r\n");
            }
            else {
                printf("pipe %d %s\r\n",i,pipe.ToString().c_str());
                uint64_t pipe_name;
                neb::CJsonObject band_list;
                if(!pipe.Get("pipe",pipe_name))
                {
                    printf("get pipe_name failed!\r\n");
                    continue;
                }
                if(!pipe.Get("band_list",band_list))
                {
                    printf("get band_list failed!\r\n");
                    continue;
                }
                printf("pipe_name:%lu\r\n",pipe_name);
                 printf("band_list:%s\r\n",band_list.ToString().c_str());
                 auto size2=band_list.GetArraySize();
                 for(int j=0;j<size2;++j)
                 {
                     uint64_t band_id;
                     if(!band_list.Get(j,band_id))
                     {
                         printf("get band_id failed!\r\n");
                         continue;
                     }
                     printf("band_id %d :%lu\r\n",j,band_id);
                 }
            }
        }
    }
    auto json_object=neb::CJsonObject::CreateInstance("./test.json");
    json_object->Add("boost",0.52);
    std::string data=json_object->ToString();
    printf("read:%s\r\n",data.c_str());
    delete json_object;
    object.SetSavePath("./test.json");
    object.SaveToFile();
    return 0;
}
