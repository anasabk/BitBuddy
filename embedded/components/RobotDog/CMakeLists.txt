add_library(RobotDog CalServo.cpp)

target_link_libraries(RobotDog 
    CalServo 
    LCD 
    MPU6050
)

target_include_directories(RobotDog PUBLIC 
    ${COMPONENT_DIR}/CalServo 
    ${COMPONENT_DIR}/LCD 
    ${COMPONENT_DIR}/MPU6050
)
