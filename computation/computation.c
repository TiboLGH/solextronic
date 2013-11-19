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
 * \file compute.c
 * \brief Main file for computation of ignition/injection timings
 * \author Thibault Bouttevin
 * \date June 2013
 *
 * This file includes all the computing core
 *
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "iniparser.h"
#include "gnuplot_i.h"

#define RED(msg, ...) fprintf( stdout, "\033[31m" msg "\n\033[0m", ##__VA_ARGS__ )
#define GREEN(msg, ...) fprintf( stdout, "\033[32m" msg "\n\033[0m", ##__VA_ARGS__ )
#define HIGH(msg, ...) fprintf( stdout, "\033[1m" msg "\n\033[0m", ##__VA_ARGS__ )
#define NORMAL(msg, ...) fprintf( stdout, msg, ##__VA_ARGS__ )
#define DEBUG(msg, ...) if(debugTrace) fprintf( stderr, "\033[1m" msg "\n\033[0m", ##__VA_ARGS__ )

#define TABLE_SIZE	12

bool debugTrace = false;
bool chartOn = false;
bool flowrateComputation = false;
    
typedef struct {
    char ignTableFile[256];
    char injTableFile[256];
    unsigned int injRate;
    unsigned int injOpen;
    int injAdv;
    int pmhAdv;
    unsigned int ignDuration;
    int tempMin;
    int tempStep;
    int tempMax;
    int batMin;
    int batStep;
    int batMax;
}Configuration_t;

Configuration_t conf;

/**** Helper ****/
double DegreeToUs(const double degree, const int RPM)
{
	return 0;
}

double UsToDegree(const double us, const int RPM)
{
	return 0;
}

/* Print help an exit with exit code exit_msg */
static void printHelp(FILE *stream, int exitMsg, const char* progName)
{
	fprintf(stream,"usage : %s [options]\n", progName);
	fprintf(stream,"Les options valides sont :\n");
	fprintf(stream,
	"  -h\t\t affiche ce message\n"
	"  -g\t\t affichage des graphes (avec gnuplot)\n"
	"  -v\t\t affichage des traces de debug\n"
	"  -f\t\t calcul du debit de l'injecteur\n"
	"  -i <inifile>\t utilisation du fichier <inifile> a la place de computation.ini\n"
	);
	exit(exitMsg);
}

/* Read tables from CSV file */
bool readCsvTable(char* fileName, double **table)
{
	return true;
}

/* Read ini file and load parameters table */
bool parseIniFile(char * iniName)
{
    dictionary *ini ;
    char *s;
    int var;

    ini = iniparser_load(iniName);
    if(!ini) {
        fprintf(stderr, "cannot parse file: %s\n", iniName);
        return false;
    }
    //iniparser_dump(ini, stderr);

    /* Get tables attributes */
    if((s = iniparser_getstring(ini, "table:injectionFile", NULL)))
    {
        strncpy(conf.injTableFile, s, 256);
    }
    if((s = iniparser_getstring(ini, "table:ignitionFile", NULL)))
    {
        strncpy(conf.ignTableFile, s, 256);
    }

    /* Get injection attribute */
    var = iniparser_getint(ini, "Injection:Rate", -1);
    conf.injRate = var; 
    var = iniparser_getint(ini, "Injection:OpenTime", -1);
    conf.injOpen = var; 
    var = iniparser_getint(ini, "Injection:Advance", -1);
    conf.injAdv = var; 
    
    /* Get ignition attribute */
    var = iniparser_getint(ini, "Ignition:PMHAdvance", -1);
    conf.pmhAdv = var; 
    var = iniparser_getint(ini, "Ignition:Duration", -1);
    conf.ignDuration = var; 


    /* Get range attribute */
    var = iniparser_getint(ini, "Range:TempMin", -1);
    conf.tempMin = var; 
    var = iniparser_getint(ini, "Range:TempStep", -1);
    conf.tempStep = var; 
    var = iniparser_getint(ini, "Range:TempMax", -1);
    conf.tempMax = var; 
    var = iniparser_getint(ini, "Range:batMin", -1);
    conf.batMin = var; 
    var = iniparser_getint(ini, "Range:batStep", -1);
    conf.batStep = var; 
    var = iniparser_getint(ini, "Range:batMax", -1);
    conf.batMax = var; 
    
    iniparser_freedict(ini);
    return true;
}

/* Dump configuration */
bool dumpConfiguration(void)
{
    HIGH("Configuration");
    HIGH("-------------");
    HIGH("Tables :");
    NORMAL("\tFichier d'avance    :      \t%s\n", conf.ignTableFile);
    NORMAL("\tFichier d'injection :      \t%s\n", conf.injTableFile);
    HIGH("Injection : ");
    NORMAL("\tDebit d'injecteur :        \t%d\n", conf.injRate); 
    NORMAL("\tOuverture de l'injecteur : \t%d us\n", conf.injOpen); 
    NORMAL("\tAvance de l'injection :    \t%d deg\n", conf.injAdv); 
    HIGH("Allumage : ");
    NORMAL("\tAvance du PMH :            \t%d deg\n", conf.pmhAdv); 
    NORMAL("\tDuree de l'allumage :      \t%d us\n", conf.ignDuration); 
    HIGH("Plages de test : ");
    NORMAL("\tTemperature min/pas/max :  \t%d/%d/%d deg\n", conf.tempMin, conf.tempStep, conf.tempMax); 
    NORMAL("\tBatterie min/pas/max :     \t%d/%d/%d v\n", conf.batMin, conf.batStep, conf.batMax); 
    
    return true;
}

/********* Main ************/
int main(int argc, char *argv[])
{
    /* read command-line arguments */
    int c;
    char iniFileName[256];
    strncpy(iniFileName, "computation.ini", 256);

    opterr = 0;
    while ((c = getopt (argc, argv, "hfvgi:")) != -1)
    {
        switch (c)
        {
            case 'h': // help
                printHelp(stdout, EXIT_SUCCESS, argv[0]);
                break;
            case 'g': // display charts with gnuplot
                chartOn = true;
                break;
            case 'v': // display debug traces
                debugTrace = true;
                break;
            case 'f': // compute injector flow rate
                flowrateComputation = true;
				break;
            case 'i': // override default ini file
                strncpy(iniFileName, optarg, 256);
                break;
            case '?':
                if (optopt == 'i')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                printHelp(stderr, EXIT_FAILURE, argv[0]);
            default:
                abort();
        }
    }

    /* Computation variables */
    double injTable[TABLE_SIZE][TABLE_SIZE];
    double ignTable[TABLE_SIZE][TABLE_SIZE];


    /* Read configuration and table files */
    /**************************************/
    parseIniFile("computation.ini");
    dumpConfiguration();
    
    /* Compute ignition curve */
    /**************************/

    /* Compute injection curve */
    /***************************/


}
