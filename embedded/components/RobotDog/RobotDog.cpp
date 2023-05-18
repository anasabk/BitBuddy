#include "RobotDog.h"


BitBuddy::BitBuddy(int mpu_bus, int mpu_addr, int pca_bus, int pca_addr, int lcd_bus, int lcd_addr)
    : pca(pca_bus, pca_addr), lcd(lcd_bus, lcd_addr), mpu6050(mpu_bus, mpu_addr),
    servos{ 
		// Top, Mid, and Low motors for each leg
		{CalServo(&pca, 6), CalServo(&pca, 7), CalServo(&pca, 8)},		// Front Right 
		{CalServo(&pca, 9), CalServo(&pca, 10), CalServo(&pca, 11)},	// Front Left
		{CalServo(&pca, 3), CalServo(&pca, 4), CalServo(&pca, 5)},		// Back Right
		{CalServo(&pca, 0), CalServo(&pca, 1), CalServo(&pca, 2)}		// Back Left
    }
{
	// for(int i = 0; i < 4; i++)
	// 	for(int j = 0; j < 3; j++)
	// 		servos[i][j].refresh_fitter(cal_pwm_list, cal_degree_list[servos[i][j].getChannel()], 20);
    
}

BitBuddy::~BitBuddy()
{
}