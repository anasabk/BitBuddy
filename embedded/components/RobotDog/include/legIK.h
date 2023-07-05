#ifndef LEGIK_H
#define LEGIK_H


#include "CalServo.h"
#include <semaphore.h>

class LegIK {
public:
    LegIK(
        double off_hip,
        double off_shld, 
        double off_knee, 
        double hip_l, 
        double l1, 
        double l2, 
        bool is_right, 
        bool is_front
    );

    ~LegIK();

    void get_degrees(const double *dest, int *thetas);
    void get_degrees(double x_mm, double y_mm, double z_mm, int *theta1, int *theta2, int *theta3);

    bool is_right();
    bool is_front();

private:
    int offsets[3];
    double hip_l, l1, l2;

    /**
     * @note Right == true
     * @note Left == false
     */
    bool is_right_flag;

    /**
     * @note Right == true
     * @note Left == false
     */
    bool is_front_flag;
};


#endif