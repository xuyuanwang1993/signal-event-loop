set(target logo-progress)
set(example_resource main.cpp)
add_executable(${target} ${example_resource})
target_link_libraries(${target} PRIVATE
    ${LIBS} SDL2 SDL2_image SDL2_ttf drm
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${target}"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
        )
list(APPEND INCLUDEPATHS "${CMAKE_FIND_ROOT_PATH}/usr/include/drm" )
include_directories(${INCLUDEPATHS})
