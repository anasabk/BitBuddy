#include "bodyFK.h"
#include <cmath>


Body::Body(
    Leg* left_front, 
    Leg* right_front, 
    Leg* left_back, 
    Leg* right_back, 
    double len_mm, 
    double width_mm) 
{
    legs[RIGHTBACK] = right_back;
    legs[RIGHTFRONT] = right_front;
    legs[LEFTBACK] = left_back;
    legs[LEFTFRONT] = left_front;

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
    leg_buf[RIGHTFRONT][0] =  30, leg_buf[RIGHTFRONT][1] =  55, leg_buf[RIGHTFRONT][2] = 0;
    leg_buf[LEFTBACK][0]   = -50, leg_buf[LEFTBACK][1]   = -55, leg_buf[LEFTBACK][2]   = 0;
    leg_buf[LEFTFRONT][0]  =  30, leg_buf[LEFTFRONT][1]  = -55, leg_buf[LEFTFRONT][2]  = 0;
}

Body::~Body() {
    // stand_up();
    // sleep(1);
    // sit_down();
}

template<size_t a_col, size_t b_col>
void matrix_mult(
    const double (&a)[b_col][a_col], 
    const double (&b)[b_col][a_col], 
    double (&buf)[b_col][a_col]) 
{
    for(int i = 0; i < b_col; i++) {
        for(int j = 0; j < b_col; j++) {
            buf[j][i] = 0;
            for(int k = 0; k < a_col; k++)
                buf[j][i] += a[j][k] * b[k][i];
        }
    }
}

template<size_t row, size_t col>
void matrix_sum(
    const double (&a)[row][col], 
    const double (&b)[row][col], 
    double (&buf)[row][col]) 
{
    for(int i = 0; i < row; i++)
        for(int j = 0; j < col; j++)
            buf[i][j] = a[i][j] + b[i][j];
}

template<size_t len>
void vector_sum(
    const double (&a)[len], 
    const double (&b)[len], 
    double (&buf)[len])
{
    for(int i = 0; i < len; i++)
        buf[i] = a[i] + b[i];
}

template<size_t len>
void vector_sub(
    const double (&a)[len], 
    const double (&b)[len], 
    double (&buf)[len])
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
    double (&Tm)[4][4],
    double (&rb)[3],
    double (&rf)[3],
    double (&lb)[3],
    double (&lf)[3]
) {
    if(z_mm < 65) {
        fprintf(stderr, "Trying to go too low, %dmm height is not possible\n", z_mm);
        return;
    }

    double Rx[4][4] = {
        {1,         0,         0, 0}, 
        {0, cos(roll),-sin(roll), 0},
        {0, sin(roll), cos(roll), 0},
        {0,         0,         0, 1}
    };

    double Ry[4][4] = {
        { cos(pitch), 0, sin(pitch), 0}, 
        {          0, 1,          0, 0},
        {-sin(pitch), 0, cos(pitch), 0},
        {          0, 0,          0, 1}
    };

    double Rz[4][4] = {
        {cos(yaw),-sin(yaw), 0, 0}, 
        {sin(yaw), cos(yaw), 0, 0},
        {        0,       0, 1, 0},
        {        0,       0, 0, 1}
    };

    double Rxy[4][4];
    double Rxyz[4][4];

    matrix_mult<4, 4>(Rx, Ry, Rxy);
    matrix_mult<4, 4>(Rxy, Rz, Rxyz);

    double T[][4] = {
        {0, 0, 0, x_mm},
        {0, 0, 0, y_mm},
        {0, 0, 0, z_mm},
        {0, 0, 0,  0}
    };
    matrix_sum<4, 4>(T, Rxyz, Tm);


    double temp[4][4] = {
        { cos(M_PI/2), 0, sin(M_PI/2),  -len_mm/2},
        {-sin(M_PI/2), 1, cos(M_PI/2),-width_mm/2},
        {           0, 0,           1,          0},
        {           0, 0,           0,          1}
    };
    double Trb[4][4];
    matrix_mult<4, 4>(Tm, temp, Trb);


    temp[0][3] *= -1;
    double Trf[4][4];
    matrix_mult<4, 4>(Tm, temp, Trf);


    temp[1][3] *= -1;
    double Tlf[4][4];
    matrix_mult<4, 4>(Tm, temp, Tlf);

    
    temp[0][3] *= -1;
    double Tlb[4][4];
    matrix_mult<4, 4>(Tm, temp, Tlb);


    rb[0] = Trb[0][3] + 92.5;
    rb[1] = Trb[1][3] + 38.75;
    rb[2] = Trb[2][3];

    rf[0] = Trf[0][3] - 92.5;
    rf[1] = Trf[1][3] + 38.75;
    rf[2] = Trf[2][3];

    lb[0] = Tlb[0][3] + 92.5;
    lb[1] = Tlb[1][3] - 38.75;
    lb[2] = Tlb[2][3];

    lf[0] = Tlf[0][3] - 92.5;
    lf[1] = Tlf[1][3] - 38.75;
    lf[2] = Tlf[2][3];
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
    double (&Tm)[4][4],
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
        Tm, rb, rf, lb, lf
    );  
}

// void Body::pose_offset(
//     double roll, 
//     double yaw, 
//     double pitch, 
//     double x_mm, 
//     double y_mm, 
//     double z_mm) 
// {
//     double Tm[4][4];
//     double rb[3];
//     double rf[3];
//     double lb[3];
//     double lf[3];

//     get_pose_offset(roll, yaw, pitch, x_mm, y_mm, z_mm, Tm, rb, rf, lb, lf);

//     legs[RIGHTBACK]->move_offset(rb);
//     legs[RIGHTFRONT]->move_offset(rf);
//     legs[LEFTBACK]->move_offset(lb);
//     legs[LEFTFRONT]->move_offset(lf);

//     last_pitch  += pitch;
//     last_roll   += roll;
//     last_yaw    += yaw;
//     last_x_mm   += x_mm;
//     last_y_mm   += y_mm;
//     last_z_mm   += z_mm;

//     offset_buf[RIGHTBACK][0]  = rb[0]; offset_buf[RIGHTBACK][1]  = rb[1]; offset_buf[RIGHTBACK][2]  = rb[2];
//     offset_buf[RIGHTFRONT][0] = rf[0]; offset_buf[RIGHTFRONT][1] = rf[1]; offset_buf[RIGHTFRONT][2] = rf[2];
//     offset_buf[LEFTBACK][0]   = lb[0]; offset_buf[LEFTBACK][1]   = lb[1]; offset_buf[LEFTBACK][2]   = lb[2];
//     offset_buf[LEFTFRONT][0]  = lf[0]; offset_buf[LEFTFRONT][1]  = lf[1]; offset_buf[LEFTFRONT][2]  = lf[2];
// }

void Body::pose(
    double roll, 
    double yaw, 
    double pitch, 
    double x_mm, 
    double y_mm, 
    double z_mm) 
{
    double Tm[4][4];

    get_pose(roll, yaw, pitch, x_mm, y_mm, z_mm, Tm, pose_buf[RIGHTBACK], pose_buf[RIGHTFRONT], pose_buf[LEFTBACK], pose_buf[LEFTFRONT]);

    last_pitch  = pitch;
    last_roll   = roll;
    last_yaw    = yaw;
    last_x_mm   = x_mm;
    last_y_mm   = y_mm;
    last_z_mm   = z_mm;

    double rb[3];
    double rf[3];
    double lb[3];
    double lf[3];

    vector_sub<3>(leg_buf[RIGHTBACK], pose_buf[RIGHTBACK], rb);
    vector_sub<3>(leg_buf[RIGHTFRONT], pose_buf[RIGHTFRONT], rf);
    vector_sub<3>(leg_buf[LEFTBACK], pose_buf[LEFTBACK], lb);
    vector_sub<3>(leg_buf[LEFTFRONT], pose_buf[LEFTFRONT], lf);

    printf("rb : %lf, %lf, %lf\n", leg_buf[RIGHTBACK][0], leg_buf[RIGHTBACK][1], leg_buf[RIGHTBACK][2]);
    printf("rf : %lf, %lf, %lf\n", leg_buf[RIGHTFRONT][0], leg_buf[RIGHTFRONT][1], leg_buf[RIGHTFRONT][2]);
    printf("lb : %lf, %lf, %lf\n", leg_buf[LEFTBACK][0], leg_buf[LEFTBACK][1], leg_buf[LEFTBACK][2]);
    printf("lf : %lf, %lf, %lf\n", leg_buf[LEFTFRONT][0], leg_buf[LEFTFRONT][1], leg_buf[LEFTFRONT][2]);

    printf("rb : %lf, %lf, %lf\n", rb[0], rb[1], rb[2]);
    printf("rf : %lf, %lf, %lf\n", rf[0], rf[1], rf[2]);
    printf("lb : %lf, %lf, %lf\n", lb[0], lb[1], lb[2]);
    printf("lf : %lf, %lf, %lf\n", lf[0], lf[1], lf[2]);

    legs[RIGHTBACK]->move(rb);
    legs[RIGHTFRONT]->move(rf);
    legs[LEFTBACK]->move(lb);
    legs[LEFTFRONT]->move(lf);
}

void Body::sit_down() {
    leg_buf[RIGHTBACK][0]  = -50, leg_buf[RIGHTBACK][1]  = -55, leg_buf[RIGHTBACK][2]  = 0;
    leg_buf[RIGHTFRONT][0] =  30, leg_buf[RIGHTFRONT][1] = -55, leg_buf[RIGHTFRONT][2] = 0;
    leg_buf[LEFTBACK][0]   = -50, leg_buf[LEFTBACK][1]   =  55, leg_buf[LEFTBACK][2]   = 0;
    leg_buf[LEFTFRONT][0]  =  30, leg_buf[LEFTFRONT][1]  =  55, leg_buf[LEFTFRONT][2]  = 0;
    
    pose(0, 0, 0, 0, 0, 70);
}

void Body::stand_up() {
    leg_buf[RIGHTBACK][0]  = -50, leg_buf[RIGHTBACK][1]  = -55, leg_buf[RIGHTBACK][2]  = 0;
    leg_buf[RIGHTFRONT][0] =  30, leg_buf[RIGHTFRONT][1] = -55, leg_buf[RIGHTFRONT][2] = 0;
    leg_buf[LEFTBACK][0]   = -50, leg_buf[LEFTBACK][1]   =  55, leg_buf[LEFTBACK][2]   = 0;
    leg_buf[LEFTFRONT][0]  =  30, leg_buf[LEFTFRONT][1]  =  55, leg_buf[LEFTFRONT][2]  = 0;
    
    pose(0, 0, 0, 0, 0, 170);
}

void Body::step_forward() {
    double rb[3];
    double rf[3];
    double lb[3];
    double lf[3];

    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);

    pose(0, 0, 0, 0, -20, 170);
    leg_buf[LEFTFRONT][0] = 110, leg_buf[LEFTFRONT][1] = 55, leg_buf[LEFTFRONT][2] = 0;
    vector_sub<3>(leg_buf[LEFTFRONT], pose_buf[LEFTFRONT], lf);
    legs[LEFTFRONT]->move(lf);
    timeNow.tv_nsec += 500000000;
    while (timeNow.tv_nsec >= 1000000000L) {
        timeNow.tv_nsec -= 1000000000L;
        timeNow.tv_sec++;}
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
    leg_buf[LEFTBACK][0] += 20;
    leg_buf[RIGHTBACK][0] += 20;
    leg_buf[RIGHTFRONT][0] += 20;
    leg_buf[LEFTFRONT][0] += 20;
    pose(0, 0, 0, 0, 0, 170);
    timeNow.tv_nsec += 500000000;
    while (timeNow.tv_nsec >= 1000000000L) {
        timeNow.tv_nsec -= 1000000000L;
        timeNow.tv_sec++;}
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);

    pose(0, 0, 0, 0, 20, 170);
    leg_buf[RIGHTBACK][0]  =  10, leg_buf[RIGHTBACK][1]  =  55, leg_buf[RIGHTBACK][2]  = 0;
    vector_sub<3>(leg_buf[RIGHTBACK], pose_buf[RIGHTBACK], rb);
    legs[RIGHTBACK]->move(rb);
    timeNow.tv_nsec += 500000000;
    while (timeNow.tv_nsec >= 1000000000L) {
        timeNow.tv_nsec -= 1000000000L;
        timeNow.tv_sec++;}
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
    leg_buf[LEFTBACK][0] += 20;
    leg_buf[RIGHTBACK][0] += 20;
    leg_buf[RIGHTFRONT][0] += 20;
    leg_buf[LEFTFRONT][0] += 20;
    pose(0, 0, 0, 0, 0, 170);
    timeNow.tv_nsec += 500000000;
    while (timeNow.tv_nsec >= 1000000000L) {
        timeNow.tv_nsec -= 1000000000L;
        timeNow.tv_sec++;}
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
}
