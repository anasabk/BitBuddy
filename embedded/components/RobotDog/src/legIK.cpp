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
    bool is_right,
    bool is_front)
{
    servos[0] = hip;
    servos[1] = shoulder;
    servos[2] = knee;

    offsets[0] = off_hip;
    offsets[1] = off_shld;
    offsets[2] = off_knee;
    
    this->hip_l = hip_l;
    this->l1 = l1;
    this->l2 = l2;

    this->is_right = is_right;
    this->is_front = is_front;

    this->running = true;

    pthread_create(
        &servo_thread_id[0], 
        NULL, 
        servo_thread, 
        new struct servo_param(&theta_buf[0], servos[0])
    );

    pthread_create(
        &servo_thread_id[1], 
        NULL, 
        servo_thread, 
        new struct servo_param(&theta_buf[1], servos[1])
    );

    pthread_create(
        &servo_thread_id[2], 
        NULL, 
        servo_thread, 
        new struct servo_param(&theta_buf[2], servos[2])
    );
}

Leg::~Leg() {
    printf("destroying leg\n");
    pthread_kill(servo_thread_id[0], SIGTERM);
    pthread_join(servo_thread_id[0], NULL);
    pthread_kill(servo_thread_id[1], SIGTERM);
    pthread_join(servo_thread_id[1], NULL);
    pthread_kill(servo_thread_id[2], SIGTERM);
    pthread_join(servo_thread_id[2], NULL);
}

void Leg::get_degree(double x_mm, double y_mm, double z_mm, double *theta1, double *theta2, double *theta3) {
    double R2_yz = pow(y_mm, 2) + pow(z_mm, 2);
    double foot_to_shoulder_sq = pow(x_mm, 2) + pow(y_mm, 2) + pow(z_mm, 2) - pow(hip_l, 2);
    double temp_theta = acos((l2*l2 - l1*l1 - foot_to_shoulder_sq) / (-2 * l1 * sqrt(foot_to_shoulder_sq)));

    double degrees[3];
    degrees[0] = (acos(hip_l / sqrt(R2_yz)) + atan(y_mm / z_mm))*180/M_PI + offsets[0];
    degrees[1] = (temp_theta - atan(x_mm / sqrt(R2_yz - hip_l*hip_l)))*180/M_PI + offsets[1];
    degrees[2] = acos((foot_to_shoulder_sq - l2*l2 - l1*l1) / (-2 * l1 * l2))*180/M_PI - 35 + offsets[2];

    if(is_right) {
        degrees[1] = 180 - degrees[1];
        degrees[2] = 180 - degrees[2];
    }

    if(is_right != is_front)
        degrees[0] = 180 - degrees[0];

    *theta1 = degrees[0];
    *theta2 = degrees[1];
    *theta3 = degrees[2];

    last_pos[0] = x_mm;
    last_pos[1] = y_mm;
    last_pos[2] = z_mm;
}

void Leg::get_degree(const double (&dest)[3], double (&thetas)[3]) {
    get_degree(dest[0], dest[1], dest[2], &thetas[0], &thetas[1], &thetas[2]);
}

void Leg::get_degree_offset(double x_mm, double y_mm, double z_mm, double *x_deg, double *y_deg, double *z_deg) {
    if(last_pos[0] == -1 || last_pos[0] == -1 || last_pos[0] == -1)
        return;

    get_degree(last_pos[0] + x_mm, last_pos[1] + y_mm, last_pos[2] + z_mm, x_deg, y_deg, z_deg);
}

void Leg::get_degree_offset(const double (&offset)[3], double (&thetas)[3]) {
    get_degree_offset(offset[0], offset[1], offset[2], &thetas[0], &thetas[1], &thetas[2]);
}

void Leg::move(double x_mm, double y_mm, double z_mm) {
    get_degree(x_mm ,y_mm, z_mm, &theta_buf[0], &theta_buf[1], &theta_buf[2]);
    pthread_kill(servo_thread_id[0], SIGCONT);
    pthread_kill(servo_thread_id[1], SIGCONT);
    pthread_kill(servo_thread_id[2], SIGCONT);
}

void Leg::move(const double (&dest)[3]) {
    move(dest[0], dest[1], dest[2]);
}

void Leg::move_offset(double x_mm, double y_mm, double z_mm) {
    get_degree_offset(x_mm ,y_mm, z_mm, &theta_buf[0], &theta_buf[1], &theta_buf[2]);
    pthread_kill(servo_thread_id[0], SIGCONT);
    pthread_kill(servo_thread_id[1], SIGCONT);
    pthread_kill(servo_thread_id[2], SIGCONT);
}

void Leg::move_offset(const double (&offset)[3]) {
    move_offset(offset[0], offset[1], offset[2]);
}

void* Leg::servo_thread(void* param) {
    printf("Entered servo thread\n");
    const double *buffer = ((struct servo_param*)param)->theta_buf;
    CalServo *servo = ((struct servo_param*)param)->servo;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCONT);
    sigaddset(&set, SIGTERM);
    int sig;

    while(sigwait(&set, &sig) == 0 && sig != SIGTERM){
        printf("Moving Servo %d %d degrees\n", servo->getChannel(), (int)(*buffer));
        // servo->sweep(*((int*)buffer), 700);
    }

    servo->set_PWM(0);
    pthread_exit(NULL);
}
