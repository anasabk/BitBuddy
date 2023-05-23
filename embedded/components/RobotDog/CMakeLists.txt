add_library(RobotDog RobotDog.cpp)

target_link_libraries(RobotDog 
    CalServo
    MPU6050
    LCD
    PCA9685
)

target_include_directories(RobotDog PUBLIC 
    ../CalServo
    ../MPU6050
    ../LCD
    ../PCA9685
)
