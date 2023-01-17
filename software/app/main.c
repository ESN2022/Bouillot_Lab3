#include <system.h>
#include <sys/alt_stdio.h>
#include <unistd.h>
#include "opencores_i2c.h"
#include <alt_types.h>
#include <stdio.h>
#include <altera_avalon_pio_regs.h>
#include <math.h>
#include <sys/alt_irq.h>
#include <alt_types.h>
#include <altera_avalon_timer_regs.h>
#include <altera_avalon_timer.h>


#define ACCEL_address 0x1D

#define ACCEL_OFSX  	0x1E
#define ACCEL_OFSY  	0x1F
#define ACCEL_OFSZ  	0x20

#define ACCEL_DATAX0  0x32
#define ACCEL_DATAX1  0x33
#define ACCEL_DATAY0  0x34
#define ACCEL_DATAY1  0x35
#define ACCEL_DATAZ0  0x36
#define ACCEL_DATAZ1  0x37


//Variables
int chip_adress = -1;
int data_i2c = 0;

//Variable de position et d'accélération de chaque axe
unsigned int DATAX0 = 0, DATAX1 = 0, x_unsigned = 0;
unsigned int DATAY0 = 0, DATAY1 = 0, y_unsigned = 0;
unsigned int DATAZ0 = 0, DATAZ1 = 0, z_unsigned = 0;
int x_signed = 0, y_signed = 0, z_signed = 0;
int X = 0, Y = 0, Z = 0;
enum axis { X_axis, Y_axis, Z_axis};

//Variables de calculs
float mg_LSB = 4.0;						//Valeur permettant la conversion en mili g (voir datasheet)
int val = 0;

//Afficheur 7 segments
int aff0 = 0, aff1 = 0, aff2 = 0, aff3 = 0, aff4 = 0, aff5 = 0;
int aff_7seg = 0;

//Variable d'interruption du timer
int t_interrupt = 0;

//Variable d'interruption du bouton poussoir
int print_on_7seg = 0;

//	Valeurs d'offset
int X_offset = 2; //en calculant on trouve que l'accelerometre a un drift de -2 sur x
int Y_offset = 0; //en calculant on trouve que l'accelerometre ne possede pas de drift sur y
int Z_offset = 5; //en calculant on trouve que l'accelerometre possede un drift de -5 sur z


//Routine d'interruption générée par le bouton poussoir
static void bouton_interrupt (void * context, alt_u32 id)
{
	//Choix de l'axe selon appui sur le bouton KEY 1 (0 => X ; 1 => Y; 2 => Z)
	if (print_on_7seg < 2){
		print_on_7seg = print_on_7seg + 1;
	}
	else{
		print_on_7seg = 0;
	}
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_1_BASE,0x01);
}

//Initialisation des interruptions du bouton poussoir
void bouton_interrupt_init(){
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(PIO_1_BASE, 0x01);
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_1_BASE,0x01);
	alt_irq_register (PIO_1_IRQ, NULL, (void*) bouton_interrupt);
}

//Routine d'interruption générée par le timer 0
static void timer_interrupt (void * context, alt_u32 id)
{
    t_interrupt = 1;
	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE, 0x00);
}

//Initialisation du timer 0
void timer_interrupt_init(){
	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE, 0x00);
	IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE , ALTERA_AVALON_TIMER_CONTROL_CONT_MSK | ALTERA_AVALON_TIMER_CONTROL_START_MSK | ALTERA_AVALON_TIMER_CONTROL_ITO_MSK);
	alt_irq_register (TIMER_0_IRQ , NULL, (void*) timer_interrupt);
}

//Function de conversion de signé en complément à 2
int comp2(unsigned int value){
	if(value & 0x8000){                         //If the MSB is 1 (negative number)
		val = -(((~value) & 0xFFFF) + 1);    //2-complement on 16 bits
	}
	else {
		val = value;
	}

	return val;
}

//Fonction de lecture sur le bus I2C
unsigned int read_data(int reg){
	I2C_start(OPENCORES_I2C_0_BASE,ACCEL_address,0);      //Start bit + slave address + write
	I2C_write(OPENCORES_I2C_0_BASE,reg,0);       			//Register to read on
	I2C_start(OPENCORES_I2C_0_BASE,ACCEL_address,1);      //Start + slave address + read bit
	data_i2c =  I2C_read(OPENCORES_I2C_0_BASE,1);           //Collect last data (stop bit)

	return data_i2c;
}

//Fonction d'écriture sur le bus I2C
void write_data(int reg, int data){
	I2C_start(OPENCORES_I2C_0_BASE,ACCEL_address,0);      //Start bit + slave address + write bit
	I2C_write(OPENCORES_I2C_0_BASE,reg,0);       			//Register to write in
	I2C_write(OPENCORES_I2C_0_BASE,data,1);       			//Write data + stop bit
}

//Fonction de lecture des axes de l'accéléromètre
void read_axis(enum axis a){
	int register0 = 0, register1 = 0;
	int DATA0 = 0, DATA1 = 0;
	if (a == X_axis){
		register0 = ACCEL_DATAX0;
		register1 = ACCEL_DATAX1;
	}
	else if (a == Y_axis){
		register0 = ACCEL_DATAY0;
		register1 = ACCEL_DATAY1;
	}
	else if (a == Z_axis){
		register0 = ACCEL_DATAZ0;
		register1 = ACCEL_DATAZ1;
	}
	else {
		alt_printf("Probleme sur la fonction read_axis\n");
	}

	//Lecture des LSB et MSB
	DATA0 = read_data(register0);
	DATA1 = read_data(register1);

	//Assignement de la valeur lue dans le registre correspondant
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
		alt_printf("Probleme sur la fonction read_axis\n");
	}
}

//Fonction de calcul et de conversion des valeurs renvoyées par l'accéléromètre
void axis_calc(enum axis a, unsigned int value0, unsigned int value1){
	int data_unsigned = 0, data_signed = 0, data_mg = 0;

	data_unsigned = (value1 << 8) | value0;

	data_signed = comp2(data_unsigned);

	data_mg = round( data_signed * mg_LSB );	//Conversion

	//Assignement des données dans les variables globales correspondantes
	if (a == X_axis){
		x_unsigned = data_unsigned;
		x_signed = data_signed;
		X = data_mg;
	}
	else if (a == Y_axis){
		y_unsigned = data_unsigned;
		y_signed = data_signed;
		Y = data_mg;
	}
	else if (a == Z_axis){
		z_unsigned = data_unsigned;
		z_signed = data_signed;
		Z = data_mg;
	}
	else {
		alt_printf("Probleme sur la fonction axis_calc\n");
	}
}

//Fonction d'écriture des valeurs d'offset dans les registres d'offset
void set_offset(enum axis a, int value){
	int reg_offset;

	//Assign the register to set the offset, based on the chosen axis
	if (a == X_axis){
		reg_offset = ACCEL_OFSX;
	}
	else if (a == Y_axis){
		reg_offset = ACCEL_OFSY;
	}
	else if (a == Z_axis){
		reg_offset = ACCEL_OFSZ;
	}
	else {
		alt_printf("Problem set_offset\n");
	}
    write_data(reg_offset,value);
}

//Fonction d'envoie des données via UART pour le debug en JTAG
void UART_print(enum axis a){
	if (a == X_axis){
		printf("X : %d ", X);
		//printf("x_unsigned : %d\t\t", x_unsigned);
		//printf("x_signed : %d\t\t", x_signed);
	}
	else if (a == Y_axis){
		printf("Y : %d ", Y);
		//printf("y_unsigned : %d\t\t", y_unsigned);
		//printf("y_signed : %d\t\t", y_signed);
	}
	else if (a == Z_axis){
		printf("Z : %d\n", Z);
		//printf("z_unsigned : %d\t\t", z_unsigned);
		//printf("z_signed : %d\t\t", z_signed);
	}
	else {
		alt_printf("Probleme sur la fonction UART_print\n");
	}
}

//Fonction permettant de voir les registres d'offset
void offset_print(){
	alt_printf("X offset: %x\t;",read_data(ACCEL_OFSX));
    alt_printf("\tY offset: %x\t",read_data(ACCEL_OFSY));
    alt_printf("\tZ offset: %x\n",read_data(ACCEL_OFSZ));
}

//Fonction d'affichage sur les afficheurs 7 segments
void aff_7seg_print(int value){
	if (value<0){
		aff5 = 10;               //Correspond au signe négatif dans le fichier bin_to_7seg.vhd
		aff4 = -value /10000;
		aff3 = (-value / 1000) % 10;
		aff2 = (-value /100) % 10;
		aff1 = (-value/10) % 10;
		aff0 = -value % 10;
	}
	else{
		aff5 = 11;				//Correspond à l'extinction du cadrant pour les nombres positifs
		aff4 = value /10000;
		aff3 = (value / 1000) % 10;
		aff2 = (value /100) % 10;
		aff1 = (value/10) % 10;
		aff0 = value % 10;
	}

	aff_7seg = (aff5 << 20) + (aff4 << 16) +(aff3 << 12) + (aff2 << 8) + (aff1 <<4) + aff0;
	//Ecriture sur le PIO
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE,aff_7seg);
}

int main(int argc, char *argv[])
{
	//I2C initialisation
    I2C_init(OPENCORES_I2C_0_BASE,ALT_CPU_CPU_FREQ,100000);

	//Search ADXL345
    chip_adress = I2C_start(OPENCORES_I2C_0_BASE,ACCEL_address,0);
    if ( chip_adress == 0){
        alt_printf("ADXL345 found at the address : 0x%x\n", ACCEL_address);
    }
    else if ( chip_adress == 1){
        alt_printf("ADXL345 not found\n", ACCEL_address);
    }
    else {
        alt_printf("Communication pb\n");
    }

    //Initialisation des périphériques d'interruption
	bouton_interrupt_init();
	timer_interrupt_init();
	
	
	//Mise a niveau des axes
	offset_print();
    set_offset(X_axis, X_offset);
    set_offset(Y_axis, Y_offset);
    set_offset(Z_axis, Z_offset);
    offset_print();


    while(1){
		if (t_interrupt == 1) {

			read_axis(X_axis);
			read_axis(Y_axis);
			read_axis(Z_axis);

			axis_calc(X_axis, DATAX0, DATAX1);
			axis_calc(Y_axis, DATAY0, DATAY1);
			axis_calc(Z_axis, DATAZ0, DATAZ1);

			UART_print(X_axis);
			UART_print(Y_axis);
			UART_print(Z_axis);
			printf("\n");

			//Affichage sur les afficheurs 7 segments
			if (print_on_7seg == 0){
				aff_7seg_print(X);
			}
			else if (print_on_7seg == 1){
				aff_7seg_print(Y);
			}
			else {
				aff_7seg_print(Z);
			}
			t_interrupt = 0;
		}
	}

    return 0;
}
