#include "approx.h"
#include "my_math.h"

#if 0 // used for atan2 test function
volatile float acc, app, error;
volatile float x, y;
#endif

float approx_sqrtf(float z) { // from Wikipedia
	int val_int = *(int*)&z; /* Same bits, but as an int */
	const int a = 0x4c000;

  val_int -= 1 << 23; /* Subtract 2^m. */
  val_int >>= 1; /* Divide by 2. */
  val_int += 1 << 29; /* Add ((b + 1) / 2) * 2^m. */
	val_int += a;

	//	val_int = (1 << 29) + (val_int >> 1) - (1 << 22) + a;

	return *(float*)&val_int; /* Interpret again as float */
}


/* ----------------------------- rsqrt ------------------------------ */

/* This is a novel and fast routine for the reciprocal square root of an
IEEE float (single precision). It was communicated to me by Mike Morton,
and has been analyzed substantially by Chris Lomont. Later (12/1/06)
Peter Capek pointed it out to me. See:

http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
http://playstation2-linux.com/download/p2lsd/fastrsqrt.pdf
http://www.beyond3d.com/content/articles/8/

The author of this has been researched but seems to be lost in history.
However, Gary Tarolli worked on it and helped to make it more widely
known, probably while he was at SGI. Gary says it goes back to 1995 or
earlier. */

//	from http://www.hackersdelight.org/hdcodetxt/rsqrt.c.txt

union INF_U {int ix; float x;};

float approx_rsqrt(float x0) {
  union INF_U inf;
	float xhalf;
// union {int ihalf; float xhalf;}; // For alternative halving step.

   inf.x = x0;                      // x can be viewed as int.
// ihalf = ix - 0x00800000;     // Alternative to line below, for x not a denorm.
   xhalf = 0.5f*inf.x;
// ix = 0x5f3759df - (ix >> 1); // Initial guess (traditional),
//                                 but slightly better:
   inf.ix = 0x5f375a82 - (inf.ix >> 1); // Initial guess.
   inf.x = inf.x*(1.5f - xhalf*inf.x*inf.x);    // Newton step.
// x = x*(1.5008908 - xhalf*x*x);  // Newton step for a balanced error.
   return inf.x;
}

/* Notes: For more accuracy, repeat the Newton step (just duplicate the
line). The routine always gets the result too low. According to Chris
Lomont, the relative error is at most -0.00175228 (I get -0.00175204).
Therefore, to cut its relative error in half, making it approximately
plus or minus 0.000876, change the 1.5f in the Newton step to 1.500876f
(1.5008908 works best for me, rel err is +-0.0008911).
   Chris says that changing the hex constant to 0x5f375a86 reduces the
maximum relative error slightly, to 0.00175124. (I get 0.00175128. But
the best I can do is use 5f375a82, which gives rel err = 0 to
-0.00175123). However, using that value seems to usually give a slightly
larger relative error, according to Chris.
*/

	
float approx_atan2f(float y, float x) {
	float a, abs_a, approx, adj=0.0;
	char negate = 0;
	
	if (x == 0) { // special cases
		if (y == 0.0)
			return 0.0; // undefined, but return 0 by convention
		else if (y < 0.0)
			return -M_PI_2;
		else
			return M_PI_2;
	}	else {
		a = y/x; // Can we get rid of this occasionally extra fdiv?
		if (a>1) {
			a = x/y;
			adj = M_PI_2;
			negate = 1;
		} else if (a<-1) {
			a = x/y;
			adj = -M_PI_2;
			negate = 1;
		}
		
		abs_a = (a < 0)? -a : a;
		approx = M_PI_4*a - a*(abs_a - 1)*(0.2447+0.0663*abs_a);
		if (negate) {
			approx = adj - approx;
		}
		
		if (x > 0)
			return approx;
		else if (y >= 0)
			return approx + M_PI;
		else
			return approx - M_PI;
	}		
	
}

#define TESTS (100.0)

#if 0
void test_atan2_approx(void){
	int nx, ny;
	
	for (nx=0; nx<2*TESTS; nx++) {
		x = (nx-TESTS)/TESTS;
		for (ny=0; ny<2*TESTS; ny++) {
			y = (ny-TESTS)/TESTS;
			acc = atan2f(y,x);
			app = approx_atan2f(y,x);
			if (acc != 0) {
				error = fabs((acc-app)/acc);
				Control_RGB_LEDs(error>0.02, error < 0.05, 0);
				if (error > 0.02)
					error += 0.01;
			}
		}
	}
}
#endif
