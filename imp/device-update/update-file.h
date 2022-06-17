#ifndef UPDATEFILE_H
#define UPDATEFILE_H
#include "log/aimy-log.h"
#include "update-type-def.h"
#include <stdint.h>
namespace aimy {

enum AimyUpdateFileType:uint8_t{
    AIMY_HEX_FILE,//.hex
    AIMY_BIN_FILE,//.mva
    AIMY_ENCRYPT_BIN_FILE,//.amva
    AIMY_UNKNOWN_FILE,//
};

class UpdateFileContext
{
public:
    UpdateFileContext();
    ~UpdateFileContext();

    bool init(const std::string&fileName,AimyUpdateFileType guessType=AIMY_UNKNOWN_FILE);
    void deInit();

    /**
     * @brief loadProgramData  init data cache according to the offset
     * @param vec_start_offset
     * @param vec_end_offset
     * @param code_start_offset
     * @param code_end_offset
     * @return false for failed
     */
    bool loadProgramData(uint32_t vec_start_offset,uint32_t vec_end_offset,uint32_t code_start_offset,uint32_t code_end_offset);

    /**
     * @brief fillUpdateSliceContext read data from cache
     * @param context
     * @return false for failed
     *
     */
    bool fillUpdateSliceContext(UpdateSliceContext &context);
    /**
     * @brief checkFile
     * @param version the version of the update program
     * @param product the product_id of the program
     * @param date the date of the program
     * @param version_offset only used by HEX file
     * @param product_offset only used by HEX file
     * @param date_offset only used by HEX file
     * @return  -1 failed ,=0 match not need updating ,>0 mismatch,need updating
     */
    int checkFile(const std::string&version,const std::string &product,const std::string&date,uint32_t version_offset,uint32_t product_offset,uint32_t date_offset);
    /**
     * @brief guessFileType get the file type,ignore case
     * @param file_name
     * @param guessType if the file type can't be specfied by the file suffix,it will return this type
     * @return .hex->AIMY_HEX_FILE .mva->AIMY_BIN_FILE  .amva->AIMY_ENCRYPT_BIN_FILE
     */
    static AimyUpdateFileType guessFileType(const std::string &file_name,AimyUpdateFileType guessType);
private:
    int checkBinFile(const std::string&version,const std::string &product,const std::string&date);
    int checkEncryptFile(const std::string&version,const std::string &product,const std::string&date);
    int checkHexFile(const std::string&version,const std::string &product,const std::string&date,uint32_t version_offset,uint32_t product_offset,uint32_t date_offset);
private:
    AimyUpdateFileType type;
    std::string updateFilename;
    FILE *fp;
    size_t fileSize;
    HexFileContext hexContext;
    //cache
    std::shared_ptr<uint8_t>vecData;
    uint32_t vecDataLen;
    std::shared_ptr<uint8_t>codeData;
    uint32_t codeDataLen;
};
}
#endif // UPDATEFILE_H
