#ifndef INVERSE_KINEMATICS_H
#define INVERSE_KINEMATICS_H


#include "CalServo.h"


class Leg {
private:
    CalServo *servos[3];
    double offsets[3];
    double hip_l, l1, l2;
    int last_pos[3];

    /**
     * @note Right == true
     * @note Left == false
     */
    bool is_right;

    /**
     * @note Right == true
     * @note Left == false
     */
    bool is_front;

    
public:
    Leg(
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
        bool is_front
    );

    ~Leg();

    bool move(double x_mm, double y_mm, double z_mm, double *x_deg, double *y_deg, double *z_deg);

    bool move_offset(double x_mm, double y_mm, double z_mm, double *x_deg, double *y_deg, double *z_deg);
};


#endif