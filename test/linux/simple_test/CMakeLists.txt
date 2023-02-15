
set(SOURCES simple_test.c)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 

add_executable(simple_test ${SOURCES})
target_link_libraries(simple_test soem)
install(TARGETS simple_test DESTINATION bin)
