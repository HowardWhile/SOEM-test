
set(SOURCES slaveinfo.c)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 

add_executable(slaveinfo ${SOURCES})
target_link_libraries(slaveinfo soem)
install(TARGETS slaveinfo DESTINATION bin)
