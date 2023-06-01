add_library( LCD LCD.cpp )

target_link_libraries(LCD I2Cdev)
target_include_directories(LCD PUBLIC ../I2Cdev)