#include <MKL25Z4.H>
#include "mma8451.h"
#include "i2c.h"
#include "delay.h"
#include <math.h>

int16_t acc_X=0, acc_Y=0, acc_Z=0;
float roll=0.0, pitch=0.0, roll_rads = 0.0, pitch_rads = 0.0;
float d180_over_pi = 57.295779f;

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
		  
			//Enabling Auto Sleep feature, with low power mode
			i2c_write_byte(MMA_ADDR, REG_CTRL2, 0x1F);
			//Setting Auto Sleep Wake up frequency to 12.5Hz
		  i2c_write_byte(MMA_ADDR, REG_CTRL1, 0x6B);
				
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
	uint16_t data[3];
	int16_t temp[3];
	
	begin:
	
	i2c_start();
	i2c_read_setup(MMA_ADDR , REG_XHI);
	
	for( i=0;i<3;i++)	{
		if(i==2)
		{
			data[i] = i2c_repeated_read(1);
			if(data[i] == error)
					goto begin;
		}
		else
		{
			data[i] = i2c_repeated_read(0);
			if(data[i] == error)
					goto begin;
		}
	}
	
	temp[0] = (int16_t)((data[0]<<8));// | (data[1]<<2));
	temp[1] = (int16_t)((data[1]<<8));// | (data[3]<<2));
	temp[2] = (int16_t)((data[2]<<8));// | (data[5]<<2));
	
	acc_X = temp[0];
	acc_Y = temp[1];
	acc_Z = temp[2];
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
	float ax = acc_X*COUNTS_PER_G_INV,
				ay = acc_Y*COUNTS_PER_G_INV,
				az = acc_Z*COUNTS_PER_G_INV;
	
	pitch_rads = asinf(ax);
	//arm_mult_f32(&pitch_rads,&d180_over_pi,&pitch,1);
	//pitch = pitch_rads*D180_OVER_PI;
	roll_rads = atan2f(ay, az);
	//arm_mult_f32(&roll_rads,&d180_over_pi,&roll,1);
	//roll = roll_rads*D180_OVER_PI;	
}

void convert_xyz_to_roll_pitch_DEF(void)
{
	float ax = AX_DEF*COUNTS_PER_G_INV,
				ay = AY_DEF*COUNTS_PER_G_INV,
				az = AZ_DEF*COUNTS_PER_G_INV;
	
	pitch_rads = asinf(ax);
	pitch = pitch_rads*D180_OVER_PI;
	roll_rads = atan2f(ay, az);
	roll = roll_rads*D180_OVER_PI;	
}
