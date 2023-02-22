
set(SOURCES delta_io.c)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 
add_executable(delta_io ${SOURCES})
target_compile_options(delta_io PRIVATE -Wno-error)
target_link_libraries(delta_io soem curses)
install(TARGETS delta_io DESTINATION bin)