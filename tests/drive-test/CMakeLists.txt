add_executable(drive-test node.cc gnublin_i2c.cpp)

if(TARGET crisp-input)
  target_link_libraries(drive-test crisp-comms crisp-input)
else(TARGET crisp-input)
  set_property(TARGET drive-test APPEND PROPERTY
    COMPILE_FLAGS -DDISABLE_CLIENT)
  target_link_libraries(drive-test crisp-comms)
endif(TARGET crisp-input)
