add_executable(drive-test node.cc gnublin_i2c.cpp)
target_link_libraries(drive-test crisp-comms crisp-input)
