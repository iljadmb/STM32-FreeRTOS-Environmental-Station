/*
 * bmp.c
 *
 *  Created on: 06.07.2026
 *      Author: ilja
 */
#include "bmp.h"

static const uint8_t bmp_ADDR = 0x76 << 1; //I2C-Adresse vom Sensor
static const uint8_t id_ADDR = 0xD0;
//static const uint8_t reset_ADDR = 0xE0; //Adresse für reset register
//static const uint8_t reset_cmd = 0xB6; //Adresse für reset selbst
static const uint8_t ctrl_measure = 0xF4;
static const uint8_t ctrl_measure_setting = 0x25; //Bits 7:5 = osrs_t = x1 = 001; Bits 4:2 = osrs_p = x1 = 001; Bits 1:0 = mode = forced = 01
static const uint8_t config = 0xF5;
static const uint8_t config_setting = 0x00;
//static const uint8_t status_reg = 0xF3;
static struct{
	uint16_t dig_T1;
	int16_t dig_T2, dig_T3;
	uint16_t dig_P1;
	int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
}Calibration;

static int32_t t_fine;

static int32_t  BMP280_compensate_T(int32_t adc_T);
static uint32_t BMP280_compensate_P(int32_t adc_P);


uint8_t BMP_WhoAmI(I2C_HandleTypeDef *hi2c){
	uint8_t who_am_I = 0;
	HAL_I2C_Master_Transmit(hi2c, bmp_ADDR, (uint8_t*)&id_ADDR, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(hi2c, bmp_ADDR, &who_am_I, 1, HAL_MAX_DELAY);
	if (who_am_I != 0x58){
	Error_Handler();
	}
	return who_am_I;
}

static void BMP_Calibrate(I2C_HandleTypeDef *hi2c){
	uint8_t cali_buffer[24];

	HAL_I2C_Mem_Read(hi2c, bmp_ADDR, 0x88, I2C_MEMADD_SIZE_8BIT, cali_buffer, 24, HAL_MAX_DELAY);

	Calibration.dig_T1 = (uint16_t)(cali_buffer[1] << 8 | cali_buffer[0]);
	Calibration.dig_T2 = (int16_t)(cali_buffer[3] << 8 | cali_buffer[2]);
	Calibration.dig_T3 = (int16_t)(cali_buffer[5] << 8 | cali_buffer[4]);
	Calibration.dig_P1 = (uint16_t)(cali_buffer[7] << 8 | cali_buffer[6]);
	Calibration.dig_P2 = (int16_t)(cali_buffer[9] << 8 | cali_buffer[8]);
	Calibration.dig_P3 = (int16_t)(cali_buffer[11] << 8 | cali_buffer[10]);
	Calibration.dig_P4 = (int16_t)(cali_buffer[13] << 8 | cali_buffer[12]);
	Calibration.dig_P5 = (int16_t)(cali_buffer[15] << 8 | cali_buffer[14]);
	Calibration.dig_P6 = (int16_t)(cali_buffer[17] << 8 | cali_buffer[16]);
	Calibration.dig_P7 = (int16_t)(cali_buffer[19] << 8 | cali_buffer[18]);
	Calibration.dig_P8 = (int16_t)(cali_buffer[21] << 8 | cali_buffer[20]);
	Calibration.dig_P9 = (int16_t)(cali_buffer[23] << 8 | cali_buffer[22]);
}

void BMP_Init(I2C_HandleTypeDef *hi2c){
	BMP_Calibrate(hi2c);
	//Einstellungen für Sensor:
	uint8_t arr1[] = {ctrl_measure, ctrl_measure_setting};
	HAL_I2C_Master_Transmit(hi2c, bmp_ADDR, arr1, 2, HAL_MAX_DELAY);
	uint8_t arr2[] = {config, config_setting};
	HAL_I2C_Master_Transmit(hi2c, bmp_ADDR, arr2, 2, HAL_MAX_DELAY);
}


void BMP_Read(I2C_HandleTypeDef *hi2c, BMP280_Data *data){
	uint8_t arr1[] = {ctrl_measure, ctrl_measure_setting};
	HAL_I2C_Master_Transmit(hi2c, bmp_ADDR, arr1, 2, HAL_MAX_DELAY);

	HAL_Delay(10);

	uint8_t arr_data1[3]; //pressure
	uint8_t arr_data2[3]; //temp

	HAL_I2C_Mem_Read(hi2c, bmp_ADDR, 0xF7, I2C_MEMADD_SIZE_8BIT, arr_data1, 3, HAL_MAX_DELAY);

	HAL_I2C_Mem_Read(hi2c, bmp_ADDR, 0xFA, I2C_MEMADD_SIZE_8BIT, arr_data2, 3, HAL_MAX_DELAY);

	int32_t raw_press = ((int32_t)arr_data1[0] << 12) | ((int32_t)arr_data1[1] << 4) | (arr_data1[2] >> 4);  // Druck aus arr_data1 (0xF7)
	int32_t raw_temp = ((int32_t)arr_data2[0] << 12) | ((int32_t)arr_data2[1] << 4) | (arr_data2[2] >> 4);  // Temp aus arr_data2 (0xFA)


	data->temp_c       = BMP280_compensate_T(raw_temp) / 100.0f;    // -> °C
	data->pressure_hpa = BMP280_compensate_P(raw_press) / 25600.0f; // -> hPa

}

// Rückgabe: Temperatur in 0.01 °C  (5123 = 51.23 °C)
static int32_t BMP280_compensate_T(int32_t adc_T)
{
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)Calibration.dig_T1 << 1))) * ((int32_t)Calibration.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)Calibration.dig_T1)) * ((adc_T >> 4) - ((int32_t)Calibration.dig_T1))) >> 12) * ((int32_t)Calibration.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

// Rückgabe: Druck in Q24.8-Format (Pa * 256).  24674867 = 96386.2 Pa
static uint32_t BMP280_compensate_P(int32_t adc_P)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)Calibration.dig_P6;
    var2 = var2 + ((var1 * (int64_t)Calibration.dig_P5) << 17);
    var2 = var2 + (((int64_t)Calibration.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)Calibration.dig_P3) >> 8) + ((var1 * (int64_t)Calibration.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)Calibration.dig_P1) >> 33;
    if (var1 == 0) return 0;   // Division durch 0 vermeiden
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)Calibration.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)Calibration.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)Calibration.dig_P7) << 4);
    return (uint32_t)p;
}









