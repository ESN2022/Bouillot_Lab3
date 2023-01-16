#include <system.h>
#include <sys/alt_stdio.h>
#include "opencores_i2c.h"
#include <alt_types.h>
#include <stdio.h>
#include <altera_avalon_pio_regs.h>


#define accel_add 0x1D
#define accel_datax0  0x32
#define accel_datax1  0x33
#define accel_datay0  0x34
#define accel_datay1  0x35
#define accel_dataz0  0x36
#define accel_dataz1  0x37

int data;
int chip_adress = -1;


unsigned int DATAX0, DATAX1, x_unsigned;
unsigned int DATAY0, DATAY1, y_unsigned;
unsigned int DATAZ0, DATAZ1, z_unsigned;

int x_signed, y_signed, z_signed;
int X, Y, Z;

enum axis { X_axis, Y_axis, Z_axis};
float g_range_settings = 2*9.81;  			//By default, the range is + or - 2g

//	7 segments
int aff0 = 0, aff1 = 0, aff2 = 0, aff3 = 0, aff4 = 0, aff5 = 0;
int aff_7seg = 0;
int val = 0;


int comp2(unsigned int value){
	if(value & 0x8000){
		val = -(((~value)& 0xFFFF) + 1);
	}
	else {
		val = value;
	}

	return val;
}

//Fonction de lecture de donnée sur le bus I2C
unsigned int read_data (int reg){
	I2C_start(OPENCORES_I2C_0_BASE,accel_add,0);      //Start bit + slave address + write
	I2C_write(OPENCORES_I2C_0_BASE,reg,0);       			//Registre où on va venir lire la donnée
	I2C_start(OPENCORES_I2C_0_BASE,accel_add,1);      //Start + slave address + read
	data =  I2C_read(OPENCORES_I2C_0_BASE,1);             	//Collect last data

	return data;
}


//Fonction de lecture sur chaque axe
void read_axis(enum axis a){
	int register0, register1;
	int DATA0=0, DATA1=0;
	if (a == X_axis){
		register0 = accel_datax0;
		register1 = accel_datax1;
	}
	else if (a == Y_axis){
		register0 = accel_datay0;
		register1 = accel_datay1;
	}
	else if (a == Z_axis){
		register0 = accel_dataz0;
		register1 = accel_dataz1;
	}
	else {
		alt_printf("Problem\n");
	}

	DATA0 = read_data(register0);
	DATA1 = read_data(register1);

	if (a == X_axis){
		DATAX0 = DATA0;
		DATAX1 = DATA1;
	}
	else if (a == Y_axis){
		DATAY0 = DATA0 ;
		DATAY1 = DATA1;
	}
	else if (a == Z_axis){
		DATAZ0 = DATA0 ;
		DATAZ1 = DATA1;
	}
	else {
		alt_printf("Erreur\n");
	}
}

//Fonction d'affichage sur 7 segments
void aff_7seg_print(int value){
	if (value<0){
		aff5 = 10;
		aff4 = -value /10000;
		aff3 = (-value / 1000) % 10;
		aff2 = (-value /100) % 10;
		aff1 = (-value/10) % 10;
		aff0 = -value % 10;
	}
	else{
		aff5 = 11;
		aff4 = value /10000;
		aff3 = (value / 1000) % 10;
		aff2 = (value /100) % 10;
		aff1 = (value/10) % 10;
		aff0 = value % 10;
	}

	aff_7seg = (aff5 << 20) + (aff4 << 16) +(aff3 << 12) + (aff2 << 8) + (aff1 <<4) + aff0;

	IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE,aff_7seg);
}


//Fonction de calcul sur chaque axe
void axis_calc(enum axis a, unsigned int value0, unsigned int value1){
	int data_unsigned = 0, data_signed = 0, data = 0;

	data_unsigned = (value1 << 8) | value0;

	data_signed = comp2(data_unsigned);

	//conversion en mili g
	data = round(data_signed*4);

	if (a == X_axis){
		x_unsigned = data_unsigned;
		x_signed = data_signed;
		X = data;
	}
	else if (a == Y_axis){
		y_unsigned = data_unsigned;
		y_signed = data_signed;
		Y = data;
	}
	else if (a == Z_axis){
		z_unsigned = data_unsigned;
		z_signed = data_signed;
		Z = data;
	}
	else {
		alt_printf("Problem\n");
	}
}

//Fonction de communication en UART
void UART_print(enum axis a){
	if (a == X_axis){
		printf("x_unsigned : %d\t\t", x_unsigned);
		printf("x_signed : %d\t\t", x_signed);
		printf("X  : %d\n", X);
	}
	else if (a == Y_axis){
		printf("y_unsigned : %d\t\t", y_unsigned);
		printf("y_signed : %d\t\t", y_signed);
		printf("Y  : %d\n", Y);
	}
	else if (a == Z_axis){
		printf("z_unsigned : %d\t\t", z_unsigned);
		printf("z_signed : %d\t\t", z_signed);
		printf("Z  : %d\n", Z);
	}
	else {
		alt_printf("Problem\n");
	}
}


int main(int argc, char *argv[])
{
    I2C_init(OPENCORES_I2C_0_BASE,ALT_CPU_CPU_FREQ,100000);

    data = I2C_start(OPENCORES_I2C_0_BASE,accel_add,0);

    while(1){
		//Lecture de la valeur de l'accéléromètre
        read_axis(X_axis);   
        read_axis(Y_axis); 
        read_axis(Z_axis);        

		//Calcul du complément à 2 et de la valeur en mm/s²
        axis_calc(X_axis, DATAX0, DATAX1);
        axis_calc(Y_axis, DATAY0, DATAY1);
        axis_calc(Z_axis, DATAZ0, DATAZ1);

		//affichage valeurs sur UART
		UART_print(X_axis);
		UART_print(Y_axis);
		UART_print(Z_axis);
		printf("\n");

		//Affichage sur 7 segments
		aff_7seg_print(x_signed);

        usleep(250000);
	}

    return 0;
}