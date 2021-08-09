#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"

#define CLOCKM 0x200
#define GPIO 0x100
#define PDDR 0x400FF014
#define PDOR 0x400FF000
#define PCR 0x40049000
#define IM_SCGC5 0x40048038

#define ON 0
#define OFF 1

#define LIMIT_VALUE 10
#define MAX_TIME 160.0

void vfnPinMode(unsigned char ubpPort, unsigned char ubPin, unsigned char ubMode);
void vfnDigitalWrite(unsigned char ubpPort, unsigned char ubPin, unsigned char ubData);
unsigned char vfnDigitalRead(unsigned char ubpPort, unsigned char ubPin);
void ADCInit(void);

int main(void) {
	/*
	 * Entrada adc PTC1
	 * */
	// ADC Data Result Register
	ADCInit();
	unsigned int * ADC0_RA;
	ADC0_RA = 0x4003B010;


	//time_t tiempo;
	char limitBroken = 0;
	int actualState = 0;
	long counter;
	long timelapse = 9;

	vfnPinMode('A', 1, 0); //Button configuration

	vfnPinMode('B', 18, 1); // Red
	vfnPinMode('B', 19, 1); //Green
	vfnPinMode('A', 13, 1); //Blue
	vfnDigitalWrite('B', 18, OFF);
	vfnDigitalWrite('B', 19, OFF);
	vfnDigitalWrite('A', 13, OFF);
	while(1){
		ADCInit();
		timelapse++;
		vfnPinMode('A', 1, 0); //Button configuration
		printf("Valor temperatura: %d \n",*ADC0_RA);
		if (*ADC0_RA >= LIMIT_VALUE && !limitBroken){
			limitBroken = 1;
			timelapse=0;
		}else if (*ADC0_RA >= LIMIT_VALUE && timelapse >= MAX_TIME){
			//BOUNCE LED FUNCTION
			//Es mejor llamar la funcion ya hecha en ensamblador-----------------------------------
			while(!vfnDigitalRead('A',1)){
				counter = 1000000;
				//printf("LLegamos a prender los leds\n");
				switch(actualState){
					case 0:
						 //Red
						vfnDigitalWrite('B', 18, ON);
						break;
					case 1:
						//Green
						vfnDigitalWrite('B', 19, ON);
						break;
					case 2:
						//Blue
						vfnDigitalWrite('A', 13, ON);
						break;
					case 3:
						//White
						vfnDigitalWrite('B', 18, ON);
						vfnDigitalWrite('B', 19, ON);
						vfnDigitalWrite('A', 13, ON);
						break;
					case 4:
						//Blue
						vfnDigitalWrite('A', 13, ON);
						break;
					case 5:
						//Green
						vfnDigitalWrite('B', 19, ON);
						break;
					case 6:
						//Red
						vfnDigitalWrite('B', 18, ON);
						actualState = -1;
						break;

				}
				actualState ++;
				while(counter){
					//printf("Counter: %d\n", counter);
					counter--;
				}
				vfnDigitalWrite('B', 18, OFF);
				vfnDigitalWrite('B', 19, OFF);
				vfnDigitalWrite('A', 13, OFF);
			}
		} else if (*ADC0_RA < LIMIT_VALUE){
			limitBroken = 0;
		}
	}
    return 0 ;
}

void vfnPinMode(unsigned char ubpPort, unsigned char ubPin, unsigned char ubMode){
    unsigned int*uwpHelper, *uwpPCR, *uwpPDDR;
    unsigned int uwOffset = (ubpPort - 'A');
    unsigned int uwcClock = CLOCKM;
    unsigned int uwBitHelper = 1;
    uwpHelper = IM_SCGC5;
    *uwpHelper = *uwpHelper | uwcClock<<uwOffset;//enciende el reloj del bloque
    uwpPCR = PCR + (0x1000 * uwOffset) + (ubPin*4);    //se asigna la direccion de memoria inicial para modificar el MUX de cada pin, inicia en pin0
    if (ubMode == 1 || ubMode == 0){
    	*uwpPCR = GPIO;
    }else{
    	*uwpPCR = 0;
    }
    uwpPDDR = PDDR + (64*uwOffset);
    uwBitHelper = uwBitHelper<<ubPin;
    if(ubMode == 1){
        *uwpPDDR = *uwpPDDR | uwBitHelper;
    }else if(ubMode == 0){
        *uwpPDDR = *uwpPDDR & ~uwBitHelper;
    }
    return;
}

void vfnDigitalWrite(unsigned char ubpPort, unsigned char ubPin, unsigned char ubData){
    unsigned int uwOffset = (ubpPort - 'A');
    unsigned int *uwpPDOR;
    unsigned int uwBitHelper = 1;
    uwpPDOR = PDOR + (64*uwOffset);
    uwBitHelper = uwBitHelper<<ubPin;
    if(ubData){
        *uwpPDOR = *uwpPDOR | uwBitHelper;
    }else{
        *uwpPDOR = *uwpPDOR & ~uwBitHelper;
    }
    return;
}
unsigned char vfnDigitalRead(unsigned char ubpPort, unsigned char ubPin){
    unsigned int uwOffset = (ubpPort - 'A');
    unsigned int *uwpPDOR;
    unsigned int uwBitHelper = 1;
    unsigned int returnValue;
    uwpPDOR = PDOR + (64*uwOffset) + 0x10;
    uwBitHelper = uwBitHelper<<ubPin;

    returnValue = *uwpPDOR & uwBitHelper;
    return returnValue? 1:0;
}
void ADCInit(void){
	//vfnPinMode('E', 21, 2);
	unsigned int * p2SIM_SCGC5;
	p2SIM_SCGC5 = 0x40048038;
	*p2SIM_SCGC5 = 0X2000;
	// Se configura SIM_SCGC6
	unsigned int * pSIM6;
	pSIM6 = 0x4004803C;
	*pSIM6 = 0x8000000;
	//PTE 21 as como analogico
	unsigned int * p2PORTE_PCR21;
	p2PORTE_PCR21 = 0x4004D054;
	*p2PORTE_PCR21 = 0x0;
	// Se configura ADC Configuration Register 1
	unsigned int * pADCCR1;
	pADCCR1 = 0x4003B008;
	*pADCCR1 = 0x0;
	 // Se configura ADC Configuration Register 2
	 unsigned int * pADCCR2;
	 pADCCR2 = 0x4003B00C;
	 *pADCCR2 = 0x0;
	 //Se configura ADC Status and Control Registers 1
	 unsigned int * pADCSCR;
	 pADCSCR = 0x4003B000;
	 *pADCSCR = 0x4;
}
