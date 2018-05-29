/*
 * TM1638.h
 *
 *  Created on: 13.08.2012
 *      Author: user
 */

#ifndef TM1638_H_
#define TM1638_H_

// Clock
#define TM1638_KEY1     0x01    // Key1
#define TM1638_KEY2     0x02    // Key2
#define TM1638_KEY3     0x04    // Key3
#define TM1638_KEY4     0x08    // Key4
#define TM1638_KEY5     0x10    // Key5
#define TM1638_KEY6     0x20    // Key6
#define TM1638_KEY7     0x40    // Key7
#define TM1638_KEY8     0x80    // Key8
//

#define STROBE_TM1638 BIT5
#define DIO BIT2
#define CLK BIT4

// TM1638 Commands Define
#define RED_TM1638 0x01 					//RED Led
#define GRE_TM1638 0x02						//Green Led
#define DATA_WRITE_INCR_ADDR 0x40			//Command to switch TM1638 for automatic increment address mode
#define DATA_WRITE_FIX_ADDR 0x44			//Command to switch TM1638 for fix address mode
#define DATA_READ_KEY_SCAN_MODE 0x42		//Command for read key code from TM1638
#define ADDRSET 0xC0						//Command to set address 0x00
#define DISP_ON 0x8F						//Command to Display ON and set (max) brightness
#define DISP_OFF 0x80						//Command to Display OFF
#define MIN_VOLT 2700						//Critycal voltage of power OFF

void init_Ports();
void init_WDT();
void init_SPI();
void SendCommand(unsigned char Command);
void SendData(unsigned int address, unsigned int data);
void ShowDig(int position, int Data, int Dot);
void ClearDig (unsigned int position, unsigned int Dot);
void ShowError();
void ShowDecNumber (unsigned long number, unsigned int dots, unsigned int startingPos);
void ShowSignedDecNumber ( signed long number, int dots);
void ShowHexNumber (unsigned long number, int dots);
void ShowString (const char * string, unsigned int dots, unsigned int pos);
void ShowLed ( int Number, int Color);
void ShowLeds(int Color);
void DisplayClean();
void SetupDisplay(char active, char intensity);
void init_Display();
int GetKey();




#endif /* TM1638_H_ */
