#include <MKL25Z4.H>
#include <math.h>
#include "mma8451.h"
#include "i2c.h"
#include "delay.h"
#include "LEDs.h"
#include "approx.h"
#include "my_math.h"



int16_t acc_X=0, acc_Y=0, acc_Z=0;
float roll_rad=0.0, pitch_rad=0.0;

//mma data ready
extern uint32_t DATA_READY;



//initializes mma8451 sensor
//i2c has to already be enabled
int init_mma()
{
	  //check for device
		if(i2c_read_byte(MMA_ADDR, REG_WHOAMI) == WHOAMI)	{
			
		  Delay(100);
		  //turn on data ready irq; defaults to int2 (PTA15)
		  i2c_write_byte(MMA_ADDR, REG_CTRL4, 0x01);
		  Delay(100);
		  //set active 14bit mode and 100Hz (0x19)
		  i2c_write_byte(MMA_ADDR, REG_CTRL1, 0x01);
				
		  //enable the irq in the NVIC
		  //NVIC_EnableIRQ(PORTA_IRQn);
		  return 1;
		}
		
		//else error
		return 0;
	
}

void read_full_xyz()
{
	int i;
	uint8_t data[6];
	
	i2c_start();
	i2c_read_setup(MMA_ADDR , REG_XHI);
	
	for( i=0;i<6;i++)	{
		if(i==5)
			data[i] = i2c_repeated_read(1);
		else
			data[i] = i2c_repeated_read(0);
	}
	
	acc_X = ((int16_t)(data[0]<<8)) | data[1];
	acc_Y = ((int16_t)(data[2]<<8)) | data[3];
	acc_Z = ((int16_t)(data[4]<<8)) | data[5];
}


void read_xyz(void)
{
	// sign extend byte to 16 bits - need to cast to signed since function
	// returns uint8_t which is unsigned
	acc_X = (int8_t) i2c_read_byte(MMA_ADDR, REG_XHI);
	Delay(100);
	acc_Y = (int8_t) i2c_read_byte(MMA_ADDR, REG_YHI);
	Delay(100);
	acc_Z = (int8_t) i2c_read_byte(MMA_ADDR, REG_ZHI);

}



void convert_xyz_to_roll_pitch(void) {
	// volatile float r2, p2, er, ep, total_error;

#if 0
	roll_rad = atan2f(ay, az);
	pitch_rad = atan2f(ax, sqrtf(ay*ay + az*az));
#else
	roll_rad = approx_atan2f(acc_Y, acc_Z);
	pitch_rad = approx_atan2f(acc_X, approx_sqrtf(acc_Y*acc_Y + acc_Z*acc_Z));
#endif		

	
#if 0	// test accuracy, check for errors
	roll_rad = atan2f(ay, az);
	pitch_rad = atan2f(ax, sqrtf(ay*ay + az*az));
	
	r2 = approx_atan2f(ay, az);
	p2 = approx_atan2f(ax, sqrtf(ay*ay + az*az));
	
	er = (roll_rad-r2)/roll_rad;
	ep = (pitch_rad-p2)/pitch_rad;
	
	total_error = er+ep;
	if (total_error < 0.01) 
		Control_RGB_LEDs(0, 1, 0);
	else if (total_error < 0.05)
		Control_RGB_LEDs(1, 1, 0);
	else if (total_error < 0.10)
		Control_RGB_LEDs(1, 0, 0);
	else 
		Control_RGB_LEDs(1, 0, 1);
#endif
}