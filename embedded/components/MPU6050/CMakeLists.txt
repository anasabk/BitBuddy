add_library(MPU6050 MPU6050.cpp)

target_link_libraries(MPU6050 I2Cdev)
target_include_directories(MPU6050 PUBLIC ../I2Cdev)
