#include "tft.h"

#ifndef __SPI_H_
#define __SPI_H_

extern void SPI_init(void);

extern void SPISend8Bit(unsigned char data);

extern void TFT_Print(char* Text, U8 X, U8 Y, U8 Scale, U16 ForeColor, U16 BackColor, U8 Display_Orientation);

extern void SendCommandSeq(const U16 * data, U16 count);

#endif
