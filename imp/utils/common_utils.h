#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H
#include<string>
#include<list>
#ifndef __ANDROID__
#define DEFAUL_LOCK_FILE_PATH "/tmp"
#else
#define DEFAUL_LOCK_FILE_PATH "/mnt"
#endif
#define TEST_LOCAL_SOCKET_PATH DEFAUL_LOCK_FILE_PATH"/aam.virtual_device.caller.client_test"
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
/**
 * @brief checkIsBuildDay 检查是否是当天构建
 * @return
 */
bool checkIsBuildDay();
/**
 * @brief parserCommandConfigFile 按换行空格对文件进行解析 清除首尾回车换行制表符空格
 * @param file_name
 * @return
 */
std::list<std::list<std::string>> parserCommandConfigFile(const std::string &file_name);
}
#endif // COMMON_UTILS_H
