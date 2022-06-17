#include "VirtualDevice/Machine.h"
#include <functional>
#include "LocalSocket.h"
#include "VirtualMachine/protocol_caller.h"
#include "VirtualMachine/protocol.h"

namespace VirtualDevice
{

Machine* Machine::smp_this = nullptr;
const char* const Machine::scm_path_service = "/tmp/aam.virtual_device.caller.service";
const char* const Machine::scm_path_client_default = "/tmp/aam.virtual_device.caller.client";

Machine::~Machine()
{
    m_status = IDLE;
    delete mp_client;

    if (m_thread_notify.joinable())
        m_thread_notify.join();
}

bool
Machine::Open(const char *path_local_socket)
{
    m_status = RUNNING;
    mp_client = new (std::nothrow) LocalSocket(path_local_socket ? path_local_socket : scm_path_client_default, scm_path_service, std::bind(&Machine::NotifyRead, this, std::placeholders::_1, std::placeholders::_2));
    if (!mp_client)
        return false;

    if (!mp_client->Listen())
        return false;

    m_thread_notify = std::thread(&Machine::ThreadNotify, this);
    Register();
    return true;
}

void
Machine::Close()
{
    m_status = IDLE;
    delete mp_client;
    mp_client = nullptr;

    if (m_thread_notify.joinable())
        m_thread_notify.join();
}

void
Machine::Register()
{
    size_t stream_length = sizeof(VDProtocol::Caller::Body);
    VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
    p_stream->command = VDProtocol::Caller::Command::CLIENT_STARTUP;
    Write(p_stream, stream_length);
}

void
Machine::QueryDevices()
{
    size_t stream_length = sizeof(VDProtocol::Caller::Body);
    VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
    p_stream->command = VDProtocol::Caller::Command::QUERY_DEVICE;
    Write(p_stream, stream_length);
}

void
Machine::QueryVirtualDevice()
{
    size_t stream_length = sizeof(VDProtocol::Caller::Body);
    VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
    p_stream->command = VDProtocol::Caller::Command::QUERY_VIRTUAL_DEVICE_VERSION;
    Write(p_stream, stream_length);
}

void
Machine::NotifyRead(const void* p_data, size_t length)
{
    if (length < sizeof(VDProtocol::FrameV1)) {
        printf("Protocol error\n");
        return;
    }

    const VDProtocol::FrameV1 *p_frame = (decltype(p_frame))p_data;
    size_t frame_length = length;
    size_t body_length, stream_length;
    const void *p_stream;

    switch (p_frame->stream_offset) {
    case 0:
        body_length = 0;
        stream_length = frame_length;
        p_stream = p_frame->body;
        break;
    case ~(decltype(p_frame->stream_offset))0:
        body_length = frame_length;
        stream_length = 0;
        p_stream = nullptr;
        break;
    default:
        body_length = p_frame->stream_offset;
        stream_length = frame_length - body_length;
        p_stream = p_frame->body + body_length;
    }

    DispatchCommand(p_stream, stream_length);
}

void
Machine::DispatchCommand(const void* p_stream, size_t length)
{
    const VDProtocol::Caller::Body *p_protocol = (decltype(p_protocol))p_stream;

#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wswitch"
#endif//__GNUC__

    switch (p_protocol->command) {
    case VDProtocol::Caller::Command::QUERY_DEVICE:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;

        VDProtocol::Caller::QueryDevice *p_info_src = (decltype(p_info_src))p_protocol->data;
        DeviceInfo *p_info_dst = (decltype(p_info_dst))notify_element.data;
        p_info_dst->status = (decltype(p_info_dst->status))p_info_src->status;
        strncpy(p_info_dst->product_id, p_info_src->product_id, sizeof(p_info_dst->product_id));
        strncpy(p_info_dst->version, p_info_src->version, sizeof(p_info_dst->version));
        strncpy(p_info_dst->date, p_info_src->date, sizeof(p_info_dst->date));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::QUERY_VIRTUAL_DEVICE_VERSION:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;

        const char *p_info_src = (decltype(p_info_src))p_protocol->data;
        char *p_info_dst = (decltype(p_info_dst))notify_element.data;
        strncpy(p_info_dst, p_info_src, sizeof(notify_element.data));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::QUERY_YUEDONG_VERSION:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;

        VDProtocol::Caller::GetYuedongVersion *p_version = (decltype(p_version))p_protocol->data;
        memcpy(notify_element.data, p_version, sizeof(*p_version));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::CONSOLE_INSERT_COIN:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::CONSOLE_MIC_STATE:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, sizeof(VDProtocol::Caller::ConsoleMicState));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::CONSOLE_STATUS:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, sizeof(VDProtocol::Caller::ConsoleStatus));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::CONSOLE_KEY:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, sizeof(VDProtocol::Caller::ConsoleKey));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::CONSOLE_VOLUME:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, sizeof(VDProtocol::Caller::ConsoleVolume));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::MUSIC_BOX_PANEL_KEY:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, sizeof(VDProtocol::Caller::MusicBoxPanelKey));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::REMOTE_KEY:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, sizeof(VDProtocol::Caller::RemoteKey));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::MIC_U_CHANNEL:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, sizeof(VDProtocol::Caller::MicUModuleChannel));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::MIC_U_SIGNAL_STRENGTH:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, sizeof(VDProtocol::Caller::MicUModuleSignalStrength));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::MIC_U_SCAN_FREQ_BAND:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, length - sizeof(VDProtocol::Caller::Body));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::MIC_U_GET_FREE_FREQ_BAND:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, sizeof(VDProtocol::Caller::MicUReplyGetFreeFreqBand));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::MIC_U_SET_FREQ_BAND:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, sizeof(VDProtocol::Caller::MicUReplySetFreqBand));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::EFFECTOR_CHANNEL_STATUS:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, sizeof(VDProtocol::Caller::EffectorChannelStatus));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::EFFECTOR_GET_SING_SWITCH:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        memcpy(notify_element.data, p_protocol->data, sizeof(VDProtocol::Caller::SingSwitch));
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::BLUE_REPLY:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        VDProtocol::Caller::BlueReply *p_body = (decltype(p_body))p_protocol->data;
        const size_t c_len_body = sizeof(*p_body) + p_body->len_msg;
        memcpy(notify_element.data, p_protocol->data, c_len_body);
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::MODULE_5G8_STATUS:
    {
        NotifyElement notify_element;
        notify_element.command = (decltype(notify_element.command))p_protocol->command;
        notify_element.data[0] = *p_protocol->data;
        m_lock_queue_notify.lock();
        m_queue_notify.push(notify_element);
        m_lock_queue_notify.unlock();
        break;
    }
    case VDProtocol::Caller::Command::ECHO_COMMAND:
    {
#ifdef DEBUG
        fprintf(stderr,"recv echo message size[%llu] ",length);
        const uint8_t *data=(const uint8_t *)p_stream;
        for(size_t i=0;i<length;++i)
        {
            fprintf(stderr," %02x",data[i]);
        }
        fprintf(stderr,"\r\n");
        for(size_t i=0;i<length;++i)
        {
            fprintf(stderr,"%c",data[i]);
        }
        fprintf(stderr,"\r\n");
#endif
        break;
    }
    }

#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif//__GNUC__

}

void
Machine::ThreadNotify()
{
    while (m_status) {
        if (!m_queue_notify.empty()) {
            m_lock_queue_notify.lock();
            const NotifyElement &frame = m_queue_notify.front();
            m_lock_queue_notify.unlock();

#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wswitch"
#endif//__GNUC__

            switch ((VDProtocol::Caller::Command)frame.command) {
            case VDProtocol::Caller::Command::QUERY_DEVICE:
                NotifyQueryDevice((const DeviceInfo*)frame.data);
                break;
            case VDProtocol::Caller::Command::QUERY_VIRTUAL_DEVICE_VERSION:
                NotifyQueryVirtualDevice((char*)frame.data);
                break;
            default:
                if (Effector::DispatchNotify(frame.command, frame.data))
                    break;

                if (ConsoleMinikV0::DispatchNotify(frame.command, frame.data))
                    break;

                if (RemoteControl::DispatchNotify(frame.command, frame.data))
                    break;

                if (MicrophoneU::DispatchNotify(frame.command, frame.data))
                    break;

                if (Bluetooth::DispatchNotify(frame.command, frame.data))
                    break;

                if (Module5G8::DispatchNotify(frame.command, frame.data))
                    break;

                if (PanelMusicBox::DispatchNotify(frame.command, frame.data))
                    break;
            }

#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif//__GNUC__

            m_lock_queue_notify.lock();
            m_queue_notify.pop();
            m_lock_queue_notify.unlock();
        }
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }
}

int
Machine::Write(const void *p_data, size_t length) const
{
    if (!p_data)
        return -1;

    size_t frame_length = sizeof(VDProtocol::FrameV1) + length;
    VDProtocol::FrameV1 *p_frame = (decltype(p_frame))alloca(frame_length);
    p_frame->frame.version = 0;
    p_frame->compress_flag = 0;
    p_frame->stream_offset = 0;
    memcpy(p_frame->body, p_data, length);

    if (mp_client && mp_client->Write(p_frame, frame_length) == (ssize_t)frame_length)
        return 0;
    return -2;
}

}//namespace VirtualDevice
