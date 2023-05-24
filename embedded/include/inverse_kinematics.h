#ifndef INVERSE_KINEMATICS_H
#define INVERSE_KINEMATICS_H


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

int step[][12] = {
    {   // bring front left leg upward and forward
        90, 60, 60,
        80, 125, 110,
        90, 150, 95,
        80, 0, 50
    },
    {   // move body forward and bring front left leg to the ground
        90, 60, 90,
        80, 125, 80,
        90, 150, 95,
        80, 0, 115
    },
    {   // move body forward and bring front left leg to the ground
        90, 60, 90,
        80, 150, 130,
        90, 150, 95,
        80, 0, 115
    },
    {   // move body forward and bring front left leg to the ground
        90, 60, 90,
        80, 150, 130,
        90, 150, 95,
        80, 0, 115
    },
};


#endif
