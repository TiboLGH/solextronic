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
 * \file computation.c
 * \brief Calcul de l'injection et de l'allumage 
 * \author Thibault Bouttevin
 * \date June 2013
 *
 * Ce programme permet de calculer les caracteristiques de l'injecteur 
 * et/ou de calculer les timings d'injection et d'allumage.
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

#define TABLE_SIZE	10

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
	int rpmTable[TABLE_SIZE];
	int loadTable[TABLE_SIZE];
	int injectorMaxDutyCycle;
	double targetAFR;
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

/* Affichage de l'aide */
static void printHelp(FILE *stream, int exitMsg, const char* progName)
{
	fprintf(stream,"usage : %s [options]\n", progName);
	fprintf(stream,"Les options valides sont :\n");
	fprintf(stream,
	"  -h\t\t affiche ce message\n"
	"  -g\t\t affichage des graphes (avec gnuplot)\n"
	"  -v\t\t affichage des traces de debug\n"
	"  -f\t\t calcul du debit de l'injecteur (calcul des tables de timings par default)\n"
	"  -i <inifile>\t utilisation du fichier <inifile> a la place de computation.ini\n"
	);
	exit(exitMsg);
}

/* From http://pjd-notes.blogspot.fr/2011/09/alternative-to-strtok3-in-c.html
 * find the next word starting at 's', delimited by characters
 * in the string 'delim', and store up to 'len' bytes into *buf
 * returns pointer to immediately after the word, or NULL if done.
 */
char *strwrd(char *s, char *buf, size_t len, char *delim)
{
	s += strspn(s, delim);
	int n = strcspn(s, delim);  /* count the span (spn) of bytes in */
	if (len-1 < n)              /* the complement (c) of *delim */
		n = len-1;
	memcpy(buf, s, n);
	buf[n] = 0;
	s += n;
	return (*s == 0) ? NULL : s;
}

/* Lecture d'une table depuis un fichier CSV */
bool readCsvTable(char* key, char* fileName, double table[TABLE_SIZE][TABLE_SIZE], int *RPM, int *load)
{
	FILE *pf = NULL;
	char line[512];
	char token[12][64];
	char *result = NULL;
	int index = 0;

	if(!fileName || !key || !table)
	{
		RED("Erreur sur les parametres\n");
		return false;
	}
	if(!(pf = fopen(fileName, "rb")))
	{
		RED("Impossible d'ouvrir le fichier %s!\n", fileName);
		return false;
	}
	// Lecture de la 1ere ligne : keyword et load en %
	result = fgets(line, 512, pf);
	if(!result)
	{
		RED("Fichier %s vide\n", fileName);
		fclose(pf);
		return false;
	}
	// interpretation et verif du keyword
	result = strwrd(line, &(token[index][0]), 64, ",");
	if(strncmp(key, &(token[0][0]), 64))
	{
		RED("Fichier %s : cle fausse ! %s / %s\n", fileName, &(token[0][0]), key);
		fclose(pf);
		return false;
	}else{
		//DEBUG("Fichier %s : cle ok ! %s / %s\n", fileName, &(token[0][0]), key);
	}
	index = 1;
	do{
		result = strwrd(result, &(token[index][0]), 64, ",");
	
		//DEBUG("Token %d : %s\n", index, &(token[index][0]));
		index++;
		if(index > TABLE_SIZE + 1)
		{
			RED("Trop de parametres !");
			fclose(pf);
			return false;
		}
	}while(result);
	for(int i=0; i<TABLE_SIZE; i++)
	{
		load[i] = atoi(&(token[i+1][0])); // load values
	}
	
	// lecture des autres lignes
	index = 0;
	int indexRPM = 0;
	while((result = fgets(line, 512, pf)))
	{
		index = 0;
		do{
			result = strwrd(result, &(token[index][0]), 64, ",");

			//DEBUG("Token %d : %s\n", index, &(token[index][0]));
			index++;
			if(index > TABLE_SIZE + 1)
			{
				RED("Trop de parametres !");
				fclose(pf);
				return false;
			}
		}while(result);
		RPM[indexRPM] = atoi(&(token[0][0]));
		for(int i=0; i<TABLE_SIZE; i++)
		{
			table[indexRPM][i] = strtod(&(token[i+1][0]), NULL);
		}
		indexRPM++;
		if(indexRPM > TABLE_SIZE)
		{
			RED("Trop de lignes !");
			fclose(pf);
			return false;
		}
	}

	fclose(pf);
	return true;
}

/* Lecture du fichier ini de config */
bool parseIniFile(char * iniName)
{
    dictionary *ini ;
    char *s;
    int var;
	double vard;

    ini = iniparser_load(iniName);
    if(!ini) {
        fprintf(stderr, "cannot parse file: %s\n", iniName);
        return false;
    }
    //iniparser_dump(ini, stderr);

    /* Lecture attributs table */
    if((s = iniparser_getstring(ini, "table:injectionFile", NULL)))
    {
        strncpy(conf.injTableFile, s, 256);
    }
    if((s = iniparser_getstring(ini, "table:ignitionFile", NULL)))
    {
        strncpy(conf.ignTableFile, s, 256);
    }

    /* Lecture attributs injection */
    var = iniparser_getint(ini, "Injection:Rate", -1);
    conf.injRate = var; 
    var = iniparser_getint(ini, "Injection:OpenTime", -1);
    conf.injOpen = var; 
    var = iniparser_getint(ini, "Injection:Advance", -1);
    conf.injAdv = var; 
    var = iniparser_getint(ini, "Injection:MaxDutyCycle", -1);
    conf.injectorMaxDutyCycle = var; 
    vard = iniparser_getdouble(ini, "Injection:TargetAFR", -1);
    conf.targetAFR = vard; 
    
    /* Lecture attributs allumage */
    var = iniparser_getint(ini, "Ignition:PMHAdvance", -1);
    conf.pmhAdv = var; 
    var = iniparser_getint(ini, "Ignition:Duration", -1);
    conf.ignDuration = var; 


    /* Lecture attributs range */
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

/* Affichage configuration */
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
    NORMAL("\tRatio max de l'injecteur : \t%d %%\n", conf.injectorMaxDutyCycle); 
    NORMAL("\tAFR cible :                \t%.1f\n", conf.targetAFR); 
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
    /* Lecture des arguments */
    int c;
    char iniFileName[256];
    strncpy(iniFileName, "computation.ini", 256);

    opterr = 0;
    while ((c = getopt (argc, argv, "hfvgi:")) != -1)
    {
        switch (c)
        {
            case 'h': // aide
                printHelp(stdout, EXIT_SUCCESS, argv[0]);
                break;
            case 'g': // affichage des graphiques avec gnuplot
                chartOn = true;
                break;
            case 'v': // affichage des traces de debug
                debugTrace = true;
                break;
            case 'f': // mode calcul du debit de l'injecteur
                flowrateComputation = true;
				break;
            case 'i': // remplace le fichier d'ini par default
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

    double injTable[TABLE_SIZE][TABLE_SIZE];
    double ignTable[TABLE_SIZE][TABLE_SIZE];


    /* Lecture de la config et des tables */
    /**************************************/
    parseIniFile("computation.ini");
    dumpConfiguration();
    
	if(!readCsvTable("Ignition", conf.ignTableFile, ignTable, conf.rpmTable, conf.loadTable))
	{
		RED("Impossible de lire le fichier ignition !");
		exit(1);
	}
	HIGH("Table d'avance :");
	char str[256];
	sprintf(str,"\t");
	for(int i=0; i<TABLE_SIZE; i++) sprintf(str,"%s%d\t", str, conf.loadTable[i]);
	printf("%s\n", str);
	for(int j=0; j<TABLE_SIZE; j++)
	{
		sprintf(str,"%d\t", conf.rpmTable[j]);
		for(int i=0; i<TABLE_SIZE; i++) sprintf(str,"%s%.1f\t", str, ignTable[j][i]);
		printf("%s\n", str);
	}
	
	if(!readCsvTable("Injection", conf.injTableFile, injTable, conf.rpmTable, conf.loadTable))
	{
		RED("Impossible de lire le fichier injection !");
		exit(1);
	}
	HIGH("Table d'injection :");
	sprintf(str,"\t");
	for(int i=0; i<TABLE_SIZE; i++) sprintf(str,"%s%d\t", str, conf.loadTable[i]);
	printf("%s\n", str);
	for(int j=0; j<TABLE_SIZE; j++)
	{
		sprintf(str,"%d\t", conf.rpmTable[j]);
		for(int i=0; i<TABLE_SIZE; i++) sprintf(str,"%s%.1f\t", str, injTable[j][i]);
		printf("%s\n", str);
	}

	/* Calculs     */
	/***************/
	if(flowrateComputation) // calcul des debit min et max de l'injecteur
	{

	}else{

		/* Calcul des timings d'allumage */
		/*********************************/

		/* Calcul des timings d'injection */
		/**********************************/
	}

}
