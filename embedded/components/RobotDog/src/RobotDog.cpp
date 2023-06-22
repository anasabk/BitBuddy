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


int send_msg(int fd, const char* buf) {
    char msg_head[64];
    int msg_size = strlen(buf);
    snprintf(msg_head, 1024, "%d|", msg_size);
    send(fd, msg_head, strlen(msg_head), 0);

    return send(fd, buf, msg_size, 0);
}

int rec_msg(int fd, char* buf, int maxlen) {
    int size = 0;
    char num;
    while(recv(fd, &num, 1, 0) > 0) {
        if(num >= '0' && num <= '9') 
            size = size*10 + num - '0';
        else
            break;
    }

    int received = 0;
    if(size > maxlen)
        received = recv(fd, buf, maxlen, 0);
    else if(size > 0)
        received = recv(fd, buf, size, 0);
    buf[received] = 0;

    return received;
}

enum RobotDog::symb RobotDog::get_symb(const char *str) {
    for(int i = 0; i < 10; i++)
        if(strcmp(str, symb_str[i]))
            return (enum RobotDog::symb)i;

    return UNKNOWN;
}

void split_args(char* str, char** args, int argc, char delim) {
    for(int i = 0; i < argc; i++)
        args[i] = 0;
    
    int len = strlen(str);
    int j = 0;
    for(int i = 0; j < argc && i < len; i++) {
        if(str[i] != delim) {
            if(args[j] == NULL) {
                args[j] = &str[i];
            }
        } else {
            str[i] = 0;
            j++;
        }
    }

    return;
}


void RobotDog::run() {
    pthread_t temp;

    is_running = true;

	pthread_create(&mpu_thread_id, NULL, mpu6050_thread, (void*)this);
	pthread_create(&hcsr04_thread_id, NULL, HCSR04_thread, (void*)this);
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

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("server address");

    int fd = -1;
    char buffer[1024];
    symb temp_symb[2];
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
        while(rec_msg(fd, buffer, 1024) > 0 && is_connected) {
            split_args(buffer, args, 2, ':');
            temp_symb[0] = get_symb(args[0]);
            temp_symb[1] = get_symb(args[1]);
            
            switch (temp_symb[0])
            {
            case POSE:
                if(temp_symb[])
                break;
            
            default:
                break;
            }
        }

        close(fd);
    }


    // running_flag = false;

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
