cmake_minimum_required (VERSION 3.13)
project(BitBuddy)

set(COMPONENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/components)

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(${COMPONENT_DIR}/LCD)
add_subdirectory(${COMPONENT_DIR}/MPU6050)
add_subdirectory(${COMPONENT_DIR}/PCA9685)
add_subdirectory(${COMPONENT_DIR}/CalServo)
add_subdirectory(${COMPONENT_DIR}/I2Cdev)
add_subdirectory(${COMPONENT_DIR}/HC_SR04)
add_subdirectory(${COMPONENT_DIR}/RobotDog)

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${COMPONENT_DIR}/MPU6050 
    ${COMPONENT_DIR}/PCA9685 
    ${COMPONENT_DIR}/LCD
    ${COMPONENT_DIR}/CalServo
    ${COMPONENT_DIR}/I2Cdev
    ${COMPONENT_DIR}/HC_SR04
    ${COMPONENT_DIR}/RobotDog
)

target_link_directories(${PROJECT_NAME} PRIVATE 
    ${COMPONENT_DIR}/MPU6050 
    ${COMPONENT_DIR}/PCA9685 
    ${COMPONENT_DIR}/LCD
    ${COMPONENT_DIR}/CalServo
    ${COMPONENT_DIR}/I2Cdev
    ${COMPONENT_DIR}/HC_SR04
    ${COMPONENT_DIR}/RobotDog
)

target_link_libraries(${PROJECT_NAME} 
    MPU6050
    PCA9685 
    LCD
    CalServo
    I2Cdev
    HC_SR04
    RobotDog
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -g")
