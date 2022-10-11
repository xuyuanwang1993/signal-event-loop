cmake_minimum_required (VERSION 2.6)
project (daemon-service)  # project here actually means solution in premake
add_executable (daemon-service commandlineTool.cpp daemon-instance.cpp main.cpp network-log.cpp unix-socket-helper.cpp)
target_link_libraries (daemon-service pthread)


INSTALL(TARGETS daemon-service
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
