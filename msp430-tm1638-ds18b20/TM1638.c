//******************************************************************************
//  Version 1.6.0 28.06.2012
//
//  MSP430G2xx3 - USCI_A0 SPI Master Interface to TM1638, Read/Write
//  command, key scanning, show digit and switch LEDs on board
//  Description: SPI communicate with a TM1638 in read and write mode
//  TM1638 - Driver for Led's indicator, 10 segment output lines,
//  8 grid output lines, 8 segment/key - 3 lines scanning.
//  See: TM1638 Display/LED module at http://www.dealextreme.com/p/81873?r=68099021
//  for example module TM1638 for arduino: http://code.google.com/p/tm1638-library/
//
//  MCLK = SMCLK/2
//
//                                MSP430G2xx3
//                    ^ +Vcc    -----------------
//                    _        |              XIN|-
//                   4k7       |                 |
//        TM1638      _      --|RST          XOUT|-
//       ---------    |   D1   |                 |
//  ----|D0    DIO|<--+--->|-->|P1.2/UCA0SDA     |
//  ----|D1       |   |--|<----|P1.1/UCA0SDO P1.0|----->LED RED
//  ----|D2       |     D2     |             P1.6|----->LED GREEN
//  ----|D3    CLk|<-----------|P1.4/UCA0SCL     |
//  ----|D4       |            |                 |
//  ----|D5 STROBE|<-----------|P1.5     		 |
//  ----|D6       |            |                 |
//  ----|D7       |            |                 |
//   +--|KEY_SCAN |            |                 |
//   |  |         |            |                 |
//  \|/
//
//		* --|<-- - diodes (1N4148 etc.)
//
//  Copyright (C) May 2012 Evgeniy Chepelev <shluzzz at gmail dot com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the version 3 GNU General Public License as
//  published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//  Built with CCS Version 5.2.0
//  
//******************************************************************************

#include  <msp430g2553.h>
#include  "TM1638.h"

// MSP430 Ports Define
#define LED_RED BIT0 						//RED Led
#define LED_GRE BIT6						//Green Led

const unsigned int ASCII[] = {					//Code table of symbols

0x00, // (32)  <space>
0x86, // (33)	!
0x22, // (34)	"
0x7E, // (35)	#
0x6D, // (36)	$
0x00, // (37)	%
0x00, // (38)	&
0x02, // (39)	'
0x30, // (40)	(
0x06, // (41)	)
0x63, // (42)	*
0x00, // (43)	+
0x04, // (44)	,
0x40, // (45)	-
0x80, // (46)	.
0x52, // (47)	/
0x3F, // (48)	0
0x06, // (49)	1
0x5B, // (50)	2
0x4F, // (51)	3
0x66, // (52)	4
0x6D, // (53)	5
0x7D, // (54)	6
0x27, // (55)	7
0x7F, // (56)	8
0x6F, // (57)	9
0x00, // (58)	:
0x00, // (59)	;
0x00, // (60)	<
0x48, // (61)	=
0x00, // (62)	>
0x53, // (63)	?
0x5F, // (64)	@
0x77, // (65)	A
0x7F, // (66)	B
0x39, // (67)	C
0x3F, // (68)	D
0x79, // (69)	E
0x71, // (70)	F
0x3D, // (71)	G
0x76, // (72)	H
0x06, // (73)	I
0x1F, // (74)	J
0x69, // (75)	K
0x38, // (76)	L
0x15, // (77)	M
0x37, // (78)	N
0x3F, // (79)	O
0x73, // (80)	P
0x67, // (81)	Q
0x31, // (82)	R
0x6D, // (83)	S
0x78, // (84)	T
0x3E, // (85)	U
0x2A, // (86)	V
0x1D, // (87)	W
0x76, // (88)	X
0x6E, // (89)	Y
0x5B, // (90)	Z
0x39, // (91)	[
0x64, // (92)	\
0x0F, // (93)	]
0x00, // (94)	^
0x08, // (95)	_
0x20, // (96)	`
0x5F, // (97)	a
0x7C, // (98)	b
0x58, // (99)	c
0x5E, // (100)	d
0x7b, // (101)	e
0x31, // (102)	f
0x6F, // (103)	g
0x74, // (104)	h
0x04, // (105)	i
0x0E, // (106)	j
0x75, // (107)	k
0x30, // (108)	l
0x55, // (109)	m
0x54, // (110)	n
0x5C, // (111)	o
0x73, // (112)	p
0x67, // (113)	q
0x50, // (114)	r
0x6D, // (115)	s
0x78, // (116)	t
0x1C, // (117)	u
0x2A, // (118)	v
0x1D, // (119)	w
0x76, // (120)	x
0x6E, // (121)	y
0x47, // (122)	z
0x46, // (123)	{
0x06, // (124)	|
0x70, // (125)	}
0x01 // (126)	~
};

const unsigned int Num[]=					//Code table of numbers
{
		0x3F,								//0
		0x06,								//1
		0x5B,								//2
		0x4F,								//3
		0x66,								//4
		0x6D,								//5
		0x7D,								//6
		0x07,								//7
		0x7F,								//8
		0x6F,								//9
		0x77,								//A
		0x7C,								//B
		0x39,								//C
		0x5E,								//D
		0x79,								//E
		0x71								//F
};

const unsigned int ERROR_DATA[] = {
  0x79, // E
  0x50, // r
  0x50, // r
  0x5C, // o
  0x50, // r
  0,
  0,
  0
};

const unsigned int MINUS = 0x40;
const unsigned int DEGREE = 0x63;
//const unsigned int C = 0x39;
const unsigned int U = 0x3E;

//Function definitions

void init_Ports()
{
	  P1DIR |= LED_RED + LED_GRE + STROBE_TM1638;
	  P1SEL = BIT1 + BIT2 + BIT4;			// Set secondary functions for PORT1
	  P1SEL2 = BIT1 + BIT2 + BIT4;			// P1.1 - TXD, P1.2 - RXD
	  P1OUT |= STROBE_TM1638;				// Set STROBE = "1" (Chip Select)
}

void init_WDT()
{
	  WDTCTL = WDTPW + WDTHOLD;				//Stop watchdog
}

void init_SPI()
{
	  UCA0CTL0 |= UCCKPL + UCMST + UCSYNC;  // 3-pin, 8-bit SPI master
	  UCA0CTL1 |= UCSSEL_2;                 // SMCLK
	  UCA0BR0 |= 0x02;                      // /2
	  UCA0BR1 = 0;                          //
	  UCA0MCTL = 0;                         // No modulation
	  UCA0CTL1 &= ~UCSWRST;                 // **Initialize USCI state machine**
}


void SendCommand(unsigned char Command) {	//Transmit Command
	P1OUT &= ~STROBE_TM1638;				//Set STROBE = "0"
	_NOP();
	_NOP();
	P1OUT |= STROBE_TM1638;					//Set STROBE = "1"
	_NOP();
	_NOP();
	P1OUT &= ~STROBE_TM1638;				//Set STROBE = "0"
	_NOP();
	UCA0TXBUF = Command;
	while (!(IFG2 & UCA0TXIFG));
	P1OUT |= STROBE_TM1638;					//Set STROBE = "1"
}

void SendData(unsigned int address, unsigned int data) {   		//Transmit Data
	SendCommand(DATA_WRITE_FIX_ADDR);
	P1OUT &= ~STROBE_TM1638;				//Set STROBE = "0"
	UCA0TXBUF = (0xC0 | address);			//Set first address
	while (!(IFG2 & UCA0TXIFG));
	UCA0TXBUF = data;						//Send data
	while (!(IFG2 & UCA0TXIFG));
	P1OUT |= STROBE_TM1638;					//Set STROBE = "1"
}

void ShowDig(int position, int Data, int Dot)			//show single digit
{
	SendData(position << 1, Num [Data] | (Dot ? 0x80 : 0) );
}

void ClearDig (unsigned int position, unsigned int Dot)
{
	SendData(position << 1, 0x00 | (Dot ? 0x80 : 0) );
}

void ShowError() {
	unsigned int j;
	for (j=0; j<8; j++) {
		SendData(j, ERROR_DATA [j]);
	}
}

void ShowDecNumber (unsigned long number, unsigned int dots, unsigned int startingPos) {
	unsigned int i;
  if (number > 99999999) {
    ShowError();
  } else {
    for (i = 0; i < 6 - startingPos; i++) {
        ShowDig(7 - i, number % 10, (dots & (1 << i)) != 0);
        number /= 10;
    }
  }
}

void ShowSignedDecNumber ( signed long number, int dots)
{
	if (number >= 0) {
		ShowDecNumber (number, dots, 0);
	} else {
		if (-number > 9999999) {
		    ShowError();
		} else {
			ShowDecNumber(-number, dots, 1);
			SendData(0, MINUS);
		}
	}
}


void ShowString (const char * string, unsigned int dots, unsigned int pos) {
	unsigned int i;

  for (i = 0; i < 8 - pos; i++) {
  	if (string[i] != '\0') {
  		SendData( (pos + i) << 1, ASCII[ string[i] - 32] | ((dots & (1 << (7 - i ))) ? 0x80 : 0));
	} else {
	  break;
	}
}
}

void ShowLed ( int Number, int Color)
{
	if (Number) {
		SendData((Number << 1)-1, Color);
	}
}

void ShowLeds(int Color) {
	unsigned int i;
	for (i=1; i<9; i++) {
	SendData((i << 1)-1, Color);
	}
}

void DisplayClean() {						//Clean RAM of TM1638
	int i;
	SendCommand(DATA_WRITE_INCR_ADDR);		//Set address mode
	for (i=0; i<17; i++) {
		SendData(i, 0x00);
		}
	P1OUT |= STROBE_TM1638;					// Set STROBE = "1" (Chip Select)
}

void SetupDisplay(char active, char intensity) {
	SendCommand (0x80 | (active ? 8 : 0) | intensity);
	P1OUT |= STROBE_TM1638;					// Set STROBE = "1"
}

void init_Display() {
	__delay_cycles(100000);					//Time to initial TM1638
	DisplayClean();							//Clean display
	SendCommand(DISP_OFF);					//Display off
	SendCommand(DATA_WRITE_FIX_ADDR);		//Set address mode
	SendCommand(ADDRSET);					//Set first adress
}

int GetKey() {
	unsigned int KeyData = 0;
	unsigned int i;
	P1OUT &= ~STROBE_TM1638;				// Set STROBE = "0"
	UCA0TXBUF = DATA_READ_KEY_SCAN_MODE;
	while (!(IFG2 & UCA0TXIFG));
	__delay_cycles(20);						//wait to scan keys ready (see datasheet)
	UCA0TXBUF = 0xff;						//Send dummy byte
	while (!(IFG2 & UCA0TXIFG));			//wait for buffer ready
											// 1'st reseiving byte = bad (unknown reason)
	for (i=0; i<4; i++) {
		UCA0TXBUF = 0xff;
		while (!(IFG2 & UCA0RXIFG));
		KeyData |= UCA0RXBUF << i;
	}
	P1OUT |= STROBE_TM1638;					//Set STROBE = "1"
	return KeyData;
}

