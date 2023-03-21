
set(SOURCES zero_err_driver.c)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 
add_executable(zero_err_driver ${SOURCES})
target_compile_options(zero_err_driver PRIVATE)
target_link_libraries(zero_err_driver soem)
install(TARGETS zero_err_driver DESTINATION bin)