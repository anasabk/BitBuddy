cmake_minimum_required (VERSION 3.13)
project(BitBuddy)

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(components/LCD)
add_subdirectory(components/MPU6050)
add_subdirectory(components/PCA9685)
# add_subdirectory(components/pigpio)

target_include_directories(${PROJECT_NAME} PUBLIC 
    components/MPU6050 
    components/PCA9685 
    components/LCD
    # components/pigpio
)

target_link_directories(${PROJECT_NAME} PRIVATE 
    components/MPU6050 
    components/PCA9685 
    components/LCD
    # components/pigpio
)

target_link_libraries(${PROJECT_NAME} 
    MPU6050
    PCA9685 
    LCD
    # pigpio
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -li2c -pthread")

