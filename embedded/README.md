# BitBuddy
## Dependencies:
### Eigen3
For matrix computations.
### libcamera-apps
libcamera-vid app is used to stream video footage.
### Pigpio
For getting input from HC-SR04 sensors.
### Pigpio
For getting input from HC-SR04 sensors.
### libi2c-dev
To control I2C devices onboard.


## Building:
```
git clone https://github.com/anasabk/BitBuddy 
mkdir build
cmake -B build -S .
cmake --build build
./build/BitBuddy <Control Station IP address>
```
