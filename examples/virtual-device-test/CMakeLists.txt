set(target virtual-device-test)
set(example_resource main.cpp)
add_definitions(-D_XOPEN_SOURCE)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SOURCE_MACHINE_TEST)
add_executable(${target} ${example_resource} ${SOURCE_MACHINE_TEST} )
target_link_libraries(${target} PRIVATE
    ${LIBS}
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${target}"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
        )
