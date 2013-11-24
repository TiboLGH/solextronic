EESchema Schematic File Version 2  date dim. 24 nov. 2013 16:17:07 CET
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
LIBS:solextronic
LIBS:arduino
LIBS:solextronic-cache
EELAYER 25  0
EELAYER END
$Descr A4 11700 8267
encoding utf-8
Sheet 1 1
Title "Solextronic"
Date "24 nov 2013"
Rev "1.0"
Comp "Le Galet Hurlant"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Label 9550 1150 0    60   ~ 0
Injection
Text Label 9550 1350 0    60   ~ 0
Pressure
Text Label 9550 1950 0    60   ~ 0
Throttle
Text Label 9550 2150 0    60   ~ 0
TopWheel
Text Label 9550 2450 0    60   ~ 0
Ignition
Wire Wire Line
	10100 2250 10350 2250
Wire Wire Line
	10350 2350 9550 2350
Wire Wire Line
	10350 2150 9550 2150
Wire Wire Line
	10350 1150 9550 1150
Wire Wire Line
	10350 1550 9550 1550
Wire Wire Line
	10150 950  10150 2050
Wire Wire Line
	10150 2050 10350 2050
Connection ~ 10250 2550
Wire Wire Line
	10250 2850 10250 1250
Wire Wire Line
	10250 1250 10350 1250
Wire Wire Line
	9550 1450 10350 1450
Wire Wire Line
	900  2950 1300 2950
Wire Wire Line
	900  2650 1300 2650
Wire Wire Line
	2050 3850 2050 3900
Wire Wire Line
	2000 3750 2000 3850
Wire Wire Line
	2150 900  2150 1050
Wire Wire Line
	1850 900  1850 1050
Connection ~ 9800 3850
Wire Wire Line
	10000 3850 9400 3850
Wire Wire Line
	10550 3850 10400 3850
Wire Wire Line
	9400 4350 9400 4450
Connection ~ 9950 4950
Wire Wire Line
	10550 4050 10550 4950
Wire Wire Line
	10550 4950 9400 4950
Wire Wire Line
	9950 4950 9950 5100
Wire Wire Line
	9800 4250 9800 4950
Connection ~ 9800 4950
Wire Wire Line
	9400 3850 9400 3800
Connection ~ 9400 4400
Wire Wire Line
	9050 4400 9400 4400
Wire Wire Line
	2000 900  2000 1050
Wire Wire Line
	1200 1950 1200 2050
Wire Wire Line
	1200 2050 1300 2050
Wire Wire Line
	2000 3850 2100 3850
Wire Wire Line
	2100 3850 2100 3750
Connection ~ 2050 3850
Wire Wire Line
	900  2750 1300 2750
Wire Wire Line
	9550 1650 10350 1650
Wire Wire Line
	10250 2550 10350 2550
Wire Wire Line
	10350 1850 10150 1850
Connection ~ 10150 1850
Wire Wire Line
	10350 1750 9550 1750
Wire Wire Line
	10350 1350 9550 1350
Wire Wire Line
	10350 1950 9550 1950
Wire Wire Line
	10350 2450 9550 2450
Text Label 9550 2350 0    60   ~ 0
TopPMH
Text Label 9550 1750 0    60   ~ 0
Tmotor
Text Label 9550 1550 0    60   ~ 0
Tair
$Comp
L +5V #PWR9
U 1 1 5292156D
P 10150 950
F 0 "#PWR9" H 10150 1040 20  0001 C CNN
F 1 "+5V" H 10150 1040 30  0000 C CNN
	1    10150 950 
	1    0    0    -1  
$EndComp
$Comp
L +BATT #PWR8
U 1 1 5292066F
P 10100 2250
F 0 "#PWR8" H 10100 2200 20  0001 C CNN
F 1 "+BATT" H 10100 2350 30  0000 C CNN
	1    10100 2250
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR10
U 1 1 5292061B
P 10250 2850
F 0 "#PWR10" H 10250 2850 30  0001 C CNN
F 1 "GND" H 10250 2780 30  0001 C CNN
	1    10250 2850
	1    0    0    -1  
$EndComp
Text Label 900  2950 0    60   ~ 0
Vbat
Text Label 9550 1450 0    60   ~ 0
FP_CLK
Text Label 9550 1650 0    60   ~ 0
FP_DATA
Text Label 900  2750 0    60   ~ 0
FP_CLK
Text Label 900  2650 0    60   ~ 0
FP_DATA
$Comp
L GND #PWR4
U 1 1 5291D22C
P 2050 3900
F 0 "#PWR4" H 2050 3900 30  0001 C CNN
F 1 "GND" H 2050 3830 30  0001 C CNN
	1    2050 3900
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR1
U 1 1 5291D213
P 1200 1950
F 0 "#PWR1" H 1200 2040 20  0001 C CNN
F 1 "+5V" H 1200 2040 30  0000 C CNN
	1    1200 1950
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR5
U 1 1 5291D1D7
P 2150 900
F 0 "#PWR5" H 2150 860 30  0001 C CNN
F 1 "+3.3V" H 2150 1010 30  0000 C CNN
	1    2150 900 
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR3
U 1 1 5291D1C8
P 2000 900
F 0 "#PWR3" H 2000 990 20  0001 C CNN
F 1 "+5V" H 2000 990 30  0000 C CNN
	1    2000 900 
	1    0    0    -1  
$EndComp
$Comp
L +BATT #PWR2
U 1 1 5291D1AB
P 1850 900
F 0 "#PWR2" H 1850 850 20  0001 C CNN
F 1 "+BATT" H 1850 1000 30  0000 C CNN
	1    1850 900 
	1    0    0    -1  
$EndComp
Text Label 9050 4400 0    60   ~ 0
Vbat
$Comp
L GND #PWR7
U 1 1 5291D0FD
P 9950 5100
F 0 "#PWR7" H 9950 5100 30  0001 C CNN
F 1 "GND" H 9950 5030 30  0001 C CNN
	1    9950 5100
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 5291D0EB
P 9400 4700
F 0 "R2" V 9480 4700 50  0000 C CNN
F 1 "10k" V 9400 4700 50  0000 C CNN
	1    9400 4700
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 5291D0E2
P 9400 4100
F 0 "R1" V 9480 4100 50  0000 C CNN
F 1 "22k" V 9400 4100 50  0000 C CNN
	1    9400 4100
	1    0    0    -1  
$EndComp
$Comp
L CP1 C1
U 1 1 5291D0C5
P 9800 4050
F 0 "C1" H 9850 4150 50  0000 L CNN
F 1 "22u/16v" H 9850 3950 50  0000 L CNN
	1    9800 4050
	1    0    0    -1  
$EndComp
$Comp
L +BATT #PWR6
U 1 1 5291D092
P 9400 3800
F 0 "#PWR6" H 9400 3750 20  0001 C CNN
F 1 "+BATT" H 9400 3900 30  0000 C CNN
	1    9400 3800
	1    0    0    -1  
$EndComp
$Comp
L DIODE D1
U 1 1 5291D007
P 10200 3850
F 0 "D1" H 10200 3950 40  0000 C CNN
F 1 "1N4007" H 10200 3750 40  0000 C CNN
	1    10200 3850
	-1   0    0    1   
$EndComp
$Comp
L CONN_2 P1
U 1 1 5291CFCF
P 10900 3950
F 0 "P1" V 10850 3950 40  0000 C CNN
F 1 "CONN_2" V 10950 3950 40  0000 C CNN
	1    10900 3950
	1    0    0    -1  
$EndComp
NoConn ~ 1300 3500
NoConn ~ 1300 3400
NoConn ~ 2700 3300
NoConn ~ 2700 3200
$Comp
L ARDUINO_NANO U1
U 1 1 528E7B58
P 2000 2200
F 0 "U1" H 2500 1000 70  0000 C CNN
F 1 "ARDUINO_NANO" H 2800 900 70  0000 C CNN
F 2 "DIL30" H 2000 2150 60  0000 C CNN
	1    2000 2200
	1    0    0    -1  
$EndComp
$Comp
L DB15 J1
U 1 1 5283F4BF
P 10800 1850
F 0 "J1" H 10820 2700 70  0000 C CNN
F 1 "DB15" H 10750 1000 70  0000 C CNN
	1    10800 1850
	1    0    0    -1  
$EndComp
$EndSCHEMATC
