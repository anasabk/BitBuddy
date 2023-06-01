#include "inv_kinematics.h"


Leg::Leg(
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
    bool is_front)
{
    servos[0] = hip;
    servos[1] = shoulder;
    servos[2] = knee;

    offsets[0] = off_hip;
    offsets[1] = off_shld;
    offsets[2] = off_knee;
    
    this->hip_l = hip_l;
    this->l1 = l1;
    this->l2 = l2;

    this->is_right = is_right;
    this->is_front = is_front;
}

Leg::~Leg()
{
}

bool Leg::move(double x_mm, double y_mm, double z_mm, double *x_deg, double *y_deg, double *z_deg) {
    double R2_yz = pow(y_mm, 2) + pow(z_mm, 2);
    double foot_to_shoulder_sq = pow(x_mm, 2) + pow(y_mm, 2) + pow(z_mm, 2) - pow(hip_l, 2);
    double temp_theta = acos((l2*l2 - l1*l1 - foot_to_shoulder_sq) / (-2 * l1 * sqrt(foot_to_shoulder_sq)));

    double degrees[3];
    degrees[0] = (acos(hip_l / sqrt(R2_yz)) + atan(y_mm / z_mm))*180/M_PI + offsets[0];
    degrees[1] = (temp_theta - atan(x_mm / sqrt(R2_yz - hip_l*hip_l)))*180/M_PI + offsets[1];
    degrees[2] = acos((foot_to_shoulder_sq - l2*l2 - l1*l1) / (-2 * l1 * l2))*180/M_PI - 35 + offsets[2];

    if(is_right) {
        degrees[1] = 180 - degrees[1];
        degrees[2] = 180 - degrees[2];
    }

    if(is_right != is_front) {
        degrees[0] = 180 - degrees[0];
    }

    printf("%lf %lf %lf, %lf %lf %lf\n", x_mm, y_mm, z_mm, degrees[0], degrees[1], degrees[2]);

    for(int i = 0; i < 3; i++)
        servos[i]->set_degree((int)degrees[i]);

    // *x_deg = degrees[0];
    // *y_deg = degrees[1];
    // *z_deg = degrees[2];

    last_pos[0] = x_mm;
    last_pos[1] = y_mm;
    last_pos[2] = z_mm;

    return true;
}

bool Leg::move_offset(double x_mm, double y_mm, double z_mm, double *x_deg, double *y_deg, double *z_deg) {
    if(last_pos[0] == -1 || last_pos[0] == -1 || last_pos[0] == -1)
        return false;

    move(last_pos[0] + x_mm, last_pos[1] + y_mm, last_pos[2] + z_mm, x_deg, y_deg, z_deg);

    return true;
}
