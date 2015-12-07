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
#include <stdbool.h>
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
#include "timing_analyzer.h"
#include "../varDef.h"
#include "../helper.h"

const char *ptyName = "/tmp/simavr-uart0";
#define RPM_QTY 5
#define SPEED_QTY 5
#define ANALOG_QTY 4
#define RETRY_SERIAL 3

#define RED(msg, ...) fprintf(stdout, "\033[31m" msg "\033[0m", ##__VA_ARGS__ )
#define GREEN(msg, ...) fprintf(stdout, "\033[32m" msg "\033[0m", ##__VA_ARGS__ )
#define HIGH(msg, ...) fprintf(stdout, "\033[1m" msg "\033[0m", ##__VA_ARGS__ )
#define V(msg, ...) if(_verbose) fprintf(stdout, msg, ##__VA_ARGS__ )

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
    TEST_IGNAUTO,
    TEST_INJTESTMODE,
    TEST_QTY
};

enum{
    ERROR = 0,
    OK = 1,
    FAIL,
    PASS,
    NOTEST
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
timing_analyzer_t timing_analyzer_injection;
timing_analyzer_t timing_analyzer_ignition;
analog_input_t analog;
volatile uint8_t    display_pwm = 0;
int fd = 0; /* access to serial port */

eeprom_data_t    eData, eDataToWrite;
Current_Data_t   gState;
bool             _verbose = false;
bool             _wave    = false;

/* Forward declaration */
int TestVersion(void);
int TestRPM(void);
int TestSpeed(void);
int TestAnalog(void);
int TestIgnitionAuto(void);
int TestInjectionTestMode(void);
int TestIgnition(void);
int TestStub(void);

int testQty;
Test_t testList[] = {
    {TEST_VERSION,      "Version", TestVersion},
    {TEST_RPM,          "RPM", TestRPM},
    {TEST_SPEED,        "Vitesse", TestSpeed},
    {TEST_ANALOG,       "Analog", TestAnalog},
    {TEST_IGNAUTO,      "AllumageAuto", TestIgnitionAuto},
    {TEST_INJTESTMODE,  "InjectionTestMode", TestInjectionTestMode},
};

/********* Helpers ************/
static void RPMtoPeriod(const uint32_t rpm, uint32_t *tHigh, uint32_t *tLow)
{
    *tHigh = 100; /* set high duration to 100us */
    *tLow  = 1000000 / (rpm / 60) - *tHigh;
}

static void SpeedtoPeriod(const uint32_t speed, uint32_t *tHigh, uint32_t *tLow)
{
    *tHigh = 1000; /* set high duration to 1ms */
    *tLow  = 1000000 / (speed / 3.6 / 1.82) - *tHigh;
}

static float BatToVoltage(const float battery)
{
    return ((battery-7) / 30.);  //-0.7 for protection diode, BattRatio shall be set to 150
}

enum {IAT = 0, CLT};

static float TempToVoltage(const float degree, int type) // 0 IAT, 1 CLT
{
    // Reverse table to get adc value vs temp
    volatile u8 tempTable[TABSIZE][2];
    for(int i=0; i < TABSIZE; i++)
    {
        tempTable[TABSIZE-1 - i][0] = (type == IAT) ? eData.iatCal[i][1] : eData.cltCal[i][1];
        tempTable[TABSIZE-1 - i][1] = (type == IAT) ? eData.iatCal[i][0] : eData.cltCal[i][0];
    }
    u8 adc = Interp1D(tempTable, (u8)degree);
    //u8 temp = Interp1D(eData.iatCal, adc);
    //V("Type %s : %f deg -> %d adc -> %d deg\n", (type == IAT) ? "IAT":"CLT", degree, adc, temp);

    return (adc * 5 / 256.);
}

static float MapToVoltage(const float kpa) // considering linear based on min/max
{
    float voltage = (kpa + eData.map0) * 5 / (eData.map5 - eData.map0) ;
    return voltage;
}

static float ThrToVoltage(const float TPS) // TPS in %
{
    float adc = (eData.tpsMax - eData.tpsMin) * TPS / 100. + eData.tpsMin;
    float voltage = adc * 5/256.;
    return voltage;
}

static void PrintArray(const u8* buffer, const int len)
{
    char str[1024];
    sprintf(str, "Len : %d, 0x", len);
    for(int i = 0; i < len; i++) sprintf(str, "%s %02X", str, *(buffer + i));
    V("%s\n", str);
    return;
}

static void PrettyPrint(const u8* buffer, const int len)
{
    char str[1024];
    sprintf(str, "Len : %d, 0x", len);
    for(int i = 0; i < len; i++) sprintf(str, "%s %d", str, *(buffer + i));
    V("%s\n", str);
    return;
}

static float TimingToAdvance(const uint32_t rpm, const uint32_t timing)
{
    // timing in us
    float period = rpm;//1000000 / (rpm / 60);
    return (360 - (timing / period) * 360);
}

/*********************************************************************/

static int SerialOpen(void)
{
    // from http://www.tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
    struct termios oldtio,newtio;
    int fd_s = 0;

    fd_s = open(ptyName, O_RDWR | O_NOCTTY ); 
    if (fd_s < 0) {perror(ptyName); exit(-1); }

    tcgetattr(fd_s, &oldtio); /* save current serial port settings */
    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
    newtio.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR; // | ICRNL | IGNCR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0; //ICANON;
    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN]  = 1; /* blocking read until 1 char received */
    tcflush(fd_s, TCIFLUSH);
    tcsetattr(fd_s,TCSANOW,&newtio);
    return fd_s;
}

static int SerialClose(int fd_s)
{
    close(fd_s);
    return OK;
}
/************** Tests definition *****************/
/*********** Basic steps *************/
static int SleepMs(const int delayms)
{
    // use simavr timebase to get sleep time in AVR world
    if(!avr)
	return FAIL;
    
    u32 end = avr->cycle + avr_usec_to_cycles(avr, 1000*delayms);
    //wait for end
    while(avr->cycle < end)
    {
	    usleep(10*1000);
    }
    return OK;
}

static int ReadFromSerial(const int nbToRead, u8 *dst)
{
    int res, nbRead = 0;
    u8 buffer[512];
    struct timeval timeout;
    fd_set readfs;

    SleepMs(200);
    timeout.tv_usec = 0;
    timeout.tv_sec  = 1;
    FD_SET(fd, &readfs);
    while(nbRead < nbToRead)
    {
        res = select(fd + 1, &readfs, NULL, NULL, &timeout);
        if ((res == 0) || (!FD_ISSET(fd, &readfs))) /* number of file descriptors with input = 0, timeout occurred. */
        {
            RED("Serial link timeout !\n");
            return FAIL;
        }else{ /* read answer */
            res = read(fd, buffer + nbRead, 255);
            nbRead += res;
            //V("<< res, NbRead : %d, %d\n", res, nbRead);
            //SleepMs(20);
        }
    }
    // buffer fully read : copy to target
    memcpy(dst, buffer, nbToRead);
    //HIGH("ReadFromSerial :\n");
    //PrintArray(buffer, nbToRead);
    return OK;
}

// read config set into eData global
static int ReadConfig(void)
{
    int res;

    const u8 toRead[] = {'r', 0, 0, (u8)(sizeof(eData)%256), (u8)(sizeof(eData)/256)};
    V(">> read conf\n");
    PrintArray(toRead, 5);
    write(fd, toRead, 5);
    res = ReadFromSerial(sizeof(eData), (u8*)&eData);
    if(res != OK) return FAIL;
    return OK;
}

// write config set in eDataToWrite, then read back result to eData
static int WriteConfig(void)
{
    int res;
    eeprom_data_t eDataTmp;

    //write new config
    const u8 toWrite[] = {'w', 0, 0, (u8)(sizeof(eData)%256), (u8)(sizeof(eData)/256)};
    V(">> write new conf\n");
    PrintArray(toWrite, 5);
    write(fd, toWrite, 5);
    // Need to slow down writing to avoid buffer overflow
    int i;
    for(i=0; i < sizeof(eDataToWrite) - 5; i+=5)
    {
        write(fd, (u8*)&eDataToWrite+i, 5);
        SleepMs(50);
    }
    write(fd, (u8*)&eDataToWrite+i, sizeof(eDataToWrite) - i);
    SleepMs(50);

    // read back and compare 
    const u8 toRead[] = {'r', 0, 0, (u8)(sizeof(eData)%256), (u8)(sizeof(eData)/256)};
    write(fd, toRead, 5);
    res = ReadFromSerial(sizeof(eDataTmp), (u8*)&eDataTmp);
    if(res != OK) return FAIL;
    // use memcmp as structures are byte aligned (no padding)
    if(memcmp(&eDataTmp, &eDataToWrite, sizeof(eDataTmp)))
    {
        //TODO investigate
        //RED("Read back eData are corrupted !");
        //return FAIL;
    }
    memcpy(&eData, &eDataTmp, sizeof(eData));
    return OK;
}

static int QueryState(void)
{
    int res, i = 0;

    /* send command */
    do{
        i++;
        //V("\n>> a\n");
        write(fd, "a", 1);
        res = ReadFromSerial(sizeof(gState), (u8*)&gState);
    }while((res != OK) && (i <= RETRY_SERIAL));
    return (res != OK)?FAIL:OK;  
}

static int QuerySignature(u8 *signature, u8 *revision)
{
    int res;

    /* send command */
    V("\n>> S\n");
    write(fd, "S", 1);
    res = ReadFromSerial(32, signature);
    V("%s\n", signature);
    if(res != OK) return FAIL;

    /* send command */
    V("\n>> Q\n");
    write(fd, "Q", 1);
    res = ReadFromSerial(20, revision);
    V("%s\n", revision);
    if(res != OK) return FAIL;

    return OK;
}

// Read/Write retry, simulation is not 100% reliable
static int WriteConfigRetry()
{
    int retry, res;
    for(retry=0; retry < 3;retry++)
    {
        res = WriteConfig();
        if(res == OK) return OK;
    }
    return FAIL;
}

static int ReadConfigRetry()
{
    int retry, res;
    for(retry=0; retry < 3;retry++)
    {
        res = ReadConfig();
        if(res == OK) return OK;
    }
    return FAIL;
}

/****** Tests Cases *******/
int TestStub(void)
{
    HIGH("Test bidon !\n");
    return NOTEST;
}

int TestVersion(void)
{
    /* This test aims to check serial link
       by querying the version and check
       it is correctly formated */
    char signature[33];
    char revision[21];

    int result = QuerySignature((u8*)signature, (u8*)revision);
    revision[20] = '\0';
    signature[32] = '\0';

    if(result != OK)
    {
        RED("Erreur dans la lecture de la signature/revision \n");
        return FAIL;
    }

    char message[256];
    sprintf(message, "Signature :%s\nRevision : %s\n", signature, revision);
    if(strncmp(signature, "** Solextronic v0.2 **     ", 32))
    {
        GREEN("%s\n", message);
        return PASS;
    }else{
        RED("%s\n", message);
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
    int subTestPassed = 0;
    int rpmTable[RPM_QTY] = {500, 2000, 4000, 6000, 10000};


    for (int i = 0; i < RPM_QTY; i++)
    {
        /* set new RPM */
        HIGH("RPM a %d tr/min\n", rpmTable[i]);
        uint32_t high, low;
        RPMtoPeriod(rpmTable[i], &high, &low);
        pulse_input_config(&pulse_input_engine, high, low);
        SleepMs(5000);
        /* query result */
        QueryState();

        float error = 100 - (100. * gState.rpm / (float)rpmTable[i]);

        if(error > tolerance || error < -tolerance)
        {       
            RED("RPM mesure a %d tr/min : %d, erreur %.1f %% \n", rpmTable[i], gState.rpm, error);
        }else{
            GREEN("RPM mesure a %d tr/min : %d, erreur %.1f %% \n", rpmTable[i], gState.rpm, error);
            subTestPassed++;
        }
    }
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
    int subTestPassed = 0;
    int speedTable[SPEED_QTY] = {10, 20, 40, 60, 90};

    for (int i = 0; i < SPEED_QTY; i++)
    {
        HIGH("Vitesse a %d km/h\n", speedTable[i]);
        /* set new speed */
        uint32_t high, low;
        SpeedtoPeriod(speedTable[i], &high, &low);
        pulse_input_config(&pulse_input_wheel, high, low);
        SleepMs(5000);
        /* query result */
        QueryState();

        float error = 100 - (100. * (gState.speed / 10.) / (float)speedTable[i]);

        if(error > tolerance || error < -tolerance)
        {       
            RED("Vitesse mesuree a %d km/h : %.1f km/h, erreur %.1f %% \n", speedTable[i], gState.speed/10., error);
        }else{
            GREEN("Vitesse mesuree a %d km/h : %.1f km/h, erreur %.1f %% \n", speedTable[i], gState.speed/10., error);
            subTestPassed++;
        }
    }

    if(subTestPassed == SPEED_QTY)
    {   
        return PASS;
    }
    return FAIL;
}

int TestAnalog(void)
{
    /* This test check the analog inputs capture and conversions for 
     * temperature, TPS, MAP and battery */

    const float tolerance = 5; //%
    int subTestPassed = 0;
    float analogTable[ANALOG_QTY][5] = {{ 95,  20, 20,  1,  30},
                                        {110, 100, 25, 50,  50},
                                        {120, 120, 30, 90,  90},
                                        {150, 180, 35, 100, 100}}; // battery, CLT, IAT, TPS, MAP

    const float analogMax[5] = { 150, 150, 40,  100, 110}; // battery, CLT, IAT, TPS, MAP
    // Update config
    if(ReadConfigRetry() != OK) 
    {
        RED("Impossible de lire la configuration");
        return FAIL;
    }
    eDataToWrite = eData;
    eDataToWrite.battRatio  = 150;
    eDataToWrite.map0       = 0;
    eDataToWrite.map5       = 110;
    eDataToWrite.tpsMin     = 20;
    eDataToWrite.tpsMax     = 150;
    if(WriteConfigRetry() != OK) return FAIL;

    // now the requests
    V("   Battery  |    CLT   |   IAT   |   TPS    |    MAP   | TPSState\n");
    for (int i = 0; i < ANALOG_QTY; i++)
    {
        /* set new inputs values */
        float voltage[5];
        voltage[0] = BatToVoltage(analogTable[i][0]);
        voltage[1] = TempToVoltage(analogTable[i][1], CLT);
        voltage[2] = TempToVoltage(analogTable[i][2], IAT);
        voltage[3] = ThrToVoltage(analogTable[i][3]);
        voltage[4] = MapToVoltage(analogTable[i][4]);
        for(int j = 0; j < 5; j++)
            analog_input_set_value(&analog, j, voltage[j]);
        SleepMs(1000);
        /* query result */
        QueryState();

        /* criteria : values are correct */
        float maxError = 0.;
        int analogResult[] = {gState.battery, gState.CLT, gState.IAT, gState.TPS, gState.MAP};
        int rawResult[]    = {gState.rawBattery, gState.rawClt, gState.rawIat, gState.rawTps, gState.rawMap};
        char message[256];
        message[0] = '\0';
        for(int j = 0; j < 5; j++)
        {
            // error over the ful dynamic
            float error = 100. * (analogResult[j] - analogTable[i][j]) / analogMax[j];
            sprintf(message, "%s %.0f%% %d %d|", message, error, analogResult[j], rawResult[j]);
            if(error < 0) error = -error;
            if(error > tolerance) 
            {
                maxError = error;
            }
        }
        V("%s 0x%x\n", message, gState.TPSState);
        if(maxError <= tolerance)
        {
            subTestPassed++;
        }
    }

    if(subTestPassed == ANALOG_QTY)
    {   
        return PASS;
    }
    return FAIL;
}

/* Injection test mode
   Enable injection for 1000 cycles and 
   open time of 1000us.
   Check that injection occurs every 10ms for 1000us with no advance/open time
*/
int TestInjectionTestMode(void)
{
    int verdict = PASS;
    const u16 injDuration = 1000;
    const u16 injCycles   = 20;
    const float tolerance = 5; //%
    // 1. Disable RPM generator
    pulse_input_config(&pulse_input_engine, 0, 1000);
    SleepMs(100);
	timing_analyzer_result_t result;
    timing_analyzer_result(&timing_analyzer_injection, &result); // reset stats
    // 2. Set injection parameters
    if(ReadConfigRetry() != OK) return FAIL;
    eDataToWrite = eData;
    eDataToWrite.injTestPW     = injDuration;
    eDataToWrite.injTestCycles = injCycles;
    if(WriteConfigRetry() != OK) return FAIL;
   
    // 3. Measure injection signal timing
    SleepMs(200 * injCycles);
    timing_analyzer_result(&timing_analyzer_injection, &result); // read stats
	V("result.period 			%d\n",result.period);
	V("result.high_duration 	%d\n",result.high_duration);
	V("result.count 			%d\n",result.count);

    // 4. Compare to expected values
    float error = 100 - (100. * result.period / 10000);
    if(error > tolerance || error < -tolerance)
    {       
        RED("Periode mesuree : %d us, erreur %.1f %% \n", result.period, error);
        verdict = FAIL;
    }else{
        GREEN("Periode mesuree : %d us, erreur %.1f %% \n", result.period, error);
    }
    error = 100 - (100. * result.high_duration / injDuration);
    if(error > 2*tolerance || error < -2*tolerance)  // not a critical point
    {       
        RED("Temps d'injection mesure : %d us, erreur %.1f %% \n", result.high_duration, error);
        verdict = FAIL;
    }else{
        GREEN("Temps d'injection mesure : %d us, erreur %.1f %% \n", result.high_duration, error);
    }
    if(result.count != injCycles)
    {       
        RED("Nombre d'injection mesure : %d /%d \n", result.count, injCycles);
        verdict = FAIL;
    }else{
        GREEN("Nombre d'injection mesure : %d /%d \n", result.count, injCycles);
    }
    
    return verdict;
}

int TestIgnitionAuto(void)
{
    int verdict = PASS;
    const u16 ignDuration = 1000;
    const float tolerance = 5; //%
    // 1. Disable RPM generator
    pulse_input_config(&pulse_input_engine, 0, 1000);
    SleepMs(1000);
	timing_analyzer_result_t result;
    timing_analyzer_reset(&timing_analyzer_ignition, 5); //reset stats, discard 5 1st cycles
    // 2. Set ignition test mode
    if(ReadConfigRetry() != OK) return FAIL;
    eDataToWrite = eData;
    eDataToWrite.PMHOffset      = 0;
    eDataToWrite.ignDuration    = ignDuration;
    eDataToWrite.ignTestMode    = 1;
    SleepMs(100);
    if(WriteConfigRetry() != OK) return FAIL;
   
    // 3. Measure ignition signal timing
    SleepMs(2000);
    timing_analyzer_result(&timing_analyzer_ignition, &result); // read stats
	V("result.period            %d\n",result.period);
	V("result.high_duration     %d\n",result.high_duration);
	V("result.count             %d\n",result.count);

    // 4. Compare to expected values
    float error = 100 - (100. * result.period / 10000);
    if(error > tolerance || error < -tolerance)
    {       
        RED("Periode mesuree : %d us, erreur %.1f %% \n", result.period, error);
        verdict = FAIL;
    }else{
        GREEN("Periode mesuree : %d us, erreur %.1f %% \n", result.period, error);
    }
    error = 100 - (100. * result.high_duration / ignDuration);
    if(error > 2*tolerance || error < -2*tolerance)  // not a critical point
    {       
        RED("Temps d'injection mesure : %d us, erreur %.1f %% \n", result.high_duration, error);
        verdict = FAIL;
    }else{
        GREEN("Temps d'injection mesure : %d us, erreur %.1f %% \n", result.high_duration, error);
    }
    eDataToWrite = eData;
    eDataToWrite.ignTestMode    = 0;
    if(WriteConfigRetry() != OK) return FAIL;
    
    return verdict;
}

int TestIgnition(void)
{
    /* This test aims to check ignition timings 
       it tries several RPM values with a given advance table */
    const float tolerance = 5; //%
    const u16 ignDuration = 1000;
    const u8 load = 50; // TODO manage load
    int verdict = PASS;
    int rpmTable[RPM_QTY] = {500, 2000, 4000, 6000, 10000};
	timing_analyzer_result_t result;
    
    // 1. Set ignition parameters and advance table
    if(ReadConfigRetry() != OK) return FAIL;
    eDataToWrite = eData;
    eDataToWrite.ignTestMode    = 0;
    eDataToWrite.PMHOffset      = 0;
    eDataToWrite.ignDuration    = ignDuration;
    //Fill advance table
    for(int i = 0; i < TABSIZE; i++)
    {
        eDataToWrite.rpmBins[i]  = 1000 * (i+1);
        eDataToWrite.loadBins[i] = 10 * (i+1);
        for(int j = 0; j < TABSIZE; j++)
        {
            eDataToWrite.ignTable[i][j] = 4*i+j;
        }
    }
    if(WriteConfigRetry() != OK) return FAIL;

    for (int i = 0; i < RPM_QTY; i++)
    {
        // 2. set new RPM
        HIGH("RPM a %d tr/min\n", rpmTable[i]);
        uint32_t high, low;
        RPMtoPeriod(rpmTable[i], &high, &low);
        pulse_input_config(&pulse_input_engine, high, low);
        SleepMs(1000);
        timing_analyzer_reset(&timing_analyzer_ignition, 5);
        SleepMs(1000 * 10000/rpmTable[i]);
        /* query result */
        QueryState();

        // 3. Measure ignition signal timing
        timing_analyzer_result(&timing_analyzer_ignition, &result); // read stats
        float advance = TimingToAdvance(result.period, result.rising_offset);
        //float advance = TimingToAdvance(rpmTable[i], result.rising_offset);
        int refAdvance = Interp2D(&(eData.ignTable[0][0]), rpmTable[i], load);
        V("result.period            %d\n", result.period);
        V("result.rising_offset     %d\n", result.rising_offset);
        V("result.falling_offset    %d\n", result.falling_offset);
        V("result.high_duration     %d\n", result.high_duration);
        V("result.count             %d\n", result.count);
        V("advance from pin         %.1f\n", advance);
        V("ref advance              %d\n", refAdvance);
        V("advance from module      %d\n", gState.advance);
        V("RPM from module          %d\n", gState.rpm);
        V("Load from module         %d\n", gState.load);

        // 4. Compare to expected values
        float error = 100 - (100. * result.period / (high + low));
        if(error > tolerance || error < -tolerance)
        {       
            RED("Periode mesuree : %d us, erreur %.1f %% \n", result.period, error);
            verdict = FAIL;
        }else{
            GREEN("Periode mesuree : %d us, erreur %.1f %% \n", result.period, error);
        }
        error = 100 - (100. * result.high_duration / ignDuration);
        if(error > tolerance || error < -tolerance)
        {       
            RED("Temps d'allumage mesure : %d us, erreur %.1f %% \n", result.high_duration, error);
            verdict = FAIL;
        }else{
            GREEN("Temps d'allumage mesure : %d us, erreur %.1f %% \n", result.high_duration, error);
        }
        error = advance - refAdvance;
        if(error > 2 || error < -2)
        {       
            RED("Avance mesuree : %.1f deg, erreur %.f deg\n", advance, error);
            verdict = FAIL;
        }else{
            GREEN("Avance mesuree : %.1f deg, erreur %.f deg\n", advance, error);
        }
    }
    return verdict;
}

/************** Core thread **********************/
static void *avr_run_thread(void * oaram)
{
    while (1) {
        int state = avr_run(avr);
        if ( state == cpu_Done || state == cpu_Crashed)
        {
            RED("fin du thread state = %d\n", state);
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
            "  -v\t\t mode verbose\n"
            "  -w\t\t capture waveform dans wave.vcd\n"
            "  -l\t\t liste des tests disponibles\n"
            "  -a\t\t lancement de tous les tests\n"
            "  -t <test>\t\t lancement du test <test>\n"
            "  -r <nbRetry>\t\t nombre de retry en cas de fail. 0 par defaut\n"
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
    int retry = 0;

    opterr = 0;
    if(argc == 1)
    {
        printHelp(stdout, EXIT_SUCCESS, argv[0]);
    }
    while ((c = getopt (argc, argv, "hvlmawr:t:")) != -1)
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
            case 'v': // verbose mode
                _verbose = true;
                break;
            case 'w': // verbose mode
                _wave = true;
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
            case 'r': // number of retry, 0 by default
                retry = atoi(optarg);
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
    V("firmware %s f=%d mmcu=%s\n", fname, (int) f.frequency, f.mmcu);

    avr_init(avr);
    avr_load_firmware(avr, &f);
    /*if(_verbose) 
    {   
        avr->log = LOG_TRACE;
    }*/

    /* External parts connections */
    uart_pty_init(avr, &uart_pty);
    uart_pty_connect(&uart_pty, '0');
    uint32_t tHigh, tLow;
    RPMtoPeriod(6000, &tHigh, &tLow);
    pulse_input_init(avr, &pulse_input_engine, "Engine", 0/*tHigh*/, tLow);
    SpeedtoPeriod(40, &tHigh, &tLow);
    pulse_input_init(avr, &pulse_input_wheel, "Wheel", tHigh, tLow);
    avr_connect_irq(pulse_input_engine.irq + IRQ_PULSE_OUT, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2));
    avr_connect_irq(pulse_input_wheel.irq  + IRQ_PULSE_OUT, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 3));
    int   adc_input[5] = {ADC_IRQ_ADC1, ADC_IRQ_ADC3, ADC_IRQ_ADC6, ADC_IRQ_ADC2, ADC_IRQ_ADC7}; 
    float adc_value[5] = {1, 1, 1, 1, 1}; 
    analog_input_init(avr, &analog, 5, adc_input, adc_value);
    timing_analyzer_init(avr, &timing_analyzer_injection, "Injection");
    avr_connect_irq(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2), timing_analyzer_injection.irq + IRQ_TIMING_ANALYZER_REF_IN);
    avr_connect_irq(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 2), timing_analyzer_injection.irq + IRQ_TIMING_ANALYZER_IN);
    timing_analyzer_init(avr, &timing_analyzer_ignition, "Ignition");
    avr_connect_irq(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2), timing_analyzer_ignition.irq + IRQ_TIMING_ANALYZER_REF_IN);
    avr_connect_irq(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 1), timing_analyzer_ignition.irq + IRQ_TIMING_ANALYZER_IN);
    
    
    /**** Manual mode ****/
    if(mode == MANUAL)
    {
        if(_wave)
        {
            /* VCD files */
            avr_vcd_init(avr, "wave.vcd", &vcd_file, 100 /* usec */);
            avr_vcd_add_signal(&vcd_file,
                    avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5),
                    1 /* bits */, "LED");
            avr_vcd_add_signal(&vcd_file,
                    avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 6),
                    1 /* bits */, "Image");

            avr_vcd_add_signal(&vcd_file, pulse_input_engine.irq + IRQ_PULSE_OUT, 1, "Pulse_engine");
            avr_vcd_add_signal(&vcd_file, pulse_input_wheel.irq  + IRQ_PULSE_OUT, 1, "Pulse_Wheel");
            avr_vcd_start(&vcd_file);
        }

        pthread_t run;
        pthread_create(&run, NULL, avr_run_thread, NULL);
        int speed = 0, rpm = 1000;
        
        while(1)
        {
            sleep(1);
            for(int j = 0; j < 4; j++)
            {
                adc_value[j] += 0.1; 
                if(adc_value[j] > 5) adc_value[j] = 0.0;
                analog_input_set_value(&analog, j, adc_value[j]);
            }
            uint32_t high, low;
            if(speed++ > 100) speed = 0;
            SpeedtoPeriod(speed, &high, &low);
            pulse_input_config(&pulse_input_wheel, high, low);
            if((rpm += 100) > 10000) rpm = 1000;
            RPMtoPeriod(rpm, &high, &low);
            pulse_input_config(&pulse_input_engine, high, low);
        }
        return 0;
    }

    /**** Automatic mode ****/
    // start AVR core
    if(_wave)
    {
        /* VCD files */
        avr_vcd_init(avr, "wave.vcd", &vcd_file, 1000);
        avr_vcd_add_signal(&vcd_file,
                avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5),
                1, "LED");
        avr_vcd_add_signal(&vcd_file,
                avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2),
                1, "RPM");
        avr_vcd_add_signal(&vcd_file,
                avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 2),
                1, "INJ");
        avr_vcd_add_signal(&vcd_file,
                avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 1),
                1, "IGN");
        avr_vcd_add_signal(&vcd_file, pulse_input_wheel.irq  + IRQ_PULSE_OUT, 1, "Pulse_Wheel");
        avr_vcd_add_signal(&vcd_file,
                avr_iomem_getirq(avr, 0x89, "OCR1AH", 8), 8, "OCR1AH");
        avr_vcd_add_signal(&vcd_file,
                avr_iomem_getirq(avr, 0x88, "OCR1AL", 8), 8, "OCR1AL");
        avr_vcd_add_signal(&vcd_file,
                avr_iomem_getirq(avr, 0x8B, "OCR1BH", 8), 8, "OCR1BH");
        avr_vcd_add_signal(&vcd_file,
                avr_iomem_getirq(avr, 0x8A, "OCR1BL", 8), 8, "OCR1BL");
        avr_vcd_add_signal(&vcd_file,
                avr_iomem_getirq(avr, 0x85, "TCNT1H", 8), 8, "TCNT1H");
        avr_vcd_add_signal(&vcd_file,
                avr_iomem_getirq(avr, 0x84, "TCNT1L", 8), 8, "TCNT1L");
        avr_vcd_add_signal(&vcd_file,
                avr_iomem_getirq(avr, 0x4E, "SPDR", 8), 8, "DEBUG");
        avr_vcd_add_signal(&vcd_file,
                avr_iomem_getirq(avr, 0x4D, "SPSR", 8), 8, "PARAM");
        avr_vcd_start(&vcd_file);
    }
    
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
        int retryCount = retry;
        do{
            HIGH("Lancement test %d : %s...\n", testList[testId].id, testList[testId].name);
            int res = (testList[testId].testFunction)();
            if(res == PASS)
            {
                GREEN("Bravo : test OK !\n");
                break;
            }
            else if(res == FAIL)
            {
                RED("Test KO :-(\n");
            }else{
                HIGH("Not run\n");
                break;
            }  
        }while((retryCount--) > 0);
    }
    else if(mode == ALL_TEST)
    {
        int testPassed = 0;
        for(int i = 0; i < TEST_QTY; i++)
        {
            int retryCount = retry;
            do{
                HIGH("Lancement test %d : %s...\n", testList[i].id, testList[i].name);
                int res = (testList[i].testFunction)();
                if(res == PASS)
                {
                    testPassed++;
                    GREEN(" Verdict %s PASS\n", testList[i].name);
                    break;
                }
                else if(res == NOTEST)
                {
                    testPassed++;
                    GREEN(" Verdict %s SKIP\n", testList[i].name);
                    break;
                }else{
                    RED(" Verdict %s FAIL\n", testList[i].name);
                }
            }while((retryCount--) > 0); 
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
