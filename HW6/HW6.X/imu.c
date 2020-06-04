#include "imu.h"

void imu_setup(){
    unsigned char who = 0;

    // read from IMU_WHOAMI
    i2c_master_start();
    i2c_master_send(IMU_ADDR);
    i2c_master_send(IMU_WHOAMI);
    i2c_master_restart();
    i2c_master_send(IMU_ADDR+1);
    who = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    if (who != 0b01101001){
        while(1){
            LATAbits.LATA4 = 1;
        }
    }

    // init IMU_CTRL1_XL
    i2c_master_start();
    i2c_master_send(IMU_ADDR);
    i2c_master_send(IMU_CTRL1_XL);
    i2c_master_send(0b10000010);
    i2c_master_stop();
    
    // init IMU_CTRL2_G
    i2c_master_start();
    i2c_master_send(IMU_ADDR);
    i2c_master_send(IMU_CTRL2_G);
    i2c_master_send(0b10001000);
    i2c_master_stop();

    // init IMU_CTRL3_C
    i2c_master_start();
    i2c_master_send(IMU_ADDR);
    i2c_master_send(IMU_CTRL3_C);
    i2c_master_send(0b00000100);
    i2c_master_stop();
    
}

void imu_read(unsigned char reg, signed short * data, int len){
    int i;
    unsigned char v[len*2];
    
    // read multiple from the imu, each data takes 2 reads so you need len*2 chars
    i2c_master_start();
    i2c_master_send(IMU_ADDR);
    i2c_master_send(reg);
    i2c_master_restart();
    i2c_master_send(IMU_ADDR+1);
    for (i = 0; i < len*2-1; i++){
        v[i] = i2c_master_recv();
        i2c_master_ack(0);
    }
    v[13] = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    
    // turn the chars into the shorts
    for (i = 0; i < len; i++){
        data[i] = v[2*i]|(v[2*i+1]<<8);  
    }
}