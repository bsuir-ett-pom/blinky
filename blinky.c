#include <ioc8051F330.h>
#include "reload_timer.h"
// System clock
#define SYSCLK                  24500000/8
#define TIMER_PRESCALER         48
#define TIMER_TICKS_PER_SEC     SYSCLK/TIMER_PRESCALER
// Timer frequencies
#define TIMER0_FREQUENCY        100
#define TIMER1_FREQUENCY        50
#define LED_FREQUENCY           1
// Timer 0, LED
#define AUX_T0                  -(TIMER_TICKS_PER_SEC/TIMER0_FREQUENCY)
#define TIMER0_RELOAD_HIGH      (AUX_T0&0xFF00)>>8
#define TIMER0_RELOAD_LOW       AUX_T0&0x00FF
// Timer 1, Button
#define AUX_T1                  -(TIMER_TICKS_PER_SEC/TIMER1_FREQUENCY)
#define TIMER1_RELOAD_HIGH      (AUX_T1&0xFF00)>>8
#define TIMER1_RELOAD_LOW       AUX_T1&0x00FF
// LED divider
#define LED_DIVIDER             TIMER0_FREQUENCY/LED_FREQUENCY
// Ports
#define pLED                    P1_bit.P13
#define pButton                 P0_bit.P07
// Global variables
volatile unsigned char divider = LED_DIVIDER;
volatile unsigned char toggle = 1;
// Code
void main(void) {
    // Disable watchdog timer
    PCA0MD &= !0x40;
    initializeTimers();
    initializePorts();
    // Enable global interrupts
    IE_bit.EA = 1;
    while (1);
}
// Initialize ports
static void initializePorts(void) {
    XBR1 = 0x40;
    P1MDOUT = 0x08;
}
// Initialize Timer0, Timer1
static void initializeTimers(void) {
    for (unsigned char i = 0; i < 2; i++) {
        reloadTimer(i);
        initTimer(i);
    }
    CKCON = 0x02;
}
// Timer 0 interrupt routine
#pragma vector = TF0_int
__interrupt void timer0Interrupt(void) {
    reloadTimer(0);
    divider--;
    if (divider == 0) {
        pLED = !pLED;
        divider = LED_DIVIDER;
    }
}
// Timer 1 interrupt routine
#pragma vector = TF1_int
__interrupt void  timer1Interrupt(void) {
    reloadTimer(1);
    if (pButton == 0 && toggle == 1) {
        TCON_bit.TR0 = !TCON_bit.TR0;            
        toggle = 0;
    }
    if (pButton == 1 && toggle == 0) {
        toggle = 1;
    }
}
// Reload timer
void reloadTimer(unsigned char timerNumber) {
    switch (timerNumber) {
        case 0:
            TH0 = TIMER0_RELOAD_HIGH;
            TL0 = TIMER0_RELOAD_LOW;
            break;
        case 1:
            TH1 = TIMER1_RELOAD_HIGH;
            TL1 = TIMER1_RELOAD_LOW;
            break;
    }
}
// Enable timer and allow interrupts, set 16-bit mode
static void initTimer(unsigned char timerNumber) {
    switch (timerNumber) {
        case 0:
            TMOD_bit.T0M1 = 0;
            TMOD_bit.T0M0 = 1;
            IE_bit.ET0 = 1;
            TCON_bit.TR0 = 1;
            break;
        case 1:
            TMOD_bit.T1M1 = 0;
            TMOD_bit.T1M0 = 1;
            IE_bit.ET1 = 1;
            TCON_bit.TR1 = 1;
            break;
    }
}