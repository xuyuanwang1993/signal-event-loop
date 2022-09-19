#include "websocket_connection_helper.h"
#include "third_party/md5/md5.h"
#include "third_party/base64/base64.h"
#define WEBSOCKET_HTTP_VERSION "HTTP/1.1"
#define WEBSOCKET_OPTION "GET"
using namespace aimy;
WebsocketConnectionHelper::WebsocketConnectionHelper(TaskScheduler *_parent,SOCKET _fd):TcpConnection(_parent,_fd),notifyNewWSConnection(this)
  ,protocalHttp(nullptr),connectionStatus(WebsocketStatus::WebsocketPassive),timeoutTimer(nullptr),websocketApiName("")
{
    protocalHttp.reset(new ProtocalHttp());
    setProtocal(protocalHttp);
}

 WebsocketConnectionHelper::~WebsocketConnectionHelper()
 {
     if(timeoutTimer)
     {
         timeoutTimer->stop();
         timeoutTimer->release();
     }
 }

void WebsocketConnectionHelper::activeSetup(const std::string & api)
{
    return invoke(Object::getCurrentThreadId(),[=](){
        if(connectionStatus!=WebsocketPassive){
            AIMY_WARNNING("only WebsocketPassive can active setup websocket connection!");
            return ;
        }
        connectionStatus=WebsocketActiveConnecting;
        websocketApiName=api;
        if(websocketApiName=="")websocketApiName="/";
        std::vector<std::string>header;
        header.push_back(WEBSOCKET_OPTION);
        header.push_back(websocketApiName);
        header.push_back(WEBSOCKET_HTTP_VERSION);
        std::map<std::string,std::string>keys;
        keys["Host"]=getPeerHostName()+":"+std::to_string(getPeerPort());
        keys["Upgrade"]="websocket";
        keys["Connection"]="Upgrade";
        keys["Sec-WebSocket-Key"]=generateSessionKey(this);
        keys["Sec-WebSocket-Version"]="13";

        auto buffer=protocalHttp->packetFrame(header,keys,nullptr,0);

        writeCache->appendFrame(buffer.first.get(),buffer.second);
        if(writeCache->frame_count()==1)
        {
            channel->enablWriting();
            channel->sync();
        }
    });
}

void WebsocketConnectionHelper::abort()
{
    return invoke(Object::getCurrentThreadId(),[=](){
        connectionStatus=WebsocketClosed;
        disconnected();
    });
}

void WebsocketConnectionHelper::setTimeout(uint32_t time_msec)
{
    return invoke(Object::getCurrentThreadId(),[=](){
        if(connectionStatus==WebsocketClosed)
        {
            AIMY_WARNNING("discard closed connection!");
            return ;
        }
        if(!timeoutTimer)
        {
            timeoutTimer=scheduler->addTimer(time_msec);
            timeoutTimer->setSingle(true);
            timeoutTimer->timeout.connect(this,std::bind(&WebsocketConnectionHelper::abort,this));
        }
        else {
            AIMY_WARNNING("timer is working!");
        }
    });
}

WebsocketConnectionHelper::WebsocketStatus WebsocketConnectionHelper::getStatus()
{
    return invoke(Object::getCurrentThreadId(),[=](){
        return  connectionStatus;
    });
}

void WebsocketConnectionHelper::setProtocal(std::shared_ptr<ProtocalBase> _protocal)
{
    TcpConnection::setProtocal(_protocal);
}

void WebsocketConnectionHelper::passiveResponse(const std::string &client_key)
{
    return invoke(Object::getCurrentThreadId(),[=](){
        if(connectionStatus!=WebsocketPassive){
            AIMY_WARNNING("only WebsocketPassive can active setup websocket connection!");
            return ;
        }
        connectionStatus=WebsocketPassiveConnecting;
        std::vector<std::string>header;
        header.push_back(WEBSOCKET_HTTP_VERSION);
        header.push_back("101");
        header.push_back("Switching Protocols");

        std::map<std::string,std::string>keys;
        keys["Upgrade"]="websocket";
        keys["Connection"]="Upgrade";
        keys["Sec-WebSocket-Accept"]=generateServerAcceptKey(client_key);

        auto buffer=protocalHttp->packetFrame(header,keys,nullptr,0);

        writeCache->appendFrame(buffer.first.get(),buffer.second);
        if(writeCache->frame_count()==1)
        {
            channel->enablWriting();
            channel->sync();
        }
    });
}

void WebsocketConnectionHelper::handleWebsocketPassiveConnection(std::shared_ptr<uint8_t> frame,uint32_t len)
{
    aimy::ProtocalHttp protocal;
    aimy::StreamHttpFrame http_frame;
    protocal.bufferToFrame(frame.get(),len,&http_frame);
    do{
        if(http_frame.header.content_length<0)
        {
            break;
        }
        if(http_frame.header.header.size()!=3)
        {
            AIMY_ERROR("miss header filed!");
            break;
        }

        std::string option=http_frame.header.header[0];
        websocketApiName=http_frame.header.header[1];
        std::string http_version=http_frame.header.header[2];
        if(option!=WEBSOCKET_OPTION||http_version!=WEBSOCKET_HTTP_VERSION)
        {
            AIMY_ERROR("false http header %s %s %s",option.c_str(),websocketApiName.c_str(),http_version.c_str());
            break;
        }
        auto iter=http_frame.header.keys.find("Sec-WebSocket-Key");
        if(iter==http_frame.header.keys.end())
        {
            AIMY_ERROR("miss Sec-WebSocket-Key");
            break;
        }
        passiveResponse(iter->second);
        return;
    }while(0);
    on_error();
}

void WebsocketConnectionHelper::handleActiveConnection(std::shared_ptr<uint8_t> frame,uint32_t len)
{
    aimy::ProtocalHttp protocal;
    aimy::StreamHttpFrame http_frame;
    protocal.bufferToFrame(frame.get(),len,&http_frame);
    do{
        if(http_frame.header.content_length<0)
        {
            break;
        }
        if(http_frame.header.header.size()!=3)
        {
            AIMY_ERROR("miss header filed!");
            break;
        }

        std::string http_version=http_frame.header.header[0];
        std::string code =http_frame.header.header[1];
        std::string code_info=http_frame.header.header[2];
        if(code!="101"||http_version!=WEBSOCKET_HTTP_VERSION)
        {
            AIMY_ERROR("setup websocket failed [%s %s %s]",code.c_str(),code_info.c_str(),http_version.c_str());
            break;
        }
        auto iter=http_frame.header.keys.find("Sec-WebSocket-Accept");
        if(iter==http_frame.header.keys.end())
        {
            AIMY_ERROR("miss Sec-WebSocket-Accept");
            break;
        }
        std::string accept_key=generateServerAcceptKey(generateSessionKey(this));
        if(accept_key!=iter->second)
        {
            AIMY_ERROR("key is not matched [local:%s - > server:%s]",accept_key.c_str(),iter->second.c_str());
            break;
        }
        handleConnectSuccess(false);
        return;
    }while(0);
    on_error();
}

void WebsocketConnectionHelper::handleConnectSuccess(bool is_passive)
{
    connectionStatus=WebsocketConnected;
    auto fd=channel->getFd();
    channel->stop();
    channel->rleaseFd();
    channel.reset();
    if(timeoutTimer)timeoutTimer->stop();
    disconnected();
    notifyNewWSConnection(websocketApiName,fd,is_passive);
}

std::string WebsocketConnectionHelper::generateSessionKey(void *ptr)
{
    auto session_key=std::to_string(reinterpret_cast<uint64_t>(ptr));
    uint8_t data[16]={0};
    MY_our_MD5DataRaw(reinterpret_cast<const uint8_t *>(session_key.c_str()),session_key.length(),data);
    auto base_64_result=base64Encode(data,16);
    return std::string(reinterpret_cast<const char *>(base_64_result.first.get()),base_64_result.second);
}


void WebsocketConnectionHelper::ws_sha1_init(ws_sha1_ctx *context) {
  context->state[0] = 0x67452301;
  context->state[1] = 0xEFCDAB89;
  context->state[2] = 0x98BADCFE;
  context->state[3] = 0x10325476;
  context->state[4] = 0xC3D2E1F0;
  context->count[0] = context->count[1] = 0;
}
union char64long16 {
  unsigned char c[64];
  uint32_t l[16];
};
#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))
static uint32_t blk0(union char64long16 *block, int i) {
/* Forrest: SHA expect BIG_ENDIAN, swap if LITTLE_ENDIAN */
#if BYTE_ORDER == LITTLE_ENDIAN
  block->l[i] =
      (rol(block->l[i], 24) & 0xFF00FF00) | (rol(block->l[i], 8) & 0x00FF00FF);
#endif
  return block->l[i];
}

/* Avoid redefine warning (ARM /usr/include/sys/ucontext.h define R0~R4) */
#undef blk
#undef R0
#undef R1
#undef R2
#undef R3
#undef R4

#define blk(i)                                                               \
  (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^ \
                              block->l[(i + 2) & 15] ^ block->l[i & 15],     \
                          1))

#define R0(v, w, x, y, z, i)                                          \
  z += ((w & (x ^ y)) ^ y) + blk0(block, i) + 0x5A827999 + rol(v, 5); \
  w = rol(w, 30);
#define R1(v, w, x, y, z, i)                                  \
  z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); \
  w = rol(w, 30);
#define R2(v, w, x, y, z, i)                          \
  z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); \
  w = rol(w, 30);
#define R3(v, w, x, y, z, i)                                        \
  z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); \
  w = rol(w, 30);
#define R4(v, w, x, y, z, i)                          \
  z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); \
  w = rol(w, 30);

void WebsocketConnectionHelper::ws_sha1_transform(uint32_t state[5], const unsigned char buffer[64]) {
  uint32_t a, b, c, d, e;
  union char64long16 block[1];

  memcpy(block, buffer, 64);
  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];
  R0(a, b, c, d, e, 0);
  R0(e, a, b, c, d, 1);
  R0(d, e, a, b, c, 2);
  R0(c, d, e, a, b, 3);
  R0(b, c, d, e, a, 4);
  R0(a, b, c, d, e, 5);
  R0(e, a, b, c, d, 6);
  R0(d, e, a, b, c, 7);
  R0(c, d, e, a, b, 8);
  R0(b, c, d, e, a, 9);
  R0(a, b, c, d, e, 10);
  R0(e, a, b, c, d, 11);
  R0(d, e, a, b, c, 12);
  R0(c, d, e, a, b, 13);
  R0(b, c, d, e, a, 14);
  R0(a, b, c, d, e, 15);
  R1(e, a, b, c, d, 16);
  R1(d, e, a, b, c, 17);
  R1(c, d, e, a, b, 18);
  R1(b, c, d, e, a, 19);
  R2(a, b, c, d, e, 20);
  R2(e, a, b, c, d, 21);
  R2(d, e, a, b, c, 22);
  R2(c, d, e, a, b, 23);
  R2(b, c, d, e, a, 24);
  R2(a, b, c, d, e, 25);
  R2(e, a, b, c, d, 26);
  R2(d, e, a, b, c, 27);
  R2(c, d, e, a, b, 28);
  R2(b, c, d, e, a, 29);
  R2(a, b, c, d, e, 30);
  R2(e, a, b, c, d, 31);
  R2(d, e, a, b, c, 32);
  R2(c, d, e, a, b, 33);
  R2(b, c, d, e, a, 34);
  R2(a, b, c, d, e, 35);
  R2(e, a, b, c, d, 36);
  R2(d, e, a, b, c, 37);
  R2(c, d, e, a, b, 38);
  R2(b, c, d, e, a, 39);
  R3(a, b, c, d, e, 40);
  R3(e, a, b, c, d, 41);
  R3(d, e, a, b, c, 42);
  R3(c, d, e, a, b, 43);
  R3(b, c, d, e, a, 44);
  R3(a, b, c, d, e, 45);
  R3(e, a, b, c, d, 46);
  R3(d, e, a, b, c, 47);
  R3(c, d, e, a, b, 48);
  R3(b, c, d, e, a, 49);
  R3(a, b, c, d, e, 50);
  R3(e, a, b, c, d, 51);
  R3(d, e, a, b, c, 52);
  R3(c, d, e, a, b, 53);
  R3(b, c, d, e, a, 54);
  R3(a, b, c, d, e, 55);
  R3(e, a, b, c, d, 56);
  R3(d, e, a, b, c, 57);
  R3(c, d, e, a, b, 58);
  R3(b, c, d, e, a, 59);
  R4(a, b, c, d, e, 60);
  R4(e, a, b, c, d, 61);
  R4(d, e, a, b, c, 62);
  R4(c, d, e, a, b, 63);
  R4(b, c, d, e, a, 64);
  R4(a, b, c, d, e, 65);
  R4(e, a, b, c, d, 66);
  R4(d, e, a, b, c, 67);
  R4(c, d, e, a, b, 68);
  R4(b, c, d, e, a, 69);
  R4(a, b, c, d, e, 70);
  R4(e, a, b, c, d, 71);
  R4(d, e, a, b, c, 72);
  R4(c, d, e, a, b, 73);
  R4(b, c, d, e, a, 74);
  R4(a, b, c, d, e, 75);
  R4(e, a, b, c, d, 76);
  R4(d, e, a, b, c, 77);
  R4(c, d, e, a, b, 78);
  R4(b, c, d, e, a, 79);
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
  /* Erase working structures. The order of operations is important,
   * used to ensure that compiler doesn't optimize those out. */
  memset(block, 0, sizeof(block));
  a = b = c = d = e = 0;
  (void) a;
  (void) b;
  (void) c;
  (void) d;
  (void) e;
}
void WebsocketConnectionHelper::ws_sha1_update(ws_sha1_ctx *context, const unsigned char *data,
                    size_t len) {
  size_t i, j;

  j = context->count[0];
  if ((context->count[0] += (uint32_t) len << 3) < j) context->count[1]++;
  context->count[1] += (uint32_t)(len >> 29);
  j = (j >> 3) & 63;
  if ((j + len) > 63) {
    memcpy(&context->buffer[j], data, (i = 64 - j));
    ws_sha1_transform(context->state, context->buffer);
    for (; i + 63 < len; i += 64) {
      ws_sha1_transform(context->state, &data[i]);
    }
    j = 0;
  } else
    i = 0;
  memcpy(&context->buffer[j], &data[i], len - i);
}

void WebsocketConnectionHelper::ws_sha1_final(unsigned char digest[20], ws_sha1_ctx *context) {
  unsigned i;
  unsigned char finalcount[8], c;

  for (i = 0; i < 8; i++) {
    finalcount[i] = (unsigned char) ((context->count[(i >= 4 ? 0 : 1)] >>
                                      ((3 - (i & 3)) * 8)) &
                                     255);
  }
  c = 0200;
  ws_sha1_update(context, &c, 1);
  while ((context->count[0] & 504) != 448) {
    c = 0000;
    ws_sha1_update(context, &c, 1);
  }
  ws_sha1_update(context, finalcount, 8);
  for (i = 0; i < 20; i++) {
    digest[i] =
        (unsigned char) ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
  }
  memset(context, '\0', sizeof(*context));
  memset(&finalcount, '\0', sizeof(finalcount));
}

void WebsocketConnectionHelper::ws_hmac_sha1(const unsigned char *key, size_t keylen,
                  const unsigned char *data, size_t datalen,
                  unsigned char out[20]) {
  ws_sha1_ctx ctx;
  unsigned char buf1[64], buf2[64], tmp_key[20], i;

  if (keylen > sizeof(buf1)) {
    ws_sha1_init(&ctx);
    ws_sha1_update(&ctx, key, keylen);
    ws_sha1_final(tmp_key, &ctx);
    key = tmp_key;
    keylen = sizeof(tmp_key);
  }

  memset(buf1, 0, sizeof(buf1));
  memset(buf2, 0, sizeof(buf2));
  memcpy(buf1, key, keylen);
  memcpy(buf2, key, keylen);

  for (i = 0; i < sizeof(buf1); i++) {
    buf1[i] ^= 0x36;
    buf2[i] ^= 0x5c;
  }

  ws_sha1_init(&ctx);
  ws_sha1_update(&ctx, buf1, sizeof(buf1));
  ws_sha1_update(&ctx, data, datalen);
  ws_sha1_final(out, &ctx);

  ws_sha1_init(&ctx);
  ws_sha1_update(&ctx, buf2, sizeof(buf2));
  ws_sha1_update(&ctx, out, 20);
  ws_sha1_final(out, &ctx);
}

std::string WebsocketConnectionHelper::generateServerAcceptKey(const std::string &sesion_key)
{
    unsigned char sha[20];
    ws_sha1_ctx sha_ctx;
    ws_sha1_init(&sha_ctx);
    ws_sha1_update(&sha_ctx, (unsigned char *) sesion_key.c_str(), sesion_key.size());
    ws_sha1_update(&sha_ctx, (unsigned char *) sha1_get_guid(), 36);
    ws_sha1_final(sha, &sha_ctx);
    auto result =base64Encode(sha,sizeof (sha));
    return std::string(reinterpret_cast<const char *>(result.first.get()),result.second);
}

void WebsocketConnectionHelper::on_recv()
{
    while(1)
    {
        auto ret=readCache->readFromFd();
        if(ret<0)break;
        else if (ret==0) {
            AIMY_DEBUG("recv remote maybe disconnected,close it!");
            on_close();
            break;
        }
        //
        auto frame=readCache->popFrame();
        if(frame.second>0)
        {
            channel->disableReading();
            channel->sync();
            switch (connectionStatus) {
            case WebsocketPassive:
                handleWebsocketPassiveConnection(frame.first,frame.second);
                break;
            case WebsocketActiveConnecting:
                handleActiveConnection(frame.first,frame.second);
                break;
            case WebsocketPassiveConnecting:
            case WebsocketClosed:
            case WebsocketConnected:
                AIMY_ERROR("error status,disconnected!");
                on_error();
                disconnected();
                break;
            }
            break;
        }
    }
}

void WebsocketConnectionHelper::on_write()
{
    while(writeCache->frame_count()>0)
    {
        auto ret=writeCache->sendCacheByFd();
        if(ret<0)break;
        if(ret==0)
        {
            AIMY_DEBUG("write remote maybe disconnected,close it!");
            on_close();
            return;
        }
    }
    if(writeCache->frame_count()==0)
    {
        channel->disableWriting();
        channel->sync();
        if(connectionStatus==WebsocketPassiveConnecting)
        {//send over;change status
            handleConnectSuccess(true);
        }
    }
}

void WebsocketConnectionHelper::on_close()
{
    connectionStatus=WebsocketClosed;
    AIMY_ERROR("websocket connection closed,disconnect it! [%s]",strerror(NETWORK_UTIL::get_socket_error(fd)));
    channel->stop();
    disconnected();
}

void WebsocketConnectionHelper::on_error()
{
    connectionStatus=WebsocketClosed;
    AIMY_ERROR("websocket connection error,disconnect it! [%s]",strerror(NETWORK_UTIL::get_socket_error(fd)));
    channel->stop();
    disconnected();
}
