add_library(PCA9685 PCA9685.cpp)

target_link_libraries(PCA9685 pigpio)
target_include_directories(PCA9685 PUBLIC ../pigpio)
