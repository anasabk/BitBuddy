#include "RobotDog.h"
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <cstring>
#include <sys/wait.h>
#include "common.h"


// Tells weither the tcp connection is there
sig_atomic_t is_connected;

// Controls all of the running loops
sig_atomic_t is_running;

// Marks the termination to the main loop
sig_atomic_t term_flag;


RobotDog::RobotDog(int mpu_bus, int mpu_addr, int pca_bus, int pca_addr, int lcd_bus, int lcd_addr)
    : pca(pca_bus, pca_addr, 50), lcd(lcd_bus, lcd_addr), hc_sr04{HC_SR04(5, 6), HC_SR04(27, 17)}, mpu6050(mpu_bus, mpu_addr),
    servos{ 
		// Top, Mid, and Low motors for each leg
		CalServo(&pca, 0), CalServo(&pca, 1), CalServo(&pca, 2),	// Back Left
		CalServo(&pca, 3), CalServo(&pca, 4), CalServo(&pca, 5),	// Back Right
		CalServo(&pca, 6), CalServo(&pca, 7), CalServo(&pca, 8),	// Front Right 
		CalServo(&pca, 9), CalServo(&pca, 10), CalServo(&pca, 11)	// Front Left
    },
    main_body(servos, 185, 77.5)
{
	for(int i = 0; i < 12; i++)
        servos[i].refresh_fitter(cal_pwm_list, cal_degree_list[servos[i].getChannel()], 20);
    
    is_running = true;
}

RobotDog::~RobotDog() {

}

void* RobotDog::telem_thread(void *param) {
    std::cout << ("Entered telemetry thread\n");
    RobotDog *robot = (RobotDog*)param;

	struct sched_param sched;
    sched.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &sched) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return NULL;
    }

    // Telemetry socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TELEM_PORT);
    addr.sin_addr.s_addr = inet_addr(CONTROLSTATION_IP_ADDR);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)  {
        perror("telemetry socket creation failed");
        return NULL;
    }

    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);
    while (is_running) {
        sendto(fd, &robot->sensor_data, sizeof(robot->sensor_data), 0, (struct sockaddr*)&addr, sizeof(addr));
        wait_real(100);
    }

    close(fd);

    std::cout << ("exiting telemetry thread\n");
    pthread_exit(NULL);
}

void sigpipe_handler(int sig) {
    is_connected = 0; 
}

void sigint_handler(int sig) {
    is_connected = 0;
    is_running = 0;
    term_flag = 1;
}

void* RobotDog::control_thread(void* param) {
    RobotDog *robot = (RobotDog*)param;

    struct sched_param sched;
    sched.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &sched) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return NULL;
    }

    // Joystick socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(JOYSTICK_PORT);
    addr.sin_addr.s_addr = inet_addr(CONTROLSTATION_IP_ADDR);

    robot->js_server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(robot->js_server_fd < 0)  {
        perror("joystick socket creation failed");
        return NULL;
    }

    struct timeval read_timeout;
    read_timeout.tv_sec = 6;
    read_timeout.tv_usec = 0;
    setsockopt(robot->js_server_fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));

    std::cout << "[RaspAxes] Sending address to client..." << std::endl;

    for (int i = 0; i < 10; i++) {
        if (sendto(robot->js_server_fd, NULL, 0, 0, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("[RaspAxes] sendto");
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // double roll_buf = 0.0;
    // double pitch_buf = 0.0;
    float yaw_buf = 0.0;
    float speed_buf = 0.0;
    float temp_ratio;

    pthread_t motion_thread;
    bool move_flag = true;
    Body::move_param move_param = {&speed_buf, &yaw_buf, &move_flag, &robot->main_body};
    pthread_create(&motion_thread, NULL, robot->main_body.move_thread, &move_param);

    float temp_dist;
    int block_count = 0;
    while(is_running) {
        if(robot->mode_flag == true) {
            std::cout << ("Entered auto mode\n");
            bool is_open = false, right_is_open, left_is_open;
            while (is_running && robot->mode_flag) {
                // Check gravity
                if(robot->sensor_data.mpu_buff.z_accel < 0.2) {
                    std::cout << "Recovering" << std::endl;
                    yaw_buf   = 0.0F;
                    speed_buf = 0.0F;
                    sleep(3);

                    int i = 0;
                    while (robot->sensor_data.mpu_buff.z_accel < 0.2 && i < 2) {
                        i++;
                        sleep(1);
                    }

                    if(i >= 2)
                        robot->main_body.recover();
                
                // Recenter gravity: correct the robot's gravity center.
                } else {
                    // std::cout << "Recentering gravity" << std::endl;
                    // roll_buf  = -atan2(robot->sensor_data.mpu_buff.y_accel, robot->sensor_data.mpu_buff.z_accel);
                    // pitch_buf = -atan2(robot->sensor_data.mpu_buff.x_accel, GRAVITY_ACCEL);
                
                    // Check the sides
                    std::cout << "Checking sides" << std::endl;
                    is_open = (robot->sensor_data.front_dist[0] > 350) && (robot->sensor_data.front_dist[1] > 350);

                    // The way is fully opened
                    if(is_open) {
                        std::cout << "The way is open" << std::endl;
                        yaw_buf   = 0.0F;
                        speed_buf = 50.0F;
                        wait_real(500);
                        block_count = 0;
                        continue;
                    
                    } else if (block_count < 1) {
                        block_count++;
                        continue;
                    }
                    
                    // Path is blocked
                    while(!is_open) {
                        // Move backwards
                        std::cout << "The way is blocked, going backwards" << std::endl;
                        yaw_buf   = 0.0F;
                        speed_buf = -40.0F;
                        wait_real(4000);

                        // Stop the movement
                        yaw_buf   = 0.0F;
                        speed_buf = 0.0F;
                        wait_real(4000);

                        // Look to the left side
                        robot->main_body.pose(0, M_PI/6, 0, 0, 0, 140);
                        wait_real(1000);

                        temp_dist = (robot->sensor_data.front_dist[0] > 350) && (robot->sensor_data.front_dist[1] > 350) ? 
                            robot->sensor_data.front_dist[0] + robot->sensor_data.front_dist[1] : 0;

                        // Look to the right side
                        robot->main_body.pose(0, -M_PI/6, 0, 0, 0, 140);
                        wait_real(1000);
                        
                        temp_dist -= (robot->sensor_data.front_dist[0] > 350) && (robot->sensor_data.front_dist[1] > 350) ? 
                            robot->sensor_data.front_dist[0] + robot->sensor_data.front_dist[1] : 0;

                        // Path is open
                        robot->main_body.pose(0, 0, 0, 0, 0, 140);
                        wait_real(500);

                        if(temp_dist != 0) {
                            yaw_buf   = M_PI/8 * (temp_dist > 0 ? 1 : -1);
                            speed_buf = 0.0F;
                            wait_real(4000);
                        }

                        is_open =  (robot->sensor_data.front_dist[0] > 350) && (robot->sensor_data.front_dist[1] > 350);
                    }
                }

                wait_real(500);
            }

            std::cout << ("exitting auto mode\n");
            yaw_buf   = 0.0;
            speed_buf = 0.0;
            wait_real(500);

        } else {
            std::cout << ("Entered manual mode\n");
            Axes buffer = {0, 0};

            while (is_running && !robot->mode_flag) {
                if (recvfrom(robot->js_server_fd, &buffer, sizeof(buffer), 0, NULL, NULL) <= 0) {
                    perror("[RaspAxes] recvfrom");
                    continue;
                }

                speed_buf = buffer.y * 60;
                yaw_buf = -buffer.x * M_PI/8;
            }

            yaw_buf   = 0.0;
            speed_buf = 0.0;

            std::cout << ("exitting manual mode\n");
        }
    }

    move_flag = false;
    pthread_join(motion_thread, NULL);

    close(robot->js_server_fd);
    pthread_exit(NULL);
}

void RobotDog::run() {
    lcd.goHome();
    lcd.printf("Hello world");
    lcd.setPosition(0, 1);
    lcd.printf("I am dug");

    servos[0].set_rad(86*M_PI/180);
    servos[1].set_rad(128*M_PI/180);
    servos[2].set_rad(4*M_PI/180);
    servos[3].set_rad(80*M_PI/180);
    servos[4].set_rad(61*M_PI/180);
    servos[5].set_rad(176*M_PI/180);
    servos[6].set_rad(90*M_PI/180);
    servos[7].set_rad(119*M_PI/180);
    servos[8].set_rad(176*M_PI/180);
    servos[9].set_rad(82*M_PI/180);
    servos[10].set_rad(69*M_PI/180);
    servos[11].set_rad(12*M_PI/180);

    pthread_t telem_thread_id;

    term_flag = 0;
    mode_flag = 0;
    is_running = 0;

	struct sigaction int_act;
	int_act.sa_handler = sigint_handler;
	sigaction(SIGINT, &int_act, nullptr);

	struct sigaction pipe_act;
	pipe_act.sa_handler = sigpipe_handler;
	sigaction(SIGPIPE, &pipe_act, nullptr);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SWITCH_PORT);
    addr.sin_addr.s_addr = inet_addr(CONTROLSTATION_IP_ADDR);

    mpu6050.calibrate();

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGPIPE);

    int fd = -1;
    struct {
        symb symbol = UNKNOWN;
        bool state = 0;
    } buffer;

    while(!term_flag) {
        mode_flag = 0;

        fd = socket(AF_INET, SOCK_STREAM, 0);
        if(fd < 0) {
            perror("socket creation failed");
            continue;
        } else
            std::cout << ("Socket created\n");

        if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
            close(fd);
            
            if(errno == EINTR) {
                std::cout << ("Connection interrupted, aborting ...\n");
                break;
            }

            perror("socket connection failed");
            sleep(3);
            continue;
        } else
            std::cout << ("Connected to the server\n");

        is_connected = 1;
        while(is_connected) {
            recv(fd, &buffer, sizeof(buffer.symbol) + sizeof(buffer.state), 0);
            std::cout << ("switch command: %d %d\n", buffer.symbol, buffer.state);

            switch (buffer.symbol) {
            case POSE:
                if(is_running) {
                    if(buffer.state)
                        main_body.stand_up();
                    else
                        main_body.sit_down();
                }
                break;
            
            case MODE:
                if(is_running)
                    mode_flag = buffer.state;
                break;
            
            case ONOFF:
                if(buffer.state && !is_running) {
                    is_running = 1;

                    sleep(2);
                    main_body.sit_down();
                    sleep(3);

                    pthread_sigmask(SIG_BLOCK, &set, NULL);

                    pthread_create(&mpu_thread_id, NULL, mpu6050_thread, (void*)this);
                    pthread_create(&hcsr04_thread_id, NULL, HCSR04_thread, (void*)this);
                    pthread_create(&control_thread_id, NULL, control_thread, (void*)this);
                    pthread_create(&telem_thread_id, NULL, telem_thread, (void*)this);
                    
                    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

                    // if((video_streamer = fork()) == 0){
                    //     execv("/bin/libcamera-vid", vid_args);
                    //     perror("Could not fork the video streamer");
                    //     exit(0);
                    // }

                } else if(!buffer.state && is_running) {
                    is_connected = false;
                }
                break;

            default:
                break;
            }
        }

        
        if(is_running) {
            is_running = 0;
            pthread_join(mpu_thread_id, NULL);
            pthread_join(hcsr04_thread_id, NULL);
            pthread_join(control_thread_id, NULL);
            pthread_join(telem_thread_id, NULL);
            // kill(video_streamer, SIGINT);
            // wait(&video_streamer);
        }

        main_body.sit_down();
        sleep(1);
        pca.set_all_pwm(0, 0);

        close(fd);
    }

    std::cout << ("Exiting\n");

    if(is_running) {
        is_running = 0;
        pthread_join(mpu_thread_id, NULL);
        pthread_join(hcsr04_thread_id, NULL);
        pthread_join(control_thread_id, NULL);
        pthread_join(telem_thread_id, NULL);
        // kill(video_streamer, SIGINT);
        // wait(&video_streamer);
    }

    main_body.sit_down();
    sleep(1);
    pca.set_all_pwm(0, 0);

    return;
}

void* RobotDog::mpu6050_thread(void* args) {
    std::cout << ("Entrering MPU6050 thread.\n");
    RobotDog *robot = (RobotDog*)args;

	struct sched_param param;
    param.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return NULL;
    }

    long dt_ms = 1000L / MPU6050_SAMPLE_FREQ_HZ;
    while (is_running) {
    	robot->mpu6050.read_data(&robot->sensor_data.mpu_buff);
        wait_real(dt_ms);
    }

    std::cout << ("Exiting MPU6050 thread.\n");

    pthread_exit(NULL);
}

void* RobotDog::HCSR04_thread(void* args) {
    std::cout << ("Entering HC-SR04 thread.\n");
    RobotDog *robot = (RobotDog*)args;

    struct sched_param param;
    param.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return NULL;
    }

    while (is_running) {
        robot->sensor_data.front_dist[0] = robot->hc_sr04[0].get_distance();
        robot->sensor_data.front_dist[1] = robot->hc_sr04[1].get_distance();

        wait_real(65);
    }

    std::cout << ("Exiting HC-SR04 thread.\n");

    pthread_exit(NULL);
}
