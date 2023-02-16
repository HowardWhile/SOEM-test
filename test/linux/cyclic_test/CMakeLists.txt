
set(SOURCES cyclic_test.c)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 
add_executable(cyclic_test ${SOURCES})
target_link_libraries(cyclic_test soem curses)
install(TARGETS cyclic_test DESTINATION bin)
