
set(SOURCES delta_rs6.cpp)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 
add_executable(delta_rs6 ${SOURCES})
target_compile_options(delta_rs6 PRIVATE)
target_link_libraries(delta_rs6 soem)
install(TARGETS delta_rs6 DESTINATION bin)