#ifndef DEVICE_UPDATER_H
#define DEVICE_UPDATER_H
#include "log/aimy-log.h"
#include "core/core-include.h"
#include "update-file.h"
#define AIMY_UPDATER_VERSION "2.0.1"
#define AIMY_UPDATER_VERSION_TIME "2022-05-26"
#define AIMY_UPDATER_VERSION_AUTHOR "xuyuanwang(469953258@qq.com)"
namespace aimy {
class AimyUpdater:public Object
{
public:
    /**
     * @brief notifyQuitMessage signal will be sent in the moment updater stop working
     */
    Signal<UpgradeQuitStatus,std::string>notifyQuitMessage;
    /**
     * @brief notifyQueryMessage signal will be sent after recv query reply
     */
    Signal<UpdateTypeDef,std::string>notifyQueryMessage;
    /**
     * @brief notifyQueryMessage signal will be sent after recv update reply
     */
    Signal<UpdateTypeDef>notifyUpdateMessage;
    /**
     * @brief notifyUpdateProgress signal  will be send when update progress is updated
     */
    Signal<double>notifyUpdateProgress;
public:
    AimyUpdater(TaskScheduler *parent);
    ~AimyUpdater();

    bool init(const std::string &url);
    bool loadUpdateTask(const std::string &file_path,uint32_t device_adress);
    void startUpdate();
    void stopUpdate();

    void queryDevice();
    void queryUPID();
    void queryWorkMode();
private:
    void handleDataCache();
    void handleFrame(const void *data,uint32_t data_len,uint32_t address,uint32_t control_code);
//request
    //normal query
    void requestQueryDevice();
    void requestQueryUPID();
    void requestQueryWorkMode();
    //update
    void requestEnterUpgradeWorkMode();
    void requestQuerySoftwareVersion();
    void requestQuerySegmentInfomation();
    void requestUploadSegment();
    void requestQuitUpgradeWorkMode(const std::string &message="unknown");
//reply
    void handleReplyQueryDevice(const void *data,uint32_t data_len,uint32_t address);
    void handleReplyQueryUPID(const void *data,uint32_t data_len,uint32_t address);
    void handleReplyQueryWorkMode(const void *data,uint32_t data_len,uint32_t address);
    //
    void handleReplyEnterUpgradeWorkMode(const void *data,uint32_t data_len,uint32_t address);
    void handleReplyQuerySoftwareVersion(const void *data,uint32_t data_len,uint32_t address);
    void handleReplyQuerySegmentInfomation(const void *data,uint32_t data_len,uint32_t address);
    void handleReplyUploadVectorTable(const void *data,uint32_t data_len,uint32_t address);
    void handleReplyUploadCodeSegment(const void *data,uint32_t data_len,uint32_t address);
    void handleReplyQuitUpgradeWorkMode(const void *data,uint32_t data_len,uint32_t address);
//network
    void on_read();
    void on_write();
    void on_close();
    void on_error();
    void on_send_timeout();
//release
    void release();
//send data
    void sendNormalData(const void *data,uint32_t data_len);
    void sendUpdateData(const void *data,uint32_t data_len);
//progress
    void updateProgress();
private://static
    //product_str+version_str+date split by '\0'
    static bool parserDeviceInfo(const void *data, uint32_t date_len, const char ** p_version, const char **p_product, const char **p_date);
private:
    TaskScheduler *const scheduler;
    std::shared_ptr<IoChannel>workChannel;
    std::string messageUrl;
    bool usingSerial;
    UpdateFileContext context;
    uint32_t updateDeviceAdress;
    //
    UpgradeQuitStatus quitStatus;
    std::string quitMessage;
    char readCache[4096];
    uint32_t dataLen;
    //
    std::list<std::pair<std::shared_ptr<uint8_t>,uint32_t>>sendDataList;
    //update data handle
    std::shared_ptr<UpdateSliceContext> sliceContext;
    std::shared_ptr<uint8_t>lastUpdateFrame;
    uint32_t lastUpdateFrameLen;
    std::shared_ptr<Timer> resendTimer;
    //statistics
    uint64_t totalPackets;
    uint64_t progressPackets;
};
}
#endif // DEVICE_UPDATER_H
