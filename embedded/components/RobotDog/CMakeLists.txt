add_library(RobotDog src/RobotDog.cpp)

target_link_libraries(RobotDog 
    CalServo
    MPU6050
    LCD
    PCA9685
    HC_SR04
)

target_include_directories(RobotDog PUBLIC 
    ../CalServo
    ../MPU6050
    ../LCD
    ../PCA9685
    ../HC_SR04
    include
)