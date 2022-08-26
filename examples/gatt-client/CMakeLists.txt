cmake_minimum_required(VERSION 3.13)
project(gatt_client)
set(target gatt_client)

set(example_resource btgatt-client.c)
file(GLOB_RECURSE TEMP_LOCAL_SOURCE ./*.c)
file(GLOB_RECURSE TEMP_LOCAL_HEADER ./*.h)
add_executable(${target} ${example_resource} ${TEMP_LOCAL_SOURCE} ${TEMP_LOCAL_HEADER} )

include_directories(${CMAKE_CURRENT_SOURCE_DIR} )
include(FindPkgConfig)
pkg_check_modules (GLIB2 glib-2.0 REQUIRED)
pkg_check_modules (DBUS dbus-1 REQUIRED)
if( $ENV{HOST_DIR_PREFIX}d STREQUAL $ENV{HOST_DIR_PREFIX}$ENV{HOST_DIR_PREFIX}d  )
    include_directories(${GLIB2_INCLUDE_DIRS})
    message(STATUS "GLIB2_INCLUDE_DIRS=${GLIB2_INCLUDE_DIRS}  ")
else()
    string(REGEX REPLACE "/home/project/buildroot/output/rockchip_[^/]+/host" ";$ENV{HOST_DIR_PREFIX}" REGEX_TEMP ${GLIB2_INCLUDE_DIRS})
    include_directories(${REGEX_TEMP})
    message(STATUS "GLIB2_INCLUDE_DIRS=${REGEX_TEMP}")
endif()

if( $ENV{HOST_DIR_PREFIX}d STREQUAL $ENV{HOST_DIR_PREFIX}$ENV{HOST_DIR_PREFIX}d  )
    include_directories(${DBUS_INCLUDE_DIRS})
    message(STATUS "DBUS_INCLUDE_DIRS=${DBUS_INCLUDE_DIRS}  ")
else()
    string(REGEX REPLACE "/home/project/buildroot/output/rockchip_[^/]+/host" ";$ENV{HOST_DIR_PREFIX}" REGEX_TEMP ${DBUS_INCLUDE_DIRS})
    message(STATUS "DBUS_INCLUDE_DIRS=${REGEX_TEMP} ")
    include_directories(${REGEX_TEMP})
endif()

target_link_libraries(${target} PRIVATE
    ${GLIB2_LIBRARIES}
    ${DBUS_LIBRARIES}
    -ldl
    ${LIBS}
)

INSTALL(TARGETS ${taget} 
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
