/*
 * sh1106.c
 *
 *  Created on: 06.07.2026
 *      Author: ilja
 */
#include "sh1106.h"
#include "fonts.h"
#include <string.h>
#include <stdlib.h>

static const uint8_t I2C_Addr = 0x3C << 1;
static const uint8_t ctrlb_cmd = 0x00;
static const uint8_t ctrlb_data = 0x40;
static uint8_t buffer[8][128];

void SH1106_Init(I2C_HandleTypeDef *hi2c){
	uint8_t cmd_buffer[] = {0x00 ,0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3,
							0x00, 0x40, 0xAD, 0x8B, 0xA1, 0xC8, 0xDA,
							0x12, 0x81, 0x80, 0xD9, 0x22, 0xDB, 0x35,
							0xA4, 0xA6, 0xAF};

	HAL_I2C_Master_Transmit(hi2c, I2C_Addr, cmd_buffer, sizeof(cmd_buffer), HAL_MAX_DELAY);
}


void SH1106_Clear(void){
    memset(buffer, 0x00, sizeof(buffer));
}


void SH1106_SetPixel(int x, int y){
	if (x < 0 || x > 127 || y < 0 || y > 63) return;
	buffer[y / 8][x] |= (1 << (y%8));

}

static void SH1106_DrawChar(int x, int y, char c){ //static, da nur intern gebraucht
	const uint8_t* bytes = font[c -32];
	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 8; j++){
			if ((bytes[i] >> j) & 1){ // bytes[i] >> j: tut das byte um j nach rechts schieben -> dann verunden mit 1
				SH1106_SetPixel(x + i, y + j);
			}
		}
	}
}

void SH1106_DrawString(int x, int y, char* c){
	for(int i = 0; c[i] != '\0'; i++){
		SH1106_DrawChar(x, y, c[i]);
		x = x +6;
	}
}



void SH1106_Flush(I2C_HandleTypeDef *hi2c){

	for(uint8_t page = 0; page < 8; page++){
	uint8_t send_data[sizeof(buffer[page]) +1];
	uint8_t send_config[] = {ctrlb_cmd, 0xB0 + page, 0x02, 0x10};

	send_data[0] = ctrlb_data;

	for(int i = 1; i < 129; i++){
		send_data[i] = buffer[page][i -1];
	}


	HAL_I2C_Master_Transmit(hi2c, I2C_Addr, send_config, sizeof(send_config), HAL_MAX_DELAY);
	HAL_I2C_Master_Transmit(hi2c, I2C_Addr, send_data, sizeof(send_data), HAL_MAX_DELAY);
	}
}

void SH1106_DrawLine(int x0, int y0, int x1, int y1){
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (1) {
        SH1106_SetPixel(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}



