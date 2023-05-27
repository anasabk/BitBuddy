#ifndef INVERSE_KINEMATICS_H
#define INVERSE_KINEMATICS_H


#include "CalServo.h"


int stand[] = {
    90, 60, 60,
    80, 125, 110,
    90, 150, 95,
    80, 40, 85
};

int sit[] = {
    90, 120, 0,
    80, 70, 180,
    90, 110, 180,
    80, 80, 0
};

int def[] = {
    90, 90, 0,
    90, 90, 180,
    90, 90, 180,
    90, 90, 0
};

int step_offset[][12] = {
    // {   // bring left front leg upward and forward
    //     90, 60, 60,
    //     80, 125, 110,
    //     90, 150, 95,
    //     80, 0, 50
    // },
    {   // bring left front leg upward and forward
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 40, -35
    },
    // {   // move body forward and bring front left leg to the ground
    //     90, 60, 90,
    //     80, 120, 90,
    //     90, 135, 95,
    //     80, 0, 115
    // },
    {   // move body forward and bring front left leg to the ground
        0,  5,   20,
        0, -5,  -20,
        0, -15,  0,
        0, 0,  0
    },
    // {   // move body forward and bring front left leg to the ground
    //     90, 60, 90,
    //     80, 120, 90,
    //     90, 135, 95,
    //     80, 0, 115
    // },
    {   // move body forward and bring front left leg to the ground
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, -40, 35
    },
    // {   // bring right back leg upward and forward
    //     90, 60, 90,
    //     80, 150, 130,
    //     90, 150, 95,
    //     80, 0, 115
    // },
    {   // bring right back leg upward and forward
        0,  0,   0,
        0,  30,  40,
        0,  0,   0,
        0,  0,   0
    },
    // {   // move body forward and bring right back leg to the ground
    //     90, 60, 90,
    //     80, 133, 50,
    //     90, 150, 95,
    //     80, 0, 115
    // },
    {   // move body forward and bring right back leg to the ground
        0, -13,  40,
        0, -30, -40,
        0, -5,  -10,
        0,  15,  0
    },
    /*  back
        4   5
        125 110
        120 90  -5  -20
        133 50  +13 -40
    */
    /*  front
        7   8
        150 95  
        135 95  -15 0
        130 85  -5  -10
    */
};


class Leg
{
private:
    CalServo *servos[3];
    int hip_l, l1, l2;
    int last_pos[3];

    /**
     * @note Right == true
     * @note Left == false
     */
    bool side_is_right;
    
public:
    Leg(CalServo *hip, CalServo *shoulder, CalServo *knee, int hip_l, int l1, int l2, bool side);
    ~Leg();

    bool move(int x_mm, int y_mm, int z_mm);

    bool move_offset(int x_mm, int y_mm, int z_mm);
};

Leg::Leg(
    CalServo *hip, 
    CalServo *shoulder, 
    CalServo *knee, 
    int hip_l, 
    int l2, 
    int l1, 
    bool side)
{
    servos[0] = hip;
    servos[1] = shoulder;
    servos[2] = knee;
    
    this->hip_l = hip_l;
    this->l1 = l1;
    this->l2 = l2;

    side_is_right = side;
}

Leg::~Leg()
{
}

bool Leg::move(int x_mm, int y_mm, int z_mm) {
    int R2_yz = y_mm*y_mm + z_mm*z_mm;
    int R2_xyz = x_mm*x_mm + y_mm*y_mm + z_mm*z_mm;

    int degrees[3];
    degrees[0] = acos(hip_l/sqrt(R2_yz))*180/M_PI - atan(z_mm / y_mm) + 90;
    degrees[1] = (acos((l2*l2 - l1*l1 - R2_xyz) / (2 * l1 * sqrt(R2_xyz))) - atan(x_mm / sqrt(R2_yz))) * 180 / M_PI;
    degrees[2] = acos((R2_xyz - l2*l2 - l1*l1) / (2 * l1 * l2)) * 180 / M_PI - 35;

    if(side_is_right) {
        degrees[0] = 180 - degrees[0];
        degrees[1] = 180 - degrees[1];
        degrees[2] = 180 - degrees[2];
    }

    printf("%d %d %d\n", degrees[0], degrees[1], degrees[2]);

    for(int i = 0; i < 3; i++) {
        servos[i]->set_degree(degrees[i]);
    }

    last_pos[0] = x_mm;
    last_pos[1] = y_mm;
    last_pos[2] = z_mm;

    return true;
}

bool Leg::move_offset(int x_mm, int y_mm, int z_mm) {
    if(last_pos[0] == -1 || last_pos[0] == -1 || last_pos[0] == -1)
        return false;

    int R2_yz = pow(last_pos[1] + y_mm, 2) + pow(last_pos[2] + z_mm, 2);
    int R2_xyz = pow(last_pos[0] + x_mm, 2) + pow(last_pos[1] + y_mm, 2) + pow(last_pos[2] + z_mm, 2);

    int degrees[3];
    degrees[0] = acos(hip_l/sqrt(R2_yz))*180/M_PI - atan(z_mm / y_mm) + 90;
    degrees[1] = (acos((l2*l2 - l1*l1 - R2_xyz) / (2 * l1 * sqrt(R2_xyz))) - atan((last_pos[0] + x_mm) / sqrt(R2_yz))) * 180 / M_PI;
    degrees[2] = acos((R2_xyz - l2*l2 - l1*l1) / (2 * l1 * l2)) * 180 / M_PI - 35;

    if(side_is_right) {
        degrees[0] = 180 - degrees[0];
        degrees[1] = 180 - degrees[1];
        degrees[2] = 180 - degrees[2];
    }

    for(int i = 0; i < 3; i++) {
        servos[i]->sweep(degrees[i], 3000);
    }

    last_pos[0] += x_mm;
    last_pos[1] += y_mm;
    last_pos[2] += z_mm;

    return true;
}


#endif
