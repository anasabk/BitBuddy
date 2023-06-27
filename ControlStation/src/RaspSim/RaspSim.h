#ifndef RASPSIM_H
#define RASPSIM_H

#include "RaspCam.h"
#include "RaspAxes.h"
#include "RaspSwitch.h"

#include <thread>

class RaspSim
{
public:
    RaspSim(bool testReconnection)
    {
        thread = std::thread(&RaspSim::run, this, testReconnection);
    }

    ~RaspSim()
    {
        std::cout << "[RaspSim] Cleaning up..." << std::endl;

        isRunning.store(false);
        thread.join();

        std::cout << "[RaspSim] Done." << std::endl;
    }

private:
    std::thread thread;
    std::atomic<bool> isRunning = true;

    void run(bool testReconnection)
    {
        while (isRunning.load())
        {
            if (testReconnection)
                std::this_thread::sleep_for(std::chrono::seconds(5));

            RaspCam raspCam;
            RaspAxes raspAxes;
            RaspSwitch raspSwitch;

            if (!testReconnection)
            {
                while (true)
                {
                    if (isRunning.load())
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    else
                        return;
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
};

#endif // RASPSIM_H
