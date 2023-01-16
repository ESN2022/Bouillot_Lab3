#include <system.h>
#include <sys/alt_stdio.h>
#include "opencores_i2c.h"


#define accel_add 0x1D


int data;


int main(int argc, char *argv[])
{
    I2C_init(OPENCORES_I2C_0_BASE,ALT_CPU_CPU_FREQ,100000);

    data = I2C_start(OPENCORES_I2C_0_BASE,accel_add,0);

    while(1){
		if ( data == 0){
			alt_printf("Accelerometre trouve a l'adresse: 0x%x\n",accel_add);
			usleep(250000);
		}
		else {
			alt_printf("Probleme d'adressage\n");
			usleep(250000);
		}
	}

    return 0;
}