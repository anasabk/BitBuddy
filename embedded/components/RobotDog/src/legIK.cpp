#include "legIK.h"
#include "common.h"
#include <pthread.h>
#include <signal.h>


LegIK::LegIK(
    double off_hip,
    double off_shld, 
    double off_knee, 
    double hip_l, 
    double l1, 
    double l2, 
    bool is_right_flag,
    bool is_front_flag)
{
    offsets[0] = off_hip;
    offsets[1] = off_shld;
    offsets[2] = off_knee;
    
    this->hip_l = hip_l;
    this->l1 = l1;
    this->l2 = l2;

    this->is_right_flag = is_right_flag;
    this->is_front_flag = is_front_flag;
}

LegIK::~LegIK() {
    
}

void LegIK::get_degrees(double x_mm, double y_mm, double z_mm, int *theta1, int *theta2, int *theta3) {
    double foot_to_shoulder_sq = pow(x_mm, 2) + pow(y_mm, 2) + pow(z_mm, 2) - pow(hip_l, 2);
    
    if (foot_to_shoulder_sq < (l1*l1 + l2*l2 - 2*l1*l2*cos(35/180*M_PI)) || 
        foot_to_shoulder_sq > pow(l1+l2, 2)) {
        printf("Length of %lfmm from foot to shoulder is not possible, aborting ...\n", foot_to_shoulder_sq);
        return;
    }
    
    double R2_yz = pow(y_mm, 2) + pow(z_mm, 2);
    double temp_theta = acos((l2*l2 - l1*l1 - foot_to_shoulder_sq) / (-2 * l1 * sqrt(foot_to_shoulder_sq)));
    double hip_dir = is_right_flag ? -1 : 1;

    double degrees[3];
    degrees[0] = (acos(hip_dir*hip_l / sqrt(R2_yz)) + atan(y_mm / fabs(z_mm)))*180/M_PI;
    degrees[1] = (temp_theta - atan(x_mm / sqrt(R2_yz - hip_l*hip_l)))*180/M_PI;
    degrees[2] = acos((foot_to_shoulder_sq - l2*l2 - l1*l1) / (-2 * l1 * l2))*180/M_PI - 35;

    if (degrees[0] > 180 || degrees[0] < 0 ||
        degrees[1] > 180 || degrees[1] < 0 ||
        degrees[2] > 180 || degrees[2] < 0) {
        printf("Out of reach, aborting ...\n");
    }

    if(is_right_flag) {
        degrees[1] = 180 - degrees[1];
        degrees[2] = 180 - degrees[2];
    }

    if(is_front_flag)
        degrees[0] = 180 - degrees[0];

    *theta1 = degrees[0] + offsets[0];
    *theta2 = degrees[1] + offsets[1];
    *theta3 = degrees[2] + offsets[2];
}

void LegIK::get_degrees(const double *dest, int *thetas) {
    get_degrees(dest[0], dest[1], dest[2], &thetas[0], &thetas[1], &thetas[2]);
}

bool LegIK::is_right() {
    return is_right_flag;
}

bool LegIK::is_front() {
    return is_front_flag;
}
