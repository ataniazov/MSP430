#ifndef ONEWIRE_H_
#define ONEWIRE_H_
#include <stdint.h>

// Port and pins definition:
#define OWPORTDIR P2DIR
#define OWPORTOUT P2OUT
#define OWPORTIN P2IN
#define OWPORTREN P2REN
#define OWPORTPIN BIT3
#define OW_LO {	OWPORTDIR |= OWPORTPIN;	OWPORTREN &= ~OWPORTPIN; OWPORTOUT &= ~OWPORTPIN; }
#define OW_HI {	OWPORTDIR |= OWPORTPIN;	OWPORTREN &= ~OWPORTPIN; OWPORTOUT |= OWPORTPIN; }
#define OW_RLS { OWPORTDIR &= ~OWPORTPIN; OWPORTREN |= OWPORTPIN; OWPORTOUT |= OWPORTPIN; }

// list of commands DS18B20:

#define DS1820_WRITE_SCRATCHPAD 	0x4E
#define DS1820_READ_SCRATCHPAD      0xBE
#define DS1820_COPY_SCRATCHPAD 		0x48
#define DS1820_READ_EEPROM 			0xB8
#define DS1820_READ_PWRSUPPLY 		0xB4
#define DS1820_SEARCHROM 			0xF0
#define DS1820_SKIP_ROM             0xCC
#define DS1820_READROM 				0x33
#define DS1820_MATCHROM 			0x55
#define DS1820_ALARMSEARCH 			0xEC
#define DS1820_CONVERT_T            0x44

// Function definitions:

int ow_reset();
void ow_write_bit(int bit);
int ow_read_bit();
void ow_write_byte(uint8_t byte);
uint8_t ow_read_byte();
void onewire_line_low();
void onewire_line_high();
void onewire_line_release();
float GetData(void);

#endif /* ONEWIRE_H_ */
