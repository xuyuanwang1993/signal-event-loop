#ifndef UPDATETYPEDEF_H
#define UPDATETYPEDEF_H
#include <stdint.h>
#include<memory>
#include<string>
#include<map>
#include<string.h>
#include<vector>
namespace aimy {

enum UpdateTypeDef:uint8_t{
    QUERY_DEVICE = 0xA1,			//查询设备信息
    QUERY_UPID = 0xA2,				//查询芯片ID
    TRANSFER_ERROR = 0xA6,			//传输出错
    QUERY_WORK_MODE = 0xB3,				//获取当前工作模式, 0: 正常模式; 1: 升级模式;

    /***************设备升级********************/
    UPGRADE_ENTER = 0xE0,			//设备进入升级状态
    UPGRADE_QUERY_SOFTWARE = 0xE1,	//查询设备软件版本
    UPGRADE_QUERY_SEGMENT = 0xE2,	//查询段地址
    UPGRADE_INT_VECTOR_TABLE = 0xE3,//上传中断向量表
    UPGRADE_CODE_SEGMENT = 0xE6,	//上传代码段
    UPGRADE_RESUME = 0xE7,			//断点续传 弃用
    UPGRADE_QUIT = 0xE8,			//退出升级
    /******************************************/
};

enum  UpgradeQuitStatus : uint8_t {
    UPGRADE_SUCCESS = 0,				//升级正常完成
    UPGRADE_ERROR_WITH_ROLLBACK = 0x61,	//升级错误退出
    UPGRADE_ERROR_WITHOUT_ROLLBACK = 0x62//没有做任何升级退出(对比版本不用升级退出)
};

enum UpdateSliceType:uint8_t{
    UPDATE_VECTOR_SLICE=UPGRADE_INT_VECTOR_TABLE,
    UPDATE_CODE_SLICE=UPGRADE_CODE_SEGMENT,
};

struct UpdateSliceContext{
    uint32_t max_slice_size;//4字节对齐
    std::shared_ptr<uint8_t>data;
    uint32_t data_len;
    UpdateSliceType type;
    std::vector<std::pair<uint32_t,uint32_t>>vector_map;
    std::vector<std::pair<uint32_t,uint32_t>>code_map;
    int seq;//包的序列号 传输过程的序列号 变化一次时 vec_seq-1 .... 0    code_seq-1 ...0
    uint32_t page_size;//单片机分页存储 不能跨页写入，这里进行相应处理
     UpdateSliceContext(uint32_t _max_slice_size):max_slice_size(_max_slice_size),data_len(0),type(UPDATE_VECTOR_SLICE),seq(-2),page_size(1024)
     {
         data.reset(new uint8_t[max_slice_size+1],std::default_delete<uint8_t[]>());
     };
};
//https://www.cnblogs.com/zhaoyanan/p/7838598.html
enum AimyHexRecordType{
    RECORD_TYPE_DATA,//>数据记录
    RECORD_TYPE_EOF,//用来标识文件记录的结束
    RECORD_TYPE_EX_SEG_ADDR,//扩展段地址
    RECORD_TYPE_START_SEG_ADDR,//段地址
    RECORD_TYPE_EX_LINEAR_ADDR,//用来标识扩展线性地址
    RECORD_TYPE_START_LINEAR_ADDR,//线性地址
};

#define AIMY_HEX_HIGH_MASK 0xFFFF0000
#define AIMY_HEX_LOW_MASK 0xFFFF
struct AimyHexCache{
    const uint32_t base_addr;
    uint8_t data_cache[AIMY_HEX_LOW_MASK+1];
    uint8_t data_flag[AIMY_HEX_LOW_MASK+1];
    AimyHexCache(uint32_t _base_addr):base_addr(_base_addr){
        memset(data_cache,0,AIMY_HEX_LOW_MASK+1);
        memset(data_flag,0,AIMY_HEX_LOW_MASK+1);
    }
};

struct HexFileContext{
    std::map<uint32_t,std::shared_ptr<AimyHexCache>>cache_map;
    /**
     * @brief readData
     * @param data_addr
     * @param buf
     * @param max_read_len
     * @return <0 failed ,>=0 copy bytes
     */
    int readData(uint32_t data_addr,void *buf,uint32_t max_read_len);
    /**
     * @brief loadHexFile parse a hex file
     * @param fd
     * @return false -> failed
     */
    bool loadHexFile(FILE *fp);
    void reset();
};
}
#endif // UPDATETYPEDEF_H
