#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H
#include<string>
#ifndef __ANDROID__
#define DEFAUL_LOCK_FILE_PATH "/tmp"
#else
#define DEFAUL_LOCK_FILE_PATH "/mnt"
#endif
namespace AIMY_UTILS {
/**
 * @brief acquireSigleInstanceLcok make your process run once
 * @param lock_file_path path to generate locke_file
 * @return  true:success false:failed
 */
bool acquireSigleInstanceLcok(const std::string &lock_file_path=DEFAUL_LOCK_FILE_PATH);
/**
 * @brief readProcName get the proc name for the pid
 * @param pid
 * @return if process is not existed,return empty string
 */
std::string readProcName(pid_t pid);

/**
 * @brief genCheckCode 0x100=异或校验码和+code
 */
uint8_t genCheckCode(const void *p_data, uint32_t length);
}
#endif // COMMON_UTILS_H
