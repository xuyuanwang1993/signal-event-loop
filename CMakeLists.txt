cmake_minimum_required(VERSION 3.13)
project(aimy-eventloop)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wno-reorder  -Wno-old-style-cast ${CMAKE_CXX_FLAGS}")
set(target ${PROJECT_NAME})

option(COMPILE_SDL_MODE "compile source relative to sdl" ON)
option(ENABLE_SHARED "generate shared library" OFF)
option(COMPILE_EXAMPLES "generate examples" ON)
option(COMPILE_LUA_EXAMPLE "lua support" ON)
#option(ENABLE_STATIC "generate static library" ON)
#配置输出及安装目录
include(${CMAKE_CURRENT_SOURCE_DIR}/compile/common.cmake)
#定义全局变量
set(SOURCES )
set(HEADERS )
set(RESOURCES )
set(INCLUDEPATHS ${CMAKE_CURRENT_SOURCE_DIR})
set(LINKPATHS )
#链接静态库直接使用lib***.a 动态库只需声明***
set(LIBS )
if(COMPILE_SDL_MODE)
find_library(SDL_LIB_PATH  libSDL2.so ${CMAKE_LIBRARY_PATH})
if( NOT ${SDL_LIB_PATH} STREQUAL "SDL_LIB_PATH-NOTFOUND"  )
    MESSAGE(STATUS "found sdl module ${SDL_LIB_PATH}")
    set(COMPILE_SDL_MODE ON)
else()
    MESSAGE(STATUS "not found sdl module")
    set(COMPILE_SDL_MODE OFF)
endif()
endif(COMPILE_SDL_MODE)
if(COMPILE_SDL_MODE)
    MESSAGE(STATUS "compile sdl module")
    add_definitions(-DUSE_SDL)
else()
    MESSAGE(STATUS "not compile sdl module")
endif()

#add_definitions(-DHID_ENCRYPT)

macro(set_parent_values)
    set(SOURCES ${SOURCES} PARENT_SCOPE)
    set(HEADERS ${HEADERS} PARENT_SCOPE)
    set(RESOURCES ${RESOURCES} PARENT_SCOPE)
    set(INCLUDEPATHS ${INCLUDEPATHS} PARENT_SCOPE)
    set(LINKPATHS ${LINKPATHS} PARENT_SCOPE)
    set(LIBS  ${LIBS} PARENT_SCOPE)
endmacro()
#添加子目录
add_subdirectory("core")
add_subdirectory("imp")
add_subdirectory("log")
add_subdirectory("third_party")

#生成库
add_library(${target}_static STATIC ${HEADERS} ${SOURCES})
set_target_properties(${target}_static PROPERTIES OUTPUT_NAME ${target})
if(ENABLE_SHARED)
    add_library(${target} SHARED ${HEADERS} ${SOURCES})
    set_target_properties(${target} PROPERTIES VERSION 1.0 SOVERSION 1)
endif()
#库生成使能

if(ENABLE_SHARED)
set_target_properties(${target} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
endif()
set_target_properties(${target}_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set(INSTALL_ITEMS ${target}_static)
if(ENABLE_SHARED)
    list(APPEND INSTALL_ITEMS ${target})
endif()

INSTALL(TARGETS ${INSTALL_ITEMS}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
foreach(item ${HEADERS} )
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/
    "" file_item_path
    ${item})
    string(REPLACE ./
    "" file_item_path
    ${file_item_path})
INSTALL(FILES ${file_item_path} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} RENAME ${file_item_path}
    )
endforeach(item)
#编译示例
list(APPEND LINKPATHS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

MESSAGE(STATUS "BUILD CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR}")
MESSAGE(STATUS "BUILD CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_BINDIR}")
MESSAGE(STATUS "BUILD CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}")
MESSAGE(STATUS "BUILD CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "BUILD CMAKE_CURRENT_BINARY_DIR ${CMAKE_INSTALL_LIBDIR}")
#引用库目录及文件搜索目录
if(${CMAKE_SYSTEM_NAME} MATCHES "Android")

else()
    add_subdirectory("sub_project/daemon-service/src")
endif()
add_subdirectory("extern_project/lua-5.2.0/src")
list(APPEND LINKPATHS ${CMAKE_INSTALL_PREFIX}/${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
link_directories(${LINKPATHS})
list(APPEND INCLUDEPATHS ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
include_directories(${INCLUDEPATHS})
if(COMPILE_EXAMPLES)
    add_subdirectory("examples")
endif()
