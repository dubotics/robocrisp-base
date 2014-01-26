add_executable(node node.cc gnublin_i2c.cpp)
target_link_libraries(node crisp-comms crisp-input)
