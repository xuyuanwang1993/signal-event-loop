#ifndef BLUEZTESTTASK_H
#define BLUEZTESTTASK_H
#include "core/core-include.h"
namespace aimy {
class BluetoothTest:public Object{
public:
    Signal<int>finshTest;
    BluetoothTest(Object *parent,int64_t timeoutMsec=15000);
    ~BluetoothTest();
public:
    void on_recv_mac(std::string mac,std::string pswd);
    void start();
    void stop();
    int result()const;
public:
    bool initBluetoothConnection();
    void releaseBluetoothConnection();
private:
    void handle_result(int result,const std::string&result_sdp="success");
private:
    void on_timeout();
    void on_setup();
    std::shared_ptr<Timer> timeoutTimer;
    std::atomic<bool> isrunning{false};
    std::string convertMac;
};
}
#endif // BLUEZTESTTASK_H
