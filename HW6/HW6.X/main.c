#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h> 
#include "i2c_master_noint.h"
#include "ssd1306.h"
#include "font.h"
#include "imu.h"

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

void drawChar(unsigned char x, unsigned char y, unsigned char letter);
void drawMessage(unsigned char x, unsigned char y, char * message);
void bar_x(signed short);
void bar_y(signed short);
 
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
    
    i2c_master_setup();
    ssd1306_setup();
    imu_setup();
    signed short imu[7];
    char message[50];

    __builtin_enable_interrupts();

    while (1) {
        _CP0_SET_COUNT(0);   
        LATAbits.LATA4 = !LATAbits.LATA4;

        imu_read(IMU_OUT_TEMP_L, imu, 7);
        
        if (0) {
            sprintf(message,"g: %d %d %d  ", imu[1], imu[2], imu[3]);
            drawMessage(0,0,message);
            sprintf(message,"a: %d %d %d  ", imu[4], imu[5], imu[6]);
            drawMessage(0,8,message);
            sprintf(message,"t: %d  ", imu[0]);
            drawMessage(0,16,message);
        } else {
            bar_x(-imu[5]);
            bar_y(imu[4]);
        }
      
        ssd1306_update();
        
        while (_CP0_GET_COUNT() < 48000000/2/20){}
    }   
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
}

void drawMessage(unsigned char x, unsigned char y, char * message){
    int i = 0;
    while (message[i] != 0){
        drawChar(x+5*i,y,message[i]);
        i++;
    } 
}

void bar_x(signed short xval){
    int lim,i;
    
    lim = (int) 74*xval/32767;
    ssd1306_clear();
    if (lim > 0){
        for (i = 0; i < lim; i++){
            ssd1306_drawPixel(74+i,16,1);
        }
    } else {
        for (i = 0; i < -1*lim; i++){
            ssd1306_drawPixel(74-i,16,1);
        }
    }
}

void bar_y(signed short yval){
    int lim,i;
    
    lim = (int) 16*yval/32767;
    if (lim > 0){
        for (i = 0; i < lim; i++){
            ssd1306_drawPixel(74,16+i,1);
        }
    } else {
        for (i = 0; i < -1*lim; i++){
            ssd1306_drawPixel(74,16-i,1);
        }
    }  
}