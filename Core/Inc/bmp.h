/*
 * bmp.h
 *
 *  Created on: 06.07.2026
 *      Author: ilja
 */

#ifndef INC_BMP_H_
#define INC_BMP_H_

#include "main.h"

typedef struct {
    float temp_c;
    float pressure_hpa;
} BMP280_Data;

uint8_t BMP_WhoAmI(I2C_HandleTypeDef *hi2c);
void BMP_Init(I2C_HandleTypeDef *hi2c);
void BMP_Read(I2C_HandleTypeDef *hi2c, BMP280_Data *data);


#endif /* INC_BMP_H_ */
