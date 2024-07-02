/*
 * File:   main.c
 * Author: Kevin
 *
 * Created on June 17, 2024, 1:35 PM
 */

#define F_CPU 16000000UL
#include <xc.h>
#include "tft.h"
#include <avr/interrupt.h>

#define SPI_DDR DDRB
#define SS      PINB2
#define MOSI    PINB3
#define SCK     PINB5
#define D_C		PIND2		//display: Data/Command 
#define Reset	PIND3		//display: Reset

volatile uint16_t counter;
uint16_t i;
uint8_t white = 0xFF;
uint8_t black = 0x0;
char text[] = "Ampel:";
const uint16_t window[] = {0xEF08, 0x1806, 0x1232, 0x1545, 0x1361, 0x164E}; //Start, Format, x1, x2, y1, y2

void SPISend8Bit(uint8_t data);
void SendCommandSeq(const uint16_t *data, uint32_t Anzahl);
ISR(TIMER1_COMPA_vect);
void Waitms(const uint16_t msWait);
void init_Timer1();
void SPI_init();
void Display_init(void);

int main(void) {
    DDRD |= (1 << D_C) | (1 << Reset); // output: PD2 -> Data/Command; PD3 -> Reset

	// calling the functions
	init_Timer1();
	SPI_init();
	sei();
	Display_init();
    
    // drawing the background
	for (i = 0; i < 23232; i++) // 132*176 = 23232
	{
		SPISend8Bit(white);
        SPISend8Bit(white);
	}

	// drawing the sqare (in the place the image should be called)
	SendCommandSeq(window, 6);
	for (i = 0; i < 400; i++) // 20*20 = 400
	{
		SPISend8Bit(black); 
        SPISend8Bit(black);
	}
       
    while (1) {
    }
}

// Timer1 interrupt service routine
ISR(TIMER1_COMPA_vect)
{
	counter++;
}

// wait for msWait milliseconds function
void Waitms(const uint16_t msWait)
{
	static uint16_t aktTime, diff;
	uint16_t countertemp;
	cli();			   // da 16 bit Variablen k�nnte ohne cli() sei() sich der Wert
	aktTime = counter; // von counter �ndern, bevor er komplett aktTime zugewiesen wird.
	sei();			   // Zuweisung erfolgt wg. 8 bit controller in 2 Schritten.
	do
	{
		cli(); // disables interrupts for a timed sequence
		countertemp = counter;
		sei();
		diff = countertemp + ~aktTime + 1;
	} while (diff < msWait);
}

// Timer1 interrupt service routine
void init_Timer1()
{
	TCCR1B |= (1 << CS10) | (1 << WGM12); // TimerCounter1ControlRegisterB Clock Select |(1<<CS10)=>prescaler = 1; WGM12=>CTC mode
	TIMSK1 |= (1 << OCIE1A);			  // TimerCounter1 Interrupt Mask Register: Output Compare Overflow Interrupt Enable
	OCR1A = 15999;						  // Aufloesung msec
}

// SPI initialization
void SPI_init()
{
	// set CS, MOSI and SCK to output
	SPI_DDR |= (1 << SS) | (1 << MOSI) | (1 << SCK);
	// enable SPI, set as master, and clock to fosc/4 or 128
	SPCR = (1 << SPE) | (1 << MSTR); //	4MHz bzw. 125kHz
}

// SPI send 8 bit
void SPISend8Bit(uint8_t data)
{
	PORTB &= ~(1 << SS); // CS low
	SPDR = data;		 // load data into register
	while (!(SPSR & (1 << SPIF)))
		;				// wait for transmission complete
	PORTB |= (1 << SS); // CS high
}

// Send command sequence
void SendCommandSeq(const uint16_t *data, uint32_t Anzahl)
{
	uint32_t index;
	uint8_t SendeByte;
	for (index = 0; index < Anzahl; index++)
	{
		PORTD |= (1 << D_C);				   // Data/Command auf High => Kommando-Modus
		SendeByte = (data[index] >> 8) & 0xFF; // High-Byte des Kommandos
		SPISend8Bit(SendeByte);
		SendeByte = data[index] & 0xFF; // Low-Byte des Kommandos
		SPISend8Bit(SendeByte);
		PORTD &= ~(1 << D_C); // Data/Command auf Low => Daten-Modus
	}
}

// Display initialization
void Display_init(void)
{
	const uint16_t InitData[] = {
		// Initialisierungsdaten fuer 16-Bit-Farben Modus
		0xFDFD, 0xFDFD,
		// pause
		0xEF00, 0xEE04, 0x1B04, 0xFEFE, 0xFEFE,
		0xEF90, 0x4A04, 0x7F3F, 0xEE04, 0x4306,
		// pause
		0xEF90, 0x0983, 0x0800, 0x0BAF, 0x0A00,
		0x0500, 0x0600, 0x0700, 0xEF00, 0xEE0C,
		0xEF90, 0x0080, 0xEFB0, 0x4902, 0xEF00,
		0x7F01, 0xE181, 0xE202, 0xE276, 0xE183,
		0x8001, 0xEF90, 0x0000,
		// pause
		0xEF08,	0x1806,	0x1200, 0x1583,	0x13AF,
		0x1600 // Querformat um 180 gedreht 176 x 132 Pixel
	};
	Waitms(300);
	PORTD &= ~(1 << Reset); // Reset-Eingang des Displays auf Low => Beginn Hardware-Reset
	Waitms(75);
	PORTB |= (1 << SS); // SSEL auf High
	Waitms(75);
	PORTD |= (1 << D_C); // Data/Command auf High
	Waitms(75);
	PORTD |= (1 << Reset); // Reset-Eingang des Displays auf High => Ende Hardware Reset
	Waitms(75);
	SendCommandSeq(&InitData[0], 2);
	Waitms(75);
	SendCommandSeq(&InitData[2], 10);
	Waitms(75);
	SendCommandSeq(&InitData[12], 23);
	Waitms(75);
	SendCommandSeq(&InitData[35], 6);
}