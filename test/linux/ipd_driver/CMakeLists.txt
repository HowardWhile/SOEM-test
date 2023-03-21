
set(SOURCES ipd_driver.c)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 
add_executable(ipd_driver ${SOURCES})
target_compile_options(ipd_driver PRIVATE)
target_link_libraries(ipd_driver soem)
install(TARGETS ipd_driver DESTINATION bin)