add_library(RobotDog RobotDog.cpp)

target_link_libraries(RobotDog CalServo)
target_include_directories(RobotDog PUBLIC ../CalServo)
