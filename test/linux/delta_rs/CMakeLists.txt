
set(SOURCES delta_rs.cpp)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 
add_executable(delta_rs ${SOURCES})
target_compile_options(delta_rs PRIVATE)
target_link_libraries(delta_rs soem)
install(TARGETS delta_rs DESTINATION bin)