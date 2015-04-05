#include <MKL25Z4.H>
#include "i2c.h"

uint8_t i2c_wait_return=0;
//init i2c0
void i2c_init(void)
{
	//clock i2c peripheral and port E
	SIM->SCGC4 |= SIM_SCGC4_I2C0_MASK;
	SIM->SCGC5 |= (SIM_SCGC5_PORTE_MASK);
	
	//set pins to I2C function
	PORTE->PCR[24] |= PORT_PCR_MUX(5);
	PORTE->PCR[25] |= PORT_PCR_MUX(5);
		
	//set to 100k baud
	//baud = bus freq/(scl_div+mul)
 	//~1.2M = 24M/(20); icr=0x00 sets scl_div to 20+-2
 	I2C0->F = (I2C_F_ICR(0x0) | I2C_F_MULT(0));
	
	//enable i2c and set to master mode
	I2C0->C1 |= (I2C_C1_IICEN_MASK );
}


//send start sequence
void i2c_start()
{
	I2C_TRAN;							/*set to transmit mode */
	I2C_M_START;					/*send start	*/
}

//send device and register addresses
void i2c_read_setup(uint8_t dev, uint8_t address)
{
	begin:
	
	I2C0->D = dev;			  /*send dev address	*/
	i2c_wait_return = i2c_wait();							/*wait for completion */
	if(i2c_wait_return==1)
	{
		goto begin;
	}
	
	I2C0->D =  address;		/*send read address	*/
	i2c_wait_return = i2c_wait();							/*wait for completion */
	if(i2c_wait_return==1)
	{
		goto begin;
	}
	
	I2C_M_RSTART;				   /*repeated start */
	I2C0->D = (dev|0x1);	 /*send dev address (read)	*/
	i2c_wait_return = i2c_wait();							 /*wait for completion */
	if(i2c_wait_return==1)
	{
		goto begin;
	}
	
	I2C_REC;						   /*set to recieve mode */

}

//read a byte and ack/nack as appropriate
uint16_t i2c_repeated_read(uint8_t isLastRead)
{
	uint16_t data;
	if(isLastRead)	{
		NACK;								/*set NACK after read	*/
	} else	{
		ACK;								/*ACK after read	*/
	}
	
	data = I2C0->D;				/*dummy read	*/
	i2c_wait_return = i2c_wait();							/*wait for completion */
	if(i2c_wait_return==1)
		return error;
	
	//Extra code, skipping lower registers
//  if(isLastRead)	{
//		NACK;								/*set NACK after read	*/
//	} else	{
//		ACK;								/*ACK after read	*/
//	}
	
	if(isLastRead)	{
		I2C_M_STOP;					/*send stop	*/
	}
	data = I2C0->D;				/*read data	*/

	return  data;					
}



//////////funcs for reading and writing a single byte
//using 7bit addressing reads a byte from dev:address
uint8_t i2c_read_byte(uint8_t dev, uint8_t address)
{
	uint8_t data;
	
	begin:
	
	I2C_TRAN;							/*set to transmit mode */
	I2C_M_START;					/*send start	*/
	I2C0->D = dev;			  /*send dev address	*/
	i2c_wait_return = i2c_wait();							/*wait for completion */
	if(i2c_wait_return==1)
	{
		goto begin;
	}
	
	I2C0->D =  address;		/*send read address	*/
	i2c_wait_return = i2c_wait();							/*wait for completion */
	if(i2c_wait_return==1)
	{
		goto begin;
	}
	
	I2C_M_RSTART;				   /*repeated start */
	I2C0->D = (dev|0x1);	 /*send dev address (read)	*/
	i2c_wait_return = i2c_wait();							 /*wait for completion */
	if(i2c_wait_return==1)
	{
		goto begin;
	}
	
	I2C_REC;						   /*set to recieve mode */
	NACK;									 /*set NACK after read	*/
	
	data = I2C0->D;					/*dummy read	*/
	i2c_wait_return = i2c_wait();								/*wait for completion */
	if(i2c_wait_return==1)
	{
		goto begin;
	}
	
	I2C_M_STOP;							/*send stop	*/
	data = I2C0->D;					/*read data	*/

	return data;
}



//using 7bit addressing writes a byte data to dev:address
void i2c_write_byte(uint8_t dev, uint8_t address, uint8_t data)
{
	begin:
	
	I2C_TRAN;							/*set to transmit mode */
	I2C_M_START;					/*send start	*/
	I2C0->D = dev;			  /*send dev address	*/
	i2c_wait_return = i2c_wait();						  /*wait for ack */
	if(i2c_wait_return==1)
	{
		goto begin;
	}
	
	I2C0->D =  address;		/*send write address	*/
	i2c_wait_return = i2c_wait();
	if(i2c_wait_return==1)
	{
		goto begin;
	}
	
	I2C0->D = data;				/*send data	*/
	i2c_wait_return = i2c_wait();
	if(i2c_wait_return==1)
	{
		goto begin;
	}
	
	I2C_M_STOP;
	
}

uint16_t cnt1=0;
uint16_t cnt2=0;

uint8_t i2c_wait(void)
{
	cnt1=0;
	while((I2C0->S & I2C_S_IICIF_MASK)==0) 
	{
		cnt1++;
		if(cnt1>150)
		{//do something
			i2c_reset();
			return 1;
			//goto begin;
		}
			
	} 
	I2C0->S |= I2C_S_IICIF_MASK;
	//if(cnt1>cnt2)
	//	cnt2=cnt1;
	
	return 0;
}

void i2c_reset(void)
{
	//Disable I2C Module 	
	I2C0->C1 &= ~(I2C_C1_IICEN_MASK );
	I2C_TRAN;
	I2C_M_START;
	//Enable I2C Module
	I2C0->C1 |= (I2C_C1_IICEN_MASK );
	
	I2C0->C1 |= (I2C_C1_MST_MASK );
	I2C_TRAN;
	I2C0->D = 0xFF;
	while((I2C0->S & I2C_S_IICIF_MASK)==0) {}
	I2C0->S |= I2C_S_IICIF_MASK;
	I2C0->S |= I2C_S_ARBL_MASK;
	
	I2C0->C1 &= ~(I2C_C1_IICEN_MASK );
	I2C_TRAN;
	I2C_M_START;
	//Enable I2C Module
	I2C0->C1 |= (I2C_C1_IICEN_MASK );
}
