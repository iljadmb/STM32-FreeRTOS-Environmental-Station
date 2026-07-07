/*
 * sh1106.h
 *
 *  Created on: 06.07.2026
 *      Author: ilja
 */

#ifndef INC_SH1106_H_
#define INC_SH1106_H_

#include "main.h"

void SH1106_Init(I2C_HandleTypeDef *hi2c);
void SH1106_Clear(void);
void SH1106_SetPixel(int x, int y);
//void SH1106_DrawChar(int x, int y, char c); -> nur intern benötigt
void SH1106_DrawString(int x, int y, char *str);
void SH1106_Flush(I2C_HandleTypeDef *hi2c);
void SH1106_DrawLine(int x0, int y0, int x1, int y1);


#endif /* INC_SH1106_H_ */
