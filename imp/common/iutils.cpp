#include "iutils.h"
#include<stdlib.h>
#include<string.h>
using namespace aimy;
#define G16(X)  0x11021                 //CRC16校验多项式x16+x12+x5+1
uint16_t Iutils::crc_rc16(const void * data,uint32_t data_len)
{
    const uint8_t *p=static_cast<const uint8_t *>(data);

    unsigned int CRC_DATA;
    unsigned char Lenght_Buf;
    unsigned char RC_Lenght;
    unsigned char k;

    CRC_DATA = 0;
    for (Lenght_Buf = 0; Lenght_Buf < data_len; Lenght_Buf++)
    {
        for (RC_Lenght = 8; RC_Lenght > 0;)
        {
            RC_Lenght--;
            CRC_DATA = (CRC_DATA << 1) | (((*p) >> RC_Lenght) & 0x01);
            if (CRC_DATA & 0x10000)
            {
                CRC_DATA ^= G16(X);
            }
        }
        p++;
    }

    for (k = 0; k < 16; k++)
    {
        CRC_DATA = CRC_DATA << 1;
        if (CRC_DATA & 0x10000)
        {
            CRC_DATA ^= G16(X);
        }
    }

    return (CRC_DATA);
}

#define rightrotate(w, n) ((w >> n) | (w) << (32-(n)))

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#	if defined(__GNUC__)

static inline void CopyUint32(void *p_out, uint32_t val)
{
    *(uint32_t*)p_out = __builtin_bswap32(val);
}

#	else

inline void CopyUint32(void *p_out, uint32_t val)
{
    *(uint32_t*)p_out = val >> 24 | ((val >> 8) & 0x0000FF00) | ((val << 8) & 0x00FF0000) | (val << 24);
}

#	endif
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#	define copy_uint32(p, val) *((uint32_t *)p) = (val)
#else
#	error "Unsupported target architecture endianess!"
#endif

static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void Iutils::sha256(const void *data,uint32_t data_len,uint8_t out[32])
{
    uint32_t h0 = 0x6a09e667;
        uint32_t h1 = 0xbb67ae85;
        uint32_t h2 = 0x3c6ef372;
        uint32_t h3 = 0xa54ff53a;
        uint32_t h4 = 0x510e527f;
        uint32_t h5 = 0x9b05688c;
        uint32_t h6 = 0x1f83d9ab;
        uint32_t h7 = 0x5be0cd19;
        int r = (int)(data_len * 8 % 512);
        int append = ((r < 448) ? (448 - r) : (448 + 512 - r)) / 8;
        uint32_t new_len = data_len + append + 8;
        uint8_t *buf = (uint8_t*)malloc(new_len);
        memset(buf + data_len, 0, append);
        if (data_len > 0) {
            memcpy(buf, data, data_len);
        }
        buf[data_len] = (unsigned char)0x80;
        uint64_t bits_len = data_len * 8;
        for (int i = 0; i < 8; i++) {
            buf[data_len + append + i] = (bits_len >> ((7 - i) * 8)) & 0xff;
        }
        uint32_t w[64];
        memset(w, 0, sizeof(w));
        size_t chunk_len = new_len / 64;
        for (size_t idx = 0; idx < chunk_len; idx++) {
            uint32_t val = 0;
            for (int i = 0; i < 64; i++) {
                val = val | (*(buf + idx * 64 + i) << (8 * (3 - i)));
                if (i % 4 == 3) {
                    w[i / 4] = val;
                    val = 0;
                }
            }
            for (int i = 16; i < 64; i++) {
                uint32_t s0 = rightrotate(w[i - 15], 7) ^ rightrotate(w[i - 15], 18) ^ (w[i - 15] >> 3);
                uint32_t s1 = rightrotate(w[i - 2], 17) ^ rightrotate(w[i - 2], 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            uint32_t a = h0;
            uint32_t b = h1;
            uint32_t c = h2;
            uint32_t d = h3;
            uint32_t e = h4;
            uint32_t f = h5;
            uint32_t g = h6;
            uint32_t h = h7;
            for (int i = 0; i < 64; i++) {
                uint32_t s_1 = rightrotate(e, 6) ^ rightrotate(e, 11) ^ rightrotate(e, 25);
                uint32_t ch = (e & f) ^ (~e & g);
                uint32_t temp1 = h + s_1 + ch + k[i] + w[i];
                uint32_t s_0 = rightrotate(a, 2) ^ rightrotate(a, 13) ^ rightrotate(a, 22);
                uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                uint32_t temp2 = s_0 + maj;
                h = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }
            h0 += a;
            h1 += b;
            h2 += c;
            h3 += d;
            h4 += e;
            h5 += f;
            h6 += g;
            h7 += h;
        }
        CopyUint32(out, h0);
        CopyUint32(out + 4, h1);
        CopyUint32(out + 8, h2);
        CopyUint32(out + 12, h3);
        CopyUint32(out + 16, h4);
        CopyUint32(out + 20, h5);
        CopyUint32(out + 24, h6);
        CopyUint32(out + 28, h7);
        free(buf);
}

std::string Iutils::getSuffix(const std::string &input,const std::string &separator)
{
    auto pos=input.find_last_of(separator);
    if(pos==std::string::npos||pos==input.length()-1)return std::string();
    return input.substr(pos+1,input.size()-pos-1);
}

std::shared_ptr<uint8_t> Iutils::allocBuffer(uint32_t len)
{
    //分配时+1的目的时 避免对方将此buffer当成char类型处理，后面如果没有终止符会引起越界访问
    if(len==0)return nullptr;
    std::shared_ptr<uint8_t> ret(new uint8_t[len+1],std::default_delete<uint8_t[]>());
    if(ret.get())memset(ret.get(),0,len+1);
    return ret;
}
