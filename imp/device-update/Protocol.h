#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdint.h>

class Protocol
{
public:
    enum class HeadFlag : uint8_t {
        FLAG_1 = 0x55,
        FLAG_2 = 0xAA
    };
    union ControlCode {
        struct {
            uint8_t read_write_flag : 1;	//0:读; 1:写;
            uint8_t response_flag : 1;		//0:设备主动返回信息; 1:设备被动返回信息;
            uint8_t dst_addr_switch : 1;	//0:不带目的地址; 1:带目的地址;
            uint8_t src_addr_switch : 1;	//0:不带源地址; 1:带源地址;
            uint8_t encrypt_switch : 1;		//0:不加密传输; 1:加密传输; 目前未使用
            uint8_t subsequent_packet : 1;	//多包长帧续传标志位, fragment置1时生效;	0:首包数据; 1:续传包数据
            uint8_t fragment : 1;			//分包号开关
            uint8_t control_frame_switch : 1;//0:协议帧; 1:控制帧; ControlProtocol::command是二维的(ControlCode::control_frame_switch, ControlProtocol::command)
        } bit;
        uint8_t byte;
    };

public:
    static void Mux(const void *p_input, uint32_t input_length, const unsigned *p_address, unsigned control_code, void *p_output, uint32_t &output_length)  ;
    /**
     * @brief DeMux
     * @param p_input
     * @param input_length
     * @param p_output
     * @param output_length
     * @param p_address
     * @param control_code
     * @return  -1 failed 0:success 1:data not enough
     */
    static int DeMux(void *p_input, uint32_t input_length, void **p_output, uint32_t &output_length, unsigned *p_address, unsigned &control_code);
    static uint32_t GetMinProtocolSize();

private:
    typedef uint8_t CheckCode;
    static CheckCode GenCheckCode(const void *p_data, uint32_t length);

private:


    typedef uint16_t Address;

#pragma pack(push, 1)


    struct AddressInfo {
        uint8_t size;//以字节为单位
        Address addresses[0];
    };
//frame context
// lenght 55 aa control_code payload_len [src_addr_len src_addr] [des_addr_len des_addr] checkcode
    struct Head {
        uint8_t length;
        HeadFlag start_flag1;//固定起始标识
        HeadFlag start_flag2;//固定起始标识
        ControlCode control_code;	//控制字
        uint8_t data_length;
        uint8_t data[0];
    };


#pragma pack(pop)
};
#endif // PROTOCOL_H
