#include "Protocol.h"
#include <string.h>
#include <stddef.h>
#include "log/aimy-log.h"
#if defined(__linux) || defined(__linux__)
#	include <endian.h>
#endif

void
Protocol::Mux(const void *p_input, uint32_t input_length, const unsigned *p_address, unsigned control_code, void *p_output, uint32_t &output_length)
{
    Head *p_head = (Head*)p_output;
    p_head->start_flag1 = HeadFlag::FLAG_1;
    p_head->start_flag2 = HeadFlag::FLAG_2;

    ControlCode code;
    code.byte = (uint8_t)control_code;
    p_head->control_code.byte = 0;
    p_head->control_code.bit.read_write_flag = code.bit.read_write_flag;
    p_head->control_code.bit.control_frame_switch = code.bit.control_frame_switch;
    p_head->data_length = input_length;

    uint8_t length = sizeof(*p_head) - sizeof(p_head->length) + input_length + sizeof(CheckCode);
    memcpy(p_head->data, p_input, input_length);

    if (p_address) {
        p_head->control_code.bit.dst_addr_switch = 1;

        AddressInfo *p_address_list = (AddressInfo*)((uint8_t*)p_head->data + input_length);
        p_address_list->size = sizeof(Address);
        p_address_list->addresses[0] = htobe16((Address)*p_address);

        length += sizeof(AddressInfo) + sizeof(Address);
    }

    p_head->length = length;
    ((uint8_t*)p_head)[length] = GenCheckCode(&p_head->control_code, length - offsetof(Head, control_code));
    output_length = length + sizeof(p_head->length);
}

int
Protocol::DeMux(void *p_input, uint32_t input_length, void **p_output, uint32_t &output_length, unsigned *p_address, unsigned &control_code)
{
    bool ret=-1;
    do{
        if (sizeof(Head) >= input_length)
         {
            ret=1;
            break;
        }

        Head *p_head = (Head*)p_input;
        if (p_head->start_flag1 != HeadFlag::FLAG_1 || p_head->start_flag2 != HeadFlag::FLAG_2)
        {
            break;
        }

        uint64_t data_length = p_head->data_length;
        uint64_t cur_length = sizeof(Head) + data_length;
        const uint8_t *p_cur_section = p_head->data + data_length;

        uint64_t dst_addr_size = 0, src_addr_size = 0;
        if (p_head->control_code.bit.dst_addr_switch) {
            cur_length += sizeof(AddressInfo);
            if (cur_length >= input_length)
            {
                ret=1;
                break;
            }

            dst_addr_size = ((AddressInfo*)p_cur_section)->size;
            cur_length += dst_addr_size;
            p_cur_section = (const uint8_t*)p_head + cur_length;
        }

        if (p_head->control_code.bit.src_addr_switch) {
            cur_length += sizeof(AddressInfo);
            if (cur_length >= input_length)
            {
                ret=1;
                break;
            }

            AddressInfo *p_src_addr_info = (AddressInfo*)p_cur_section;
            src_addr_size = p_src_addr_info->size;
            if (!src_addr_size)
            {
                break;
            }

            cur_length += src_addr_size;
            if (cur_length >= input_length)
            {
                ret=1;
                break;
            }
            p_cur_section = (const uint8_t*)p_head + cur_length;

            if (p_address) {
                *p_address = be16toh(p_src_addr_info->addresses[0]);
            }
        }

        if (cur_length + sizeof(CheckCode) > input_length)
        {
            ret=1;
            break;
        }

        auto check_code = GenCheckCode(&p_head->control_code, cur_length - offsetof(Head, control_code));
        if (((uint8_t*)p_head)[cur_length] != check_code)
        {
            break;
        }
        control_code = (unsigned)p_head->control_code.byte;
        *p_output = p_head->data;
        output_length = p_head->data_length;
        ret=0;
    }while(0);
    return ret;
}

uint32_t Protocol::GetMinProtocolSize()
{
    return sizeof(Head) + sizeof(uint8_t) + sizeof(CheckCode) + sizeof(AddressInfo) + sizeof(Address);
}

Protocol::CheckCode
Protocol::GenCheckCode(const void *p_data, uint32_t length)
{
    const CheckCode *p_src = (decltype(p_src))p_data;
    const uint64_t c_unit_count = length / sizeof(*p_src);
    CheckCode check_code = 0;
    for (uint64_t i = 0; i < c_unit_count; ++i)
        check_code ^= p_src[i];
    return check_code;
}
