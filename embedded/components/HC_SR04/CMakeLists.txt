add_library(HC_SR04 HC_SR04.cpp)

target_link_libraries(HC_SR04 pigpio)
target_include_directories(HC_SR04 PUBLIC ../pigpio)
