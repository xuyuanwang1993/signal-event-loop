file(GLOB_RECURSE Gdbus_SRC "${ble_wificonfig_SOURCE_DIR}/src/gdbus/*.c")
file(GLOB_RECURSE Src_SRC "${ble_wificonfig_SOURCE_DIR}/src/src/*.c")

add_executable(ble_wificonfig 
        "${ble_wificonfig_SOURCE_DIR}/src/advertising.c"
        "${ble_wificonfig_SOURCE_DIR}/src/aimy_ble.c"
	 ${Gdbus_SRC}
         ${Src_SRC})

include(FindPkgConfig)
find_package(PkgConfig)
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


target_link_libraries (ble_wificonfig ${GLIB2_LIBRARIES})
target_link_libraries (ble_wificonfig ${DBUS_LIBRARIES})

install(TARGETS ble_wificonfig DESTINATION bin)

target_include_directories(ble_wificonfig PUBLIC 
	"${ble_wificonfig_SOURCE_DIR}/include"
	"${CMAKE_THREAD_LIBS_INIT}"
	${RAPIDJSON_INCLUDE_DIR})


target_link_libraries(ble_wificonfig pthread) 
