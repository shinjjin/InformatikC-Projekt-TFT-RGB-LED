#ifndef __TFT_H_
#define __TFT_H_

//TFT-Display-Signale
#define SPI_DDR DDRB
#define CS      PINB2
#define MOSI    PINB3
#define SCK     PINB5
#define D_C		PIND2		//display: Data/Command
#define Reset	PIND3		//display: Reset

//Datentyp-Definitionen
#define U8  uint8_t
#define U16	uint16_t
#define U32	uint32_t	
#define S8  int8_t
#define S16 int16_t
#define S32 int32_t
	
//Farbdefinitionen fuer 16-Bit-Modus 
#define TFT_16BitGreen 		    	0x07E0
#define TFT_16BitBlack 		    	0x0000
#define TFT_16BitOrange		    	0xFCC6
#define TFT_16BitDark_Blue          0x0013
#define TFT_16BitBright_Blue        0xEEDF
#define TFT_16BitYellow             0xFFE0
#define TFT_16BitBright_Yellow      0xFFCC
#define TFT_16BitRed	        	0xF800
#define TFT_16BitBlue		    	0x331F
#define TFT_16BitWhite 		    	0xFFFF
	
//Pixel-Anzahl des Displays
#define TFT_MAX_X  131
#define TFT_MAX_Y  175

//Display-Ausrichtungen
#define TFT_Portrait	 0		//Hochformat
#define TFT_Portrait180  1		//Hochformat um 180� gedreht
#define TFT_Landscape 	 2		//Querformat
#define TFT_Landscape180 3		//Querformat um 180� gedreht

#endif
