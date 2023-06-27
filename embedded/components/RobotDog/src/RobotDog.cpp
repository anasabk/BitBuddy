#include "RobotDog.h"
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <cstring>


const char* RobotDog::symb_str[] = {
    "ONOFF",
    "ON",
    "OFF",
    "POSE",
    "STAND",
    "SIT",
    "MODE",
    "MAN",
    "AUTO",
    "CENTER"
};

// Tells weither the tcp connection is there
sig_atomic_t is_connected;

// Controls all of the running loops
sig_atomic_t is_running;

// Marks the termination to the main loop
sig_atomic_t term_flag;


RobotDog::RobotDog(int mpu_bus, int mpu_addr, int pca_bus, int pca_addr, int lcd_bus, int lcd_addr)
    : pca(pca_bus, pca_addr, 50), lcd(lcd_bus, lcd_addr), hc_sr04{HC_SR04(27, 17), HC_SR04(5, 6)}, mpu6050(mpu_bus, mpu_addr),
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

    MPU6050::MPU6050_data_t offsets = {
        0.0306, 0.0120, 0.0211, 0, 0.7868, -0.8433, -0.7314
    };
    mpu6050.set_offsets(&offsets);
}

RobotDog::~RobotDog() {

}

void* read_thread(void *param) {
    MPU6050::MPU6050_data_t *buf = (MPU6050::MPU6050_data_t*)param;
    while(is_running) {
        printf("accel: x=%.4lf y=%.4lf z=%.4lf / gyro: x=%.4lf y=%.4lf z=%.4lf \n", 
            buf->x_accel,
            buf->y_accel,
            buf->z_accel,
            buf->x_rot,
            buf->y_rot,
            buf->z_rot
        );
        sleep(1);
    }

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

enum RobotDog::symb RobotDog::get_symb(const char *str) {
    for(int i = 0; i < 10; i++)
        if(strcmp(str, symb_str[i]) == 0)
            return (enum RobotDog::symb)i;

    return UNKNOWN;
}

void* RobotDog::control_thread(void* param) {
    RobotDog *robot = (RobotDog*)param;

    // Joystick socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8081);
    addr.sin_addr.s_addr = inet_addr("192.168.43.165");

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

    while(is_running) {
        if(robot->mode_flag == true) {
            printf("Entered auto mode\n");
            while (is_running && robot->mode_flag) {
                if(robot->mpu_buff.x_accel < 0) {
                    int i = 0;
                    while (robot->mpu_buff.x_accel < 0.2 && i < 2) {
                        i++;
                        sleep(1);
                    }

                    robot->main_body.recover();
                }
                sleep(3);
            }
            printf("exitting auto mode\n");

        } else {
            printf("Entered manual mode\n");
            Axes buffer = {0, 0};
            
            pthread_t motion_thread;
            bool move_flag = true;
            float speed = 0.0;
            float rot = 0.0;
            Body::move_param move_param = {&speed, &rot, &move_flag, &robot->main_body};
            pthread_create(&motion_thread, NULL, robot->main_body.move_thread, &move_param);

            while (is_running && !robot->mode_flag) {
                printf("joystick loop\n");
                printf("x:%f y:%f\n", buffer.x, buffer.y);
                if (recvfrom(robot->js_server_fd, &buffer, sizeof(buffer), 0, NULL, NULL) <= 0) {
                    perror("[RaspAxes] recvfrom");
                    continue;
                }

                speed = buffer.y * 40;
                rot = -buffer.x * M_PI/16;
            }

            move_flag = false;
            pthread_join(motion_thread, NULL);

            printf("exitting manual mode\n");
        }
    }

    close(robot->js_server_fd);
    pthread_exit(NULL);
}

void RobotDog::run() {
    lcd.goHome();
    lcd.printf("BitBuddy");

    pthread_t temp;

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
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("192.168.43.165");

    int fd = -1;
    CS_msg_s buffer = {"\0\0\0\0\0\0\0\0", false};
    symb temp_symb;
    char *args[2];
    while(!term_flag) {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if(fd < 0) {
            perror("socket creation failed");
            continue;
        } else
            printf("Socket created\n");

        if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
            close(fd);
            
            if(errno == EINTR) {
                printf("Connection interrupted, aborting ...\n");
                break;
            }

            perror("socket connection failed");
            sleep(3);
            continue;
        } else
            printf("Connected to the server\n");

        is_connected = 1;
        while(recv(fd, &buffer, sizeof(CS_msg_s), 0) > 0 && is_connected) {
            printf("%s %d\n", buffer.name, buffer.state);
            switch (get_symb(buffer.name)) {
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
                    
                    servos[0].set_degree(86);
                    servos[1].set_degree(128);
                    servos[2].set_degree(4);
                    servos[3].set_degree(80);
                    servos[4].set_degree(61);
                    servos[5].set_degree(176);
                    servos[6].set_degree(90);
                    servos[7].set_degree(119);
                    servos[8].set_degree(176);
                    servos[9].set_degree(82);
                    servos[10].set_degree(69);
                    servos[11].set_degree(12);

                    sleep(2);
                    main_body.sit_down();
                    sleep(3);

                    pthread_create(&mpu_thread_id, NULL, mpu6050_thread, (void*)this);
                    pthread_create(&hcsr04_thread_id, NULL, HCSR04_thread, (void*)this);
                    pthread_create(&control_thread_id, NULL, control_thread, (void*)this);
                    pthread_create(&temp, NULL, read_thread, (void*)(&this->mpu_buff));

                } else if(!buffer.state && is_running) {
                    is_running = 0;
                    sleep(1);
                    pthread_join(mpu_thread_id, NULL);
                    pthread_join(hcsr04_thread_id, NULL);
                    pthread_join(control_thread_id, NULL);
                    pthread_join(temp, NULL);
                    main_body.sit_down();
                    sleep(2);
                }
                break;

            default:
                break;
            }
        }

        close(fd);
    }

    if(is_running) {
        is_running = 0;
        sleep(1);
        pthread_join(mpu_thread_id, NULL);
        pthread_join(hcsr04_thread_id, NULL);
        pthread_join(control_thread_id, NULL);
        pthread_join(temp, NULL);
        main_body.sit_down();
        sleep(2);
    }

    return;
}

void* RobotDog::mpu6050_thread(void* args) {
    printf("Entrering MPU6050 thread.\n");
    RobotDog *robot = (RobotDog*)args;

	struct sched_param param;
    param.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return NULL;
    }

    // Get current time
    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);

    long dt_ns = 1000000000L / MPU6050_SAMPLE_FREQ_HZ;

    while (is_running) {
    	robot->mpu6050.read_data(&robot->mpu_buff);

        // Add 10ms to current time
        timeNow.tv_nsec += dt_ns; // 10 ms in nanoseconds

        // Handle overflow
        while (timeNow.tv_nsec >= 1000000000L) {
            timeNow.tv_nsec -= 1000000000L;
            timeNow.tv_sec++;
        }

        // Sleep until the next 10ms point
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
    }

    printf("Exiting MPU6050 thread.\n");

    pthread_exit(NULL);
}

void* RobotDog::HCSR04_thread(void* args) {
    printf("Entering HC-SR04 thread.\n");
    RobotDog *robot = (RobotDog*)args;

    struct sched_param param;
    param.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return NULL;
    }

    // Get current time
    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);

    long dt_ns = 1000000000L / HC_SR04_SAMPLE_FREQ_HZ;

    while (is_running) {
        robot->front_dist[0] = robot->hc_sr04[0].get_distance();
        robot->front_dist[1] = robot->hc_sr04[1].get_distance();

        // Add dt_ns to current time
        timeNow.tv_nsec += dt_ns; // dt_ns in nanoseconds

        // Handle overflow
        while (timeNow.tv_nsec >= 1000000000L) {
            timeNow.tv_nsec -= 1000000000L;
            timeNow.tv_sec++;
        }

        // Sleep until the next dt_ns point
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
    }

    printf("Exiting HC-SR04 thread.\n");

    pthread_exit(NULL);
}
