add_library( LCD LCD.cpp )

target_link_libraries(LCD pigpio)
target_include_directories(LCD PUBLIC ../pigpio)
