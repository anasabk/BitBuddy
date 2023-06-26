#include "bodyFK.h"
#include <cmath>


Body::Body(
    CalServo (&servos)[12],
    double len_mm, 
    double width_mm) 
    : legs {
        Leg(&servos[0], 4, &servos[1], 17, &servos[2], 1, 55, 110, 130, false, false),
        Leg(&servos[3],-10, &servos[4], -3, &servos[5], -7, 55, 110, 130,  true, false),
        Leg(&servos[6], 0, &servos[7], -2, &servos[8], -3, 55, 110, 130,  true, true),
        Leg(&servos[9], 8, &servos[10], 6, &servos[11],5, 55, 110, 130, false, true),
    }
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
        {0, 0, 0,    0}
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

    // printf("new values:\n");
    // printf("rb : %lf, %lf, %lf\n", rb[0], rb[1], rb[2]);
    // printf("rf : %lf, %lf, %lf\n", rf[0], rf[1], rf[2]);
    // printf("lb : %lf, %lf, %lf\n", lb[0], lb[1], lb[2]);
    // printf("lf : %lf, %lf, %lf\n", lf[0], lf[1], lf[2]);

    legs[RIGHTBACK].move(rb);
    legs[RIGHTFRONT].move(rf);
    legs[LEFTBACK].move(lb);
    legs[LEFTFRONT].move(lf);
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

void wait_real(struct timespec *timeNow, long ms) {
    timeNow->tv_nsec += ms * 1000000;
    while (timeNow->tv_nsec >= 1000000000L) {
        timeNow->tv_nsec -= 1000000000L;
        timeNow->tv_sec++;}
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, timeNow, nullptr);
}

void Body::move_forward(double rot_rad, double dist, int step_num) {
    if(dist/step_num > 40.0) dist = step_num*40.0;

    const double l_leen_off =  20.0;
    const double r_leen_off = -30.0;
    const double f_leen_off =  15.0;
    const double b_leen_off = -15.0;
    double drift_offset = 2;

    double turn_buf[4][3];
    double temp[4][4];
    get_pose(0, -rot_rad/(double)step_num, 0, 0, 0, 140, temp, turn_buf[RIGHTBACK], turn_buf[RIGHTFRONT], turn_buf[LEFTBACK], turn_buf[LEFTFRONT]);

    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);

    int steps_gone = 0;
    double temp_vector[3];
    for(int i = 3; steps_gone < step_num; i -= 2) {
        if(i < 0) i = -i + 1;

        // Leen to the right back
        pose(
            0, 
            0, 
            0, 
            legs[i].is_front() ? b_leen_off : f_leen_off, 
            legs[i].is_right() ? l_leen_off : r_leen_off, 
            140
        );
        wait_real(&timeNow, 250);

        // Position the leg
        leg_buf[i][2] = 50;
        vector_sub<3>(leg_buf[i], pose_buf[i], temp_vector);
        legs[i].move(temp_vector);
        wait_real(&timeNow, 250);

        leg_buf[i][0] = (legs[i].is_front() ? 15 :-50) + dist/step_num * 2 - turn_buf[i][0];
        leg_buf[i][1] = (legs[i].is_right() ?-55 : 55) - turn_buf[i][1];
        vector_sub<3>(leg_buf[i], pose_buf[i], temp_vector);
        legs[i].move(temp_vector);
        wait_real(&timeNow, 250);

        leg_buf[i][2] = 0;
        vector_sub<3>(leg_buf[i], pose_buf[i], temp_vector);
        legs[i].move(temp_vector);
        wait_real(&timeNow, 250);
        
        // Go forward
        leg_buf[LEFTBACK][0]   -= dist/step_num + drift_offset - turn_buf[LEFTBACK][0];
        leg_buf[RIGHTBACK][0]  -= dist/step_num - drift_offset - turn_buf[RIGHTBACK][0];
        leg_buf[RIGHTFRONT][0] -= dist/step_num - drift_offset - turn_buf[RIGHTFRONT][0];
        leg_buf[LEFTFRONT][0]  -= dist/step_num + drift_offset - turn_buf[LEFTFRONT][0];
        leg_buf[LEFTBACK][1]   += turn_buf[LEFTBACK][1];
        leg_buf[RIGHTBACK][1]  += turn_buf[RIGHTBACK][1];
        leg_buf[RIGHTFRONT][1] += turn_buf[RIGHTFRONT][1];
        leg_buf[LEFTFRONT][1]  += turn_buf[LEFTFRONT][1];
        pose(
            0, 
            0, 
            0, 
            legs[i].is_front() ? b_leen_off : f_leen_off, 
            legs[i].is_right() ? l_leen_off : r_leen_off, 
            140
        );
        wait_real(&timeNow, 250);

        steps_gone++;
    }
}

void* Body::move_thread(void *param) {
    const float *rot_rad = ((Body::move_param*)param)->rot;
    const float *speed = ((Body::move_param*)param)->speed;
    const bool *run_flag = ((Body::move_param*)param)->run_flag;
    Body* body = ((Body::move_param*)param)->body;

    const double l_leen_off =  20.0;
    const double r_leen_off = -30.0;
    const double f_leen_off =  15.0;
    const double b_leen_off = -15.0;
    double drift_offset = 2;

    double new_pose_buf[4][3];
    double temp[4][4];

    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);

    double temp_vector[3];
    int leg_num = 0;
    int pause_counter = 0;
    while(*run_flag) {
        if(fabsl(*speed) + fabsl(*rot_rad) < 0.0001)
            pause_counter++;
        else
            pause_counter = 0;

        if(pause_counter == 2){
            wait_real(&timeNow, 100);
            continue;
        }

        if(leg_num == -1) leg_num = 0;

        // Get the new pose of the legs
        body->get_pose(
            0, -*rot_rad, 0, 
            -*speed, 0, 140, 
            temp, 
            new_pose_buf[RIGHTBACK], 
            new_pose_buf[RIGHTFRONT], 
            new_pose_buf[LEFTBACK], 
            new_pose_buf[LEFTFRONT]
        );


        // Leen to the opposite side
        body->pose(0, 0, 0, 0, body->legs[leg_num].is_right() ? l_leen_off : r_leen_off, 140);
        wait_real(&timeNow, 250);


        // Position the leg
        body->leg_buf[leg_num][2] = 50;
        vector_sub<3>(body->leg_buf[leg_num], body->pose_buf[leg_num], temp_vector);
        body->legs[leg_num].move(temp_vector, 150);
        wait_real(&timeNow, 200);

        body->leg_buf[leg_num][0] = (body->legs[leg_num].is_front() ? 15 :-50) - new_pose_buf[leg_num][0];
        body->leg_buf[leg_num][1] = (body->legs[leg_num].is_right() ?-55 : 55) - new_pose_buf[leg_num][1];
        vector_sub<3>(body->leg_buf[leg_num], body->pose_buf[leg_num], temp_vector);
        body->legs[leg_num].move(temp_vector, 150);
        wait_real(&timeNow, 200);

        body->leg_buf[leg_num][2] = 0;
        vector_sub<3>(body->leg_buf[leg_num], body->pose_buf[leg_num], temp_vector);
        body->legs[leg_num].move(temp_vector, 150);
        wait_real(&timeNow, 150);


        leg_num = -(leg_num - 3);

        // Position the leg
        body->leg_buf[leg_num][2] = 50;
        vector_sub<3>(body->leg_buf[leg_num], body->pose_buf[leg_num], temp_vector);
        body->legs[leg_num].move(temp_vector, 150);
        wait_real(&timeNow, 200);

        body->leg_buf[leg_num][0] = (body->legs[leg_num].is_front() ? 15 :-50) - new_pose_buf[leg_num][0];
        body->leg_buf[leg_num][1] = (body->legs[leg_num].is_right() ?-55 : 55) - new_pose_buf[leg_num][1];
        vector_sub<3>(body->leg_buf[leg_num], body->pose_buf[leg_num], temp_vector);
        body->legs[leg_num].move(temp_vector, 150);
        wait_real(&timeNow, 200);

        body->leg_buf[leg_num][2] = 0;
        vector_sub<3>(body->leg_buf[leg_num], body->pose_buf[leg_num], temp_vector);
        body->legs[leg_num].move(temp_vector, 150);
        wait_real(&timeNow, 200);


        // Go forward
        body->leg_buf[LEFTBACK][0]   -= + drift_offset - new_pose_buf[LEFTBACK][0];
        body->leg_buf[RIGHTBACK][0]  -= - drift_offset - new_pose_buf[RIGHTBACK][0];
        body->leg_buf[RIGHTFRONT][0] -= - drift_offset - new_pose_buf[RIGHTFRONT][0];
        body->leg_buf[LEFTFRONT][0]  -= + drift_offset - new_pose_buf[LEFTFRONT][0];
        body->leg_buf[LEFTBACK][1]   += new_pose_buf[LEFTBACK][1];
        body->leg_buf[RIGHTBACK][1]  += new_pose_buf[RIGHTBACK][1];
        body->leg_buf[RIGHTFRONT][1] += new_pose_buf[RIGHTFRONT][1];
        body->leg_buf[LEFTFRONT][1]  += new_pose_buf[LEFTFRONT][1];


        // Go to the next leg pair
        leg_num -= 2;
    }

    pthread_exit(NULL);
}

void Body::recover() {
    struct timespec time;

    clock_gettime(CLOCK_MONOTONIC, &time);
    sit_down();
    wait_real(&time, 999);

    clock_gettime(CLOCK_MONOTONIC, &time);
    legs[LEFTBACK].move_d(90, 180, 0, 700);
    legs[RIGHTBACK].move_d(90, 0, 180, 700);
    legs[RIGHTFRONT].move_d(90, 0, 180, 700);
    legs[LEFTFRONT].move_d(90, 180, 0, 700);
    wait_real(&time, 999);

    clock_gettime(CLOCK_MONOTONIC, &time);
    legs[LEFTBACK].move_d(150, 180, 0, 700);
    legs[LEFTFRONT].move_d(30, 180, 0, 700);
    wait_real(&time, 999);

    clock_gettime(CLOCK_MONOTONIC, &time);
    legs[LEFTBACK].move_d(30, 180, 0, 700);
    legs[LEFTFRONT].move_d(150, 180, 0, 700);
    wait_real(&time, 999);

    clock_gettime(CLOCK_MONOTONIC, &time);
    sit_down();
    wait_real(&time, 999);
}
