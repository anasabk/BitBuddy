add_library(PCA9685 PCA9685.cpp)

target_link_libraries(PCA9685 I2Cdev)
target_include_directories(PCA9685 PUBLIC ../I2Cdev)