// Simple IR Transceiver

// Pinout:
// P1.[0..3] - Common anode LEDs

// P1.6 - RX LED
// P1.7 - TX LED

// P2.0 - 1. button
// P2.1 - 2. button
// P2.2 - 3. button
// P2.3 - 4. button
// P2.4 - IR LED OUTPUT
// P2.5 - 38kHz IR RECEIVER

#include <msp430g2452.h>

#define BEAT_FREQ       512
#define BUTDEB_LEN      (BEAT_FREQ / 4)
// #define MAX_STATE       9
#define STATE_LEN       4

#define WAIT_TIME       BEAT_FREQ
// Transmitter
#define EN_TX   0x7F
#define DIS_TX  0xFF

// Receiver
#define EN_RX   0xBF
#define DIS_RX  0xFF
// #define ERROR_STATE     0x0A
#define ERROR_STATE     5

// Transmitter
// xmit - flag to start 38kHz IR LED modulation
// xmitstate - state machine for serial protocol
// trnsm_currstate - currently selected data (button state) [0-3]
// savestate - Prevent transm_currstate state from changing during xmit
// butdeb - Button debounce counters
volatile int xmit = 0, xmitstate = 0, trnsm_currstate = 0, savestate = 0,
        butdeb[4] = { 0, 0, 0, 0 };

// Receiver
// rcvr_currstate - currently received data (led state) [0-3]
// txmask - mask to set transmitter led on/off
// rxmask - mask to set receiver led on/off
// txhold_counter - hold transmitter state for a while
// rxhold_counter - hold receiver state for a while
volatile unsigned int rcvr_currstate = 0, rcvrstate = 0, newstate = 0,
        stateage = 0, txmask = DIS_TX, rxmask = DIS_RX, txhold_counter = 0,
        rxhold_counter = 0;

// Common anode LED states.  Active low. {[0-3], [All led's on - ERROR]}

// 0xFF - NORMAL state (All LEDs off)
// 0xFO - ERROR  state (All LEDs on)
unsigned const char leds[] = { 0xFF, 0xFE, 0xFD, 0xFB, 0xF7, 0xF0 };

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    //WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    // P1DIR = 0xFF; // Set P1 to output direction
    P1DIR = 0xCF; // Set P1 to output direction
    // P1OUT = 0xFF; // Set the LEDs off
    P1OUT = 0xCF; // Set the LEDs off
    // P2DIR = 0xFC; // Set P2.0/1 to input for buttons
    P2DIR = 0xD0; // Set P2.0/1/2/3/5 to input for buttons
    // P2OUT = 0x20; // Set IR LED off
    P2OUT = 0x10; // Set IR LED off
    // P2REN = 0x03; // Enable pulldown resistors
    P2REN = 0x0F; // Enable pulldown resistors
    // P2IES = 0x03; // Positive edge trigger
    P2IES = 0x2F; // Negative edge trigger
    P2IFG = 0x00; // Clear pin change interrupt flags
    // P2IE = 0x03; // Enable button interrupts
    P2IE = 0x2F; // Enable button interrupts

    TACTL = TASSEL_1 | MC_1; // Set the timer A to ACLK, Up mode
    TACCR0 = (0x8000 / BEAT_FREQ) - 1; // Reset timer at BEAT frequency
    TACCTL0 = CCIE; // Clear the timer and enable timer interrupt

    __enable_interrupt();
    //_BIS_SR(LPM1_bits + GIE);

    while (1)
    {
        while (xmit)
        {
            // Delay some cycles to keep frequency at 38kHz
            // This is much less than expected due to the
            // lost time to the while loop's condition test
            // and the bitwise math on P2OUT
            // This value was found with an oscilloscope.
            __delay_cycles(2);
            //P2OUT ^= BIT5;
            P2OUT ^= BIT4;
        }
        // Turn off IR LED
        //P2OUT &= ~BIT5;
        P2OUT &= ~BIT4;
    }
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void)
{
    // Transmitter
    xmitstate = (xmitstate + 1) % BEAT_FREQ;

    // Set transmit flag for START BIT3 BIT2 BIT1 BIT0 STOP
    if (xmitstate == 0 || xmitstate == (STATE_LEN + 1))
    { // START & STOP bits
        xmit = 1;
        txmask = EN_TX;
        savestate = trnsm_currstate; // Prevent state from changing during xmit
    }
    else if (xmitstate < (STATE_LEN + 1))
    { // 4 bits of state MSBF
        xmit = !((savestate >> (STATE_LEN - xmitstate)) & 0x1);
    }
    else
    {
        xmit = 0;
        txmask = DIS_TX;
    }
    // Update display
    // P1OUT = leds[trnsm_currstate] & txmask;

    // Decrement debounce counters, but only to zero
    butdeb[0] ? butdeb[0]-- : 1;
    //    if (butdeb[0]){
    //        butdeb[0] = butdeb[0] - 1;
    //    } else {
    //        1;
    //    }

    butdeb[1] ? butdeb[1]-- : 1;
    butdeb[2] ? butdeb[2]-- : 1;
    butdeb[3] ? butdeb[3]-- : 1;

    // Hold transmitter state for WAIT_TIME cycle
    if (txhold_counter == 0)
    {
        trnsm_currstate = 0;
    }
    else
    {
        txhold_counter--;
    }

    // Receiver
    // Heartbeat interrupt
    stateage++;
    // If we haven't seen an update in two cycles, something's wrong
    if (stateage > 2 * BEAT_FREQ)
    {
        // rcvr_currstate = ERROR_STATE;
        rcvr_currstate = 0;
    }

    rxmask = DIS_RX;
    if (rcvrstate > 1)
        rxmask = EN_RX;

    if (rcvr_currstate == 1 || rcvr_currstate == 2 || rcvr_currstate == 3
            || rcvr_currstate == 4)
    { // Hold last received state for WAIT_TIME cycle
        P1OUT = leds[rcvr_currstate] & txmask & rxmask;
        rxhold_counter = WAIT_TIME;
    }

    else
    {
        if (rxhold_counter == 0)
        {
            P1OUT = leds[0] & txmask & rxmask;
        }
        else
        {
            rxhold_counter--;
        }
    }
}

// Timer A1 interrupt service routine
// Receiver
// Sampling pulse for IR data
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1(void)
{
    // P2OUT ^= 0x01; // Debug tick
    if (rcvrstate == 1)
    { // Test for START bit
        if (P2IN & 0x20)
        { // No start bit
            rcvrstate = 0;
            P2IE |= 0x20;
            TACCTL1 = 0;
        }
    }
    else if (rcvrstate == STATE_LEN + 2)
    { // Test for STOP bit
        if (P2IN & 0x20)
        { // No stop bit
            rcvrstate = 0;
            P2IE |= 0x20;
            TACCTL1 = 0;
        }
    }
    else if (rcvrstate == STATE_LEN + 3)
    { // Finish receive
        rcvrstate = 0;
        P2IE |= 0x20;
        TACCTL1 = 0;
        rcvr_currstate = newstate;
        stateage = 0;
    }
    else
    { // State bit
        newstate = (newstate << 1) | ((P2IN >> 5) & 0x01);
    }
    rcvrstate++;
    TAIV &= ~(0x02); // CLEAR INTERRUPT
}

#pragma vector=PORT2_VECTOR
__interrupt void Port2(void)
{
    // Transmitter
    if ((P2IFG & 0x01) && !butdeb[0])
    { // 1. button pressed
        butdeb[0] = BUTDEB_LEN;
        // currstate = currstate ? (currstate - 1) : MAX_STATE;
        trnsm_currstate = 1;
        txhold_counter = WAIT_TIME;
    }
    else if ((P2IFG & 0x02) && !butdeb[1])
    { // 2. button pressed
        butdeb[1] = BUTDEB_LEN;
        // currstate = (currstate + 1) % (MAX_STATE + 1);
        trnsm_currstate = 2;
        txhold_counter = WAIT_TIME;
    }
    else if ((P2IFG & 0x04) && !butdeb[2])
    { // 3. button pressed
        butdeb[2] = BUTDEB_LEN;
        // currstate = (currstate + 1) % (MAX_STATE + 1);
        trnsm_currstate = 3;
        txhold_counter = WAIT_TIME;
    }
    else if ((P2IFG & 0x08) && !butdeb[3])
    { // 4. button pressed
        butdeb[3] = BUTDEB_LEN;
        // currstate = (currstate + 1) % (MAX_STATE + 1);
        trnsm_currstate = 4;
        txhold_counter = WAIT_TIME;
    }

    // Receiver
    // Start bit edge
    if (P2IFG & 0x20)
    {
        rcvrstate = 1; // Begin receive
        newstate = 0;
        // Set Timer0A1 to half a beat later
        TACCR1 = (TAR + (0x8000 / 2 / BEAT_FREQ)) % (TACCR0 + 1);
        TACCTL1 = CCIE;
        // Turn off edge interrupt
        P2IE &= ~0x20;
    }

    // Clear pin change (button) interrupt flags
    P2IFG = 0x00;
}
