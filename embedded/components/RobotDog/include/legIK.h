#ifndef LEGIK_H
#define LEGIK_H


#include "CalServo.h"
#include <semaphore.h>

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
        bool is_front,
        pthread_mutex_t *body_sem
    );

    ~Leg();

    void get_degree(const double (&dest)[3], int (&thetas)[3]);
    void get_degree(double x_mm, double y_mm, double z_mm, int *theta1, int *theta2, int *theta3);

    void get_degree_offset(const double (&offset)[3], int (&thetas)[3]);
    void get_degree_offset(double x_mm, double y_mm, double z_mm, int *theta1, int *theta2, int *theta3);

    void move(const double (&dest)[3], int speed_ms = 200);
    void move(double x_mm, double y_mm, double z_mm, int speed_ms = 200);

    void move_d(double x_mm, double y_mm, double z_mm, int speed_ms = 200);

    void move_offset(const double (&offset)[3], int speed_ms = 200);
    void move_offset(double x_mm, double y_mm, double z_mm, int speed_ms = 200);

    bool is_right();
    bool is_front();

private:
    CalServo *servos[3];
    pthread_t servo_thread_id;
    pthread_mutex_t *body_sem;
    int theta_buf[3];
    int speed_buf;
    int offsets[3];
    double last_pos[3];
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

    struct servo_param {
        const int *theta_buf;
        const int *speed_buf;
        CalServo **servos;
        pthread_mutex_t *body_sem;

        servo_param(
            const int *theta_buf,
            const int *speed_buf,
            CalServo **servos,
            pthread_mutex_t *body_sem) 
        {
            this->speed_buf = speed_buf;
            this->servos = servos;
            this->theta_buf = theta_buf;
            this->body_sem = body_sem;
        }
    };

    static void* servo_thread(void* param);

};


#endif