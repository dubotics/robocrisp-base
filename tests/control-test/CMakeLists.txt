add_executable(control-test node.cc gnublin_i2c.cpp)
target_link_libraries(control-test crisp-comms crisp-input)
