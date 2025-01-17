#define F_CPU 16000000UL
#include <xc.h>
#include "spi.h"
#include "tft.h"
#include <avr/interrupt.h>

// defining variables
volatile uint16_t counter;
volatile uint8_t state = 0;
const uint16_t window[] = {0xEF08, 0x1806, 0x1232, 0x1545, 0x1361, 0x164E}; // Start, Format, x1, x2, y1, y2

// defining functions
ISR(TIMER1_COMPA_vect);
void Waitms(const uint16_t msWait);
void Timer1_init();
void PWM_init();
ISR(PCINT0_vect);
void Display_init(void);

int main(void)
{
	uint16_t i;
	char text[] = "Ampel: ";

	// setting up the ports
	DDRB &= ~(1 << PORTB1);			   // input: PB1 -> Taster
	PORTB |= (1 << PORTB1);			   // pull-up resistor
	DDRD |= (1 << DDD5) | (1 << DDD6); // output: PD5, PD6 -> PWM
	PWM_init();						   // setting up PWM
	DDRD |= (1 << PD7);

	// pinchange interrupt
	PCICR |= (1 << PCIE0);	 // enable pin change interrupt
	PCMSK0 |= (1 << PCINT1); // enable pin change interrupt for PB1

	// setting up display
	DDRD |= (1 << D_C) | (1 << Reset); // output: PD2 -> Data/Command; PD3 -> Reset (initializing display pins)
	Timer1_init();
	SPI_init();
	sei();
	Display_init();

	// drawing the background
	for (i = 0; i < 23232; i++) // 132*176 = 23232
	{
		TFT_SPI_16BitPixelSend(TFT_16BitWhite);
	}

	// drawing the sqare
	SendCommandSeq(window, 6);
	for (i = 0; i < 400; i++) // 20*20 = 400
	{
		TFT_SPI_16BitPixelSend(TFT_16BitBlack);
	}

	// Schrift uebertragen
	TFT_Print(text, 50, 20, 2, TFT_16BitBlue, TFT_16BitWhite, TFT_Landscape180); // Übergabe von 7 "Werten": Adresse des 1. Elements von mytext, x1, y1, scale,
																				 // Schriftfarbe, Hintergrundfarbe, Display-Orientierung
	while (1)
	{
		if (state == 1)
		{
			PORTD |= (1 << PD7);
			OCR0A = 0;
			OCR0B = 255;
			SendCommandSeq(window, 6);
			for (i = 0; i < 400; i++) // 20*20 = 400
			{
				TFT_SPI_16BitPixelSend(TFT_16BitRed);
			}
			Waitms(1000);
			PORTD &= ~(1 << PD7);

			OCR0A = 255;
			OCR0B = 255;
			SendCommandSeq(window, 6);
			for (i = 0; i < 400; i++) // 20*20 = 400
			{
				TFT_SPI_16BitPixelSend(TFT_16BitYellow);
			}
			Waitms(1000);

			OCR0A = 255;
			OCR0B = 0;
			for (i = 0; i < 400; i++) // 20*20 = 400
			{
				TFT_SPI_16BitPixelSend(TFT_16BitGreen);
			}
			Waitms(1000);

			// off
			OCR0A = 0;
			OCR0B = 0;
			state = 0;
			for (i = 0; i < 400; i++) // 20*20 = 400
			{
				TFT_SPI_16BitPixelSend(TFT_16BitBlack);
			}
		}
	}
}

ISR(TIMER1_COMPA_vect)
{
	counter++;
}

ISR(PCINT0_vect)
{
	state = 1;
}

void Waitms(const uint16_t msWait)
{
	static uint16_t aktTime, diff;
	uint16_t countertemp;
	cli();			   // da 16 bit Variablen könnte ohne cli() sei() sich der Wert
	aktTime = counter; // von counter ändern, bevor er komplett aktTime zugewiesen wird.
	sei();			   // Zuweisung erfolgt wg. 8 bit controller in 2 Schritten.
	do
	{
		cli();
		countertemp = counter;
		sei();
		diff = countertemp + ~aktTime + 1;
	} while (diff < msWait);
}

void Timer1_init()
{
	TCCR1B |= (1 << CS10) | (1 << WGM12); // TimerCounter1ControlRegisterB Clock Select |(1<<CS10)=>prescaler = 1; WGM12=>CTC mode
	TIMSK1 |= (1 << OCIE1A);			  // TimerCounter1 Interrupt Mask Register: Output Compare Overflow Interrupt Enable
	OCR1A = 15999;						  // direkte Zahl macht Sinn; overflow register OCR1A berechnet mit division 64 => unlogischer Registerwert
}

void PWM_init()
{
	TCCR0A |= (1 << WGM00) | (1 << WGM01);	 // fast PWM mode
	TCCR0B |= (1 << CS02);					 // prescaler 256
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1); 
	OCR0A = 0;
	OCR0B = 0;
}

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
		0xEF08, 0x1806, 0x1200, 0x1583, 0x13AF,
		0x1600 // Querformat um 180 gedreht 176 x 132 Pixel
	};
	Waitms(300);
	PORTD &= ~(1 << Reset); // Reset-Eingang des Displays auf Low => Beginn Hardware-Reset
	Waitms(75);
	PORTB |= (1 << CS); // SSEL auf High
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