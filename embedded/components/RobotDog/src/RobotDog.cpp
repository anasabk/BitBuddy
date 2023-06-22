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

sig_atomic_t is_connected;
sig_atomic_t is_running;


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
    is_running = false;
    sleep(1);
    pthread_join(mpu_thread_id, NULL);
    pthread_join(hcsr04_thread_id, NULL);
    main_body.sit_down();
    sleep(2);
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
}

void sigpipe_handler(int sig) {
   is_connected = 0; 
}

void sigint_handler(int sig) {
    is_running = 0;
    is_connected = 0; 
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
    addr.sin_addr.s_addr = inet_addr("192.168.43.174");

    while(is_running) {
        if(robot->mode_flag == true) {
            // Auto
        } else {
            robot->js_server_fd = socket(AF_INET, SOCK_DGRAM, 0);
            if(robot->js_server_fd < 0)  {
                perror("joystick socket creation failed");
                return;
            }

            std::cout << "[RaspAxes] Sending address to client..." << std::endl;

            for (int i = 0; i < 10; i++)
            {
                if (sendto(robot->js_server_fd, NULL, 0, 0, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
                    perror("[RaspAxes] sendto");
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            Axes buffer;
            bool js_connected = true;
            while (js_connected) {
                if (recvfrom(robot->js_server_fd, &buffer, sizeof(buffer), 0, NULL, NULL) == -1) {
                    perror("[RaspAxes] recvfrom");
                    continue;
                }

                if(buffer.x > -0.00001 && buffer.x < 0.00001 && buffer.y > -0.00001 && buffer.y < 0.00001)
                    continue;

                robot->main_body.move_forward(buffer.x * M_PI/4, buffer.y * 40);
            }
        }
    }
}

void RobotDog::run() {
    pthread_t temp;

    is_running = true;
    mode_flag = 0;

	pthread_create(&mpu_thread_id, NULL, mpu6050_thread, (void*)this);
	pthread_create(&hcsr04_thread_id, NULL, HCSR04_thread, (void*)this);
    pthread_create(&control_thread_id, NULL, control_thread, (void*)this);
    pthread_create(&temp, NULL, read_thread, (void*)(&this->mpu_buff));
    
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

	struct sigaction int_act;
	int_act.sa_handler = sigint_handler;
	sigaction(SIGINT, &int_act, nullptr);

	struct sigaction pipe_act;
	pipe_act.sa_handler = sigpipe_handler;
	sigaction(SIGPIPE, &pipe_act, nullptr);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("192.168.43.174");

    int fd = -1;
    CS_msg_s buffer = {"\0\0\0\0\0\0\0\0", false};
    symb temp_symb;
    char *args[2];
    while(is_running) {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if(fd < 0)  {
            perror("socket creation failed");
            continue;
        }

        if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
            perror("socket connection failed");
            close(fd);
            continue;
        }

        is_connected = 1;
        while(recv(fd, &buffer, sizeof(CS_msg_s), 0) > 0 && is_connected) {
            printf("%s %d %d\n", buffer.name, buffer.state, get_symb(buffer.name));
            switch (get_symb(buffer.name)) {
            case POSE:
                if(buffer.state)
                    main_body.sit_down();
                else
                    main_body.stand_up();
                break;
            
            case MODE:
                mode_flag = buffer.state;
                break;
            
            case ONOFF:
                if(buffer.state)
                    //ON
                    break;
                else
                    //OFF
                break;

            default:
                break;
            }
        }

        close(fd);
    }

    return;
}

void* RobotDog::mpu6050_thread(void* args) {
    RobotDog *robot = (RobotDog*)args;

	struct sched_param param;
    param.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return NULL;
    }

    robot->mpu6050.read_data(&robot->mpu_buff);

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

    pthread_exit(NULL);
}

void* RobotDog::HCSR04_thread(void* args) {
    RobotDog *robot = (RobotDog*)args;

    struct sched_param param;
    param.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return NULL;
    }

    // Check all HC_SR04 sensors
    for (int i = 0; i < NUM_HCSR04; i++) {
        robot->hc_sr04[i].get_distance();

        // Test connection
        if (robot->hc_sr04[i].get_distance() == 0) {
            std::cerr << "HC_SR04[" << i << "] connection error!" << std::endl;
            return NULL;
        }
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

    pthread_exit(NULL);
}
