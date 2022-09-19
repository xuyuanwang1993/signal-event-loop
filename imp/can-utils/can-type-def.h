#ifndef CANTYPEDEF_H
#define CANTYPEDEF_H
#include <stdint.h>
#include<assert.h>
#include<string.h>
#include "core/object.h"
namespace aimy
{
/**
* 1.状态通知使用单独的信号通知，不使用聚合接口
* 2.踏板定义如下
*  编码规则：从0x01开始，先1P后2P，区域顺时针，从左至右，从上往下
*  'o' 代表手红外传感器
*  ‘.’ d代表压力传感器
*  left_f:左前
*  central:中间
*  right_f:右前
*  left_b:左后
*  right_b:右后
*
*                                                      -----------------------------
*                                                                   主屏幕
*                                                      -----------------------------
*              01   02   03      04   05   06                        ||                        41   42   43      44   45   46
*         o    o    o    o   |   o    o    o    o                    ||                   o    o    o    o   |   o    o    o    o
*           .19        .1A   |     .21         .22                   ||                     .59        .5A   |     .61         .62
*        20             1B.  |  .28                                  ||                  60             5B.  |  .68
*     o  .   left_f       /     \         right_f . o                ||               o  .   left_f       /     \         right_f . o
*  18 o           1C.  /  .29  2A. \  .27        23 o 07             ||            58 o           5C.  /  .69  6A. \  .67        63 o 47
*  17 o  .1F   1D.  /                 \ 26.    24.  o 08             ||            57 o  .5F   5D.  /                 \ 66.    64.  o 48
*  16 o     .1E  /   .30           2B.   \  25.     o 09             ||            56 o     .5E  /   .70           6B.   \  65.     o 49
*    -----------            central        -----------               ||              -----------            central        -----------
*  15 o     .31  \   .2F           2C.   /   39.    o 0A             ||            55 o     .71  \   .6F           6C.   /   79.    o 4A
*  14 o  38     32. \      2E   2D    /  .40    3A. o 0B             ||            54 o  78     72. \      6E   6D    /  .80    7A. o 4B
*  13 o  .   left_b    \   .    .  /      right_b   o 0C             ||            53 o  .   left_b    \   .    .  /      right_b   o 4C
*     o             33.   \     /    .3F            o                ||               o             73.   \     /    .7F            o
*        .37            34.  |   .3E            3B.                  ||                  .77            74.  |   .7E            7B.
*           .36      35.     |      .3D     3C.                      ||                     .76      75.     |      .7D     7C.
*         o    o    o    o   |   o    o    o    o                    ||                   o    o    o    o   |   o    o    o    o
*              12   11   10      0F   0E   0D                        ||                        52   51   50      4F   4E   4D
*                          1P踏板                                                                           2P踏板
*
*/
/**
 * candum can0 -L //查看can网卡输出
 *
 * //初始化can网卡
 * ip link set can0 down
 * ip link set can0 up type can bitrate 200000 loopback off
 * ifconfig can0 up
 *
 * //
 * ip -details link show can0
 *
 * E5成名模拟测试设备
 * 测试前要发送进入测试模式命令 0 // 1
 * 1P脚开关：1P踏板中间灯  16  //26
 * 2P脚开关: 2P踏板中间灯  21   //31
 * F-5A ：双色灯内灯  R/ALL 状态亮  45 48  // 44 46 47
 * F-5B ：叶子灯 B/ALL 状态亮   41 43 //40 42
 * F-3 ：红灯 1P生命灯R   9  //8 10
 * F-3 ：绿灯 1P生命灯G   10 //8 9
 *
 * 正常测试需先发送广播命令进入游戏模式
 */

//
enum CanCommandType : uint32_t
{
    C_CAN_ENTER_TEST_MODE,            //进入测试模式
    C_CAN_EXIT_TEST_MODE,             //退出测试模式

    C_CAN_REQUEST_CHECK_1P_GROUP,    //检查1P踏板通讯组
    C_CAN_REQUEST_CHECK_2P_GROUP,    //检查2P踏板通讯组
    C_CAN_REQUEST_CHECK_FRONT_STAGE_LIGHT_GROUP, //检查前舞台灯光通讯组
    C_CAN_REQUEST_CHECK_BACK_STAGE_LIGHT_GROUP, //检查后舞台灯光通讯组
    C_CAN_REQUEST_CHECK_CENTRAL_CONTROL_GROUP,  //检测中控台通讯组

    C_CAN_ENTER_STAGE_MODE_DEFAULT, // 进入舞台默认模式

    C_CAN_SET_1P_LIFE_LIGHT_OFF,        //关闭1P生命灯
    C_CAN_SET_1P_LIFE_LIGHT_RED,   //将1P生命灯设成红色
    C_CAN_SET_1P_LIFE_LIGHT_GREEN, //将1P生命灯设成绿色
    C_CAN_SET_2P_LIFE_LIGHT_OFF,        //关闭2P生命灯
    C_CAN_SET_2P_LIFE_LIGHT_RED,   //将2P生命灯设成红色
    C_CAN_SET_2P_LIFE_LIGHT_GREEN, //将2P生命灯设成绿色

    C_CAN_SET_1P_LEFT_FRONT_LIGHT_ON,//1P左前灯开
    C_CAN_SET_1P_LEFT_BACK_LIGHT_ON,//1P左后灯开
    C_CAN_SET_1P_MID_CENTRAL_LIGHT_ON,//1P中间灯开
    C_CAN_SET_1P_RIGHT_FRONT_LIGHT_ON,//1P右前灯开
    C_CAN_SET_1P_RIGHT_BACK_LIGHT_ON,//1P右后灯开
    C_CAN_SET_2P_LEFT_FRONT_LIGHT_ON,//2P左前灯开
    C_CAN_SET_2P_LEFT_BACK_LIGHT_ON,//2P左后灯开
    C_CAN_SET_2P_MID_CENTRAL_LIGHT_ON,//2P中间灯开
    C_CAN_SET_2P_RIGHT_FRONT_LIGHT_ON,//2P右前灯开
    C_CAN_SET_2P_RIGHT_BACK_LIGHT_ON,//2P右后灯开
    C_CAN_SET_1P_LEFT_FRONT_LIGHT_OFF,//1P左前灯关
    C_CAN_SET_1P_LEFT_BACK_LIGHT_OFF,//1P左后灯关
    C_CAN_SET_1P_MID_CENTRAL_LIGHT_OFF,//1P中间灯关
    C_CAN_SET_1P_RIGHT_FRONT_LIGHT_OFF,//1P右前灯关
    C_CAN_SET_1P_RIGHT_BACK_LIGHT_OFF,//1P右后灯关
    C_CAN_SET_2P_LEFT_FRONT_LIGHT_OFF,//2P左前灯关
    C_CAN_SET_2P_LEFT_BACK_LIGHT_OFF,//2P左后灯关
    C_CAN_SET_2P_MID_CENTRAL_LIGHT_OFF,//2P中间灯关
    C_CAN_SET_2P_RIGHT_FRONT_LIGHT_OFF,//2P右前灯关
    C_CAN_SET_2P_RIGHT_BACK_LIGHT_OFF,//2P右后灯关

    C_CAN_SET_STROBE_LIGHT_ON, //频闪灯开
    C_CAN_SET_STROBE_LIGHT_OFF, //频闪灯关

    C_CAN_SET_CONSOLE_LIGHT_OFF, //控台灯关
    C_CAN_SET_CONSOLE_LIGHT_RED, //控台灯R
    C_CAN_SET_CONSOLE_LIGHT_GREEN, //控台灯G
    C_CAN_SET_CONSOLE_LIGHT_BLUE, //控台灯B

    C_CAN_SET_LEAF_LIGHT_OFF, //叶子灯关
    C_CAN_SET_LEAF_LIGHT_ALL, //叶子灯全开
    C_CAN_SET_LEAF_LIGHT_RED, //叶子灯R
    C_CAN_SET_LEAF_LIGHT_BLUE, //叶子灯B

    C_CAN_SET_INTERNAL_TWO_COLOR_LIGHT_OFF, //双色灯内灯关
    C_CAN_SET_INTERNAL_TWO_COLOR_LIGHT_RED, //双色灯内灯R
    C_CAN_SET_INTERNAL_TWO_COLOR_LIGHT_GREEN, //双色灯内灯G
    C_CAN_SET_INTERNAL_TWO_COLOR_LIGHT_BLUE, //双色灯内灯B
    C_CAN_SET_INTERNAL_TWO_COLOR_LIGHT_ALL, //双色灯全开

    C_CAN_SET_EXTERNAL_TWO_COLOR_LIGHT_OFF, //双色灯外灯关
    C_CAN_SET_EXTERNAL_TWO_COLOR_LIGHT_RED, //双色灯外灯R
    C_CAN_SET_EXTERNAL_TWO_COLOR_LIGHT_GREEN, //双色灯外灯G
    C_CAN_SET_EXTERNAL_TWO_COLOR_LIGHT_BLUE, //双色灯外灯B
    C_CAN_SET_EXTERNAL_TWO_COLOR_LIGHT_ALL, //双色灯外灯全开

    C_CAN_SET_PILLAR_LIGHT_OFF, //音柱灯光关
    C_CAN_SET_PILLAR_LIGHT_RED, //音柱灯光R
    C_CAN_SET_PILLAR_LIGHT_GREEN, //音柱灯光G
    C_CAN_SET_PILLAR_LIGHT_BLUE, //音柱灯光B
    C_CAN_SET_PILLAR_LIGHT_ALL, //音柱灯光全开

    C_CAN_SET_MAJOR_SPOTLIGHT_OFF, // 招牌射灯(聚光灯)关
    C_CAN_SET_MAJOR_SPOTLIGHT_RED, // 招牌射灯R
    C_CAN_SET_MAJOR_SPOTLIGHT_GREEN, // 招牌射灯G
    C_CAN_SET_MAJOR_SPOTLIGHT_BLUE, // 招牌射灯B
    C_CAN_SET_MAJOR_SPOTLIGHT_ALL, // 招牌射灯全开
};

struct canControlFrame{
    static const uint32_t MAX_CAN_PAYLOAD_SIZE=256;
    const uint32_t default_capacity;
    std::shared_ptr<uint8_t>data;
    uint32_t data_len;
    std::list<uint32_t>optionCanIdList;
    bool error_flag=false;
    canControlFrame():default_capacity(0),data(nullptr),data_len(0){

    }
    canControlFrame(uint32_t capacity,uint32_t canId):default_capacity(capacity<MAX_CAN_PAYLOAD_SIZE*2?MAX_CAN_PAYLOAD_SIZE*2:capacity),data(nullptr),data_len(0){
        data.reset(new uint8_t[default_capacity+1],std::default_delete<uint8_t[]>());
        memset(data.get(),0,default_capacity+1);
        optionCanIdList.push_back(canId);
    }
};

enum CanLightDataType : uint8_t
{
    CAN_BLUE_LIGHT = 1 << 0,                                          //蓝灯
    CAN_GREEN_LIGHT = 1 << 1,                                         //绿灯
    CAN_RED_LIGHT = 1 << 2,                                           //红灯
    CAN_ALL_LIGHT = CAN_BLUE_LIGHT | CAN_GREEN_LIGHT | CAN_RED_LIGHT, //所有颜色

    CAN_LIGHT_BRIGHTNESS_BASE=0X05,//亮度 上下可调整五级 比如0x00 代表亮度减少5级 0x09亮度增加4级，0x5亮度不变

};
#define CAN_LIGHT_CHANNEL_MASK  (0XF8)
enum CanLightOperationType : uint8_t
{
    CAN_LIGHT_OPEN = 0x01,  //常亮  参数 要操作的灯
    CAN_LIGHT_CLOSE = 0x02, //熄灭  参数 要操作的灯
    CAN_LIGHT_PAUSE = 0X03,//暂停，不再接收处理控制命令，除取消暂停和复位
    CAN_LIGHT_RECOVER = 0x04,//取消暂停
    CAN_LIGHT_AUTO_MODE= 0x05,//自控模式
    CAN_LIGHT_CLOLORFUL_MODE = 0x06,//多色模式
    CAN_LIGHT_SHINE_MODE = 0x07,//闪亮  参数 要操作的灯
    CAN_LIGHT_SPIN_ANGLE = 0x08,//旋转角度   参数 0~35  表示0~360度,每个刻度为10度
    CAN_LIGHT_MOVE_ANGLE = 0x09,//仰视角度   参数 0~17  表示0~180度,每个刻度为10度
    CAN_LIGHT_AUTO_SPIN = 0x0A,//自转
    CAN_LIGHT_SET_BRIGHTNESS = 0x0B,//亮度设置    参数 0~6级亮度
    CAN_LIGHT_CHANGE_SPIN_MODE = 0x0C,//转向        参数 0:正转 1: 反转 2:静止
    /*
        0:单点：
1:双点：
2:三点三角形
3:四点：
4:逐点单行快画线（喷泉）：
5:逐点多行快画线（喷泉）：
6:中心扩散成面（一拍）
7:面收缩为中心点（一拍）
8:单行快变（一拍完成）：
9:单行随机跑。
10:单行慢变（多拍完成，用于无鼓点时）：
11:全图慢变（多拍完成）：
    */
    CAN_LIGHT_RENDER_PICTURE = 0x0D,//图案

    CAN_LIGHT_COMMON_SET_BPM=CAN_LIGHT_CHANNEL_MASK|0x00,//设置BPM
    CAN_LIGHT_COMMON_SYNC=CAN_LIGHT_CHANNEL_MASK|0x01,//同步信号
    CAN_LIGHT_COMMON_RESET=CAN_LIGHT_CHANNEL_MASK|0x02,//复位命令
    CAN_LIGHT_COMMON_CHANGE_BRIGHTNESS=CAN_LIGHT_CHANNEL_MASK|0x03,//调整亮度

};

enum BoardFiledType :uint8_t{
    HAND_1P_FRONT_FILED, //1P手前
    HAND_1P_BACK_FILED, //1P手后
    HAND_1P_LEFT_FILED, //1P手左
    HAND_1P_RIGHT_FILED, //1P手右

    HAND_2P_FRONT_FILED, //2P手前
    HAND_2P_BACK_FILED,  //2P手后
    HAND_2P_LEFT_FILED,  //2P手左
    HAND_2P_RIGHT_FILED, //2P手右

    FOOT_1P_LEFT_FRONT_FILED, //1P脚左前
    FOOT_1P_RIGHT_FRONT_FILED, //1P脚右前
    FOOT_1P_MID_FILED, //1P中间
    FOOT_1P_LEFT_BACK_FILED, //1P脚左后
    FOOT_1P_RIGHT_BACK_FILED, //1P脚右后

    FOOT_2P_LEFT_FRONT_FILED, //2P脚左前
    FOOT_2P_RIGHT_FRONT_FILED, //2P脚右前
    FOOT_2P_MID_FILED, //2P中间
    FOOT_2P_LEFT_BACK_FILED, //2P脚左后
    FOOT_2P_RIGHT_BACK_FILED, //2P脚右后
};

enum BoardFiledStatus :uint8_t{
    BoardPressed=0,//按下
    BoardHoldOn=1,//保持
    BoardReleased=2,//弹起
};

enum ScreenTouchEvent:uint8_t{
    ScreenPressed=0x81,//按下
    ScreenHoldOn=0x82,//保持
    ScreenReleased=0x84,//弹起
};

enum BoardSensorId:uint8_t{
    HAND_1P_FRONT_1=0X01,
    HAND_1P_FRONT_2,
    HAND_1P_FRONT_3,
    HAND_1P_FRONT_4,
    HAND_1P_FRONT_5,
    HAND_1P_FRONT_6,

    HAND_1P_RIGHT_1=0X07,
    HAND_1P_RIGHT_2,
    HAND_1P_RIGHT_3,
    HAND_1P_RIGHT_4,
    HAND_1P_RIGHT_5,
    HAND_1P_RIGHT_6,

    HAND_1P_BACK_1=0X0D,
    HAND_1P_BACK_2,
    HAND_1P_BACK_3,
    HAND_1P_BACK_4,
    HAND_1P_BACK_5,
    HAND_1P_BACK_6,

    HAND_1P_LEFT_1=0X13,
    HAND_1P_LEFT_2,
    HAND_1P_LEFT_3,
    HAND_1P_LEFT_4,
    HAND_1P_LEFT_5,
    HAND_1P_LEFT_6,

    FOOT_1P_LEFT_FRONT_1=0x19,
    FOOT_1P_LEFT_FRONT_2,
    FOOT_1P_LEFT_FRONT_3,
    FOOT_1P_LEFT_FRONT_4,
    FOOT_1P_LEFT_FRONT_5,
    FOOT_1P_LEFT_FRONT_6,
    FOOT_1P_LEFT_FRONT_7,
    FOOT_1P_LEFT_FRONT_8,

    FOOT_1P_RIGHT_FRONT_1=0x21,
    FOOT_1P_RIGHT_FRONT_2,
    FOOT_1P_RIGHT_FRONT_3,
    FOOT_1P_RIGHT_FRONT_4,
    FOOT_1P_RIGHT_FRONT_5,
    FOOT_1P_RIGHT_FRONT_6,
    FOOT_1P_RIGHT_FRONT_7,
    FOOT_1P_RIGHT_FRONT_8,

    FOOT_1P_MID_1=0x29,
    FOOT_1P_MID_2,
    FOOT_1P_MID_3,
    FOOT_1P_MID_4,
    FOOT_1P_MID_5,
    FOOT_1P_MID_6,
    FOOT_1P_MID_7,
    FOOT_1P_MID_8,

    FOOT_1P_LEFT_BACK_1=0x31,
    FOOT_1P_LEFT_BACK_2,
    FOOT_1P_LEFT_BACK_3,
    FOOT_1P_LEFT_BACK_4,
    FOOT_1P_LEFT_BACK_5,
    FOOT_1P_LEFT_BACK_6,
    FOOT_1P_LEFT_BACK_7,
    FOOT_1P_LEFT_BACK_8,

    FOOT_1P_RIGHT_BACK_1=0x39,
    FOOT_1P_RIGHT_BACK_2,
    FOOT_1P_RIGHT_BACK_3,
    FOOT_1P_RIGHT_BACK_4,
    FOOT_1P_RIGHT_BACK_5,
    FOOT_1P_RIGHT_BACK_6,
    FOOT_1P_RIGHT_BACK_7,
    FOOT_1P_RIGHT_BACK_8,

    HAND_2P_FRONT_1=0X41,
    HAND_2P_FRONT_2,
    HAND_2P_FRONT_3,
    HAND_2P_FRONT_4,
    HAND_2P_FRONT_5,
    HAND_2P_FRONT_6,

    HAND_2P_RIGHT_1=0X47,
    HAND_2P_RIGHT_2,
    HAND_2P_RIGHT_3,
    HAND_2P_RIGHT_4,
    HAND_2P_RIGHT_5,
    HAND_2P_RIGHT_6,

    HAND_2P_BACK_1=0X4D,
    HAND_2P_BACK_2,
    HAND_2P_BACK_3,
    HAND_2P_BACK_4,
    HAND_2P_BACK_5,
    HAND_2P_BACK_6,

    HAND_2P_LEFT_1=0X53,
    HAND_2P_LEFT_2,
    HAND_2P_LEFT_3,
    HAND_2P_LEFT_4,
    HAND_2P_LEFT_5,
    HAND_2P_LEFT_6,

    FOOT_2P_LEFT_FRONT_1=0x59,
    FOOT_2P_LEFT_FRONT_2,
    FOOT_2P_LEFT_FRONT_3,
    FOOT_2P_LEFT_FRONT_4,
    FOOT_2P_LEFT_FRONT_5,
    FOOT_2P_LEFT_FRONT_6,
    FOOT_2P_LEFT_FRONT_7,
    FOOT_2P_LEFT_FRONT_8,

    FOOT_2P_RIGHT_FRONT_1=0x61,
    FOOT_2P_RIGHT_FRONT_2,
    FOOT_2P_RIGHT_FRONT_3,
    FOOT_2P_RIGHT_FRONT_4,
    FOOT_2P_RIGHT_FRONT_5,
    FOOT_2P_RIGHT_FRONT_6,
    FOOT_2P_RIGHT_FRONT_7,
    FOOT_2P_RIGHT_FRONT_8,

    FOOT_2P_MID_1=0x69,
    FOOT_2P_MID_2,
    FOOT_2P_MID_3,
    FOOT_2P_MID_4,
    FOOT_2P_MID_5,
    FOOT_2P_MID_6,
    FOOT_2P_MID_7,
    FOOT_2P_MID_8,

    FOOT_2P_LEFT_BACK_1=0x71,
    FOOT_2P_LEFT_BACK_2,
    FOOT_2P_LEFT_BACK_3,
    FOOT_2P_LEFT_BACK_4,
    FOOT_2P_LEFT_BACK_5,
    FOOT_2P_LEFT_BACK_6,
    FOOT_2P_LEFT_BACK_7,
    FOOT_2P_LEFT_BACK_8,

    FOOT_2P_RIGHT_BACK_1=0x79,
    FOOT_2P_RIGHT_BACK_2,
    FOOT_2P_RIGHT_BACK_3,
    FOOT_2P_RIGHT_BACK_4,
    FOOT_2P_RIGHT_BACK_5,
    FOOT_2P_RIGHT_BACK_6,
    FOOT_2P_RIGHT_BACK_7,
    FOOT_2P_RIGHT_BACK_8,
};

enum CanPhysicalAddr:uint32_t{
    CanPhysicalAddrGroupUnknown=0x00,
    CanPhysicalAddrApplicationGroup=0x0001,//应用消息组  2000
    CanPhysicalAddr1PHandGroup=0x3001,//1P手传感器灯光控制组
    CanPhysicalAddr2PHandGroup=0x3002,//2P手传感器灯光控制组
    CanPhysicalAddr1PBoardGroup=0x3003, //1P踏板控制组  6006021
    CanPhysicalAddr2PBoardGroup=0x3004, //2P踏板控制组  6008021
    CanPhysicalAddrFrontStageLightGroup=0x2001, //前台灯光控制组  4002021
    CanPhysicalAddrBackStageLightGroup=0x2002, //后台灯光控制组   4004021
    CanPhysicalAddrCentralGroup=0x2003, //主控组   4006021
    CanPhysicalAddrBroadcastGroup=0xFFFF, //广播组 1FFE021
};

enum canDeviceGroup:uint8_t{
    CanDeviceApplicationGroup=0x00,//应用组
    CanDeviceLightGroupEeffect1=0x01,//灯光组1，音乐灯效
    CanDeviceLightGroupEeffect2=0x02,//灯光组2,踏板灯效
    CanDeviceBoardGroup=0x03,//踏板传感器组
    CanDeviceTouchScreenGroup=0x04,//触屏组
    CanDeviceKeyBoardGroup=0x05,//键盘组
    CanDeviceOtherGroup=0x06,//其它组
    CanDeviceBroadcastGroup=0xFF,//广播组
};

#define GENERATE_CAN_ADDR(physicalAddr,group,index) ((((physicalAddr)&0xFFFF)<<16)|(((group)&0xFF)<<8)|((index)&0xFF))
#define READ_CAN_PHYSICAL_ADDR(canDeviceAddr) (((canDeviceAddr)>>16)&0xFFFF)
#define READ_CAN_DEVICE_GROUP(canDeviceAddr) (((canDeviceAddr)>>8)&0xFF)
#define READ_CAN_DEVICE_GROUP_INDEX(canDeviceAddr) ((canDeviceAddr)&0xFF)
#define READ_CAN_DEVICE_GROUP_ADDR(canDeviceAddr) ((canDeviceAddr)&0xFFFF)
enum CanDeviceAddr:uint32_t{
    APPLICATION_DEVICE_ADDR=GENERATE_CAN_ADDR(CanPhysicalAddrApplicationGroup,CanDeviceApplicationGroup,1),//应用数据
    BROADCAST_DEVICE_ADDR=GENERATE_CAN_ADDR(CanPhysicalAddrBroadcastGroup,CanDeviceBroadcastGroup,0XFF),//广播地址
    RGB_EFFECT_1P_LIGHT_ADDR=GENERATE_CAN_ADDR(CanPhysicalAddrFrontStageLightGroup,CanDeviceLightGroupEeffect1,1),//1P顶架三色背景灯的颜色亮暗控制
    RGB_EFFECT_LIGHT_ADDR_1=GENERATE_CAN_ADDR(CanPhysicalAddrFrontStageLightGroup,CanDeviceLightGroupEeffect1,2),//左边三个双色射灯组外圈LED灯的亮暗控制
    RGB_EFFECT_LIGHT_ADDR_2=GENERATE_CAN_ADDR(CanPhysicalAddrFrontStageLightGroup,CanDeviceLightGroupEeffect1,3),//左边三个双色射灯组内射灯的亮暗控制
    RGB_EFFECT_LIGHT_ADDR_3=GENERATE_CAN_ADDR(CanPhysicalAddrCentralGroup,CanDeviceLightGroupEeffect1,4),//左边三个招牌射灯组的亮暗控制
    RGB_EFFECT_CONTROL_LIGHT_GROUP_ADDR=GENERATE_CAN_ADDR(CanPhysicalAddrCentralGroup,CanDeviceLightGroupEeffect1,5),//实现控台LED的颜色亮暗控制
    RGB_EFFECT_LIGHT_ADDR_4=GENERATE_CAN_ADDR(CanPhysicalAddrCentralGroup,CanDeviceLightGroupEeffect1,6),//右边三个招牌射灯组的亮暗控制
    RGB_EFFECT_LIGHT_ADDR_5=GENERATE_CAN_ADDR(CanPhysicalAddrFrontStageLightGroup,CanDeviceLightGroupEeffect1,7),//右边三个双色射灯组内射灯的亮暗控制
    RGB_EFFECT_LIGHT_ADDR_6=GENERATE_CAN_ADDR(CanPhysicalAddrFrontStageLightGroup,CanDeviceLightGroupEeffect1,8),//右边三个双色射灯组外圈LED灯的亮暗控制
    RGB_EFFECT_2P_LIGHT_ADDR=GENERATE_CAN_ADDR(CanPhysicalAddrFrontStageLightGroup,CanDeviceLightGroupEeffect1,9),//2P顶架三色背景灯的颜色亮暗控制
    RGB_EFFECT_LIGHT_ADDR_8_RG=GENERATE_CAN_ADDR(CanPhysicalAddrFrontStageLightGroup,CanDeviceLightGroupEeffect1,10),//频闪灯的闪烁频率和亮度控制
    RGB_EFFECT_LIGHT_ADDR_8_B=GENERATE_CAN_ADDR(CanPhysicalAddrCentralGroup,CanDeviceLightGroupEeffect1,10),//T台灯的控制
    RGB_EFFECT_LIGHT_ADDR_9=GENERATE_CAN_ADDR(CanPhysicalAddrFrontStageLightGroup,CanDeviceLightGroupEeffect1,11),//频闪灯的闪烁频率和亮度控制 蓝灯未使用

    RGB_BOARD_LIGHT_ADDR_1=GENERATE_CAN_ADDR(CanPhysicalAddr1PHandGroup,CanDeviceLightGroupEeffect2,1),//1P手感应器指示灯  R 手左 G手前 B 手右
    RGB_BOARD_LIGHT_ADDR_2=GENERATE_CAN_ADDR(CanPhysicalAddr1PHandGroup,CanDeviceLightGroupEeffect2,2),//1P手感应器指示灯 R 手后 GB未使用
    RGB_BOARD_LIGHT_ADDR_3=GENERATE_CAN_ADDR(CanPhysicalAddr2PHandGroup,CanDeviceLightGroupEeffect2,3),//2P手感应器指示灯 R 手左 G手前 B 手右
    RGB_BOARD_LIGHT_ADDR_4=GENERATE_CAN_ADDR(CanPhysicalAddr2PHandGroup,CanDeviceLightGroupEeffect2,4),//2P手感应器指示灯 R 手后 GB未使用
    RGB_BOARD_LIGHT_ADDR_5=GENERATE_CAN_ADDR(CanPhysicalAddr1PBoardGroup,CanDeviceLightGroupEeffect2,5),//1P脚感应器指示灯 R 左前 G右前 B右后
    RGB_BOARD_LIGHT_ADDR_6_R=GENERATE_CAN_ADDR(CanPhysicalAddr1PBoardGroup,CanDeviceLightGroupEeffect2,6),//1P脚感应器指示灯 R 左后
    RGB_BOARD_LIGHT_ADDR_6_GB=GENERATE_CAN_ADDR(CanPhysicalAddr2PBoardGroup,CanDeviceLightGroupEeffect2,6),//2P脚感应器指示灯 G左前 B右前
    RGB_BOARD_LIGHT_ADDR_7=GENERATE_CAN_ADDR(CanPhysicalAddr2PBoardGroup,CanDeviceLightGroupEeffect2,7),//2P脚感应器指示灯 R 右后 G左后
    RGB_BOARD_LIGHT_ADDR_8_R=GENERATE_CAN_ADDR(CanPhysicalAddr1PBoardGroup,CanDeviceLightGroupEeffect2,8),//1P脚感应器指示灯 R 中间
    RGB_BOARD_LIGHT_ADDR_8_G=GENERATE_CAN_ADDR(CanPhysicalAddr2PBoardGroup,CanDeviceLightGroupEeffect2,8),//2P脚感应器指示灯 G 中间 B未使用
    RGB_BOARD_LIGHT_ADDR_9=GENERATE_CAN_ADDR(CanPhysicalAddrBackStageLightGroup,CanDeviceLightGroupEeffect2,9),//1P顶架叶子射灯 R左前 G右前 B右后
    RGB_BOARD_LIGHT_ADDR_10_R=GENERATE_CAN_ADDR(CanPhysicalAddrBackStageLightGroup,CanDeviceLightGroupEeffect2,10),//1P顶架叶子射灯 R左后
    RGB_BOARD_LIGHT_ADDR_10_GB=GENERATE_CAN_ADDR(CanPhysicalAddrBackStageLightGroup,CanDeviceLightGroupEeffect2,10),//2P顶架叶子射灯 G左前 B右前
    RGB_BOARD_LIGHT_ADDR_11=GENERATE_CAN_ADDR(CanPhysicalAddrBackStageLightGroup,CanDeviceLightGroupEeffect2,11),//2P顶架叶子射灯 R左前 G右前 B未使用

    SENSOR_1P_HAND_GROUP_ADDR_1=GENERATE_CAN_ADDR(CanPhysicalAddr1PHandGroup,CanDeviceBoardGroup,1),//1P手感应器开关 手前
    SENSOR_1P_HAND_GROUP_ADDR_2=GENERATE_CAN_ADDR(CanPhysicalAddr1PHandGroup,CanDeviceBoardGroup,2),//1P手感应器开关 手后
    SENSOR_1P_HAND_GROUP_ADDR_3=GENERATE_CAN_ADDR(CanPhysicalAddr1PHandGroup,CanDeviceBoardGroup,3),//1P手感应器开关 手左
    SENSOR_1P_HAND_GROUP_ADDR_4=GENERATE_CAN_ADDR(CanPhysicalAddr1PHandGroup,CanDeviceBoardGroup,4),//1P手感应器开关 手右
    SENSOR_1P_FOOT_GROUP_ADDR_1=GENERATE_CAN_ADDR(CanPhysicalAddr1PBoardGroup,CanDeviceBoardGroup,5),//1P脚感应器开关 左前
    SENSOR_1P_FOOT_GROUP_ADDR_2=GENERATE_CAN_ADDR(CanPhysicalAddr1PBoardGroup,CanDeviceBoardGroup,6),//1P脚感应器开关 右前
    SENSOR_1P_FOOT_GROUP_ADDR_3=GENERATE_CAN_ADDR(CanPhysicalAddr1PBoardGroup,CanDeviceBoardGroup,7),//1P脚感应器开关 左后
    SENSOR_1P_FOOT_GROUP_ADDR_4=GENERATE_CAN_ADDR(CanPhysicalAddr1PBoardGroup,CanDeviceBoardGroup,8),//1P脚感应器开关 右后
    SENSOR_1P_FOOT_GROUP_ADDR_5=GENERATE_CAN_ADDR(CanPhysicalAddr1PBoardGroup,CanDeviceBoardGroup,9),//1P脚感应器开关 中间
    SENSOR_2P_HAND_GROUP_ADDR_1=GENERATE_CAN_ADDR(CanPhysicalAddr2PHandGroup,CanDeviceBoardGroup,10),//2P手感应器开关 手前
    SENSOR_2P_HAND_GROUP_ADDR_2=GENERATE_CAN_ADDR(CanPhysicalAddr2PHandGroup,CanDeviceBoardGroup,11),//2P手感应器开关 手后
    SENSOR_2P_HAND_GROUP_ADDR_3=GENERATE_CAN_ADDR(CanPhysicalAddr2PHandGroup,CanDeviceBoardGroup,12),//2P手感应器开关 手左
    SENSOR_2P_HAND_GROUP_ADDR_4=GENERATE_CAN_ADDR(CanPhysicalAddr2PHandGroup,CanDeviceBoardGroup,13),//2P手感应器开关 手右
    SENSOR_2P_FOOT_GROUP_ADDR_1=GENERATE_CAN_ADDR(CanPhysicalAddr2PBoardGroup,CanDeviceBoardGroup,14),//2P脚感应器开关 左前
    SENSOR_2P_FOOT_GROUP_ADDR_2=GENERATE_CAN_ADDR(CanPhysicalAddr2PBoardGroup,CanDeviceBoardGroup,15),//2P脚感应器开关 右前
    SENSOR_2P_FOOT_GROUP_ADDR_3=GENERATE_CAN_ADDR(CanPhysicalAddr2PBoardGroup,CanDeviceBoardGroup,16),//2P脚感应器开关 左后
    SENSOR_2P_FOOT_GROUP_ADDR_4=GENERATE_CAN_ADDR(CanPhysicalAddr2PBoardGroup,CanDeviceBoardGroup,17),//2P脚感应器开关 右后
    SENSOR_2P_FOOT_GROUP_ADDR_5=GENERATE_CAN_ADDR(CanPhysicalAddr2PBoardGroup,CanDeviceBoardGroup,18),//2P脚感应器开关 中间

    TOUCH_SCREEN_GROUP_ADDR=GENERATE_CAN_ADDR(CanPhysicalAddrCentralGroup,CanDeviceTouchScreenGroup,1),//触摸屏

    KEYBOARD_GROUP_ADDR_1=GENERATE_CAN_ADDR(CanPhysicalAddrCentralGroup,CanDeviceKeyBoardGroup,1),//投币设备1
    KEYBOARD_GROUP_ADDR_2=GENERATE_CAN_ADDR(CanPhysicalAddrCentralGroup,CanDeviceKeyBoardGroup,2),//投币设备2
    KEYBOARD_GROUP_ADDR_6=GENERATE_CAN_ADDR(CanPhysicalAddrCentralGroup,CanDeviceKeyBoardGroup,6),//测试功能按键


    OTHERDEVICE_GROUP_ADDR_1=GENERATE_CAN_ADDR(CanPhysicalAddrCentralGroup,CanDeviceOtherGroup,1),//音柱灯条控制1
    OTHERDEVICE_GROUP_ADDR_2=GENERATE_CAN_ADDR(CanPhysicalAddrCentralGroup,CanDeviceOtherGroup,2),//音柱灯条控制2
    RGB_OTHERDEVICE_GROUP_ADDR_20=GENERATE_CAN_ADDR(CanPhysicalAddrCentralGroup,CanDeviceOtherGroup,20),//1p生命灯 R G  , B未使用
    RGB_OTHERDEVICE_GROUP_ADDR_21=GENERATE_CAN_ADDR(CanPhysicalAddrCentralGroup,CanDeviceOtherGroup,21),//2p生命灯 R G ,B未使用
};


#define BroadCastStateHigh 0xFA00
#define BroadCastOperationHigh 0xFB00

enum BroadCastCommand:uint16_t{
    BC_PlayingGuideAnimation=BroadCastStateHigh|0x10,//播放开头动画状态
    BC_WaitInertCoin=BroadCastStateHigh|0x20,//等待投币状态
    BC_WaitSelectPlayerMode=BroadCastStateHigh|0x30,//投币后选择1p,2p状态
    BC_WaitSelectGameMode=BroadCastStateHigh|0x40,//选择完1p,2p后进入选择模式状态
    BC_WaitSelectGameSong=BroadCastStateHigh|0x50,//选择模式完以后进入选歌状态
    BC_PlayingGame=BroadCastStateHigh|0x60,//进入正常游戏状态
    BC_PlayingGameState=BroadCastStateHigh|0x70,//游戏状态
    BC_PlayingGameState_1=BroadCastStateHigh|0x71,//单人模式以及双人合作模式下普通玩法状态
    BC_PlayingGameState_2=BroadCastStateHigh|0x72,//单人模式以及双人合作模式下下combo值改变发生的状态
    BC_PlayingGameState_3=BroadCastStateHigh|0x73,//单人模式以及双人合作模式下下freetime开启发生的状态
    BC_PlayingGameState_4=BroadCastStateHigh|0x74,//双人pk模式下普通状态
    BC_PlayingGameState_5=BroadCastStateHigh|0x75,//双人pk模式下某个玩家领先状态
    BC_PlayingGameState_6=BroadCastStateHigh|0x76,//双人pk模式下某个玩家被KO时的状态
    BC_PlayingGameCalculateScores=BroadCastStateHigh|0x80,//游戏结束计分统计时候的状态
    BC_ShowLeaderBoard=BroadCastStateHigh|0x90,//排行榜状态
    BC_SelectSongWithMp3Source=BroadCastStateHigh|0xA0,//在MP3里选歌的状态
    BC_PlayingMp3Source=BroadCastStateHigh|0xB0,//播放MP3的歌曲进行的状态
    BC_HardwareReady=BroadCastStateHigh|0xE0,//硬件准备好状态

    BC_InertCoinOperation=BroadCastOperationHigh|0x10,//投币动作
    BC_InertCoinOperation_1=BroadCastOperationHigh|0x11,//投币，但没有投满
    BC_InertCoinOperation_2=BroadCastOperationHigh|0x12,//投币，达到要求投满
    BC_InertUsbMp3SourceDevice=BroadCastOperationHigh|0x20,//插入U盘准备进行MP3播放的动作
    BC_PulloutUsbMp3SourceDevice=BroadCastOperationHigh|0x30,//MP3拔出的动作
    BC_BoardOperation=BroadCastOperationHigh|0x40,//对踏板动作
    BC_BoardOperation_1=BroadCastOperationHigh|0x41,//打开板踏1
    BC_BoardOperation_2=BroadCastOperationHigh|0x42,//打开踏板2
    BC_BoardOperation_3=BroadCastOperationHigh|0x43,//打开两个踏板
    BC_BoardOperation_4=BroadCastOperationHigh|0x49,//关闭踏板1
    BC_BoardOperation_5=BroadCastOperationHigh|0x4A,//关闭踏板2
    BC_BoardOperation_6=BroadCastOperationHigh|0x4B,//关闭两个踏板

    BC_EnterDeviceSyncState=0x0100,//进入设备同步状态,大量修改灯光前需进入此状态
};
enum PillarCmdType:uint8_t{
    SET_PILLAR_SYSTEM_MODE=0x00,//系统工作模式
    SET_PILLAR_COLOR=0x01,//设置音柱灯颜色
    SET_PILLAR_MOVE_MODE=0x02,//设置运动模式
    SET_PILLAR_BPM=0x03,//设置BPM值 （led运动速度）
    SET_PILLAR_LEVEL=0x04,//信号电平值  决定LED的最高点
};
enum PillarSystemMode:uint8_t{
    PILLAR_MODE_FREE=0X00,//自由模式
    PILLAR_MODE_CONTROL=0X01,//受控模式
    PILLAR_MODE_USER_SET=0X02,//测试模式 红绿蓝白交替亮灯
    PILLAR_MODE_STOPPED=0X03,//关闭音柱
};

enum PillarColorModeDef:uint8_t{
    PILLAR_COLOR_WHITE=0X00,//白
    PILLAR_COLOR_BLUE,//蓝
    PILLAR_COLOR_GREEN,//绿
    PILLAR_COLOR_RED,//红
    PILLAR_COLOR_CYAN,//青
    PILLAR_COLOR_ORANGE,//橙
    PILLAR_COLOR_YELLOW,//黄
    PILLAR_COLOR_EFFECT_PICTURE,//效果图色
    PILLAR_COLOR_CASCADE_TRICHROMATIC//三基色级联
};

enum PillarMoveModeDef:uint8_t{
    PILLAR_MOVE_UP_TO_DOWN_RGB=0x00,//从上往下，红绿蓝三色跑马
    PILLAR_MOVE_DOEN_TO_UP_RGB,//从下往上，红绿蓝三色跑马
    PILLAR_MOVE_UP_TO_DOWN_3LED,//从上往下，三个LED为单元跑马，可七色
    PILLAR_MOVE_DOWN_TO_UP_3LED,//从下往上，三个LED为单元跑马，可七色
    PILLAR_MOVE_BORDER_TO_MIDDLE_3LED,//从两边往中间，三个LED为单元跑马，可七色
    PILLAR_MOVE_MIDDLE_TO_BORDER_3LED,//从中间往两边，三个LED为单元跑马，可七色
    PILLAR_MOVE_UP_TO_DOWN_INCREASE_BY_STEP=0X06,//从上往下，逐级递增，可七色
    PILLAR_MOVE_DOWN_TO_UP_DECLINE_BY_STEP=0x07,//从下往上，逐级递减，可七色
    PILLAR_MOVE_DOWN_TO_UP_INCREASE_BY_STEP,//从下往上，逐级递增，可七色
    PILLAR_MOVE_UP_TO_DOWN_DECLINE_BY_STEP,//从上往下，逐级递减，可七色
    PILLAR_MOVE_BORDER_TO_MIDDLE_INCREASE_BY_STEP,//从两头往中间，逐级递增，可七色
    PILLAR_MOVE_MIDDLE_TO_BORDER_DECLINE_BY_STEP,//从中间往两头，逐级递减，可七色
    PILLAR_MOVE_MIDDLE_TO_BORDER_INCREASE_BY_STEP,//从中间往两边，逐级递增，可七色
    PILLAR_MOVE_BORDER_TO_MIDDLE_DECLINE_BY_STEP=0X0D,//从两边往中间，逐级递减，可七色
    PILLAR_MOVE_UP_TO_DOWN_INCREASE_BY_STEP_RGB,//从上往下，三基色间隔为三依次递增
    PILLAR_MOVE_DOWN_TO_UP_DECLINE_BY_STEP_RGB,//从下往上，三基色间隔为三依次递减
    PILLAR_MOVE_DOWN_TO_UP_INCREASE_BY_STEP_RGB,//从下往上，三基色间隔为三依次递增
    PILLAR_MOVE_UP_TO_DOWN_DECLINE_BY_STEP_RGB//从上往下，三基色间隔为三依次递减
};

enum PillarBpmLimt:uint8_t{
    PILLAR_BPM_BELOW_BOUND=0X00,
    PILLAR_BPM_UP_BOUND=0xF0,
};

enum PillarLevelLimt:uint8_t{
    PILLAR_LEVEL_BELOW_BOUND=0X00,
    PILLAR_LEVEL_UP_BOUND=0x64,
};
enum CanControlBitType{
    CAN_CONTROL_OPTION_READ_WRITE=0x1,//0 read 1 write
    CAN_CONTROL_OPTION_QUERY=0x2,//0 request 1 response
    CAN_CONTROL_OPTION_STATUS=0x4,//0 success 1 error
    CAN_CONTROL_OPTION_WORK_STATE=0x8,//0 idle 1 busy
    CAN_CONTROL_OPTION_RESERVE=0XF,//reserve
};

class CanControlBase:public Object{
public://signals
    /**
     * @brief 后台(测试)按钮点击一次
     */
    Signal<> notifyBackgroundButtonPressed;
    /**
     * @brief 投币按钮点击一次
     */
    Signal<> notifyInsertCoin;
    /**
     * @brief 1P踏板通讯组测试通讯正常
     */
    Signal<> notify1PGroupSuccess;
    /**
     * @brief 2P踏板通讯组测试通讯正常
     */
    Signal<> notify2PGroupSuccess;
    /**
     * @brief 前舞台灯通讯组通讯正常
     */
    Signal<> notifyFrontStageGroupSuccess;
    /**
     * @brief 后舞台灯通讯组通讯正常
     */
    Signal<> notifyBackStageGroupSuccess;
    /**
     * @brief 主控通讯组通讯正常
     */
    Signal<> notifyCentralControlSuccess;
    /**
     * @brief 测试模式下,传感器状态发生变化 ，对红外传感器而言，false代表被无遮挡，true代表遮挡。对压力传感器而言，true代表被压下，false代表释放
     */
    Signal<uint8_t/*BoardSensorId*/,bool> notifyTestSensorStatusChanged;
    /**
     * @brief 踏板区域状态变化
     */
    Signal<BoardFiledType,BoardFiledStatus>notifyBoardFiledStatusChanged;
    /**
     * @brief notifyScreenTouchEvent 触屏点击事件
     */
    Signal<uint16_t,uint16_t,ScreenTouchEvent>notifyScreenTouchEvent;
public:
    /**
     * @brief CanControlBase
     * @param parent  事件循环对象
     * @param can_device can设备名称 如can0
     * @param baud_rate 波特率 can设备工作波特率
     * 输入的相应参数会被用于重新初始化can设备
     */
    CanControlBase(Object *parent,const std::string &can_device="can0")
        :Object(parent),notifyBackgroundButtonPressed(this),notifyInsertCoin(this),
          notify1PGroupSuccess(this),notify2PGroupSuccess(this),notifyFrontStageGroupSuccess(this),notifyBackStageGroupSuccess(this)
        ,notifyCentralControlSuccess(this),notifyTestSensorStatusChanged(this),notifyBoardFiledStatusChanged(this),
          notifyScreenTouchEvent(this),  canDeviceName(can_device){

    }
    virtual ~CanControlBase(){}
    /**
     * @brief sendTestControlCommand 发送单个控制命令组
     * @param cmd 待发送的控制命令组
     * @return 若无法发送，返回false，发送成功返回true
     */
    virtual bool sendTestControlCommand(CanCommandType cmd)=0;
    /**
     * @brief sendTestControlCommandList 发送多个控制命令组
     */
    virtual bool sendTestControlCommandList(const std::list<CanCommandType>& cmdList)=0;
    /**
     * @brief initCan 初始化can设备，开启can网卡，设置其工作波特率
     * @return 若失败返回false
     */
    virtual bool initCan()=0;
    /**
     * @brief startCan 启动can服务
     * @return
     */
    virtual bool startCan()=0;
    /**
     * @brief stopCan 停止can服务
     */
    virtual void stopCan()=0;
    /**
     * @brief sendBroadCastCommand 发送广播命令，主要用于同步及设置硬件状态
     */
    virtual bool sendBroadCastCommand(BroadCastCommand /*cmd*/){return false;}
    /**
     * @brief sendLightControlCommand
     * @param type 灯光控制命令
     * @param data 灯光控制数据部分 根据CanLightDataType 中描述填充
     * std::optional must compile with -std=c++17
     * 如果某个命令不需要数据，此位应置false
     */
    virtual bool sendLightControlCommand(CanDeviceAddr /*addr*/,CanLightOperationType /*type*/,uint8_t /*data */,bool /*withData*/){return false;}
    /**
     * @brief sendPillarControlCommand
     * @param type 音柱控制命令
     * @param data 音柱控制数据部分 
     * 如果某个命令不需要数据，此位应置false
     */
    virtual bool sendPillarControlCommand(CanDeviceAddr /*addr*/,PillarCmdType /*type*/,uint8_t /*data */,bool /*withData*/){return false;}
protected:
    static const uint32_t CAN_HEAD_SIZE=2+1+1+2;
    virtual std::list<canControlFrame> getTestCommandFrameList(CanCommandType type)=0;
    virtual void sendCanFrame(const canControlFrame&frame)=0;
    virtual canControlFrame packCanFrame(const uint8_t *data,uint32_t dataLen,uint16_t localAddr, uint16_t remoteAddr,uint32_t remoteCanId)
    {//default implement
        canControlFrame frame;
#ifdef DEBUG
        assert(dataLen>0);
#endif
        if(dataLen==0)return frame;
        frame.data_len=dataLen+8;
        frame.data.reset(new uint8_t[frame.data_len+1],std::default_delete<uint8_t[]>());
        memset(frame.data.get(),0,frame.data_len+1);
        auto data_ptr=frame.data.get();
        data_ptr[0]=0X55;
        data_ptr[1]=0xAA;
        data_ptr[3]=dataLen+2;
        data_ptr[4]=(localAddr>>8)&0xff;
        data_ptr[5]=(localAddr)&0xff;
        data_ptr[6]=(remoteAddr>>8)&0xff;
        data_ptr[7]=(remoteAddr)&0xff;
        memcpy(data_ptr+8,data,dataLen);
        uint8_t sum=0;
        for(uint8_t i=3;i<frame.data_len;++i)
        {
            sum+=data_ptr[i];
        }
        data_ptr[2]=0x100-sum;
        uint32_t index=0;
        //
        frame.optionCanIdList.push_front(remoteCanId|(index<<5));
        uint32_t offset=frame.data_len-8;
        while(offset>0)
        {
            ++index;
            frame.optionCanIdList.push_front(remoteCanId|(index<<5));
            if(offset<8)break;
            offset-=8;
        }
        return frame;
    }
    virtual canControlFrame unpackCanFrame(std::shared_ptr<canControlFrame>frameCache)
    {// 0x55 0xAA checksum payload_len addrhigh addrlow ...(payload)
        //checksum = 0x100 -sum(payload_len/addrhigh/addrlow/payload)
        //每次只解析一帧
        canControlFrame frame;
        uint32_t offset=0;
        auto data_ptr=frameCache->data.get();
        auto data_len=frameCache->data_len;
        do{
            //wait recv
            if(frameCache->data_len<CAN_HEAD_SIZE)break;
            bool find=false;

            while(offset+CAN_HEAD_SIZE<data_len)
            {
                if((data_ptr[offset]==0x55)&&(data_ptr[offset+1]=0xAA))
                {
                    find=true;
                    break;
                }
                ++offset;
            }
            if(!find){
                frame.error_flag=true;
                break;
            }
            uint32_t payload_len=data_ptr[offset+3];
            //wait recv
            if(payload_len+offset+CAN_HEAD_SIZE>data_len){
                if(data_len>=canControlFrame::MAX_CAN_PAYLOAD_SIZE)
                {
                    offset+=2;
                    continue;
                }
                break;
            }
            uint32_t frame_len=payload_len+CAN_HEAD_SIZE;
            //filter
            if(payload_len>canControlFrame::MAX_CAN_PAYLOAD_SIZE)
            {
                offset+=frame_len;
                break;
            }
            //check
            uint8_t check_sum=0;
            for(uint32_t i=3;i<frame_len;++i)
            {
                check_sum+=data_ptr[offset+i];
            }
            check_sum=0x100-check_sum;
            //check success
            if(check_sum==data_ptr[2])
            {
                frame.data_len=frame_len;
                frame.optionCanIdList.push_back(frameCache->optionCanIdList.back());
                frame.data.reset(new uint8_t[frame_len+1],std::default_delete<uint8_t[]>());
                memset(frame.data.get(),0,frame_len+1);
                memcpy(frame.data.get(),data_ptr+offset,frame_len);
            }
            else {
                frame.error_flag=true;
                offset+=2;
                continue;
            }
            offset+=frame_len;
        }while(0);
        //move buf
        if(offset>0)
        {
            auto left_len=data_len-offset;
            if(left_len>0)memmove(frameCache->data.get(),frameCache->data.get()+offset,left_len);
            frameCache->data_len=left_len;
        }
        return frame;
    }

    canControlFrame packControlCanFrame(uint8_t operationType/*CanLightOperationType*/,uint8_t value/*CanLightDataType*/,uint16_t localAddr, uint16_t remoteAddr,uint32_t remoteCanId)
    {
        uint8_t data[2]={operationType,value};
        return packCanFrame(data,2,localAddr,remoteAddr,remoteCanId);
    }
    canControlFrame unpackControlFrame(std::shared_ptr<canControlFrame>frameCache)
    {
        return unpackCanFrame(frameCache);
    }
    virtual uint32_t physicalAddrToCanId(uint32_t/*CanPhysicalAddr*/ idGroup,uint8_t controlId=CAN_CONTROL_OPTION_READ_WRITE)
    {
        uint16_t desADDr=idGroup;
        uint8_t pack_num=0;
        uint16_t desH=desADDr>>8;
        uint16_t desL=desADDr&0xFF;
        uint32_t stdId=(desH<<3)|(desL>>5);
        uint32_t extid=((desL&0x1f)<<13)|(pack_num<<5)|(controlId&0x1f);
        stdId &=0x7FF;
        extid &=0x3FFFF;
        stdId=stdId<<18;
        uint32_t can_id=stdId|extid;
        return can_id;
    }
    static CanPhysicalAddr canIdToPhysicalAddr(uint32_t canid)
    {
        uint8_t addrH=(canid>>21)&0xFF;
        uint8_t addrL=(canid>>13)&0xFF;
        uint16_t addr=(addrH<<8)|addrL;
        switch (addr) {
        case CanPhysicalAddrApplicationGroup:
            return CanPhysicalAddrApplicationGroup;
        case CanPhysicalAddr1PHandGroup:
            return CanPhysicalAddr1PHandGroup;
        case CanPhysicalAddr2PHandGroup:
            return CanPhysicalAddr2PHandGroup;
        case CanPhysicalAddr1PBoardGroup:
            return CanPhysicalAddr1PBoardGroup;
        case CanPhysicalAddr2PBoardGroup:
            return CanPhysicalAddr2PBoardGroup;
        case CanPhysicalAddrFrontStageLightGroup:
            return CanPhysicalAddrFrontStageLightGroup;
        case CanPhysicalAddrBackStageLightGroup:
            return CanPhysicalAddrBackStageLightGroup;
        case CanPhysicalAddrCentralGroup:
            return CanPhysicalAddrCentralGroup;
        case CanPhysicalAddrBroadcastGroup:
            return CanPhysicalAddrBroadcastGroup;
        default:
            return CanPhysicalAddrGroupUnknown;
        }
    }
protected:
    /**
     * @brief canDeviceName can设备名称
     */
    std::string canDeviceName;
};
} // namespace aimy
#endif // CANTYPEDEF_H
