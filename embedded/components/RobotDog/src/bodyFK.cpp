#include "bodyFK.h"
#include "common.h"
#include <cmath>
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <signal.h>


Body::Body(
    CalServo (&servos)[12],
    double len_mm, 
    double width_mm) 
    : legs {
        LegIK(  4*M_PI/180,-5*M_PI/180, 1*M_PI/180, 55, 110, 130, false, false),
        LegIK(-10*M_PI/180,-3*M_PI/180,-7*M_PI/180, 55, 110, 130,  true, false),
        LegIK(  0*M_PI/180,-2*M_PI/180,-3*M_PI/180, 55, 110, 130,  true, true),
        LegIK(  8*M_PI/180, 6*M_PI/180, 5*M_PI/180, 55, 110, 130, false, true),
    },
    servo_buf {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
{
    for (int i = 0; i < 12; i++)
        this->servos[i] = &servos[i];

    this->len_mm = len_mm;
    this->width_mm = width_mm;

    this->last_roll = 0;
    this->last_yaw = 0;
    this->last_pitch = 0;
    this->last_x_mm = 0;
    this->last_y_mm = 0;
    this->last_z_mm = 0;

    pose_buf[RIGHTBACK][0]  = 0, pose_buf[RIGHTBACK][1]  = 0, pose_buf[RIGHTBACK][2]  = -70;
    pose_buf[RIGHTFRONT][0] = 0, pose_buf[RIGHTFRONT][1] = 0, pose_buf[RIGHTFRONT][2] = -70;
    pose_buf[LEFTBACK][0]   = 0, pose_buf[LEFTBACK][1]   = 0, pose_buf[LEFTBACK][2]   = -70;
    pose_buf[LEFTFRONT][0]  = 0, pose_buf[LEFTFRONT][1]  = 0, pose_buf[LEFTFRONT][2]  = -70;

    leg_buf[RIGHTBACK][0]  = -50, leg_buf[RIGHTBACK][1]  =  55, leg_buf[RIGHTBACK][2]  = 0;
    leg_buf[RIGHTFRONT][0] =  15, leg_buf[RIGHTFRONT][1] =  55, leg_buf[RIGHTFRONT][2] = 0;
    leg_buf[LEFTBACK][0]   = -50, leg_buf[LEFTBACK][1]   = -55, leg_buf[LEFTBACK][2]   = 0;
    leg_buf[LEFTFRONT][0]  =  15, leg_buf[LEFTFRONT][1]  = -55, leg_buf[LEFTFRONT][2]  = 0;
}

Body::~Body() {
    
}

template<size_t len>
void vector_sum(
    const double *a, 
    const double *b, 
    double *buf)
{
    for(int i = 0; i < len; i++)
        buf[i] = a[i] + b[i];
}

template<size_t len>
void vector_sub(
    const double *a, 
    const double *b, 
    double *buf)
{
    for(int i = 0; i < len; i++)
        buf[i] = a[i] - b[i];
}

/**
 * @brief
 * 
 * @note Clockwise is negative.
 * 
 * @param roll 
 * @param yaw 
 * @param pitch 
 * @param xm 
 * @param ym 
 * @param zm 
 * @param Tm 
 * @param rb 
 * @param rf 
 * @param lb 
 * @param lf 
 */
void Body::get_pose(
    double roll, 
    double yaw, 
    double pitch, 
    double x_mm, 
    double y_mm, 
    double z_mm,
    double (&rb)[3],
    double (&rf)[3],
    double (&lb)[3],
    double (&lf)[3]
) {
    if(z_mm < 65) {
        fprintf(stderr, "Trying to go too low, %dmm height is not possible\n", z_mm);
        return;
    }

    Eigen::Matrix4d Rx = (Eigen::Matrix4d() << 
        1,         0,         0, 0, 
        0, cos(roll),-sin(roll), 0,
        0, sin(roll), cos(roll), 0,
        0,         0,         0, 1
    ).finished(); 
    // std::cout << "Rx: " << Rx << std::endl;

    Eigen::Matrix4d Ry = (Eigen::Matrix4d() << 
         cos(pitch), 0, sin(pitch), 0, 
                  0, 1,          0, 0,
        -sin(pitch), 0, cos(pitch), 0,
                  0, 0,          0, 1
    ).finished();
    // std::cout << "Ry: " << Ry << std::endl;

    Eigen::Matrix4d Rz = (Eigen::Matrix4d() << 
        cos(yaw),-sin(yaw), 0, 0, 
        sin(yaw), cos(yaw), 0, 0,
                0,       0, 1, 0,
                0,       0, 0, 1
    ).finished();
    // std::cout << "Rz: " << Rz << std::endl;

    Eigen::Matrix4d Rxyz = Rx * Ry * Rz;
    // std::cout << "Rxyz: " << Rxyz << std::endl;

    Eigen::Matrix4d T = (Eigen::Matrix4d() << 
        0, 0, 0, x_mm,
        0, 0, 0, y_mm,
        0, 0, 0, z_mm,
        0, 0, 0,    0
    ).finished();
    // std::cout << "T: " << Rxyz << std::endl;

    T += Rxyz;
    // std::cout << "T * Rxyz: " << Rxyz << std::endl;


    Eigen::Matrix4d temp = (Eigen::Matrix4d() << 
         cos(M_PI/2), 0, sin(M_PI/2),  -len_mm/2,
        -sin(M_PI/2), 1, cos(M_PI/2),-width_mm/2,
                   0, 0,           1,          0,
                   0, 0,           0,          1
    ).finished();
    Eigen::Matrix4d Trb = T * temp;


    temp(0, 3) *= -1;
    Eigen::Matrix4d Trf = T * temp;


    temp(1, 3) *= -1;
    Eigen::Matrix4d Tlf = T * temp;

    
    temp(0, 3) *= -1;
    Eigen::Matrix4d Tlb = T * temp;


    rb[0] = Trb(0, 3) + 92.5;
    rb[1] = Trb(1, 3) + 38.75;
    rb[2] = Trb(2, 3);
    rf[0] = Trf(0, 3) - 92.5;
    rf[1] = Trf(1, 3) + 38.75;
    rf[2] = Trf(2, 3);
    lb[0] = Tlb(0, 3) + 92.5;
    lb[1] = Tlb(1, 3) - 38.75;
    lb[2] = Tlb(2, 3);
    lf[0] = Tlf(0, 3) - 92.5;
    lf[1] = Tlf(1, 3) - 38.75;
    lf[2] = Tlf(2, 3);
}

/**
 * @brief
 * 
 * @note Clockwise is negative.
 * 
 * @param roll 
 * @param yaw 
 * @param pitch 
 * @param xm 
 * @param ym 
 * @param zm 
 * @param Tm 
 * @param rb 
 * @param rf 
 * @param lb 
 * @param lf 
 */
void Body::get_pose_offset(
    double roll, 
    double yaw, 
    double pitch, 
    double x_mm, 
    double y_mm, 
    double z_mm,
    double (&rb)[3],
    double (&rf)[3],
    double (&lb)[3],
    double (&lf)[3]
) {
    get_pose(
        last_roll + roll,
        last_yaw + yaw,
        last_pitch + pitch, 
        last_x_mm + x_mm, 
        last_y_mm + y_mm, 
        last_z_mm + z_mm,
        rb, rf, lb, lf
    );  
}

void Body::pose(
    double roll, 
    double yaw, 
    double pitch, 
    double x_mm, 
    double y_mm, 
    double z_mm) 
{
    get_pose(roll, yaw, pitch, x_mm, y_mm, z_mm, pose_buf[RIGHTBACK], pose_buf[RIGHTFRONT], pose_buf[LEFTBACK], pose_buf[LEFTFRONT]);

    last_pitch  = pitch;
    last_roll   = roll;
    last_yaw    = yaw;
    last_x_mm   = x_mm;
    last_y_mm   = y_mm;
    last_z_mm   = z_mm;

    // printf("leg buffer:\n");
    // printf("rb : %lf, %lf, %lf\n", leg_buf[RIGHTBACK][0], leg_buf[RIGHTBACK][1], leg_buf[RIGHTBACK][2]);
    // printf("rf : %lf, %lf, %lf\n", leg_buf[RIGHTFRONT][0], leg_buf[RIGHTFRONT][1], leg_buf[RIGHTFRONT][2]);
    // printf("lb : %lf, %lf, %lf\n", leg_buf[LEFTBACK][0], leg_buf[LEFTBACK][1], leg_buf[LEFTBACK][2]);
    // printf("lf : %lf, %lf, %lf\n", leg_buf[LEFTFRONT][0], leg_buf[LEFTFRONT][1], leg_buf[LEFTFRONT][2]);

    // printf("pose buffer:\n");
    // printf("rb : %lf, %lf, %lf\n", pose_buf[RIGHTBACK][0], pose_buf[RIGHTBACK][1], pose_buf[RIGHTBACK][2]);
    // printf("rf : %lf, %lf, %lf\n", pose_buf[RIGHTFRONT][0], pose_buf[RIGHTFRONT][1], pose_buf[RIGHTFRONT][2]);
    // printf("lb : %lf, %lf, %lf\n", pose_buf[LEFTBACK][0], pose_buf[LEFTBACK][1], pose_buf[LEFTBACK][2]);
    // printf("lf : %lf, %lf, %lf\n", pose_buf[LEFTFRONT][0], pose_buf[LEFTFRONT][1], pose_buf[LEFTFRONT][2]);

    printf("final buffer:\n");
    double temp_buf[3];
    for(int i = 0; i < 4; i++) {
        vector_sub<3>(leg_buf[i], pose_buf[i], temp_buf);
        // printf("%d : %lf, %lf, %lf /", i, temp_buf[0], temp_buf[1], temp_buf[2]);
        legs[i].get_angles(temp_buf, &servo_buf[i*3]);
        // printf(" %f, %f, %f\n", servo_buf[i*3], servo_buf[i*3 + 1], servo_buf[i*3 + 2]);
    }

    move(150);
}

void Body::sit_down() {
    leg_buf[RIGHTBACK][0]  = -50, leg_buf[RIGHTBACK][1]  = -55, leg_buf[RIGHTBACK][2]  = 0;
    leg_buf[RIGHTFRONT][0] =  15, leg_buf[RIGHTFRONT][1] = -55, leg_buf[RIGHTFRONT][2] = 0;
    leg_buf[LEFTBACK][0]   = -50, leg_buf[LEFTBACK][1]   =  55, leg_buf[LEFTBACK][2]   = 0;
    leg_buf[LEFTFRONT][0]  =  15, leg_buf[LEFTFRONT][1]  =  55, leg_buf[LEFTFRONT][2]  = 0;
    
    pose(0, 0, 0, 0, 0, 70);
}

void Body::stand_up() {
    leg_buf[RIGHTBACK][0]  = -50, leg_buf[RIGHTBACK][1]  = -55, leg_buf[RIGHTBACK][2]  = 0;
    leg_buf[RIGHTFRONT][0] =  15, leg_buf[RIGHTFRONT][1] = -55, leg_buf[RIGHTFRONT][2] = 0;
    leg_buf[LEFTBACK][0]   = -50, leg_buf[LEFTBACK][1]   =  55, leg_buf[LEFTBACK][2]   = 0;
    leg_buf[LEFTFRONT][0]  =  15, leg_buf[LEFTFRONT][1]  =  55, leg_buf[LEFTFRONT][2]  = 0;
    
    pose(0, 0, 0, 0, 0, 140);
}

void* Body::move_thread(void *param) {
    printf("Entering movement thread\n");
    const float *rot_rad = ((Body::move_param*)param)->rot;
    const float *speed = ((Body::move_param*)param)->speed;
    const bool *run_flag = ((Body::move_param*)param)->run_flag;
    Body* body = ((Body::move_param*)param)->body;

    struct sched_param sched;
    sched.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &sched) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return NULL;
    }

    const double l_leen_off =  35.0;
    const double r_leen_off = -35.0;
    const double f_leen_off =  15.0;
    const double b_leen_off = -15.0;
    double drift_offset = 0;

    double new_pose_buf[4][3];
    
    int leg_num = 0;
    int pause_counter = 0;
    double temp_v[3];
    float prev[2];
    while(*run_flag) {
        if(*speed < 0.0001F && *speed > -0.0001F && *rot_rad < 0.0001F && *rot_rad > -0.0001F) {
            if(pause_counter < 2) pause_counter++;
        } else
            pause_counter = 0;

        if(pause_counter >= 2){
            printf("wait\n");
            wait_real(300);
            continue;
        }

        printf("Performing");

        if(leg_num == -1) leg_num = 0;

        // Update the new pose of the legs, only when necessary
        printf("old %f %f\nnew %f %f\n", prev[0], prev[1], *speed, *rot_rad);
        if ((prev[0] != *speed) || (prev[1] != *rot_rad)) 
        {
            printf("updating\n");
            body->get_pose(
                0, -*rot_rad, 0, 
                -*speed, 0, 140,
                new_pose_buf[RIGHTBACK], 
                new_pose_buf[RIGHTFRONT], 
                new_pose_buf[LEFTBACK], 
                new_pose_buf[LEFTFRONT]
            );

            prev[0] = *speed;
            prev[1] = *rot_rad;
        }


        // Leen to the opposite side
        body->pose(0, 0, 0, body->legs[leg_num].is_front() ? b_leen_off : f_leen_off, body->legs[leg_num].is_right() ? l_leen_off : r_leen_off, 140);


        // Position the leg
        body->leg_buf[leg_num][2] = 50;
        vector_sub<3>(body->leg_buf[leg_num], body->pose_buf[leg_num], temp_v);
        body->legs[leg_num].get_angles(temp_v, &body->servo_buf[leg_num*3]);
        body->move(80);

        body->leg_buf[leg_num][0] = (body->legs[leg_num].is_front() ? 15 :-50) - new_pose_buf[leg_num][0];
        body->leg_buf[leg_num][1] = (body->legs[leg_num].is_right() ?-55 : 55) - new_pose_buf[leg_num][1];
        vector_sub<3>(body->leg_buf[leg_num], body->pose_buf[leg_num], temp_v);
        body->legs[leg_num].get_angles(temp_v, &body->servo_buf[leg_num*3]);
        body->move(80);

        body->leg_buf[leg_num][2] = 0;
        vector_sub<3>(body->leg_buf[leg_num], body->pose_buf[leg_num], temp_v);
        body->legs[leg_num].get_angles(temp_v, &body->servo_buf[leg_num*3]);
        body->move(80);


        if(leg_num > 1) {
            // Go forward
            body->leg_buf[LEFTBACK][0]   += new_pose_buf[LEFTBACK][0] - drift_offset;
            body->leg_buf[RIGHTBACK][0]  += new_pose_buf[RIGHTBACK][0] + drift_offset;
            body->leg_buf[RIGHTFRONT][0] += new_pose_buf[RIGHTFRONT][0] + drift_offset;
            body->leg_buf[LEFTFRONT][0]  += new_pose_buf[LEFTFRONT][0] - drift_offset;
            body->leg_buf[LEFTBACK][1]   += new_pose_buf[LEFTBACK][1];
            body->leg_buf[RIGHTBACK][1]  += new_pose_buf[RIGHTBACK][1];
            body->leg_buf[RIGHTFRONT][1] += new_pose_buf[RIGHTFRONT][1];
            body->leg_buf[LEFTFRONT][1]  += new_pose_buf[LEFTFRONT][1];

            // Go to the next leg pair
            leg_num -= 2;

        } else {
            leg_num = -(leg_num - 3);
        }
    }

    printf("Exiting movement thread\n");
    pthread_exit(NULL);
}

void Body::recover() {
    struct timespec time;

    clock_gettime(CLOCK_MONOTONIC, &time);
    sit_down();
    wait_real(1000);

    servo_buf[0] = M_PI/2, servo_buf[1]  = M_PI, servo_buf[2]  = M_PI;
    servo_buf[3] = M_PI/2, servo_buf[4]  =    0, servo_buf[5]  =    0;
    servo_buf[6] = M_PI/2, servo_buf[7]  =    0, servo_buf[8]  =    0;
    servo_buf[9] = M_PI/2, servo_buf[10] = M_PI, servo_buf[11] = M_PI;
    move(1000);

    servo_buf[0] = 150*M_PI/180, servo_buf[1]  = M_PI, servo_buf[2]  = 0;
    servo_buf[9] =       M_PI/6, servo_buf[10] = M_PI, servo_buf[11] = 0;
    move(1000);

    servo_buf[0] =       M_PI/6, servo_buf[1]  = M_PI, servo_buf[2]  = 0;
    servo_buf[9] = 150*M_PI/180, servo_buf[10] = M_PI, servo_buf[11] = 0;
    move(1000);

    clock_gettime(CLOCK_MONOTONIC, &time);
    sit_down();
    wait_real(1000);
}

void Body::move(long dur) {
    int dt_ms = sqrt(dur);
    
    float dtheta[12];
    for(int i = 0; i < 12; i++) {
        dtheta[i] = (servo_buf[i] - servos[i]->get_last_rad()) / dt_ms;
        // printf("servo %d, %f %f degrees\n", servos[i]->getChannel(), servo_buf[i], dtheta[i]);
    }

    int servo_num;
    for(int i = 0; i < dt_ms; i++) {
        for(servo_num = 0; servo_num < 12; servo_num++)
            servos[servo_num]->set_rad_off(dtheta[servo_num]);

        wait_real(dt_ms);
    }
}
