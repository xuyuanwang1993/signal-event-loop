set(target json_format)
set(example_resource main.cpp)
add_executable(${target} ${example_resource})
target_link_libraries(${target} PRIVATE
    ${LIBS}
)
