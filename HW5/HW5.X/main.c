#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "ws2812b.h"

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 00000000 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = ON // allow multiple reconfigurations
#pragma config IOL1WAY = ON // allow multiple reconfigurations

void heart_beat(void);

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0;
    TRISBbits.TRISB4 = 1;

    ws2812b_setup();
    int numLEDs = 4; int i;
    float brightness = 0.01;
    wsColor c[4];
    wsColor *pointer;
    
    __builtin_enable_interrupts();

    while (1) {
        heart_beat();
        
        for (i = 0; i < 360; i++) {
            c[0] = HSBtoRGB(i, 1, brightness);
            if (i + 60 > 360) {
                c[1] = HSBtoRGB(i - 300, 1, brightness);
            } else {
                c[1] = HSBtoRGB(i + 60, 1, brightness);
                }
            if (i + 120 > 360) {
                c[2] = HSBtoRGB(i - 240, 1, brightness);
            } else {
                c[2] = HSBtoRGB(i + 120, 1, brightness);
                }
            if (i + 180 > 360) {
                c[3] = HSBtoRGB(i - 180, 1, brightness);
            } else {
                c[3] = HSBtoRGB(i + 180, 1, brightness);
                }
            pointer = &c;
            ws2812b_setColor(pointer,numLEDs);
        }

        /*wsColor c[2] = {
            {.r=0x7F,.b=0x00,.g=0x00},
            {.r=0x00,.b=0x00,.g=0x7F}
        };
        pointer = &c;
        ws2812b_setColor(pointer,numLEDs);
        //HSBtoRGB(60, 1, 0.5);*/
    }
}

void heart_beat(){
        // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        // remember the core timer runs at half the sysclk
        _CP0_SET_COUNT(0);   
        LATAbits.LATA4 = 1;
        while(_CP0_GET_COUNT() < 24000000/8){}
        LATAbits.LATA4 = 0;
        while(_CP0_GET_COUNT() < 24000000/4){}
        LATAbits.LATA4 = 1;
}