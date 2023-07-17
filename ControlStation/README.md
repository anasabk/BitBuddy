# Control Station

Control Station is a program built in Qt and C++ to control the robot remotely. It has features such as object detection, mapping, pathfinding and telemetry.

## Dependencies

Control Station has been tested on Ubuntu 20.04 and Ubuntu 22.04.

Qt 6.5.1 needs to be installed to build and run it. [You can download the installer from here.](https://www.qt.io/download-qt-installer-oss?hsCtaTracking=99d9dd4f-5681-48d2-b096-470725510d34%7C074ddad0-fdef-4e53-8aa8-5e8a876d6ab4) (In the installer, untick Qt Design Studio, expand Qt 6.5.1 and tick Desktop gcc 64-bit.)

For mapping and pathfinding to work, bit_buddy.cc from [MappingAndLocalization](https://github.com/anasabk/BitBuddy/tree/main/MappingAndLocalization) needs to be compiled with [ORB_SLAM3](https://github.com/UZ-SLAMLab/ORB_SLAM3).

## Building

Open Qt Creator, click on "Open Project" and choose CMakeLists.txt in ControlStation.

Build the project by pressing Ctrl+B.

Copy the inside of runtime_files in ControlStation and paste it in the build directory.

Inside the mapping directory in the build directory, extract ORBvoc.txt.tar.gz file and put the program compiled from bit_buddy.cc in there and rename it "slam".

## Running

Run the program by running it directly from the build directory or pressing Ctrl+R in Qt Creator.

## Usage

You can toggle the switches by clicking on them or pressing number keys. You can move the joystick with mouse or using the WASD keys. After you start mapping and a point cloud is constructed, the first click on "Stop Mapping" will save the point cloud to a file. The second click will stop the mapping process. Or you can use "Kill Mapping" to send SIGKILL signal to the process.

### Pathfinding Usage

When you click "Start Pathfinding", a 2D map is created and a path planning interface is displayed on a separate window and also the UI. On this interface, yellow fields are empty spaces, purple areas are obstacles and the red star is the position of the robot.
On the interface, firstly, the user can determine the initial position of the robot by pressing the right mouse click on map (currently only works on the separate window). Then, the user selects the desired destination by left mouse click on map. Finally, the path planning algorithm starts working.
If the destination is on a reachable position, the algorithm finds the path and the movement simulation is started. The robot reaches the destination specified by the blue star by following the identified path. If the destination is not reachable, the blue star is placed without displaying any path and “open set is empty” message is displayed on terminal.
