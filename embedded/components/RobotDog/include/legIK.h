#ifndef INVERSE_KINEMATICS_H
#define INVERSE_KINEMATICS_H


#include "CalServo.h"
#include "bodyFK.h"


class Leg {
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

    bool get_degree(const double (&dest)[3], double (&thetas)[3]);
    bool get_degree(double x_mm, double y_mm, double z_mm, double *theta1, double *theta2, double *theta3);

    bool get_degree_offset(const double (&offset)[3], double (&thetas)[3]);
    bool get_degree_offset(double x_mm, double y_mm, double z_mm, double *theta1, double *theta2, double *theta3);

    bool move(const double (&dest)[3]);
    bool move(double x_mm, double y_mm, double z_mm);

    bool move_offset(const double (&offset)[3]);
    bool move_offset(double x_mm, double y_mm, double z_mm);

private:
    CalServo *servos[3];
    double theta_buf[3];
    pthread_t servo_thread_id[3];
    double offsets[3];
    double hip_l, l1, l2;
    int last_pos[3];
    bool running;

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

    struct servo_param {
        const double* theta_buf;
        CalServo *servo;

        servo_param(
            const double* theta_buf,
            CalServo *servo) 
        {
            this->servo = servo;
            this->theta_buf = theta_buf;
        }
    };

    static void* servo_thread(void* param);

};


#endif