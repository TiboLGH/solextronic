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
 * \file model.c
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
#ifdef MODEL_STANDALONE
#include "iniparser.h"
#include "gnuplot_i.h"
#endif
#include "model.h"

#define RED(msg, ...) fprintf( stdout, "\033[31m" msg "\n\033[0m", ##__VA_ARGS__ )
#define GREEN(msg, ...) fprintf( stdout, "\033[32m" msg "\n\033[0m", ##__VA_ARGS__ )
#define HIGH(msg, ...) fprintf( stdout, "\033[1m" msg "\n\033[0m", ##__VA_ARGS__ )
#define NORMAL(msg, ...) fprintf( stdout, msg, ##__VA_ARGS__ )
#define V(msg, ...) if(debugTrace) fprintf( stderr, "\033[1m" msg "\n\033[0m", ##__VA_ARGS__ )

#define TABLE_SIZE	10
const double R = 8.3144621;
const double MMAir = 28.965338; // g/mol
const double P = 101325; // pression atm
const double MVfuel = 0.750; // masse volumique essence en g/cm3
const double CYL = 5e-5; // 50cm3 in m3


bool debugTrace          = false;
bool chartOn             = false;
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
	int CID;
}Configuration_t;

Configuration_t conf; /* from ini file */

eeprom_data_t   eData;  /* firmware config */
current_data_t  gState; /* current state of firmware including sensors values */

/**** Helper ****/
void SetVerbosity(bool enable)
{
    debugTrace = enable;
}

double DegreeToUs(const double degree, const int RPM)
{
    double period = 60e6/RPM; //us
	return (360-degree) * period / 360; 
}

double UsToDegree(const double us, const int RPM)
{
    double period = 60e6/RPM; //us
    return (360 - (us / period) * 360);
}

double Interp2DFloating(uint8_t *table, uint16_t rpm, uint8_t load, uint16_t rpmBins[], uint8_t loadBins[])
{
    unsigned int rpmIdx, loadIdx;
  
    /****** 1. Step 1 : locate surrounding values in table  *******/
    /****** 1.1. RPM coordinate lookup *******/
    // Check for limit
    if(rpm > rpmBins[TABSIZE-1])
    {
        // force to max value, no extrapolation 
        rpm = rpmBins[TABSIZE-1];
        rpmIdx = TABSIZE-1;
    }
    else if(rpm < rpmBins[0])
    {
        rpm = rpmBins[0];
        rpmIdx = 0;
    }
    else // in the table
    {
        for(rpmIdx=0; rpmIdx<TABSIZE-1; ++rpmIdx)
        {
            if (rpm >= rpmBins[rpmIdx] && rpm <= rpmBins[rpmIdx+1])
            {
                break;
            }
        }
    }

    /****** 1.2. Load coordinate lookup *******/
    // Check for limit
    if(load > loadBins[TABSIZE-1])
    {
        // force to max value, no extrapolation 
        load = loadBins[TABSIZE-1];
        loadIdx = TABSIZE-1;
    }
    else if(load < loadBins[0])
    {
        load = loadBins[0];
        loadIdx = 0;
    }
    else // in the table
    {
        for(loadIdx=0; loadIdx<TABSIZE-1; ++loadIdx)
        {
            if (load >= loadBins[loadIdx] && load <= loadBins[loadIdx+1])
            {
                break;
            }
        }
    }
  
    /****** 2. Step 2 : interpolation between points of step 1  *******/
    unsigned int rpmDelta = rpmBins[rpmIdx+1] - rpmBins[rpmIdx];
    unsigned int loadDelta = loadBins[loadIdx+1] - loadBins[loadIdx];
    /****** 2.1. First interpolate along rpm axis  *******/
    unsigned int valueLow, valueHigh, delta;

    if(!rpmDelta)
    {
        valueLow  = table[rpmIdx * TABSIZE + loadIdx];
        valueHigh = table[rpmIdx * TABSIZE + loadIdx+1];
    }else{    
        // Compute value low = interpolation along rpm for lower load
        delta = table[(rpmIdx+1) * TABSIZE + loadIdx] - table[rpmIdx * TABSIZE + loadIdx];
        valueLow = table[rpmIdx * TABSIZE + loadIdx] + ((rpm - rpmBins[rpmIdx]) * (long)delta) / rpmDelta;
        // Compute value high = interpolation along rpm for higher load
        delta = table[(rpmIdx+1) * TABSIZE + loadIdx+1] - table[rpmIdx * TABSIZE + loadIdx+1];
        valueHigh = table[rpmIdx * TABSIZE + loadIdx+1] + ((rpm - rpmBins[rpmIdx]) * (long)delta) / rpmDelta;
    }

    /****** 2.2. Second interpolate along load axis  *******/
    if(!loadDelta)
    {
        return (double)valueLow;
    }else{
        delta = valueHigh - valueLow;
        return (double)(valueLow + ((load - loadBins[loadIdx]) * (long)delta) / (double)loadDelta);
    }
}

/******************** Model Implementation ************************/

double ComputeK(double targetAFR, double pAtm)
{
	//K = MMAir.CYL.Patm / AFR.R
	double k = (MMAir * CYL * pAtm*1000) / (targetAFR/10. * R) * 1000;
	return k;
}

int ComputeInjection(eeprom_data_t eData, current_data_t gState, res_t *result)
{
    result->VE = Interp2DFloating(&(eData.injTable[0][0]), gState.rpm, gState.load, &(eData.rpmBins[0]), &(eData.loadBins[0]));   
	// apply formula, K has been computed before (semi-static). QFuel is in mg
	double injQFuel = ComputeK(eData.targetAfr, 100) * result->VE / (gState.IAT + 273);

	// add various enrichments
	int enrich = 0;
    /*if(overheat)
    { 
        enrich += eData.injOverheat;
    }*/
	/*if(intState.afterStartPeriod)
	{
		enrich += gState.injAfterStartEnrich;
		intState.afterStartPeriod--;
	}else{
		gState.injAfterStartEnrich = 0;
	}*/
	
	enrich += gState.injWarmupEnrich;
    
	// TODO set adjustement for acceleration
    
    // Add runtime offset
    enrich += gState.injOffset;
    
	// turn into injector pulse width
	double injPulseWidth = (injQFuel * (100 + enrich)) / (100 * eData.injectorRate) + eData.injectorOpen;
    result->duration = injPulseWidth;
    result->advance  = eData.injAdv;
    result->start    = DegreeToUs(result->advance, gState.rpm);

    return 1;
}

int ComputeIgnition(eeprom_data_t eData, current_data_t gState, res_t *result)
{
    result->VE = Interp2DFloating(&(eData.injTable[0][0]), gState.rpm, gState.load, &(eData.rpmBins[0]), &(eData.loadBins[0]));   
    
    result->duration = eData.ignDuration;
    result->advance  = Interp2DFloating(&(eData.ignTable[0][0]), gState.rpm, gState.load, &(eData.rpmBins[0]), &(eData.loadBins[0]));   
    result->start    = DegreeToUs(result->advance, gState.rpm);

    return 1;
}

double ComputeLoad(eeprom_data_t conf, current_data_t state)
{
    return 50; // TODO : implement load computation model
}

/*******************************************************************/

#ifdef MODEL_STANDALONE

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
	"  -i <inifile>\t utilisation du fichier <inifile> a la place de model.ini\n"
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
		//V("Fichier %s : cle ok ! %s / %s\n", fileName, &(token[0][0]), key);
	}
	index = 1;
	do{
		result = strwrd(result, &(token[index][0]), 64, ",");
	
		//V("Token %d : %s\n", index, &(token[index][0]));
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

			//V("Token %d : %s\n", index, &(token[index][0]));
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
    var = iniparser_getint(ini, "Injection:CID", -1);
    conf.CID = var; 
    
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

/* Affichage tables 2D */
bool Print2DTable(double *table, int *xBins, int *yBins, int xSize, int ySize)
{
	char str[256];
	sprintf(str,"\t");
	for(int i=0; i<xSize; i++) sprintf(str,"%s%d\t", str, *(xBins + i));
	printf("%s\n", str);
	for(int j=0; j<ySize; j++)
	{
		sprintf(str,"%d\t", *(yBins + j));
		for(int i=0; i<TABLE_SIZE; i++) sprintf(str,"%s%.1f\t", str, *(table + j*xSize + i));
		printf("%s\n", str);
	}
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
    NORMAL("\tDebit d'injecteur :        \t%d g/min\n", conf.injRate); 
    NORMAL("\tOuverture de l'injecteur : \t%d us\n", conf.injOpen); 
    NORMAL("\tAvance de l'injection :    \t%d deg\n", conf.injAdv); 
    NORMAL("\tRatio max de l'injecteur : \t%d %%\n", conf.injectorMaxDutyCycle); 
    NORMAL("\tAFR cible :                \t%.1f\n", conf.targetAFR); 
    NORMAL("\tCylindree :                \t%d cm3\n", conf.CID); 
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
    strncpy(iniFileName, "model.ini", 256);

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
    parseIniFile("model.ini");
    dumpConfiguration();
    
	if(!readCsvTable("Ignition", conf.ignTableFile, ignTable, conf.rpmTable, conf.loadTable))
	{
		RED("Impossible de lire le fichier ignition !");
		exit(1);
	}
	HIGH("Table d'avance :");
	Print2DTable((double *)ignTable, conf.loadTable, conf.rpmTable, TABLE_SIZE, TABLE_SIZE);
	
	if(!readCsvTable("Injection", conf.injTableFile, injTable, conf.rpmTable, conf.loadTable))
	{
		RED("Impossible de lire le fichier injection !");
		exit(1);
	}
	HIGH("Table d'injection :");
	Print2DTable((double *)injTable, conf.loadTable, conf.rpmTable, TABLE_SIZE, TABLE_SIZE);

	/* Calculs     */
	/***************/
	if(flowrateComputation) // calcul des debit min et max de l'injecteur
	{
		/* Principe de calcul :
		 * Pour chaque point du tableau des RPMs/charge, on calcule
		 * la quantite de carburant necessaire en considerant le
		 * remplissage donne dans la table des VE(RPM, load) pour un 
		 * ratio air/essence donne par TargetAFR.
		 * A partir de cette quantitee d'essence, on calcule le debit 
		 * de l'injecteur pour respecter les conditions suivantes :
		 *  - temps d'injection max = 40% cycle (remontee piston)
		 *  - temps mini > temps d'ouverture 
		 */
		
		// Calcul de la quantite de carburant a injecter pour chaque cycle
		double fuel[TABLE_SIZE][TABLE_SIZE];
		for(int i = 0; i < TABLE_SIZE; i++)
		{
			for(int j = 0; j < TABLE_SIZE; j++)
			{
				/* PV = nRT
				 * n = PV / (RT)
				 * Mair = n * MMAir
				 * Mfuel = Mair / AFR
				 * P : on considere la pression atmospherique 1015hPa
				 * V : volume = cylindree * remplissage
				 * T : temperatures min et max sont testees 
				 */
				double Mair = MMAir * P * injTable[i][j]/101. * (conf.CID/1e6) / (R * (conf.tempMax + 273)); // en gramme
				fuel[i][j] = Mair / conf.targetAFR;
			}
		}
		/* calcul du debit en fct du temps d'injection et de la quantite de carburant
		 * flowrate = fuel / Tinj;
		 * Tinj = 40% * cycle
		 */
		double flowrate[TABLE_SIZE][TABLE_SIZE];
		for(int i = 0; i < TABLE_SIZE; i++)
		{
			for(int j = 0; j < TABLE_SIZE; j++)
			{
				double Tinj = 0.4 * 60000. / (double)conf.rpmTable[i] - conf.injOpen/1000.; //en msec
				flowrate[i][j] = fuel[i][j] / Tinj * 1000 * 60; // en g/min
				//flowrate[i][j] /= MVfuel; // en cm3/min
			}
		}
		HIGH("Table de debits (g/min) :");
		Print2DTable((double *)flowrate, conf.loadTable, conf.rpmTable, TABLE_SIZE, TABLE_SIZE);

	}else{

		/* Calcul des timings d'allumage
		 * -----------------------------
		 * Principe de calcul :
		 * pour chaque point de la table d'avance a l'allumage,
		 * on calcule le timing de declenchement de l'etincelle
		 * (i.e. l'allumage lui-meme) et le timing de relachement de 
		 * la commande (dead time utilise pour bien desamorcer le 
		 * thyristor)
		 * Ces timings sont le delai APRES le PMH */
		double spark[TABLE_SIZE][TABLE_SIZE];
		double deadTime[TABLE_SIZE][TABLE_SIZE];
		for(int i = 0; i < TABLE_SIZE; i++)
		{
			for(int j = 0; j < TABLE_SIZE; j++)
			{
				spark[i][j] =  DegreeToUs(360. - ignTable[i][j], conf.rpmTable[i]); // en us
				deadTime[i][j] = spark[i][j] + conf.ignDuration; // en us
			}
		}
		HIGH("Table des avances (us) :");
		Print2DTable((double *)spark, conf.loadTable, conf.rpmTable, TABLE_SIZE, TABLE_SIZE);
		HIGH("Table des dead time (us) :");
		Print2DTable((double *)deadTime, conf.loadTable, conf.rpmTable, TABLE_SIZE, TABLE_SIZE);
		// TODO : ecrire les resultats dans un fichier

		/* Calcul des timings d'injection
		 * ------------------------------
		 * Principe de calcul :
		 * pour chaque point de la table de VE, on calcule 
		 * la quantitee de carburant a injecter et donc le temps d'injection
		 * A partir de la, on calcule le timing de debut d'injection pour etre 
		 * centre sur le point d'avance a l'injection (= periode avec le meilleur
		 * flux d'air d'admission)
		 */
		double duration[TABLE_SIZE][TABLE_SIZE];
		double injAvance[TABLE_SIZE][TABLE_SIZE];
		for(int i = 0; i < TABLE_SIZE; i++)
		{
			for(int j = 0; j < TABLE_SIZE; j++)
			{
				spark[i][j] =  DegreeToUs(360. - ignTable[i][j], conf.rpmTable[i]); // en us
				deadTime[i][j] = spark[i][j] + conf.ignDuration; // en us
			}
		}
		HIGH("Duree d'injection (us) :");
		Print2DTable((double *)duration, conf.loadTable, conf.rpmTable, TABLE_SIZE, TABLE_SIZE);
		HIGH("Avance d'injection (us) :");
		Print2DTable((double *)injAvance, conf.loadTable, conf.rpmTable, TABLE_SIZE, TABLE_SIZE);
		// TODO : ecrire les resultats dans un fichier

	}

}
#endif //MODEL_STANDALONE
