/**
 * This file is part of ORB-SLAM3
 *
 * Copyright (C) 2017-2020 Carlos Campos, Richard Elvira, Juan J. Gómez
 * Rodríguez, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
 * Copyright (C) 2014-2016 Raúl Mur-Artal, José M.M. Montiel and Juan D. Tardós,
 * University of Zaragoza.
 *
 * ORB-SLAM3 is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * ORB-SLAM3 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ORB-SLAM3. If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>

#include <opencv2/core/core.hpp>

#include <System.h>
#include <csignal>

using namespace std;

int flag = 1;

void signalHandler(int signum) {
    flag = 0;
}


int main(int argc, char **argv) {
    signal(SIGINT, signalHandler);

    if(argc != 3)
    {
        cerr << endl << "argc:" << argc << "!= 3"<< endl;
    }

    // open settings to get image resolution
    cv::FileStorage fsSettings(argv[2], cv::FileStorage::READ);
    if(!fsSettings.isOpened()) {
        cerr << "Failed to open settings file at: " << argv[2] << endl;
        exit(-1);
    }
    fsSettings.release();

    // Retrieve paths to images
    vector<double> vTimestamps;
    // Create SLAM system. It initializes all system threads and gets ready to
    // process frames.
    ORB_SLAM3::System SLAM(argv[1], argv[2], ORB_SLAM3::System::MONOCULAR, true);

    // Vector for tracking time statistics
    vector<float> vTimesTrack;
    cv::VideoCapture cap("udp://@:8087");
    // Check if camera opened successfully
    if (!cap.isOpened()) {
        std::cout << "Error opening video stream or file" << endl;
        return -1;
    }

    // Main loop

    int cnt_empty_frame = 0;
    int img_id = 0;
    int nImages = cap.get(cv::CAP_PROP_FRAME_COUNT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    double frame_diff_s = 1./fps;
    double start = chrono::duration_cast<chrono::duration<double, std::milli>>(chrono::system_clock::now().time_since_epoch()).count();
    while (flag) {
        cv::Mat im,im_track;
        bool success = cap.read(im);
        if (!success) {
            cnt_empty_frame++;
            std::cout<<"Empty frame...\n";
            if (cnt_empty_frame > 100)
                break;
            continue;
        }
        im_track = im.clone();
        
        double tframe = chrono::duration_cast<chrono::duration<double, std::milli>>(chrono::system_clock::now().time_since_epoch()).count() - start;
        std::cout << "timestamp: " << tframe << std::endl;
        time(NULL);
        ++img_id;

    #ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    #else
        std::chrono::monotonic_clock::time_point t1 = std::chrono::monotonic_clock::now();
    #endif

        // Pass the image to the SLAM system
        SLAM.TrackMonocular(im_track, tframe);

    #ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    #else
        std::chrono::monotonic_clock::time_point t2 = std::chrono::monotonic_clock::now();
    #endif

        double ttrack =
            std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1)
                .count();

        if (img_id % 100 == 0) {
            std::cout<<"Video FPS: "<<1./frame_diff_s<<"\n";
            std::cout<<"ORB-SLAM 3 running at: "<<1./ttrack<< " FPS\n";
        }
        vTimesTrack.push_back(ttrack);

        // Wait to load the next frame
        if (ttrack < frame_diff_s)
            usleep((frame_diff_s - ttrack) * 1e6);
    }

    //    // Stop all threads
    SLAM.Shutdown();
    std::cout << "shutting down" << std::endl;

    // Tracking time statistics
    // sort(vTimesTrack.begin(), vTimesTrack.end());
    // float totaltime = 0;
    // for (auto ni = 0; ni < vTimestamps.size(); ni++) {
    //     totaltime += vTimesTrack[ni];
    // }
    // cout << "-------" << endl << endl;
    // cout << "median tracking time: " << vTimesTrack[nImages / 2] << endl;
    // cout << "mean tracking time: " << totaltime / nImages << endl;

    // Save 3D points and timestamps of all keyframes they are visible in
    SLAM.getMap()->GetCurrentMap()->SaveWithTimestamps("bit_buddy_map_pts_and_keyframes.txt");
    std::cout << "bit_buddy_map_pts_and_keyframes written" << std::endl;
    // Save 3D points as obj file
    SLAM.getMap()->GetCurrentMap()->Save("bit_buddy_map_pts_out.obj");
    std::cout << "bit_buddy_map_pts_out written" << std::endl;

    // Save camera trajectory
    SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");
    std::cout << "KeyFrameTrajectory written" << std::endl;

    return 0;
}
