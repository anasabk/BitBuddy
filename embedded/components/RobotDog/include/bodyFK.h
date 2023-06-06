#ifndef LEGIK_H
#define LEGIK_H

#include "legIK.h"

class Body {
public:
    enum LegSide {
        RIGHTBACK = 1,
        RIGHTFRONT = 2,
        LEFTBACK = 0,
        LEFTFRONT = 3
    };

    Body(
        Leg* left_front, 
        Leg* right_front, 
        Leg* left_back, 
        Leg* right_back, 
        int len_mm, 
        int width_mm
    );
    
    ~Body();

    void get_pose_offset(
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
    );

    void get_pose(
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


private:
    Leg *legs[4];
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
};


#endif
