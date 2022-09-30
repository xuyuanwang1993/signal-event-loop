set(target websocket-test)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../extern_project/sol-master)
file(GLOB_RECURSE LOCAL_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/../../extern_project/sol-master/*.hpp)
set(example_resource main.cpp ${LOCAL_HEADER})
add_executable(${target} ${example_resource})


target_link_libraries(${target} PRIVATE
    ${LIBS} -llua -ldl
)
