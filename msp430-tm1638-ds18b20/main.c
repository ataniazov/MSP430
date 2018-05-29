#include "msp430g2553.h"
#include "stdint.h"
#include "onewire.h"
#include "delay.h"
#include "TM1638.h"

// MSP430 Ports Define
#define LED_RED BIT0                        //RED Led
#define LED_GRE BIT6                        //Green Led

/***************************************************************/

void timer_init()
{
    CCTL0 = CCIE;
    CCR0 = 20000;
    TACTL = TASSEL_1 + MC_1;
}

void tm1638_float(float data)
{
//    unsigned long int integer_part;
//    integer_part = data*10000;
    ShowDecNumber(data * 10000, 16, 1);
}

// ##################### One-Wire ###############################

void ow_portsetup()
{
    OWPORTDIR |= OWPORTPIN;
    OWPORTOUT |= OWPORTPIN;
    OWPORTREN |= OWPORTPIN;
}

uint16_t ReadDS1820(void)
{
    unsigned int i;
    uint16_t byte = 0;
    for (i = 16; i > 0; i--)
    {
        byte >>= 1;
        if (ow_read_bit())
        {
            byte |= 0x8000;
        }
    }
    return byte;
}

float GetData(void)
{
    uint16_t temp;
    ow_reset();
    ow_write_byte(0xcc); // skip ROM command
    ow_write_byte(0x44); // convert T command
    OW_HI
    DELAY_MS(750); // at least 750 ms for the default 12-bit resolution
    ow_reset();
    ow_write_byte(0xcc); // skip ROM command
    ow_write_byte(0xbe); // read scratchpad command
    temp = ReadDS1820();

    if (temp < 0x8000)
    {
        return (temp * 0.0625);
    }
    else
    {
        temp = (~temp) + 1;
        return (temp * 0.0625);
    }
}

int ow_reset()
{
    OW_LO
    DELAY_US(480);                              // 480us minimum
    OW_RLS
    DELAY_US(40);                               // slave waits 15-60us
    if (OWPORTIN & OWPORTPIN)
        return 1;   // line should be pulled down by slave
    DELAY_US(300);                          // slave TX presence pulse 60-240us
    if (!(OWPORTIN & OWPORTPIN))
        return 2;   // line should be "released" by slave
    return 0;
}


void ow_write_bit(int bit)
{
    OW_HI
    if (bit)
    {
        OW_LO
        DELAY_US(5);
        OW_RLS
        DELAY_US(56);
    }
    else
    {
        OW_LO
        DELAY_US(60);
        OW_RLS
        DELAY_US(1);
    }
}


int ow_read_bit()
{
    int bit = 0;
//  DELAY_US(1); // recovery, min 1us
    OW_LO
    DELAY_US(5); // hold min 1us
    OW_RLS
    DELAY_US(10); // 15us window
    if (OWPORTIN & OWPORTPIN)
    {
        bit = 1;
    }
    DELAY_US(46); // rest of the read slot
    return bit;
}


void ow_write_byte(uint8_t byte)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        ow_write_bit(byte & 1);
        byte >>= 1;
    }
}

uint8_t ow_read_byte()
{
    unsigned int i;
    uint8_t byte = 0;
    for (i = 0; i < 8; i++)
    {
        byte >>= 1;
        if (ow_read_bit())
            byte |= 0x80;
    }
    return byte;
}

// ################# Clock ######################
struct
{
    unsigned char h, m, s;
} t;

enum
{
    State_Normal, State_Temp, State_SetTime
} state;

void showTemp()
{
    float temperate = GetData();
    ShowDig(0, 2, 1);
    ShowSignedDecNumber(temperate * 100, 4);
}

void showTime()
{
    ShowDig(0, 1, 1);
    ShowDig(2, t.h / 10, 0);
    ShowDig(3, t.h % 10, 1);
    ShowDig(4, t.m / 10, 0);
    ShowDig(5, t.m % 10, 1);
    ShowDig(6, t.s / 10, 0);
    ShowDig(7, t.s % 10, 0);
}

void showSetTime()
{
    ShowDig(0, 3, 1);
    ShowDig(2, t.h / 10, 0);
    ShowDig(3, t.h % 10, 1);
    ShowDig(4, t.m / 10, 0);
    ShowDig(5, t.m % 10, 1);
    ShowDig(6, t.s / 10, 0);
    ShowDig(7, t.s % 10, 0);
}

void timer0_init()
{
//    CCTL0 = CCIE;
//     CCR0 = 20000;
//     TACTL = TASSEL_1 + MC_1;

    // ACLK and Timer1_A

    // Set the timer A to ACLK, Up mode
    TACTL = TASSEL_1 | MC_1;
    // Reset timer at 32768-1 frequency (0x8000 = 32768)
    TACCR0 = (0x8000) - 1;
    // Clear the timer and enable timer interrupt
    TACCTL0 = CCIE;

}
// ##############################################

int main()
{
    init_WDT();
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    init_Ports();
    init_SPI();
    ow_portsetup();
    init_Display();
    //timer_init();
    timer0_init();

    SetupDisplay(1, 1);

    // ########### Clock ###########

    t.h = t.m = t.s = 0;

    _BIS_SR(GIE);
    state = State_Normal;

    int keys;
    while (1)
    {
        keys = GetKey();
        switch (state)
        {
        case State_Normal:
            showTime();
            if (keys == TM1638_KEY3)
            {
                state = State_SetTime;
            }
            if (keys == TM1638_KEY2)
            {
                state = State_Temp;
                DisplayClean();
            }
            break;
        case State_Temp:
            showTemp();
            if (keys == TM1638_KEY1)
            {
                state = State_Normal;
                DisplayClean();
            }
            break;
        case State_SetTime:
            showSetTime();
            if (keys == TM1638_KEY8)
                t.s = 0;
            __delay_cycles(50000);
            if (keys == TM1638_KEY7)
                t.m = (t.m + 1) % 60;
            if (keys == TM1638_KEY5)
                t.m = (t.m > 0) ? (t.m - 1) : 59;
            __delay_cycles(50000);
            if (keys == TM1638_KEY6)
                t.h = (t.h + 1) % 24;
            if (keys == TM1638_KEY4)
                t.h = (t.h > 0) ? (t.h - 1) : 23;
            __delay_cycles(50000);
            if (keys == TM1638_KEY1)
            {
                TAR = 0;
                state = State_Normal;
            }
            break;
        }
//        __delay_cycles(1000000);
    }
    // #############################
//    __bis_SR_register(CPUOFF + GIE);            // LPM0, TA0_ISR will force exit
//    _BIS_SR(LPM0_bits + GIE);                 // Enter LPM0 w/ interrupt
}

/*
 #pragma vector=TIMER0_A0_VECTOR
 __interrupt void Timer_A(void)
 {
 __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
 float temperate = GetData();
 ShowSignedDecNumber(temperate * 10000, 16);
 CCR0 += 10000;
 __bis_SR_register(CPUOFF + GIE);          // LPM0, TA0_ISR will force exit
 }
 */

//// ################# Clock ######################
#pragma vector= TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void)
{
    P1OUT ^= BIT1;
    //TACCTL0 &= ~CCIFG;
    if (state == State_SetTime)
        return;
    if (++t.s >= 60)
    {
        t.s = 0;
        if (++t.m >= 60)
        {
            t.m = 0;
            if (++t.h >= 24)
            {
                t.h = 0;
            }
        }
    }
}
