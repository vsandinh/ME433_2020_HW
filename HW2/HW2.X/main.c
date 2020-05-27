#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<math.h>

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

void initSPI();
unsigned char spi_io(unsigned char o);
 
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
    LATAbits.LATA4 = 1;
    TRISBbits.TRISB4 = 1;
    
    initSPI();

    __builtin_enable_interrupts();
    
    int i; 
    unsigned short V = 0;
    unsigned char c;
    unsigned short p;
    
    //generate sine wave
    #define TABLE_SIZE 201
    #define TWO_PI (3.14159 * 2)
    float sinewave [TABLE_SIZE];
    float phaseIncrement = TWO_PI/((TABLE_SIZE-1)/2);
    float currentPhase = 0.0;
    for (i = 0; i < TABLE_SIZE; i++){
        sinewave[i] = 4095/2*sin(currentPhase)+4095/2;
        currentPhase += phaseIncrement;
    }
    
    //generate triangle wave
    float trianglewave[TABLE_SIZE];
    float amp = 4095/(TABLE_SIZE/2);
    for (i=0; i <= TABLE_SIZE/2; i++)
    {
        trianglewave[i] = amp*i;
    }    
    for (i=TABLE_SIZE/2+1; i < TABLE_SIZE; i++)
    {
        trianglewave[i] = amp*(TABLE_SIZE-1-i);
    }    
    
    i = 0;
    while (1) {
        c = 0;
        V = trianglewave[i];
        p = (c<<15);
        p = p|(0b111<<12); 
        p = p|V;
        // write data over SPI1
        LATAbits.LATA0 = 0; // bring CS low
        spi_io(p>>8); // write the 1st byte
        spi_io(p); // write the 1st byte
        c = 1;
        LATAbits.LATA0 = 1; //bring CS high
        LATAbits.LATA0 = 0; // bring CS low
        V = sinewave[i];
        p = (c<<15);
        p = p|(0b111<<12); 
        p = p|V;
        spi_io(p>>8); // write the 1st byte
        spi_io(p); // write the 2nd byte
        LATAbits.LATA0 = 1; //bring CS high
        
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < 24000000 / 100) { // 1/100 s
        }
        
        i++;
        if (i == TABLE_SIZE-1) {
            i = 0;
        }
    }
}

// initialize SPI1
void initSPI() {
    // Pin B14 has to be SCK1
    // Turn off analog pins
    ANSELA = 0; // 1 for analog
    // Make A0 an output pin for CS
    TRISAbits.TRISA0 = 0;
    LATAbits.LATA0 = 1;
    // Make A1 SDO1
    RPA1Rbits.RPA1R = 0b0011;
    // Make B5 SDI1
    SDI1Rbits.SDI1R = 0b0001;
    
    // setup SPI1
    SPI1CON = 0; // turn off the spi module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = 1000; // 1000 for 12 kHz, 1 for 12 MHz; // baud rate to 10 MHz [SPI4BRG = (48000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.CKE = 1; // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.ON = 1; // turn on spi
}

// send a byte via spi and return the response
unsigned char spi_io(unsigned char o) {
    SPI1BUF = o;
    while (!SPI1STATbits.SPIRBF) { // wait to receive the byte
        ;
    }
    return SPI1BUF;
}
