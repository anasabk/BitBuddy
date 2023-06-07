#ifndef LEGIK_H
#define LEGIK_H


#include "CalServo.h"


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

    void get_degree(const double (&dest)[3], int (&thetas)[3]);
    void get_degree(double x_mm, double y_mm, double z_mm, int *theta1, int *theta2, int *theta3);

    void get_degree_offset(const double (&offset)[3], int (&thetas)[3]);
    void get_degree_offset(double x_mm, double y_mm, double z_mm, int *theta1, int *theta2, int *theta3);

    void move(const double (&dest)[3]);
    void move(double x_mm, double y_mm, double z_mm);

    void move_offset(const double (&offset)[3]);
    void move_offset(double x_mm, double y_mm, double z_mm);

private:
    CalServo *servos[3];
    int theta_buf[3];
    pthread_t servo_thread_id[3];
    int offsets[3];
    double hip_l, l1, l2;
    double last_pos[3];
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
        const int* theta_buf;
        CalServo *servo;

        servo_param(
            const int* theta_buf,
            CalServo *servo) 
        {
            this->servo = servo;
            this->theta_buf = theta_buf;
        }
    };

    static void* servo_thread(void* param);

};


#endif