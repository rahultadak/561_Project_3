#ifndef MMA8451_H
#define MMA8451_H

#define MMA_ADDR 0x3A

#define REG_XHI 0x01
#define REG_XLO 0x02
#define REG_YHI 0x03
#define REG_YLO 0x04
#define REG_ZHI	0x05
#define REG_ZLO 0x06

#define REG_WHOAMI 0x0D
#define REG_CTRL1  0x2A
#define REG_CTRL2  0x2B
#define REG_CTRL4  0x2D

#define WHOAMI 0x1A

#define COUNTS_PER_G_INV (0.00006103515f)
#define M_PI (3.141593f)
#define D180_OVER_PI (57.295779f)

#define AX_DEF		(3840)
#define AY_DEF		(-2080)
#define AZ_DEF		(15616)
	
int init_mma(void);
void read_full_xyz(void);
void read_xyz(void);
void convert_xyz_to_roll_pitch(void);

void convert_xyz_to_roll_pitch_DEF(void);

extern float roll, pitch, roll_rads, pitch_rads;
extern int16_t acc_X, acc_Y, acc_Z;

#endif
