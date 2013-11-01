/***************************************************************************
 *   Copyright (C) 2013 by Thibault Bouttevin                              *
 *   thibault.bouttevin@gmail.com                                          *
 *   www.legalethurlant.fr.st                                              *
 *                                                                         *
 *   This file is part of SolexTronic                                      *
 *                                                                         *
 *   SolexTronic is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 /**
 * \file simMain.c
 * \brief Main file of the simulation
 * \author Thibault Bouttevin
 * \date June 2013
 *
 * This file includes main function and main loop
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <sys/fcntl.h>
#include <pthread.h>

#include "sim_avr.h"
#include "avr_ioport.h"
#include "avr_timer.h"
#include "avr_adc.h"
#include "sim_elf.h"
#include "sim_gdb.h"
#include "sim_vcd_file.h"
#include "sim_irq.h"
#include "sim_time.h"

#include "uart_pty.h"
#include "pulse_input.h"
#include "analog_input.h"

const char *ptyName = "/tmp/simavr-uart0";
#define RPM_QTY 5
#define SPEED_QTY 5
#define ANALOG_QTY 4

#define RED(msg, ...) fprintf( stdout, "\033[31m" msg "\033[0m", ##__VA_ARGS__ )
#define GREEN(msg, ...) fprintf( stdout, "\033[32m" msg "\033[0m", ##__VA_ARGS__ )
#define HIGH(msg, ...) fprintf( stdout, "\033[1m" msg "\033[0m", ##__VA_ARGS__ )

enum{
    MANUAL = 0,
    ALL_TEST,
    ONE_TEST,
    LIST_TEST,
};

enum{
    TEST_VERSION,
    TEST_RPM,
    TEST_SPEED,
    TEST_ANALOG,
    TEST_HV_SUPPLY,
    TEST_QTY
};

enum{
	ERROR = 0,
	OK = 1,
	FAIL,
	PASS
};

typedef struct{
    int id;
    char name[256];
	int (*testFunction)(void);
}Test_t;



avr_t * avr = NULL;
avr_vcd_t vcd_file;
uart_pty_t uart_pty;
pulse_input_t pulse_input_engine;
pulse_input_t pulse_input_wheel;
analog_input_t analog;
volatile uint8_t	display_pwm = 0;
int fd = 0;	/* access to serial port */

/* Forward declaration */
int TestVersion(void);
int TestRPM(void);
int TestSpeed(void);
int TestAnalog(void);
int TestHvSupply(void);
int TestStub(void);

int testQty;
Test_t testList[] = {
    {TEST_VERSION, "Version", TestVersion},
    {TEST_RPM,     "RPM", TestRPM},
    {TEST_SPEED,   "Vitesse", TestSpeed},
    {TEST_ANALOG,  "Entrees analogiques", TestAnalog},
    {TEST_HV_SUPPLY,  "Alimentation Flyback", TestHvSupply},
};

/********* Helpers ************/
void RPMtoPeriod(const uint32_t rpm, uint32_t *tHigh, uint32_t *tLow)
{
    *tHigh = 100; /* set high duration to 100us */
    *tLow  = 1000000 / (rpm / 60) - *tHigh;
}

void SpeedtoPeriod(const uint32_t speed, uint32_t *tHigh, uint32_t *tLow)
{
    *tHigh = 1000; /* set high duration to 1ms */
    *tLow  = 1000000 / (speed / 3.6 / 1.82) - *tHigh;
}

float BatToVoltage(const float batterie)
{
    return (batterie * 1 / 30.);
}

float TempToVoltage(const float degree) // for LM35 10mV/deg
{
    return (degree * 10 / 1000.);
}

float ThrToVoltage(const float throttle)
{
	//TODO include min/max values
    return (throttle * 5 / 100.);
}

static void pwm_changed_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
	display_pwm = value;
}

int SerialOpen(void)
{
	// from http://www.tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
	struct termios oldtio,newtio;
	int fd_s = 0;

    fd_s = open(ptyName, O_RDWR | O_NOCTTY ); 
    if (fd_s < 0) {perror(ptyName); exit(-1); }
        
    tcgetattr(fd_s, &oldtio); /* save current serial port settings */
    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
    newtio.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL | IGNCR;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;
    tcflush(fd_s, TCIFLUSH);
    tcsetattr(fd_s,TCSANOW,&newtio);
    return fd_s;
}

static int SerialClose(int fd)
{
	close(fd);
    return OK;
}
/************** Tests definition *****************/
/*********** Basic steps *************/
int SleepMs(const int delayms)
{
	// TODO : use simavr timebase to get sleep time in AVR world
    usleep(delayms * 1000);
	return OK;
}

int QueryCommand(char *command, char *answer)
{
	int res;
	char buffer[256];
	struct timeval timeout;
	fd_set readfs;
	
	/* send command */
	printf(">> %s\n", command);
	write(fd, command, strlen(command));
	/* wait for answer : timeout set to 0.5sec */
	timeout.tv_usec = 500000;
	timeout.tv_sec  = 0;
	FD_SET(fd, &readfs);
	res = select(fd + 1, &readfs, NULL, NULL, &timeout);
	if ((res == 0) || (!FD_ISSET(fd, &readfs))) /* number of file descriptors with input = 0, timeout occurred. */
	{
		RED("Serial link timeout !\n");
        return FAIL;
	}else{ /* read answer */
		res = read(fd, buffer, 255);
        buffer[res]=0;
		printf("<< %s\n", buffer);
		strncpy(answer, buffer, 256);
	}
	return OK;
}

int SetArgs(int *args, const int argNum, char *command)
{
    sprintf(command, "%c", args[0]);
    for(int i = 1; i < argNum; i++)
    {
        sprintf(command, "%s %d", command, args[i]);
    }    
    sprintf(command, "%s\r", command);
	return OK;
}

int GetArgs(int *args, int *argNum, char *command)
{
    int8_t str[8];
    int8_t c; 
    uint8_t index = 0, indexRead = 0;
    *argNum = 0;
    // Extract digit str
    while((c = command[indexRead]))
    {
        if(c == '\r')
        {
            str[index++] = 0;
            args[*argNum] = atol((char*)str);
            (*argNum)++;
            return OK;
        }
        else if(c == ' ')
        {
            str[index++] = 0;
            index = 0;
            args[*argNum] = atol((char*)str);
            (*argNum)++;
        }
        else if(isdigit(c))
        {
            str[index++] = c;
        }
        else
        {
            args[*argNum] = c;
            (*argNum)++;
            indexRead++;
        }
        indexRead++;
    }
    return OK;
}


/****** Tests Cases *******/
int TestStub(void)
{
    HIGH("Test bidon !\n");
    return PASS;
}

int TestVersion(void)
{
	/* This test aims to check serial link
		by querying the version and check
		it is correctly formated */

	int args[1] = {'v'};
	int rspArgs[10];
	int rspArgNum;
	char command[256], answer[256];
	
	SetArgs(args, 1, command);
    QueryCommand(command, answer);
	GetArgs(rspArgs, &rspArgNum, answer);

	// criteria : 5 args in response,  1st is "v"
	char message[256];
	sprintf(message, "Version %d.%d, hw %d, hash %x\n", rspArgs[1], rspArgs[2], rspArgs[3], rspArgs[4]);
    printf("%s", message); 
	if((rspArgNum == 5) &&
	    (rspArgs[0] == 'v') &&
		(rspArgs[1] == 0) &&
		(rspArgs[2] == 1) &&
		(rspArgs[3] == 1))
	{
	    GREEN("Version %d.%d, hw %d, hash %x\n", rspArgs[1], rspArgs[2], rspArgs[3], rspArgs[4]);
		return PASS;
	}else{
	    RED("Version %d.%d, hw %d, hash %x\n", rspArgs[1], rspArgs[2], rspArgs[3], rspArgs[4]);
		return FAIL;
	}
	return FAIL;
}

int TestRPM(void)
{
	/* This test aims to check RPM measurement 
	it tries several RPM values and read serial
	command to get measured RPM */

	const float tolerance = 5; //%
	int args[2] = {'g', 12};
	int rspArgs[10];
	int rspArgNum, subTestPassed = 0;
	char command[256], answer[256];
	int rpmTable[RPM_QTY] = {500, 2000, 4000, 6000, 10000};

	SetArgs(args, 2, command);

	for (int i = 0; i < RPM_QTY; i++)
	{
		/* set new RPM */
        HIGH("Test RPM a %d tr/min\n", rpmTable[i]);
		uint32_t high, low;
		RPMtoPeriod(rpmTable[i], &high, &low);
		pulse_input_config(&pulse_input_engine, high, low);
		SleepMs(1000);
		/* query result */
		QueryCommand(command, answer);
		GetArgs(rspArgs, &rspArgNum, answer);
				
		if(rspArgNum != 5)
		{
			RED("Erreur sur le nombre d'arguments : %d recus\n", rspArgNum);
			continue;
		}
		/* criteria : arg #3 (RPM) value is correct */
		int measRpm = rspArgs[3];
		float error = 100 - (100. * measRpm / (float)rpmTable[i]);
		
		if(error > tolerance || error < -tolerance)
		{		
            RED("RPM mesure a %d tr/min : %d, erreur %.1f %% \n", rpmTable[i], measRpm, error);
		}else{
            GREEN("RPM mesure a %d tr/min : %d, erreur %.1f %% \n", rpmTable[i], measRpm, error);
			subTestPassed++;
		}
	}
	printf("subtest %d\n", subTestPassed);
	if(subTestPassed == RPM_QTY)
	{	
		return PASS;
	}
	return FAIL;
}


int TestSpeed(void)
{
	/* This test aims to check speed measurement 
	it tries several speed values and read serial
	command to get measured speed */

	const float tolerance = 5; //%
	int args[2] = {'g', 12};
	int rspArgs[10];
	int rspArgNum, subTestPassed = 0;
	char command[256], answer[256];
	int speedTable[SPEED_QTY] = {10, 20, 40, 60, 90};

	SetArgs(args, 2, command);

	for (int i = 0; i < SPEED_QTY; i++)
	{
        HIGH("Test Vitesse a %d km/h\n", speedTable[i]);
		/* set new speed */
		uint32_t high, low;
		SpeedtoPeriod(speedTable[i], &high, &low);
		pulse_input_config(&pulse_input_wheel, high, low);
		SleepMs(5000);
		/* query result */
		QueryCommand(command, answer);
		GetArgs(rspArgs, &rspArgNum, answer);
				
		if(rspArgNum != 5)
		{
			RED("Erreur sur le nombre d'arguments : %d recus\n", rspArgNum);
			continue;
		}
		/* criteria : arg #4 (Speed) value is correct */
		int measSpeed = rspArgs[4]; // in 1/10 km/h
		float error = 100 - (100. * (measSpeed / 10.) / (float)speedTable[i]);
		
		if(error > tolerance || error < -tolerance)
		{		
            RED("Vitesse mesure a %d km/h : %.1f km/h, erreur %.1f %% \n", speedTable[i], measSpeed/10., error);
		}else{
            GREEN("Vitesse mesure a %d km/h : %.1f km/h, erreur %.1f %% \n", speedTable[i], measSpeed/10., error);
			subTestPassed++;
		}
	}
	
	printf("subtest %d\n", subTestPassed);
	if(subTestPassed == SPEED_QTY)
	{	
		return PASS;
	}
    return FAIL;
}

int TestAnalog(void)
{
	/* This test check the analog inputs
	 * conversions for temperature, throttle
	 * and batterie */

	const float tolerance = 5; //%
	int args[6] = {'g', 6, 100, 100, 100, 100};
	int rspArgs[10];
	int rspArgNum, subTestPassed = 0;
	char command[256], answer[256], message[256];
	float analogTable[ANALOG_QTY][4] = {{100, 20, 20, 0},
                                        {110, 100, 25, 50},
                                        {120, 120, 30, 90},
                                        {150, 180, 35, 100}}; // battery, tempMotor, tempAdm, throttle

    // set conversion ratios to 100%
	//SetArgs(args, 6, command);
    //QueryCommand(command, answer);

    // now the requests
    args[0] = 'g';
    args[1] = 13;
	SetArgs(args, 2, command);

    avr_vcd_start(&vcd_file);
	for (int i = 0; i < ANALOG_QTY; i++)
	{
		/* set new inputs values */
		float voltage[4];
		voltage[0] = BatToVoltage(analogTable[i][0]);
		voltage[1] = TempToVoltage(analogTable[i][1]);
		voltage[2] = TempToVoltage(analogTable[i][2]);
		voltage[3] = ThrToVoltage(analogTable[i][3]);
		for(int j = 0; j < 4; j++)
            analog_input_set_value(&analog, j, voltage[j]);
		SleepMs(1000);
		/* query result */
		QueryCommand(command, answer);
		GetArgs(rspArgs, &rspArgNum, answer);
				
		if(rspArgNum != 7)
		{
			RED("Erreur sur le nombre d'arguments : %d recus\n", rspArgNum);
			continue;
		}
		/* criteria : values are correct */
        printf("Expected values : %.0f %.0f %.0f %.0f\n", analogTable[i][0], analogTable[i][1], analogTable[i][2], analogTable[i][3]);
		float maxError = 0.;
		for(int j = 0; j < 4; j++)
		{
			float error = 100 - (100. * (rspArgs[j+2]) / analogTable[i][j]);
			sprintf(message, "Erreur %d / %.0f %.1f %% \n", (rspArgs[j+2]), analogTable[i][j], error);
            //printf("%s", message);
			if(error < 0) error = -error;
			if(error > maxError) maxError = error;
		}
		if(maxError > tolerance)
		{		
			RED("Test failed\n");
		}else{
			GREEN("Test passed\n");
			subTestPassed++;
		}
	}
	
	if(subTestPassed == ANALOG_QTY)
	{	
		return PASS;
	}
    return FAIL;
}

int TestHvSupply(void)
{
    return FAIL;
}
/************** Core thread **********************/
static void *avr_run_thread(void * oaram)
{
	while (1) {
		int state = avr_run(avr);
		if ( state == cpu_Done || state == cpu_Crashed)
        {
            printf("fin du thread state = %d\n", state);
			break;
        }
	}
	return NULL;
}

/* Print help an exit with exit code exit_msg */
static void printHelp(FILE *stream, int exitMsg, const char* progName)
{
	fprintf(stream,"usage : %s [options] elfFile\n", progName);
	fprintf(stream,"Les options valides sont :\n");
	fprintf(stream,
	"  -h\t\t affiche ce message\n"
	"  -m\t\t mode manuel\n"
	"  -l\t\t liste des tests disponibles\n"
    "  -a\t\t lancement de tous les tests\n"
    "  -t <test>\t\t lancement du test <test>\n"
    );
	exit(exitMsg);
}

/********* Main ************/
int main(int argc, char *argv[])
{
    char elfName[256], testName[256];
    /* read command-line arguments */
    int c;
    int mode = LIST_TEST;

    opterr = 0;
    if(argc == 1)
    {
        printHelp(stdout, EXIT_SUCCESS, argv[0]);
    }
    while ((c = getopt (argc, argv, "hlmat:")) != -1)
    {
        switch (c)
        {
            case 'h': // help
                printHelp(stdout, EXIT_SUCCESS, argv[0]);
                break;
            case 'l': // list of available tests
                for(int i = 0; i < TEST_QTY; i++)
                {
                    printf("Test id %d : %s\n", testList[i].id, testList[i].name);
                }
                exit(EXIT_SUCCESS);
                break;
            case 'm': // manual mode
                mode = MANUAL;
                break;
            case 'a': // run all tests
                mode = ALL_TEST;
                break;
            case 't': // run test <test>
                mode = ONE_TEST;
                strncpy(testName, optarg, 256);
                break;
            case '?':
                if (optopt == 't')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                printHelp(stderr, EXIT_FAILURE, argv[0]);
            default:
                abort();
        }
    }
    if((optind == argc) || (argc - optind > 1))
    {
        fprintf (stderr, "ELF file is mandatory !\n");
        printHelp(stderr, EXIT_FAILURE, argv[0]);
    }
    strncpy(elfName, argv[optind], 256);


	elf_firmware_t f;
	const char * fname = "../solextronic.elf";
	elf_read_firmware(fname, &f);
    
    strcpy(f.mmcu, "atmega328");
    f.frequency = 16000000;
    f.vcc  = 5000;
    f.avcc = 5000;
    f.aref = 5000;
	avr = avr_make_mcu_by_name(f.mmcu);
	if (!avr) {
		fprintf(stderr, "%s: AVR '%s' not known\n", argv[0], f.mmcu);
		exit(1);
	}
	printf("firmware %s f=%d mmcu=%s\n", fname, (int) f.frequency, f.mmcu);

	avr_init(avr);
	avr_load_firmware(avr, &f);

    /* External parts connections */
    uart_pty_init(avr, &uart_pty);
	uart_pty_connect(&uart_pty, '0');
	uint32_t tHigh, tLow;
    RPMtoPeriod(6000, &tHigh, &tLow);
    pulse_input_init(avr, &pulse_input_engine, "Engine", tHigh, tLow);
    SpeedtoPeriod(40, &tHigh, &tLow);
    pulse_input_init(avr, &pulse_input_wheel, "Wheel", tHigh, tLow);
	avr_connect_irq(pulse_input_engine.irq + IRQ_PULSE_OUT, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2));
	avr_connect_irq(pulse_input_wheel.irq  + IRQ_PULSE_OUT, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 3));
    avr_irq_t * i_pwm = avr_io_getirq(avr, AVR_IOCTL_TIMER_GETIRQ('0'), TIMER_IRQ_OUT_PWM1);
	avr_irq_register_notify(i_pwm, pwm_changed_hook, NULL);	
    int   adc_input[4] = {ADC_IRQ_ADC0, ADC_IRQ_ADC1, ADC_IRQ_ADC2, ADC_IRQ_ADC3}; 
    float adc_value[4] = {1, 1, 1, 1}; 
    analog_input_init(avr, &analog, 4, adc_input, adc_value);
	
    
    /**** Manual mode ****/
    if(mode == MANUAL)
    {
		/* VCD files */
		avr_vcd_init(avr, "gtkwave_output.vcd", &vcd_file, 100 /* usec */);
		avr_vcd_add_signal(&vcd_file,
				avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5),
				1 /* bits */, "LED");
		avr_vcd_add_signal(&vcd_file,
				avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 6),
				1 /* bits */, "Image");

		avr_vcd_add_signal(&vcd_file, pulse_input_engine.irq + IRQ_PULSE_OUT, 1, "Pulse_engine");
		avr_vcd_add_signal(&vcd_file, pulse_input_wheel.irq  + IRQ_PULSE_OUT, 1, "Pulse_Wheel");
		avr_vcd_add_signal(&vcd_file, i_pwm, 8 /* bits */, "PWM" );
		avr_vcd_start(&vcd_file);

		pthread_t run;
		pthread_create(&run, NULL, avr_run_thread, NULL);
    	
        while(1)
        {
            sleep(1);
        }
        return 0;
    }

    /**** Automatic mode ****/
	// start AVR core
    /* VCD files */
    avr_vcd_init(avr, "gtkwave_output.vcd", &vcd_file, 10000);
    avr_vcd_add_signal(&vcd_file,
            avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5),
            1, "LED");
    avr_vcd_add_signal(&vcd_file,
            avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), IOPORT_IRQ_PIN_ALL),
            8, "ADMUX");
    avr_vcd_add_signal(&vcd_file,
            avr_io_getirq(avr, AVR_IOCTL_ADC_GETIRQ, ADC_IRQ_OUT_TRIGGER),
            8, "ADMUX_2");
	pthread_t run;
	pthread_create(&run, NULL, avr_run_thread, NULL);
    SleepMs(1000); 
	// Connection to serial port
    fd = SerialOpen();
	
    SleepMs(1000); 
	if(mode == ONE_TEST)
	{
		int testId = 0;
		/* get id of the requested test */
		for(int i = 0; i < TEST_QTY; i++)
		{
			if(strcmp(testName, testList[i].name) == 0){
				testId = i;
				break;
			}
		}	
		//run test
		HIGH("Lancement test %d : %s...\n", testList[testId].id, testList[testId].name);
		int res = (testList[testId].testFunction)();
		if(res == PASS)
		{
			GREEN("Bravo : test OK !\n");
		}else{
			RED("Test KO :-(\n");
		}	
	}
	else if(mode == ALL_TEST)
	{
		int testPassed = 0;
		for(int i = 0; i < TEST_QTY; i++)
		{
			HIGH("Lancement test %d : %s...\n", testList[i].id, testList[i].name);
			int res = (testList[i].testFunction)();
			if(res == PASS) testPassed++;
		}
		if(testPassed == TEST_QTY)
		{
			GREEN("Bravo : tous les tests sont OK !\n");
		}else{
			RED("Seulement %d tests sur %d sont OK :-(\n", testPassed, TEST_QTY);
		}	
	}

    SerialClose(fd);
	return 0;
}