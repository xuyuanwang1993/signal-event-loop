#include "can-util.h"
#include "log/aimy-log.h"
using namespace aimy;
const static std::string tag="can_raw";
CanUtilRaw::CanUtilRaw(Object *parent, const std::string &can_device):CanControlBase(parent,can_device)
{
    AIMY_DEBUG("[%s]init can util raw",tag.c_str());
}

CanUtilRaw::~CanUtilRaw()
{
    stopCan();
    AIMY_WARNNING("[%s]release can util raw",tag.c_str());
}

bool CanUtilRaw::sendTestControlCommand(CanCommandType cmd)
{
    auto frame_list=getTestCommandFrameList(cmd);
    if(frame_list.empty())return false;
    for(auto &i:frame_list)
    {
        sendCanFrame(i);
    }
    return true;
}

bool CanUtilRaw::sendTestControlCommandList(const std::list<CanCommandType>&cmdList)
{
    bool ret=true;
    for(auto i:cmdList)
    {
        ret=sendTestControlCommand(i);
        if(!ret)break;
    }
    return ret;
}

bool CanUtilRaw::initCan()
{
    AIMY_DEBUG("[%s]init can",tag.c_str());
    return true;
}

bool CanUtilRaw::startCan()
{
    AIMY_DEBUG("[%s]start can",tag.c_str());
    return true;
}

void CanUtilRaw::stopCan()
{
    AIMY_DEBUG("[%s]stop can",tag.c_str());
}

std::list<canControlFrame> CanUtilRaw::getTestCommandFrameList(CanCommandType type)
{
    std::list<canControlFrame> retList;
    switch (type) {
    case C_CAN_ENTER_TEST_MODE:{
        uint8_t data[1]={0xF0};
        retList.push_back(std::forward<canControlFrame>(packCanFrame(data,1,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),READ_CAN_DEVICE_GROUP_ADDR(BROADCAST_DEVICE_ADDR),physicalAddrToCanId(CanPhysicalAddrBroadcastGroup))));
        break;}
    case C_CAN_EXIT_TEST_MODE:{
        uint8_t data[1]={0xF1};
        retList.push_back(std::forward<canControlFrame>(packCanFrame(data,1,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),READ_CAN_DEVICE_GROUP_ADDR(BROADCAST_DEVICE_ADDR),physicalAddrToCanId(CanPhysicalAddrBroadcastGroup))));
        break;}

    case C_CAN_REQUEST_CHECK_1P_GROUP:{
        uint8_t data[2]={0xE0,0x01};
        retList.push_back(std::forward<canControlFrame>(packCanFrame(data,2,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),0x0305,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        break;}
    case C_CAN_REQUEST_CHECK_2P_GROUP:{
        uint8_t data[2]={0xE0,0x02};
        retList.push_back(std::forward<canControlFrame>(packCanFrame(data,2,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),0x030E,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}

    case C_CAN_REQUEST_CHECK_BACK_STAGE_LIGHT_GROUP:{
        uint8_t data[2]={0xE0,0x04};
        retList.push_back(std::forward<canControlFrame>(packCanFrame(data,2,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),0x0108,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        retList.push_back(std::forward<canControlFrame>(packCanFrame(data,2,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),0x0108,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        break;}
    case C_CAN_REQUEST_CHECK_FRONT_STAGE_LIGHT_GROUP:{
        uint8_t data[2]={0xE0,0x03};
        retList.push_back(std::forward<canControlFrame>(packCanFrame(data,2,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),0x0103,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        break;}
    case C_CAN_REQUEST_CHECK_CENTRAL_CONTROL_GROUP:{
        uint8_t data[2]={0xE0,0x05};
        retList.push_back(std::forward<canControlFrame>(packCanFrame(data,2,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),0x0501,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}

    case C_CAN_ENTER_STAGE_MODE_DEFAULT:{
        AIMY_WARNNING("[%s] todo for stage mode",tag.c_str());
        break;}

    case C_CAN_SET_1P_LIFE_LIGHT_OFF:{
        uint16_t device_addr=0x0614;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_1P_LIFE_LIGHT_RED:{
        uint16_t device_addr=0x0614;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_RED_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_1P_LIFE_LIGHT_GREEN:{
        uint16_t device_addr=0x0614;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}

    case C_CAN_SET_2P_LIFE_LIGHT_OFF:{
        uint16_t device_addr=0x0615;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_2P_LIFE_LIGHT_RED:{
        uint16_t device_addr=0x0615;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_RED_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_2P_LIFE_LIGHT_GREEN:{
        uint16_t device_addr=0x0615;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}

    case C_CAN_SET_1P_LEFT_FRONT_LIGHT_ON:{
        uint16_t device_addr=0x0205;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_RED_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        break;}
    case C_CAN_SET_1P_RIGHT_FRONT_LIGHT_ON:{
        uint16_t device_addr=0x0205;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        break;}
    case C_CAN_SET_1P_MID_CENTRAL_LIGHT_ON:{
        uint16_t device_addr=0x0208;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_RED_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}
    case C_CAN_SET_1P_LEFT_BACK_LIGHT_ON:{
        uint16_t device_addr=0x0206;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_RED_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}
    case C_CAN_SET_1P_RIGHT_BACK_LIGHT_ON:{
        uint16_t device_addr=0x0205;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_BLUE_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        break;}

    case C_CAN_SET_2P_LEFT_FRONT_LIGHT_ON:{
        uint16_t device_addr=0x0206;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}
    case C_CAN_SET_2P_RIGHT_FRONT_LIGHT_ON:{
        uint16_t device_addr=0x0206;
        uint8_t  operationType=CAN_LIGHT_OPEN;
        uint8_t  status=CAN_BLUE_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}
    case C_CAN_SET_2P_MID_CENTRAL_LIGHT_ON:{
        uint16_t device_addr=0x0208;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}
    case C_CAN_SET_2P_LEFT_BACK_LIGHT_ON:{
        uint16_t device_addr=0x0207;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}
    case C_CAN_SET_2P_RIGHT_BACK_LIGHT_ON:{
        uint16_t device_addr=0x0207;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_RED_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}

    case C_CAN_SET_1P_LEFT_FRONT_LIGHT_OFF:{
        uint16_t device_addr=0x0205;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_RED_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        break;}
    case C_CAN_SET_1P_RIGHT_FRONT_LIGHT_OFF:{
        uint16_t device_addr=0x0205;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        break;}
    case C_CAN_SET_1P_MID_CENTRAL_LIGHT_OFF:{
        uint16_t device_addr=0x0208;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_RED_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}
    case C_CAN_SET_1P_LEFT_BACK_LIGHT_OFF:{
        uint16_t device_addr=0x0206;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_RED_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}
    case C_CAN_SET_1P_RIGHT_BACK_LIGHT_OFF:{
        uint16_t device_addr=0x0205;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_BLUE_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        break;}

    case C_CAN_SET_2P_LEFT_FRONT_LIGHT_OFF:{
        uint16_t device_addr=0x0206;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}
    case C_CAN_SET_2P_RIGHT_FRONT_LIGHT_OFF:{
        uint16_t device_addr=0x0206;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_BLUE_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}
    case C_CAN_SET_2P_MID_CENTRAL_LIGHT_OFF:{
        uint16_t device_addr=0x0208;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr1PBoardGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}
    case C_CAN_SET_2P_LEFT_BACK_LIGHT_OFF:{
        uint16_t device_addr=0x0207;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}
    case C_CAN_SET_2P_RIGHT_BACK_LIGHT_OFF:{
        uint16_t device_addr=0x0207;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_RED_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddr2PBoardGroup))));
        break;}

    case C_CAN_SET_STROBE_LIGHT_ON:{
        uint16_t device_addr=0x010A;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=0x010B;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0x010A;
        operationType=CAN_LIGHT_OPEN;
        status=CAN_GREEN_LIGHT|CAN_RED_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=0x010B;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        break;}
    case C_CAN_SET_STROBE_LIGHT_OFF:{
        uint16_t device_addr=0x010A;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=0x010B;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        break;}


    case C_CAN_SET_CONSOLE_LIGHT_OFF:{
        uint16_t device_addr=0x0105;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_CONSOLE_LIGHT_RED:{
        uint16_t device_addr=0x0105;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_RED_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_CONSOLE_LIGHT_GREEN:{
        uint16_t device_addr=0x0105;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_CONSOLE_LIGHT_BLUE:{
        uint16_t device_addr=0x0105;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_BLUE_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}


    case C_CAN_SET_LEAF_LIGHT_OFF:{
        uint16_t device_addr=0x0209;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        device_addr=0x020A;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        device_addr=0x020B;
        status=CAN_RED_LIGHT|CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        break;}
    case C_CAN_SET_LEAF_LIGHT_ALL:{
        uint16_t device_addr=0x0209;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        device_addr=0x020A;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        device_addr=0x020B;
        status=CAN_RED_LIGHT|CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        break;}
    case C_CAN_SET_LEAF_LIGHT_RED:{
        uint16_t device_addr=0x0209;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_RED_LIGHT|CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        device_addr=0x020B;
        status=CAN_RED_LIGHT|CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        //OPEN
        operationType=CAN_LIGHT_OPEN;
        device_addr=0x0209;
        status=CAN_BLUE_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        device_addr=0x020A;
        status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        break;}
    case C_CAN_SET_LEAF_LIGHT_BLUE:{
        uint16_t device_addr=0x0209;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_BLUE_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        device_addr=0x020A;
        status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        //open
        operationType=CAN_LIGHT_OPEN;
        device_addr=0x0209;
        status=CAN_RED_LIGHT|CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        device_addr=0x020B;
        status=CAN_RED_LIGHT|CAN_GREEN_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        break;}

    case C_CAN_SET_INTERNAL_TWO_COLOR_LIGHT_OFF:{
        uint16_t device_addr=0x0103;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0X0107;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        break;}
    case C_CAN_SET_INTERNAL_TWO_COLOR_LIGHT_RED:{
        uint16_t device_addr=0x0103;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0X0107;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_RED_LIGHT;
        device_addr=0x0103;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0x0107;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        break;}
    case C_CAN_SET_INTERNAL_TWO_COLOR_LIGHT_GREEN:{
        uint16_t device_addr=0x0103;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0X0107;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_GREEN_LIGHT;
        device_addr=0x0103;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0x0107;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        break;}
    case C_CAN_SET_INTERNAL_TWO_COLOR_LIGHT_BLUE:{
        uint16_t device_addr=0x0103;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0X0107;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_BLUE_LIGHT;
        device_addr=0x0103;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0x0107;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        break;}
    case C_CAN_SET_INTERNAL_TWO_COLOR_LIGHT_ALL:{
        uint16_t device_addr=0x0103;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0X0107;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        break;}

    case C_CAN_SET_EXTERNAL_TWO_COLOR_LIGHT_OFF:{
        uint16_t device_addr=0x0102;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0X0108;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        break;}
    case C_CAN_SET_EXTERNAL_TWO_COLOR_LIGHT_RED:{
        uint16_t device_addr=0x0102;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0X0108;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_RED_LIGHT;
        device_addr=0x0102;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0x0108;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        break;}
    case C_CAN_SET_EXTERNAL_TWO_COLOR_LIGHT_GREEN:{
        uint16_t device_addr=0x0102;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0X0108;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_GREEN_LIGHT;
        device_addr=0x0102;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0x0108;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        break;}
    case C_CAN_SET_EXTERNAL_TWO_COLOR_LIGHT_BLUE:{
        uint16_t device_addr=0x0102;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0X0108;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_BLUE_LIGHT;
        device_addr=0x0102;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0x0108;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        break;}
    case C_CAN_SET_EXTERNAL_TWO_COLOR_LIGHT_ALL:{
        uint16_t device_addr=0x0102;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        device_addr=0X0108;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrFrontStageLightGroup))));
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrBackStageLightGroup))));
        break;}

    case C_CAN_SET_PILLAR_LIGHT_OFF:{
        uint16_t device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_1);
        uint8_t operationType=SET_PILLAR_SYSTEM_MODE;
        uint8_t status=PILLAR_MODE_STOPPED;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_2);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_PILLAR_LIGHT_RED:{
        uint8_t operationType=SET_PILLAR_SYSTEM_MODE;
        uint8_t status=PILLAR_MODE_USER_SET;
        uint16_t device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_1);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_2);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        //
        operationType=SET_PILLAR_COLOR;
        status=PILLAR_COLOR_RED;
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_1);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_2);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_PILLAR_LIGHT_GREEN:{
        uint8_t operationType=SET_PILLAR_SYSTEM_MODE;
        uint8_t status=PILLAR_MODE_USER_SET;
        uint16_t device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_1);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_2);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        //
        operationType=SET_PILLAR_COLOR;
        status=PILLAR_COLOR_GREEN;
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_1);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_2);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_PILLAR_LIGHT_BLUE:{
        uint8_t operationType=SET_PILLAR_SYSTEM_MODE;
        uint8_t status=PILLAR_MODE_USER_SET;
        uint16_t device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_1);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_2);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        //
        operationType=SET_PILLAR_COLOR;
        status=PILLAR_COLOR_BLUE;
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_1);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_2);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_PILLAR_LIGHT_ALL:{
        uint8_t operationType=SET_PILLAR_SYSTEM_MODE;
        uint8_t status=PILLAR_MODE_USER_SET;
        uint16_t device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_1);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_2);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        //
        operationType=SET_PILLAR_COLOR;
        status=PILLAR_COLOR_WHITE;
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_1);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=READ_CAN_DEVICE_GROUP_ADDR(OTHERDEVICE_GROUP_ADDR_2);
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}

    case C_CAN_SET_MAJOR_SPOTLIGHT_OFF:{
        uint16_t device_addr=0x0104;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=0X0106;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_MAJOR_SPOTLIGHT_RED:{
        uint16_t device_addr=0x0104;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=0X0106;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_RED_LIGHT;
        device_addr=0x0104;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=0x0106;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_MAJOR_SPOTLIGHT_GREEN:{
        uint16_t device_addr=0x0104;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=0X0106;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_GREEN_LIGHT;
        device_addr=0x0104;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=0x0106;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_MAJOR_SPOTLIGHT_BLUE:{
        uint16_t device_addr=0x0104;
        uint8_t operationType=CAN_LIGHT_CLOSE;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=0X0106;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        operationType=CAN_LIGHT_OPEN;
        status=CAN_BLUE_LIGHT;
        device_addr=0x0104;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=0x0106;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    case C_CAN_SET_MAJOR_SPOTLIGHT_ALL:{
        uint16_t device_addr=0x0104;
        uint8_t operationType=CAN_LIGHT_OPEN;
        uint8_t status=CAN_ALL_LIGHT;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        device_addr=0X0106;
        retList.push_back(std::forward<canControlFrame>(packControlCanFrame(operationType,status,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),device_addr,physicalAddrToCanId(CanPhysicalAddrCentralGroup))));
        break;}
    default:
        AIMY_ERROR("[%s]undefined can control cmd->%d",tag.c_str(),type);
        break;
    }
    return retList;
}

void CanUtilRaw::sendCanFrame(const canControlFrame&frame)
{
    uint32_t offset=0;
    for(auto iter:frame.optionCanIdList)
    {
        uint32_t send_offset=offset;
        uint32_t send_len=(send_offset+8<=frame.data_len)?8:(frame.data_len-send_offset);
        if(send_len==0)break;
        std::string cmd="cansend "+canDeviceName+" ";
        uint32_t len=8+1+send_len*2;
        char buf[32];
        memset(buf,0,32);
        uint32_t canId=iter;
        sprintf(buf,"%02X%02X%02X%02X#",(canId>>24)&0xff,(canId>>16)&0xff,(canId>>8)&0xff,canId&0xff);
        int fill_offset=9;
        for(uint32_t i=0;i<send_len;++i)
        {
            sprintf(buf+fill_offset,"%02X",frame.data.get()[send_offset+i]);
            fill_offset+=2;
        }
        cmd+=std::string(buf,len);
        AIMY_DEBUG("[%s]can_cmd:%s",tag.c_str(),cmd.c_str());
#if defined (__aarch64__) || defined (__arm__)
        system(cmd.c_str());
#endif

        offset+=send_len;
    }
}
