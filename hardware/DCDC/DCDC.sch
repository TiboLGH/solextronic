EESchema Schematic File Version 2  date dim. 20 avril 2014 19:41:46 CEST
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
EELAYER 43  0
EELAYER END
$Descr A4 11700 8267
encoding utf-8
Sheet 1 1
Title "DC-DC Converter"
Date "20 apr 2014"
Rev "0.1"
Comp "Le Galet Hurlant"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Connection ~ 7600 1000
Wire Wire Line
	7600 1350 7600 1000
Wire Wire Line
	7450 1750 7450 1650
Wire Wire Line
	7450 1650 7200 1650
Connection ~ 7600 2250
Wire Wire Line
	7600 1750 7600 2250
Wire Wire Line
	7250 2150 7250 2250
Wire Wire Line
	8150 1000 8450 1000
Connection ~ 6100 1000
Wire Wire Line
	6100 1000 6100 1450
Wire Wire Line
	6100 1950 6100 2000
Wire Wire Line
	6100 3050 6100 3150
Connection ~ 5550 1750
Wire Wire Line
	5550 1900 5550 1400
Wire Wire Line
	5550 1400 5450 1400
Wire Wire Line
	1800 3000 2150 3000
Wire Wire Line
	3300 3850 2000 3850
Wire Wire Line
	1450 3900 1450 4000
Wire Wire Line
	1450 3500 1450 2400
Wire Wire Line
	1450 2400 2500 2400
Wire Wire Line
	3600 2800 3350 2800
Wire Wire Line
	4650 4350 4650 4550
Wire Wire Line
	4650 3400 4650 3850
Wire Wire Line
	3700 1400 3700 1500
Wire Wire Line
	3700 900  3700 1000
Wire Wire Line
	1100 1600 1100 1650
Wire Wire Line
	2000 2050 1100 2050
Wire Wire Line
	4350 2000 4350 2100
Wire Wire Line
	4350 1500 4050 1500
Connection ~ 4050 1000
Wire Wire Line
	4650 1400 4650 3000
Wire Wire Line
	3350 3400 3350 3550
Wire Wire Line
	850  3100 850  3000
Wire Wire Line
	850  3000 1000 3000
Wire Wire Line
	2150 2800 1300 2800
Wire Wire Line
	1500 1000 1050 1000
Wire Wire Line
	1050 900  1150 900 
Wire Wire Line
	1050 1200 1150 1200
Wire Wire Line
	1150 1200 1150 1300
Wire Wire Line
	1500 1100 1050 1100
Wire Wire Line
	550  2400 850  2400
Wire Wire Line
	850  2400 850  2500
Wire Wire Line
	1300 3200 1300 3300
Wire Wire Line
	850  3600 850  3700
Connection ~ 4350 1000
Wire Wire Line
	4350 1400 4350 1600
Connection ~ 4350 1500
Wire Wire Line
	1100 2050 1100 2100
Connection ~ 1900 1650
Wire Wire Line
	1100 1650 1400 1650
Wire Wire Line
	1900 1650 3500 1650
Wire Wire Line
	3500 1650 3500 3000
Wire Wire Line
	3500 3000 3350 3000
Connection ~ 2000 1650
Wire Wire Line
	3700 1000 4650 1000
Wire Wire Line
	4250 3200 4350 3200
Wire Wire Line
	4650 3850 3800 3850
Wire Wire Line
	4000 2800 4000 2900
Wire Wire Line
	3350 2800 3350 2400
Wire Wire Line
	3350 2400 3000 2400
Wire Wire Line
	2150 3400 1450 3400
Connection ~ 1450 3400
Wire Wire Line
	2150 3200 2000 3200
Wire Wire Line
	2000 3200 2000 3850
Wire Wire Line
	2000 4250 2000 4350
Wire Wire Line
	3350 3200 3750 3200
Wire Wire Line
	5450 1000 5650 1000
Wire Wire Line
	6100 2500 6100 2550
Wire Wire Line
	5500 2250 5950 2250
Wire Wire Line
	8350 1150 8350 1000
Connection ~ 8350 1000
Wire Wire Line
	7250 2250 8350 2250
Wire Wire Line
	8350 2250 8350 1550
Connection ~ 7450 2250
Wire Wire Line
	7650 2300 7650 2250
Connection ~ 7650 2250
Wire Wire Line
	7250 1750 7250 1650
Connection ~ 7250 1650
Wire Wire Line
	6400 1650 6700 1650
Wire Wire Line
	7750 1000 6050 1000
Text Label 6400 1650 0    60   ~ 0
Trigger
$Comp
L C C8
U 1 1 5353FAF3
P 7250 1950
F 0 "C8" H 7300 2050 50  0000 L CNN
F 1 "100nF" H 7300 1850 50  0000 L CNN
	1    7250 1950
	-1   0    0    1   
$EndComp
$Comp
L R R11
U 1 1 5353FAAB
P 6950 1650
F 0 "R11" V 7050 1650 50  0000 C CNN
F 1 "220" V 6950 1650 50  0000 C CNN
	1    6950 1650
	0    1    1    0   
$EndComp
$Comp
L R R12
U 1 1 5353FAA0
P 7450 2000
F 0 "R12" V 7530 2000 50  0000 C CNN
F 1 "1k" V 7450 2000 50  0000 C CNN
	1    7450 2000
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR01
U 1 1 5353FA7D
P 7650 2300
F 0 "#PWR01" H 7650 2300 30  0001 C CNN
F 1 "GND" H 7650 2230 30  0001 C CNN
	1    7650 2300
	1    0    0    -1  
$EndComp
$Comp
L DIODESCH D3
U 1 1 5353FA72
P 8350 1350
F 0 "D3" H 8350 1450 40  0000 C CNN
F 1 "EF5400" H 8350 1250 40  0000 C CNN
	1    8350 1350
	0    1    1    0   
$EndComp
Text Label 5500 2250 0    60   ~ 0
Feedback
$Comp
L GND #PWR02
U 1 1 5353F7E7
P 6100 3150
F 0 "#PWR02" H 6100 3150 30  0001 C CNN
F 1 "GND" H 6100 3080 30  0001 C CNN
	1    6100 3150
	1    0    0    -1  
$EndComp
$Comp
L POT 10k
U 1 1 5353F7CE
P 6100 2250
F 0 "10k" H 6100 2150 50  0000 C CNN
F 1 "POT" H 6100 2250 50  0000 C CNN
	1    6100 2250
	0    -1   -1   0   
$EndComp
$Comp
L R R10
U 1 1 5353F79A
P 6100 2800
F 0 "R10" V 6180 2800 50  0000 C CNN
F 1 "6.8k" V 6100 2800 50  0000 C CNN
	1    6100 2800
	-1   0    0    1   
$EndComp
$Comp
L R R9
U 1 1 5353F787
P 6100 1700
F 0 "R9" V 6180 1700 50  0000 C CNN
F 1 "1M" V 6100 1700 50  0000 C CNN
	1    6100 1700
	-1   0    0    1   
$EndComp
$Comp
L C C9
U 1 1 5353F635
P 7950 1000
F 0 "C9" H 8000 1100 50  0000 L CNN
F 1 "1.5uF/400v" H 8000 900 50  0000 L CNN
	1    7950 1000
	0    -1   -1   0   
$EndComp
$Comp
L CONNECTOR P2
U 1 1 5353F4B9
P 5550 1750
F 0 "P2" H 5900 1850 70  0000 C CNN
F 1 "GND" H 5900 1650 70  0000 C CNN
	1    5550 1750
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR03
U 1 1 5353F4AC
P 5550 1900
F 0 "#PWR03" H 5550 1900 30  0001 C CNN
F 1 "GND" H 5550 1830 30  0001 C CNN
	1    5550 1900
	1    0    0    -1  
$EndComp
$Comp
L DIODESCH D2
U 1 1 5353F478
P 5850 1000
F 0 "D2" H 5850 1100 40  0000 C CNN
F 1 "EF5400" H 5850 900 40  0000 C CNN
	1    5850 1000
	1    0    0    -1  
$EndComp
Text Notes 7750 5700 2    600  ~ 120
Not Tested !
Text Label 1800 3000 0    60   ~ 0
Feedback
$Comp
L GND #PWR04
U 1 1 5353F311
P 2000 4350
F 0 "#PWR04" H 2000 4350 30  0001 C CNN
F 1 "GND" H 2000 4280 30  0001 C CNN
	1    2000 4350
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR05
U 1 1 5353F2DA
P 1450 4000
F 0 "#PWR05" H 1450 4000 30  0001 C CNN
F 1 "GND" H 1450 3930 30  0001 C CNN
	1    1450 4000
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR06
U 1 1 5353F294
P 4000 2900
F 0 "#PWR06" H 4000 2900 30  0001 C CNN
F 1 "GND" H 4000 2830 30  0001 C CNN
	1    4000 2900
	1    0    0    -1  
$EndComp
$Comp
L C C6
U 1 1 5353F28C
P 3800 2800
F 0 "C6" H 3850 2900 50  0000 L CNN
F 1 "100nF" H 3850 2700 50  0000 L CNN
	1    3800 2800
	0    -1   -1   0   
$EndComp
$Comp
L C C3
U 1 1 5353F1DF
P 2000 1850
F 0 "C3" H 2050 1950 50  0000 L CNN
F 1 "100nF" H 2050 1750 50  0000 L CNN
	1    2000 1850
	1    0    0    -1  
$EndComp
$Comp
L R R3
U 1 1 5353F1A8
P 1650 1650
F 0 "R3" V 1730 1650 50  0000 C CNN
F 1 "22" V 1650 1650 50  0000 C CNN
	1    1650 1650
	0    1    1    0   
$EndComp
$Comp
L GND #PWR07
U 1 1 5353D771
P 1100 2100
F 0 "#PWR07" H 1100 2100 30  0001 C CNN
F 1 "GND" H 1100 2030 30  0001 C CNN
	1    1100 2100
	1    0    0    -1  
$EndComp
$Comp
L CP1 C1
U 1 1 5353D76C
P 1100 1850
F 0 "C1" H 1150 1950 50  0000 L CNN
F 1 "100uF/25v" H 1150 1750 50  0000 L CNN
	1    1100 1850
	1    0    0    -1  
$EndComp
$Comp
L +12V #PWR08
U 1 1 5353D765
P 1100 1600
F 0 "#PWR08" H 1100 1550 20  0001 C CNN
F 1 "+12V" H 1100 1700 30  0000 C CNN
	1    1100 1600
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR09
U 1 1 5353D70C
P 4350 2100
F 0 "#PWR09" H 4350 2100 30  0001 C CNN
F 1 "GND" H 4350 2030 30  0001 C CNN
	1    4350 2100
	1    0    0    -1  
$EndComp
$Comp
L DIODESCH D1
U 1 1 5353D6BC
P 4350 1800
F 0 "D1" H 4350 1900 40  0000 C CNN
F 1 "SR360" H 4350 1700 40  0000 C CNN
	1    4350 1800
	0    -1   -1   0   
$EndComp
$Comp
L R R7
U 1 1 5353D6AE
P 4050 1250
F 0 "R7" V 4130 1250 50  0000 C CNN
F 1 "120/1W" V 3950 1250 50  0000 C CNN
	1    4050 1250
	-1   0    0    1   
$EndComp
$Comp
L C C7
U 1 1 5353D688
P 4350 1200
F 0 "C7" H 4400 1300 50  0000 L CNN
F 1 "2.2nF" H 4400 1100 50  0000 L CNN
	1    4350 1200
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR010
U 1 1 5353D566
P 3700 1500
F 0 "#PWR010" H 3700 1500 30  0001 C CNN
F 1 "GND" H 3700 1430 30  0001 C CNN
	1    3700 1500
	1    0    0    -1  
$EndComp
$Comp
L CP1 C5
U 1 1 5353D458
P 3700 1200
F 0 "C5" H 3750 1300 50  0000 L CNN
F 1 "100uF/25v" V 3550 1000 50  0000 L CNN
	1    3700 1200
	1    0    0    -1  
$EndComp
$Comp
L +12V #PWR011
U 1 1 5353D43C
P 3700 900
F 0 "#PWR011" H 3700 850 20  0001 C CNN
F 1 "+12V" H 3700 1000 30  0000 C CNN
	1    3700 900 
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR012
U 1 1 5353D0EA
P 850 3700
F 0 "#PWR012" H 850 3700 30  0001 C CNN
F 1 "GND" H 850 3630 30  0001 C CNN
	1    850  3700
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 5353D0E3
P 850 3350
F 0 "R2" V 930 3350 50  0000 C CNN
F 1 "22k" V 850 3350 50  0000 C CNN
	1    850  3350
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR013
U 1 1 5353C694
P 1300 3300
F 0 "#PWR013" H 1300 3300 30  0001 C CNN
F 1 "GND" H 1300 3230 30  0001 C CNN
	1    1300 3300
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 5353C660
P 850 2750
F 0 "R1" V 930 2750 50  0000 C CNN
F 1 "2.2k" V 850 2750 50  0000 C CNN
	1    850  2750
	-1   0    0    1   
$EndComp
Text Label 550  2400 0    60   ~ 0
Enable
$Comp
L CONNECTOR P3
U 1 1 5353C5F4
P 8450 1000
F 0 "P3" H 8800 1100 70  0000 C CNN
F 1 "TO COIL" H 8800 900 70  0000 C CNN
	1    8450 1000
	1    0    0    -1  
$EndComp
$Comp
L THYRISTOR T2
U 1 1 5353C564
P 7600 1550
F 0 "T2" H 7500 1650 40  0000 C CNN
F 1 "TIC106M" H 7600 1450 40  0000 C CNN
	1    7600 1550
	0    -1   1    0   
$EndComp
Text Label 1500 1100 2    60   ~ 0
Trigger
Text Label 1500 1000 2    60   ~ 0
Enable
$Comp
L GND #PWR014
U 1 1 5353C4E1
P 1150 1300
F 0 "#PWR014" H 1150 1300 30  0001 C CNN
F 1 "GND" H 1150 1230 30  0001 C CNN
	1    1150 1300
	1    0    0    -1  
$EndComp
$Comp
L CONN_4 P1
U 1 1 5353C4C6
P 700 1050
F 0 "P1" V 650 1050 50  0000 C CNN
F 1 "CONN_4" V 750 1050 50  0000 C CNN
	1    700  1050
	-1   0    0    1   
$EndComp
$Comp
L NPN Q1
U 1 1 5353A79A
P 1200 3000
F 0 "Q1" H 1200 2850 50  0000 R CNN
F 1 "2N2222" H 1200 3150 50  0000 R CNN
	1    1200 3000
	1    0    0    -1  
$EndComp
$Comp
L R R5
U 1 1 5353A752
P 3550 3850
F 0 "R5" V 3630 3850 50  0000 C CNN
F 1 "1k" V 3550 3850 50  0000 C CNN
	1    3550 3850
	0    -1   -1   0   
$EndComp
$Comp
L C C2
U 1 1 5353A727
P 1450 3700
F 0 "C2" H 1500 3800 50  0000 L CNN
F 1 "1nF" H 1500 3600 50  0000 L CNN
	1    1450 3700
	1    0    0    -1  
$EndComp
$Comp
L R R4
U 1 1 5353A6A8
P 2750 2400
F 0 "R4" V 2830 2400 50  0000 C CNN
F 1 "33k" V 2750 2400 50  0000 C CNN
	1    2750 2400
	0    -1   -1   0   
$EndComp
$Comp
L C C4
U 1 1 5353A69B
P 2000 4050
F 0 "C4" H 2050 4150 50  0000 L CNN
F 1 "C" H 2050 3950 50  0000 L CNN
	1    2000 4050
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR015
U 1 1 5353A68F
P 4650 4550
F 0 "#PWR015" H 4650 4550 30  0001 C CNN
F 1 "GND" H 4650 4480 30  0001 C CNN
	1    4650 4550
	1    0    0    -1  
$EndComp
$Comp
L R R6
U 1 1 5353A674
P 4000 3200
F 0 "R6" V 4080 3200 50  0000 C CNN
F 1 "22" V 4000 3200 50  0000 C CNN
	1    4000 3200
	0    -1   -1   0   
$EndComp
$Comp
L R R8
U 1 1 5353A641
P 4650 4100
F 0 "R8" V 4730 4100 50  0000 C CNN
F 1 "0.33/1W" V 4650 4100 50  0000 C CNN
	1    4650 4100
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR016
U 1 1 53539CC9
P 3350 3550
F 0 "#PWR016" H 3350 3550 30  0001 C CNN
F 1 "GND" H 3350 3480 30  0001 C CNN
	1    3350 3550
	1    0    0    -1  
$EndComp
$Comp
L +12V #PWR017
U 1 1 53539C5B
P 1150 900
F 0 "#PWR017" H 1150 850 20  0001 C CNN
F 1 "+12V" H 1150 1000 30  0000 C CNN
	1    1150 900 
	1    0    0    -1  
$EndComp
$Comp
L MOSFET_N Q2
U 1 1 53539BA8
P 4550 3200
F 0 "Q2" H 4560 3370 60  0000 R CNN
F 1 "IRF540" H 4560 3050 60  0000 R CNN
	1    4550 3200
	1    0    0    -1  
$EndComp
$Comp
L UC3845 U1
U 1 1 53539B62
P 2750 3150
F 0 "U1" H 2500 3650 60  0000 C CNN
F 1 "UC3845" H 2650 2750 60  0000 C CNN
	1    2750 3150
	1    0    0    -1  
$EndComp
$Comp
L TRANSFO T1
U 1 1 535397B1
P 5050 1200
F 0 "T1" H 5050 1450 70  0000 C CNN
F 1 "2:50" H 5050 900 70  0000 C CNN
	1    5050 1200
	1    0    0    -1  
$EndComp
$EndSCHEMATC
