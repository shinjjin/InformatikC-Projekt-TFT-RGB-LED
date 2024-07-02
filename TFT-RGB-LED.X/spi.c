#include <xc.h>                        // LPC11xx definitions
#include "spi.h"
#include "tft.h"

/* ----------------------------------------------------------------
   Initialisierung des SPI Interfaces f�r eine 8-Bit Kommunikation 
   als Master. Es erfolgt nur ein unidirektionaler Datenverkehr vom
   Master zum Slave => kein MISO
   ---------------------------------------------------------------- */ 
void SPI_init()
{
	//set CS, MOSI and SCK to output
	SPI_DDR |= (1 << CS) | (1 << MOSI) | (1 << SCK);

	//enable SPI, set as master, and clock to fosc/4 or 128
	SPCR = (1 << SPE) | (1 << MSTR);// | (1 << SPR1) | (1 << SPR0); 4MHz bzw. 125kHz
	//SPSR |= 0x1;
}



/* ----------------------------------------------------------------
   Senden von 8-Bit ueber das SPI-Interface 
   Achtung: Der Slave muss vorher �ber Chip-Select (CS) 
            aktiviert werden. Nach der Uebertragung sollte CS
			wieder inaktiv werden.
   ---------------------------------------------------------------- */
void SPISend8Bit(uint8_t data){
	PORTB &= ~(1<<CS);				//CS low
	SPDR = data;					//load data into register
	while(!(SPSR & (1 << SPIF)));	//wait for transmission complete
	PORTB |= (1<<CS);				//CS high
}