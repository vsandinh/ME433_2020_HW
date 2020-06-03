#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h> 
#include "i2c_master_noint.h"
#include "ssd1306.h"
#include "font.h"

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

void init_mcp23017(void);
void drawChar(unsigned char x, unsigned char y, unsigned char letter);
void drawMessage(unsigned char x, unsigned char y, char * message);
//void setPin(unsigned char address, unsigned char register_s, unsigned char value);
//unsigned char readPin(unsigned char address, unsigned char register_r);
//unsigned char button_read; 
//int button_int;
 
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
    
    init_mcp23017();
    ssd1306_setup();

    __builtin_enable_interrupts();

    while (1) {
        // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        // remember the core timer runs at half the sysclk
        char message[50];
        int i;
        float FPS;
        
        _CP0_SET_COUNT(0);   
        LATAbits.LATA4 = 1;
        while(_CP0_GET_COUNT() < 24000000/2){}
        LATAbits.LATA4 = 0;
        while(_CP0_GET_COUNT() < 24000000){}
        LATAbits.LATA4 = 1;
        
        for (i = 0; i < 5000; i++){
            _CP0_SET_COUNT(0);
            sprintf(message,"i = %d",i);
            drawMessage(10,10,message);
            FPS = (float)24000000/_CP0_GET_COUNT();
            sprintf(message,"FPS = %f",FPS);
            drawMessage(10,20,message);
        }
    }   
}

void init_mcp23017(){
    i2c_master_setup();
    
    i2c_master_start();
    i2c_master_send(0b01000000);
    i2c_master_send(0x00);
    i2c_master_send(0x00);
    i2c_master_stop();
    
    i2c_master_start();
    i2c_master_send(0b01000000);
    i2c_master_send(0x01);
    i2c_master_send(0xFF);
    i2c_master_stop();    
}

void drawChar(unsigned char x, unsigned char y, unsigned char letter){
    unsigned char color;
    int i,j;
    for (i = 0; i < 8; i++){
        for (j = 0; j < 5; j++){
            color = ((ASCII[letter - 0x20][j]>>i)&1);
            ssd1306_drawPixel(x+j,y+i,color); 
        }
    }
    ssd1306_update();
}

void drawMessage(unsigned char x, unsigned char y, char * message){
    int i = 0;
    while (message[i] != 0){
        drawChar(x+5*i,y,message[i]);
        i++;
    }
}