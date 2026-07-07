/*
 * imu.h
 *
 *  Created on: 06.07.2026
 *      Author: ilja
 */

#ifndef INC_IMU_H_
#define INC_IMU_H_

#include "main.h"

typedef struct {
    int16_t accX, accY, accZ;
    int16_t temp;
    int16_t gyroX, gyroY, gyroZ;
} MPU9250_Data;

void MPU9250_Init(I2C_HandleTypeDef *hi2c);
uint8_t MPU9250_WhoAmI(I2C_HandleTypeDef *hi2c);
void MPU9250_Read(I2C_HandleTypeDef *hi2c, MPU9250_Data *data);

#endif /* INC_IMU_H_ */
