set(target object-test)
set(example_resource main.cpp)
add_executable(${target} ${example_resource})
target_link_libraries(${target} PRIVATE
    ${LIBS}
)
INSTALL(TARGETS ${taget} 
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})