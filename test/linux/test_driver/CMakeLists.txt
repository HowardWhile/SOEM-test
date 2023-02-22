
set(SOURCES test_driver.c)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 
add_executable(test_driver ${SOURCES})
target_compile_options(test_driver PRIVATE -Wno-error)
target_link_libraries(test_driver soem)
install(TARGETS test_driver DESTINATION bin)