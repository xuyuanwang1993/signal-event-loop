set(target bus_error_test)
set(example_resource main.cpp)
add_executable(${target} ${example_resource})
target_link_libraries(${target} PRIVATE
    ${LIBS}
)
