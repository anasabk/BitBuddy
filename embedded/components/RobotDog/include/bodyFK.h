#ifndef BODYFK_H
#define BODYFK_H

#include "legIK.h"

class Body {
public:
    enum LegSide {
        LEFTBACK = 0,
        RIGHTBACK = 1,
        RIGHTFRONT = 2,
        LEFTFRONT = 3
    };

    struct move_param {
        const float *speed;
        const float *yaw;
        const bool *run_flag;
        Body *body;
    };

    Body(
        CalServo (&servos)[12],
        double len_mm, 
        double width_mm
    );
    
    ~Body();

    void get_pose_offset(
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
    );

    void get_pose(
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
    );

    void pose_offset(
        double roll, 
        double yaw, 
        double pitch, 
        double x_mm, 
        double y_mm, 
        double z_mm
    );

    void pose(
        double roll, 
        double yaw, 
        double pitch, 
        double x_mm, 
        double y_mm, 
        double z_mm
    );

    void stand_up();

    void sit_down();

    void move_forward(double rot_rad, double dist, int step_num = 4);

    static void* move_thread(void* param);

    void recenter();

    void recover();


private:
    CalServo *servos[12];
    pthread_t servo_thread_id;
    LegIK legs[4];
    double len_mm;
    double width_mm;

    double last_roll;
    double last_yaw;
    double last_pitch;
    double last_x_mm;
    double last_y_mm;
    double last_z_mm;

    double leg_buf[4][3];
    double pose_buf[4][3];
    float servo_buf[12];

    void move(long dur);
};


#endif
