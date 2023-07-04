#include "legIK.h"
#include <pthread.h>
#include <signal.h>


Leg::Leg(
    CalServo *hip, 
    double off_hip,
    CalServo *shoulder,
    double off_shld, 
    CalServo *knee,
    double off_knee, 
    double hip_l, 
    double l1, 
    double l2, 
    bool is_right_flag,
    bool is_front_flag)
{
    servos[0] = hip;
    servos[1] = shoulder;
    servos[2] = knee;

    theta_buf[0] = 0;
    theta_buf[1] = 0;
    theta_buf[2] = 0;

    speed_buf = 0;

    offsets[0] = off_hip;
    offsets[1] = off_shld;
    offsets[2] = off_knee;
    
    this->hip_l = hip_l;
    this->l1 = l1;
    this->l2 = l2;

    this->is_right_flag = is_right_flag;
    this->is_front_flag = is_front_flag;

    sem_init(&lock_count, 0, 3);

    pthread_create(
        servo_thread_id, 
        NULL, 
        servo_thread, 
        new struct servo_param(&theta_buf[0], &speed_buf, servos[0], &lock_count)
    );

    pthread_create(
        &servo_thread_id[1], 
        NULL, 
        servo_thread, 
        new struct servo_param(&theta_buf[1], &speed_buf, servos[1], &lock_count)
    );

    pthread_create(
        &servo_thread_id[2], 
        NULL, 
        servo_thread, 
        new struct servo_param(&theta_buf[2], &speed_buf, servos[2], &lock_count)
    );
}

Leg::~Leg() {
    printf("destroying leg\n");
    
    pthread_kill(servo_thread_id[0], SIGTERM);
    pthread_kill(servo_thread_id[1], SIGTERM);
    pthread_kill(servo_thread_id[2], SIGTERM);
    pthread_join(servo_thread_id[0], NULL);
    pthread_join(servo_thread_id[1], NULL);
    pthread_join(servo_thread_id[2], NULL);

    sem_destroy(&lock_count);
    printf("leg destroyted successfully\n");
}

void Leg::get_degree(double x_mm, double y_mm, double z_mm, int *theta1, int *theta2, int *theta3) {
    double foot_to_shoulder_sq = pow(x_mm, 2) + pow(y_mm, 2) + pow(z_mm, 2) - pow(hip_l, 2);
    
    if (foot_to_shoulder_sq < (l1*l1 + l2*l2 - 2*l1*l2*cos(35/180*M_PI)) || 
        foot_to_shoulder_sq > pow(l1+l2, 2)) {
        printf("Length of %lfmm from foot to shoulder is not possible, aborting ...\n", foot_to_shoulder_sq);
        return;
    }
    
    double R2_yz = pow(y_mm, 2) + pow(z_mm, 2);
    double temp_theta = acos((l2*l2 - l1*l1 - foot_to_shoulder_sq) / (-2 * l1 * sqrt(foot_to_shoulder_sq)));
    double hip_dir = is_right_flag ? -1 : 1;

    double degrees[3];
    degrees[0] = (acos(hip_dir*hip_l / sqrt(R2_yz)) + atan(y_mm / fabs(z_mm)))*180/M_PI;
    degrees[1] = (temp_theta - atan(x_mm / sqrt(R2_yz - hip_l*hip_l)))*180/M_PI;
    degrees[2] = acos((foot_to_shoulder_sq - l2*l2 - l1*l1) / (-2 * l1 * l2))*180/M_PI - 35;

    if (degrees[0] > 180 || degrees[0] < 0 ||
        degrees[1] > 180 || degrees[1] < 0 ||
        degrees[2] > 180 || degrees[2] < 0) {
        printf("Out of reach, aborting ...\n");
    }

    if(is_right_flag) {
        degrees[1] = 180 - degrees[1];
        degrees[2] = 180 - degrees[2];
    }

    if(is_front_flag)
        degrees[0] = 180 - degrees[0];

    *theta1 = degrees[0] + offsets[0];
    *theta2 = degrees[1] + offsets[1];
    *theta3 = degrees[2] + offsets[2];

    last_pos[0] = x_mm;
    last_pos[1] = y_mm;
    last_pos[2] = z_mm;
}

void Leg::get_degree(const double (&dest)[3], int (&thetas)[3]) {
    get_degree(dest[0], dest[1], dest[2], &thetas[0], &thetas[1], &thetas[2]);
}

void Leg::get_degree_offset(double x_mm, double y_mm, double z_mm, int *x_deg, int *y_deg, int *z_deg) {
    if(last_pos[0] == -1 || last_pos[0] == -1 || last_pos[0] == -1)
        return;

    get_degree(last_pos[0] + x_mm, last_pos[1] + y_mm, last_pos[2] + z_mm, x_deg, y_deg, z_deg);
}

void Leg::get_degree_offset(const double (&offset)[3], int (&thetas)[3]) {
    get_degree_offset(offset[0], offset[1], offset[2], &thetas[0], &thetas[1], &thetas[2]);
}

void Leg::move(double x_mm, double y_mm, double z_mm, int speed_ms) {
    sem_wait(&lock_count);
    sem_wait(&lock_count);
    sem_wait(&lock_count);

    get_degree(x_mm ,y_mm, z_mm, &theta_buf[0], &theta_buf[1], &theta_buf[2]);
    speed_buf = speed_ms;

    pthread_kill(servo_thread_id[0], SIGCONT);
    pthread_kill(servo_thread_id[1], SIGCONT);
    pthread_kill(servo_thread_id[2], SIGCONT);
}

void Leg::move(const double (&dest)[3], int speed_ms) {
    move(dest[0], dest[1], dest[2]);
}

void Leg::move_offset(double x_mm, double y_mm, double z_mm, int speed_ms) {
    sem_wait(&lock_count);
    sem_wait(&lock_count);
    sem_wait(&lock_count);

    get_degree_offset(x_mm ,y_mm, z_mm, &theta_buf[0], &theta_buf[1], &theta_buf[2]);
    speed_buf = speed_ms;

    pthread_kill(servo_thread_id[0], SIGCONT);
    pthread_kill(servo_thread_id[1], SIGCONT);
    pthread_kill(servo_thread_id[2], SIGCONT);
}

void Leg::move_offset(const double (&offset)[3], int speed_ms) {
    move_offset(offset[0], offset[1], offset[2], speed_ms);
}

void Leg::move_d(double theta1, double theta2, double theta3, int speed_ms) {
    sem_wait(&lock_count);
    sem_wait(&lock_count);
    sem_wait(&lock_count);

    theta_buf[0] = theta1;
    theta_buf[1] = theta2;
    theta_buf[2] = theta3;
    speed_buf = speed_ms;

    pthread_kill(servo_thread_id[0], SIGCONT);
    pthread_kill(servo_thread_id[1], SIGCONT);
    pthread_kill(servo_thread_id[2], SIGCONT);
}

void* Leg::servo_thread(void* param) {
    printf("Entered servo thread\n");
    const int *theta_buf = ((struct servo_param*)param)->theta_buf;
    const int *speed_buf = ((struct servo_param*)param)->speed_buf;
    CalServo *servo = ((struct servo_param*)param)->servo;
    sem_t *lock_sem = ((struct servo_param*)param)->lock_sem;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCONT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    int sig;
    while(sigwait(&set, &sig) == 0 && sig != SIGTERM) {
        servo->sweep(*theta_buf, *speed_buf);
        sem_post(lock_sem);
    }

    printf("Exitting servo thread\n");
    servo->set_PWM(0);
    delete(param);
    pthread_exit(NULL);
}

bool Leg::is_right() {
    return is_right_flag;
}

bool Leg::is_front() {
    return is_front_flag;
}
