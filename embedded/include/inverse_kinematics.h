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


#endif
