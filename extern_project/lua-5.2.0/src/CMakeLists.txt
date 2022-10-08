cmake_minimum_required (VERSION 2.6)
project (lua)  # project here actually means solution in premake


# 1. lua static library
# how to rename library name?
add_library (lualib STATIC lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c lbitlib.c lcorolib.c ldblib.c liolib.c lmathlib.c loslib.c lstrlib.c ltablib.c loadlib.c linit.c)
set_target_properties(lualib PROPERTIES OUTPUT_NAME "lua") # rename the library output name

# 2. lua interpreter
link_directories (${LUA_BINARY_DIR})
add_executable (lua lua.c)
target_link_libraries (lua lualib)
target_link_libraries( lua m )

# 3. lua compiler
link_directories (${LUA_BINARY_DIR})
add_executable (luac luac.c)
target_link_libraries (luac lualib)
target_link_libraries( luac m )

set(INSTALL_ITEMS ${target}_static)
if(ENABLE_SHARED)
    list(APPEND INSTALL_ITEMS ${target})
endif()

INSTALL(TARGETS lualib luac lua
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
set(HEADERS )
file(GLOB_RECURSE LOCAL_HEADER ./*.h ./*.hpp )
list(APPEND HEADERS ${LOCAL_HEADER})
set(CMAKE_INSTALL_INCLUDEDIR "include")
foreach(item ${HEADERS} )
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/
        "" file_item_path
        ${item})
    string(REPLACE ./
        "" file_item_path
        ${file_item_path})
    INSTALL(FILES ${file_item_path} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lua RENAME ${file_item_path}
        )
endforeach(item)