HEADERS += \
    $$PWD/core/core-include.h \
    $$PWD/core/epoll-task-scheduler.h \
    $$PWD/core/event-loop.h \
    $$PWD/core/io-channel.h \
    $$PWD/core/network-util.h \
    $$PWD/core/object.h \
    $$PWD/core/platform.h \
    $$PWD/core/select-task-scheduler.h \
    $$PWD/core/task-pipe.h \
    $$PWD/core/task-scheduler.h \
    $$PWD/core/thread-pool.h \
    $$PWD/imp/bluez-looptest/at-device-task.h \
    $$PWD/imp/can-utils/can-type-def.h \
    $$PWD/imp/can-utils/can-util-socket.h \
    $$PWD/imp/can-utils/can-util.h \
    $$PWD/imp/changelog-format.h \
    $$PWD/imp/commandlineTool.h \
    $$PWD/imp/common/serial_utils.h \
    $$PWD/imp/debug-helper.h \
    $$PWD/imp/device-discover/device-discover.h \
    $$PWD/imp/dns-solver.h \
    $$PWD/imp/udp-echo-server.h \
    $$PWD/log/aimy-log.h \
    $$PWD/third_party/json/cjson-interface.hpp \
    $$PWD/third_party/json/cjson.h

SOURCES += \
    $$PWD/core/epoll-task-scheduler.cpp \
    $$PWD/core/event-loop.cpp \
    $$PWD/core/io-channel.cpp \
    $$PWD/core/network-util.cpp \
    $$PWD/core/object.cpp \
    $$PWD/core/select-task-scheduler.cpp \
    $$PWD/core/task-pipe.cpp \
    $$PWD/core/task-scheduler.cpp \
    $$PWD/core/thread-pool.cpp \
    $$PWD/imp/bluez-looptest/at-device-task.cpp \
    $$PWD/imp/can-utils/can-util-socket.cpp \
    $$PWD/imp/can-utils/can-util.cpp \
    $$PWD/imp/changelog-format.cpp \
    $$PWD/imp/commandlineTool.cpp \
    $$PWD/imp/debug-helper.cpp \
    $$PWD/imp/device-discover/device-discover.cpp \
    $$PWD/imp/dns-solver.cpp \
    $$PWD/imp/udp-echo-server.cpp \
    $$PWD/log/aimy-log.cpp \
    $$PWD/third_party/json/cjson-interface.cpp \
    $$PWD/third_party/json/cjson.cpp

INCLUDEPATH += $$PWD
