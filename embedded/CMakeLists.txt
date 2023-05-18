cmake_minimum_required (VERSION 3.13)
project(BitBuddy)

set(COMPONENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/components)

add_executable(${PROJECT_NAME} src/main.cpp)

add_subdirectory(${COMPONENT_DIR}/LCD)
add_subdirectory(${COMPONENT_DIR}/MPU6050)
add_subdirectory(${COMPONENT_DIR}/PCA9685)
add_subdirectory(${COMPONENT_DIR}/CalServo)
add_subdirectory(${COMPONENT_DIR}/I2Cdev)
add_subdirectory(${COMPONENT_DIR}/HC_SR04)
# add_subdirectory(${COMPONENT_DIR}/pigpio)

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${COMPONENT_DIR}/MPU6050 
    ${COMPONENT_DIR}/PCA9685 
    ${COMPONENT_DIR}/LCD
    ${COMPONENT_DIR}/CalServo
    ${COMPONENT_DIR}/I2Cdev
    ${COMPONENT_DIR}/HC_SR04
    include
    # ${COMPONENT_DIR}/pigpio
)

target_link_directories(${PROJECT_NAME} PRIVATE 
    ${COMPONENT_DIR}/MPU6050 
    ${COMPONENT_DIR}/PCA9685 
    ${COMPONENT_DIR}/LCD
    ${COMPONENT_DIR}/CalServo
    ${COMPONENT_DIR}/I2Cdev
    ${COMPONENT_DIR}/HC_SR04
    # ${COMPONENT_DIR}/pigpio
)

target_link_libraries(${PROJECT_NAME} 
    MPU6050
    PCA9685 
    LCD
    CalServo
    I2Cdev
    HC_SR04
    # pigpio
)

target_link_options(BitBuddy PUBLIC "LINKER:-li2c,-pthread")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o -li2c -pthread")

