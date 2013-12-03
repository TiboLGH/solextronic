EESchema Schematic File Version 2  date dim. 24 nov. 2013 16:21:07 CET
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:testBoard-cache
EELAYER 25  0
EELAYER END
$Descr A4 11700 8267
encoding utf-8
Sheet 1 1
Title "Carte de test pour Solextronic"
Date "24 nov 2013"
Rev "1"
Comp "Le Galet Hurlant (https://code.google.com/p/solextronic/)"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	3050 3100 3050 3300
Wire Wire Line
	2950 3300 2950 2400
Wire Wire Line
	2950 2400 1850 2400
Wire Wire Line
	2150 1100 2150 1800
Wire Wire Line
	2150 1800 1850 1800
Wire Wire Line
	4300 2900 4750 2900
Wire Wire Line
	4750 2900 4750 1550
Connection ~ 4350 1100
Wire Wire Line
	4750 1150 4750 1100
Wire Wire Line
	4750 1100 4350 1100
Wire Wire Line
	3800 1600 1850 1600
Wire Wire Line
	1850 1900 3700 1900
Wire Wire Line
	2300 4500 2300 4600
Wire Wire Line
	3750 4950 1950 4950
Wire Wire Line
	1950 4950 1950 4100
Wire Wire Line
	4900 4400 4900 4500
Wire Wire Line
	4300 4500 4400 4500
Wire Wire Line
	3750 5400 3750 5500
Wire Wire Line
	1850 2700 3700 2700
Wire Wire Line
	1850 2300 3700 2300
Wire Wire Line
	7250 2350 6400 2350
Wire Wire Line
	7250 1550 6400 1550
Connection ~ 7000 1800
Connection ~ 7500 1400
Wire Wire Line
	2650 2000 1850 2000
Wire Wire Line
	2300 2800 1850 2800
Wire Wire Line
	1850 1500 2300 1500
Wire Wire Line
	2300 1500 2300 3250
Connection ~ 2300 2800
Wire Wire Line
	1850 2200 2650 2200
Wire Wire Line
	2650 2200 2650 1100
Connection ~ 2650 2000
Connection ~ 7500 1000
Wire Wire Line
	7500 2200 7500 750 
Connection ~ 7500 1800
Connection ~ 7000 2200
Wire Wire Line
	7000 1000 7000 2450
Connection ~ 7000 1400
Wire Wire Line
	6400 1150 7250 1150
Wire Wire Line
	7250 1950 6400 1950
Wire Wire Line
	3700 2100 1850 2100
Wire Wire Line
	1850 2500 3700 2500
Wire Wire Line
	3700 4100 4750 4100
Connection ~ 3750 4950
Wire Wire Line
	3700 4300 4300 4300
Wire Wire Line
	4300 4300 4300 4500
Wire Wire Line
	3800 4500 3700 4500
Connection ~ 3750 4500
Wire Wire Line
	3750 4500 3750 5000
Wire Wire Line
	1950 4100 2300 4100
Wire Wire Line
	2100 4750 2100 4850
Wire Wire Line
	2300 4350 2100 4350
Wire Wire Line
	4650 4350 4400 4350
Wire Wire Line
	4400 4350 4400 4500
Wire Wire Line
	8150 4300 8150 4450
Wire Wire Line
	8150 4300 8400 4300
Wire Wire Line
	6050 4300 5850 4300
Wire Wire Line
	5850 4700 5850 4800
Wire Wire Line
	6050 4050 5700 4050
Wire Wire Line
	7500 4450 7500 4950
Connection ~ 7500 4450
Wire Wire Line
	7550 4450 7450 4450
Wire Wire Line
	8050 4250 8050 4450
Wire Wire Line
	8050 4250 7450 4250
Connection ~ 7500 4900
Wire Wire Line
	7450 4050 8500 4050
Wire Wire Line
	7500 5350 7500 5450
Wire Wire Line
	8050 4450 8150 4450
Wire Wire Line
	8650 4350 8650 4450
Wire Wire Line
	5700 4050 5700 4900
Wire Wire Line
	5700 4900 7500 4900
Wire Wire Line
	6050 4450 6050 4550
Wire Wire Line
	1850 1700 3700 1700
Wire Wire Line
	4300 1600 4350 1600
Wire Wire Line
	4350 1600 4350 1550
Wire Wire Line
	4350 1050 4350 1150
Wire Wire Line
	3800 2900 1850 2900
Wire Wire Line
	4650 3300 4650 3400
Wire Wire Line
	4650 3400 4800 3400
Wire Wire Line
	4800 3400 4800 3300
Wire Wire Line
	1850 2600 2850 2600
Wire Wire Line
	2850 2600 2850 3300
Wire Wire Line
	2300 3100 2750 3100
Wire Wire Line
	2750 3100 2750 3300
Connection ~ 2300 3100
$Comp
L +5V #PWR01
U 1 1 52921929
P 3050 3100
F 0 "#PWR01" H 3050 3190 20  0001 C CNN
F 1 "+5V" H 3050 3190 30  0000 C CNN
	1    3050 3100
	1    0    0    -1  
$EndComp
$Comp
L CONN_4 P1
U 1 1 52921903
P 2900 3650
F 0 "P1" V 2850 3650 50  0000 C CNN
F 1 "CONN_4" V 2950 3650 50  0000 C CNN
	1    2900 3650
	0    1    1    0   
$EndComp
$Comp
L +5V #PWR02
U 1 1 5280E0C6
P 4800 3300
F 0 "#PWR02" H 4800 3390 20  0001 C CNN
F 1 "+5V" H 4800 3390 30  0000 C CNN
	1    4800 3300
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR03
U 1 1 5280E0A8
P 4650 3300
F 0 "#PWR03" H 4650 3400 30  0001 C CNN
F 1 "VCC" H 4650 3400 30  0000 C CNN
	1    4650 3300
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 5280DFB8
P 4050 2900
F 0 "R2" V 4130 2900 50  0000 C CNN
F 1 "680" V 4050 2900 50  0000 C CNN
	1    4050 2900
	0    -1   -1   0   
$EndComp
$Comp
L LED D2
U 1 1 5280DFA2
P 4750 1350
F 0 "D2" H 4750 1450 50  0000 C CNN
F 1 "Yellow" H 4750 1250 50  0000 C CNN
	1    4750 1350
	0    1    1    0   
$EndComp
$Comp
L R R1
U 1 1 5280DF87
P 4050 1600
F 0 "R1" V 4130 1600 50  0000 C CNN
F 1 "680" V 4050 1600 50  0000 C CNN
	1    4050 1600
	0    -1   -1   0   
$EndComp
$Comp
L +BATT #PWR04
U 1 1 5280DF78
P 4350 1050
F 0 "#PWR04" H 4350 1000 20  0001 C CNN
F 1 "+BATT" H 4350 1150 30  0000 C CNN
	1    4350 1050
	1    0    0    -1  
$EndComp
Text Label 3700 1900 2    60   ~ 0
Vitesse
Text Label 3700 1700 2    60   ~ 0
RPM
$Comp
L LM555N U2
U 1 1 5280DE0C
P 6750 4250
F 0 "U2" H 6750 4350 70  0000 C CNN
F 1 "LM555N" H 6750 4150 70  0000 C CNN
	1    6750 4250
	1    0    0    -1  
$EndComp
Text Label 8500 4050 2    60   ~ 0
Vitesse
$Comp
L C C4
U 1 1 5280DE0B
P 7500 5150
F 0 "C4" H 7550 5250 50  0000 L CNN
F 1 "22uF" H 7550 5050 50  0000 L CNN
	1    7500 5150
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR05
U 1 1 5280DE0A
P 7500 5450
F 0 "#PWR05" H 7500 5450 30  0001 C CNN
F 1 "GND" H 7500 5380 30  0001 C CNN
	1    7500 5450
	1    0    0    -1  
$EndComp
$Comp
L R R4
U 1 1 5280DE09
P 7800 4450
F 0 "R4" V 7880 4450 50  0000 C CNN
F 1 "1k" V 7800 4450 50  0000 C CNN
	1    7800 4450
	0    -1   -1   0   
$EndComp
$Comp
L +5V #PWR06
U 1 1 5280DE08
P 8650 4350
F 0 "#PWR06" H 8650 4440 20  0001 C CNN
F 1 "+5V" H 8650 4440 30  0000 C CNN
	1    8650 4350
	1    0    0    -1  
$EndComp
$Comp
L C C3
U 1 1 5280DE07
P 5850 4500
F 0 "C3" H 5900 4600 50  0000 L CNN
F 1 "10nF" H 5900 4400 50  0000 L CNN
	1    5850 4500
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR07
U 1 1 5280DE06
P 5850 4800
F 0 "#PWR07" H 5850 4800 30  0001 C CNN
F 1 "GND" H 5850 4730 30  0001 C CNN
	1    5850 4800
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR08
U 1 1 5280DE05
P 6050 4450
F 0 "#PWR08" H 6050 4540 20  0001 C CNN
F 1 "+5V" H 6050 4540 30  0000 C CNN
	1    6050 4450
	1    0    0    -1  
$EndComp
$Comp
L POT RV6
U 1 1 5280DE04
P 8400 4450
F 0 "RV6" H 8400 4350 50  0000 C CNN
F 1 "100k" H 8400 4450 50  0000 C CNN
	1    8400 4450
	1    0    0    -1  
$EndComp
$Comp
L POT RV1
U 1 1 5280DDC7
P 4650 4500
F 0 "RV1" H 4650 4400 50  0000 C CNN
F 1 "100k" H 4650 4500 50  0000 C CNN
	1    4650 4500
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR09
U 1 1 5280DD6E
P 2300 4500
F 0 "#PWR09" H 2300 4590 20  0001 C CNN
F 1 "+5V" H 2300 4590 30  0000 C CNN
	1    2300 4500
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR010
U 1 1 5280DD29
P 2100 4850
F 0 "#PWR010" H 2100 4850 30  0001 C CNN
F 1 "GND" H 2100 4780 30  0001 C CNN
	1    2100 4850
	1    0    0    -1  
$EndComp
$Comp
L C C1
U 1 1 5280DD03
P 2100 4550
F 0 "C1" H 2150 4650 50  0000 L CNN
F 1 "10nF" H 2150 4450 50  0000 L CNN
	1    2100 4550
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR011
U 1 1 5280DCF0
P 4900 4400
F 0 "#PWR011" H 4900 4490 20  0001 C CNN
F 1 "+5V" H 4900 4490 30  0000 C CNN
	1    4900 4400
	1    0    0    -1  
$EndComp
$Comp
L R R3
U 1 1 5280DCB9
P 4050 4500
F 0 "R3" V 4130 4500 50  0000 C CNN
F 1 "1k" V 4050 4500 50  0000 C CNN
	1    4050 4500
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR012
U 1 1 5280DC9B
P 3750 5500
F 0 "#PWR012" H 3750 5500 30  0001 C CNN
F 1 "GND" H 3750 5430 30  0001 C CNN
	1    3750 5500
	1    0    0    -1  
$EndComp
$Comp
L C C2
U 1 1 5280DC92
P 3750 5200
F 0 "C2" H 3800 5300 50  0000 L CNN
F 1 "2.2uF" H 3800 5100 50  0000 L CNN
	1    3750 5200
	1    0    0    -1  
$EndComp
Text Label 4750 4100 2    60   ~ 0
RPM
Text Label 3700 2900 2    60   ~ 0
Injection
Text Label 3700 1600 2    60   ~ 0
Allumage
Text Label 3700 2700 2    60   ~ 0
Pression
Text Label 3700 2500 2    60   ~ 0
Tair
Text Label 3700 2300 2    60   ~ 0
Tmoteur
Text Label 3700 2100 2    60   ~ 0
Papillon
Text Label 6400 2350 0    60   ~ 0
Pression
Text Label 6400 1950 0    60   ~ 0
Tair
Text Label 6400 1550 0    60   ~ 0
Tmoteur
Text Label 6400 1150 0    60   ~ 0
Papillon
$Comp
L GND #PWR013
U 1 1 527FB03D
P 7000 2450
F 0 "#PWR013" H 7000 2450 30  0001 C CNN
F 1 "GND" H 7000 2380 30  0001 C CNN
	1    7000 2450
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR014
U 1 1 527FB01F
P 7500 750
F 0 "#PWR014" H 7500 840 20  0001 C CNN
F 1 "+5V" H 7500 840 30  0000 C CNN
	1    7500 750 
	1    0    0    -1  
$EndComp
$Comp
L LED D1
U 1 1 527AC021
P 4350 1350
F 0 "D1" H 4350 1450 50  0000 C CNN
F 1 "RED" H 4350 1250 50  0000 C CNN
	1    4350 1350
	0    1    1    0   
$EndComp
$Comp
L LM555N U1
U 1 1 527ABF79
P 3000 4300
F 0 "U1" H 3000 4400 70  0000 C CNN
F 1 "LM555N" H 3000 4200 70  0000 C CNN
	1    3000 4300
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR015
U 1 1 527ABC4A
P 2650 1100
F 0 "#PWR015" H 2650 1190 20  0001 C CNN
F 1 "+5V" H 2650 1190 30  0000 C CNN
	1    2650 1100
	1    0    0    -1  
$EndComp
$Comp
L +BATT #PWR016
U 1 1 527ABC24
P 2150 1100
F 0 "#PWR016" H 2150 1050 20  0001 C CNN
F 1 "+BATT" H 2150 1200 30  0000 C CNN
	1    2150 1100
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR017
U 1 1 527ABC09
P 2300 3250
F 0 "#PWR017" H 2300 3250 30  0001 C CNN
F 1 "GND" H 2300 3180 30  0001 C CNN
	1    2300 3250
	1    0    0    -1  
$EndComp
$Comp
L POT RV2
U 1 1 527ABBD3
P 7250 1000
F 0 "RV2" H 7250 900 50  0000 C CNN
F 1 "1k" H 7250 1000 50  0000 C CNN
	1    7250 1000
	-1   0    0    1   
$EndComp
$Comp
L POT RV3
U 1 1 527ABBC1
P 7250 1400
F 0 "RV3" H 7250 1300 50  0000 C CNN
F 1 "1k" H 7250 1400 50  0000 C CNN
	1    7250 1400
	-1   0    0    1   
$EndComp
$Comp
L POT RV4
U 1 1 527ABBAA
P 7250 1800
F 0 "RV4" H 7250 1700 50  0000 C CNN
F 1 "1k" H 7250 1800 50  0000 C CNN
	1    7250 1800
	-1   0    0    1   
$EndComp
$Comp
L POT RV5
U 1 1 527ABB9B
P 7250 2200
F 0 "RV5" H 7250 2100 50  0000 C CNN
F 1 "1k" H 7250 2200 50  0000 C CNN
	1    7250 2200
	-1   0    0    1   
$EndComp
$Comp
L DB15 J1
U 1 1 527ABB44
P 1400 2200
F 0 "J1" H 1420 3050 70  0000 C CNN
F 1 "DB15" H 1350 1350 70  0000 C CNN
	1    1400 2200
	-1   0    0    1   
$EndComp
$EndSCHEMATC