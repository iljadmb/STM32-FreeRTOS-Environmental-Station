/*
 * imu.c
 *
 *  Created on: 06.07.2026
 *      Author: ilja
 */

#include "imu.h"

static const uint8_t MPU_gyro_acc_ADDR = 0x68 << 1; //Adresse für Gyro+Acc.
//static const uint8_t MPU_magnet_ADDR = 0x0C << 1; //Adresse für Magnet
static const uint8_t MPU_gyro_acc_WHO_AM_I_ADD = 0x75;//Test ob MPU an ist
static const uint8_t PWR_MGMT_1_ADDR = 0x6B; //SleepBit löschen
static const uint8_t AccX_ADDR = 0x3B; //erste RegisterAdresse fürs Auslesen


void MPU9250_Init(I2C_HandleTypeDef *hi2c){
	uint8_t wake_up[2] = {PWR_MGMT_1_ADDR, 0x00};
	HAL_I2C_Master_Transmit(hi2c, MPU_gyro_acc_ADDR, wake_up, 2, HAL_MAX_DELAY);
	HAL_Delay(100);
}

uint8_t MPU9250_WhoAmI(I2C_HandleTypeDef *hi2c){
    uint8_t test_if_connected = 0;

    HAL_I2C_Master_Transmit(hi2c, MPU_gyro_acc_ADDR,(uint8_t*)&MPU_gyro_acc_WHO_AM_I_ADD, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(hi2c, MPU_gyro_acc_ADDR, &test_if_connected, 1, HAL_MAX_DELAY);
    return test_if_connected;
}

void MPU9250_Read(I2C_HandleTypeDef *hi2c, MPU9250_Data *data){
	uint8_t sensor_buffer[14];
	HAL_I2C_Master_Transmit(hi2c, MPU_gyro_acc_ADDR, (uint8_t*)&AccX_ADDR, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(hi2c, MPU_gyro_acc_ADDR, sensor_buffer, 14, HAL_MAX_DELAY);

	    data->accX  = (int16_t)(sensor_buffer[0]  << 8 | sensor_buffer[1]);
	    data->accY  = (int16_t)(sensor_buffer[2]  << 8 | sensor_buffer[3]);
	    data->accZ  = (int16_t)(sensor_buffer[4]  << 8 | sensor_buffer[5]);
	    data->temp  = (int16_t)(sensor_buffer[6]  << 8 | sensor_buffer[7]);
	    data->gyroX = (int16_t)(sensor_buffer[8]  << 8 | sensor_buffer[9]);
	    data->gyroY = (int16_t)(sensor_buffer[10] << 8 | sensor_buffer[11]);
	    data->gyroZ = (int16_t)(sensor_buffer[12] << 8 | sensor_buffer[13]);
}
