# Introduction #
La communication avec les logiciels de controle a travers une interface RS232 ou Bluetooth.

Cette page decrit les commandes utilisees.

### Commandes ###

Les commandes sont celles de Megasquirt MSII decritent dans [cette page](http://www.msextra.com/doc/ms2extra/RS232_MS2.html). Les commandes sont un peu simplifiees en omettant le CANiD et table index, inutilises par Solextronic.

<blockquote><table cellpadding='5' border='1'>
<tr><th>Commande</th><th>Requete</th><th>Reponse</th><th>Format</th></tr>
<tr><td>VersionInfo</td><td>S</td><td>Signature : " Solextronic v0.2 beta    "</td><td>chaine 32 caracteres</td></tr>
<tr><td>QueryCommand</td><td>Q</td><td>Revision : "MSII Rev 2.90500   "</td><td>chaine 20 caracteres</td></tr>
<tr><td>OutputChannel</td><td>A</td><td>Structure outputChannel/gState</td><td>Chaine binaire de 26 octets (taille de la structure)</td></tr>
<tr><td>currentTime</td><td>c</td><td>xxxx</td><td>Nombre de secondes depuis le demarrage</td></tr>
<tr><td>TableWriteCommand</td><td>t data</td><td>/</td><td>table a ecrire</td></tr>
<tr><td>burnCommand</td><td>b</td><td>/</td><td>Ecrit les parametres stockes en RAM en flash</td></tr>
<tr><td>readCommand</td><td>r offset_L offset_H numb_points_L numb_points_H</td><td>les octets demandes dans la structure Constants/eData</td><td>Permet de lire un ou plusieurs parametres dans la structure de configuration eData</td></tr>
<tr><td>writeCommand</td><td>w offset_L offset_H numb_points_L numb_points_H data</td><td>/</td><td>Permet d'ecrire un ou plusieurs parametres dans la structure de configuration eData</td></tr>
<tr><td>getCommand</td><td>a</td><td>structure outputChannel/gState</td><td>Identique a 'A'</td></tr>
<tr><td>checkCommand</td><td>y</td><td>0</td><td>Retourne systematiquement 0</td></tr>
<tr><td>echoCommand</td><td>e offset_L offset_H numb_points_L numb_points_H data</td><td><a href='data.md'>data</a></td><td>Identique a 'w' mais la reponse contient les octets ecrits</td></tr>
<tr><td>resetCommand</td><td>!x or !!x or !!!</td><td>/</td><td>reset</td></tr>
</table></blockquote>

### Structure Constants ###
Cette structure contient les parametres de configuration. Dans le microcontroleur, cette structure est stockee en EEPROM avec une image en RAM. Toutes les commandes travaillent sur l'image en RAM, l'EEPROM est mise a jour en tache de fond.

```

ratio           = array,  U08,   0,  [5],  "%",    1.00000,   0.00000,   0,   200,  0 ;  ratios to adjust ADC/DAC conversion
timerLed        = scalar, U16,   6,       "ms",    1.00000,   0.00000,   0, 65535,  0 ;  internal timer
HVstep          = scalar, U08,   8,     "step",    1.00000,   0.00000,   0,    50,  0 ;  step of high voltage loop in %. 0 set manual mode
HVmanual        = scalar, U08,   9,        "%",    1.00000,   0.00000,   0,   200,  0 ;  HT duty cycle in manual mode in %
wheelSize       = scalar, U08,  10,        "m",    0.01000,   0.00000, 1.0,  2.55,  2 ;  distance run in one wheel rotation
PMHOffset       = scalar, U08,  11,      "deg",    1.00000,   0.00000,   0,   255,  0 ;  PMH offset in deg
maxRPM          = scalar, U16,  12,      "RPM",    1.00000,   0.00000,   0, 16000,  0 ;  RPM limitation, 0 if no limit
maxTemp         = scalar, U16,  14,     "degC",    1.00000,   0.00000,   0,   200,  0 ;  overheating threshold, 0 if no limit
minBat          = scalar, U08,  16,        "v",    0.10000,   0.00000, 7.0,  12.0,  1 ;  alarm on low battery
igniDuration    = scalar, U16,  17,       "us",    1.00000,   0.00000,   0,  1000,  0 ;  ignition duration
starterAdv      = scalar, U08,  19,      "deg",    1.00000,   0.00000,   0,   255,  0 ;  advance during crancking
igniOverheat    = scalar, U08,  20,      "deg",    1.00000,   0.00000,   0,    20,  0 ;  advance decrease in case of overheating
noSparkAtDec    = bits,   U08,  21,  [0:0], "spark at dec", "no spark at dec"         ;  1 to stop ignition when deceleration
injOpen         = scalar, U16,  22,       "us",    1.00000,   0.00000,  200,  2000, 0 ;  time to open injector
injRate         = scalar, U16,  24,    "g/sec",    1.00000,   0.00000,   50,   500, 0 ;  flow rate of injector
injAdv          = scalar, U08,  26,      "deg",    1.00000,   0.00000,    0,   255, 0 ;  mean injection advance
starterInj      = scalar, U16,  27,       "us",    1.00000,   0.00000,  200,  5000, 0 ;  injection duration during crancking
injOverheat     = scalar, U08,  29,        "%",    1.00000,   0.00000,    0,    50, 0 ;  injection increase in case of overheating
injFullOpen     = scalar, U16,  30,       "us",    1.00000,   0.00000,  500,  5000, 0 ;  injection duration at full throttle
noInjAtDec      = bits,   U08,  32,  [0:0], "inj at dec", "no inj at dec"             ;  1 to stop injection when deceleration
injStart        = scalar, U16,  33,       "us",    1.00000,   0.00000,  200,  2000, 0 ;  injector opening duration
holdPWM         = scalar, U08,  35,        "%",    1.00000,   0.00000,   10,   100, 0 ;  PWM ratio during hold time
igniPolarity    = bits,   U08,  36,  [0:0], "Active on low", "Active on high"                 ;  0 active at low state
injPolarity     = bits,   U08,  37,  [0:0], "Active on low", "Active on high"                 ;  0 active at low state
pmhPolarity     = bits,   U08,  38,  [0:0], "Active on low", "Active on high"                 ;  0 active at low state
pumpPolarity    = bits,   U08,  39,  [0:0], "Active on low", "Active on high"                 ;  0 active at low state
injTestPW       = scalar, U16,  40,       "us",    1.00000,   0.00000,  200, 10000, 0 ;  pulse width of injector test mode
injTestCycles   = scalar, U16,  41,        " ",    1.00000,   0.00000,    0,  1000, 0 ;  number of cycle of injector test mode. 0 to stop
rpmBins         = array,  U16,  43, [10],"RPM",    1.00000,   0.00000,    0, 10000, 0 ;  table of RPM indexes
loadBins        = array,  U08,  63, [10],  "%",    1.00000,   0.00000,    0,   100, 0 ;  table of load/MAF indexes
injTable        = array,  U08,  73,[1x1],   "%",    1.00000,   0.00000,  10,   130,  0 ;  table for injection
igniTable       = array,  U08,  74,[1x1], "deg",    1.00000,   0.00000,   0,   200,  0 ;  table for ignition
user1           = scalar, U16,  75,        " ",    1.00000,   0.00000,    0,  1000, 0 ;  for debug
user2           = scalar, U16,  77,        " ",    1.00000,   0.00000,    0,  1000, 0 ;  for debug
user3           = scalar, U16,  79,        " ",    1.00000,   0.00000,    0,  1000, 0 ;  for debug
```

### Structure OutputChannel ###
Cette structure contient l'etat courant. Dans le microcontroleur, cette structure est stockee en RAM.

```

battery          = scalar,     U08,    0,   "v",    0.100,  0.0
tempMotor        = scalar,     U08,    1,  "°C",    1.000,  0.0
tempAir          = scalar,     U08,    2,  "°C",    1.000,  0.0
throttle         = scalar,     U08,    3,   "%",    1.000,  0.0
pressure         = scalar,     U16,    4, "kPa",    1.000,  0.0
HVvalue          = scalar,     U08,    6,   "%",    1.000,  0.0 ; PWM duty cycle for High voltage supply
rpm              = scalar,     U16,    7, "RPM",    1.000,  0.0
speed            = scalar,     U16,    9,"km/h",    1.000,  0.0
engine           = scalar,     U08,   10,"bits",    1.000,  0.0
; Engine status - bit field for engine
; ready:    bit 0 => 0 = engine not ready 1 = ready to run
; crank:    bit 1 => 0 = engine not cranking 1 = engine cranking
; startw:   bit 2 => 0 = not in startup warmup 1 = in warmup enrichment
; warmup:   bit 3 => 0 = not in warmup 1 = in warmup
; tpsaen:   bit 4 => 0 = not in TPS acceleration mode 1 = TPS acceleration mode
; tpsden:   bit 5 => 0 = not in deacceleration mode 1 = in deacceleration mode
; revlim:   bit 6 => 0 = not in rev limiter mode 1 = rev limiter mode
; overheat: bit 7 => 0 = not in overheat mode 1 = overheat mode
ready            = bits,   U08,   10, [0:0]
crank            = bits,   U08,   10, [1:1]
startw           = bits,   U08,   10, [2:2]
warmup           = bits,   U08,   10, [3:3]
tpsaen           = bits,   U08,   10, [4:4]
tpsden           = bits,   U08,   10, [5:5]
revlim           = bits,   U08,   10, [6:6]
overheat         = bits,   U08,   10, [7:7]
injPulseWidth    = scalar,     U16,   11,  "us",    1.000,  0.0 ; Injector active time
injStart         = scalar,     U16,   13, "deg",    1.000,  0.0 ; Injector start time before PMH
advance          = scalar,     U08,   15, "deg",    1.000,  0.0 ; Spark advance in deg before PMH
second           = scalar,     U16,   17, "sec",    1.000,  0.0 ; Current time in sec : will be set only when asked by UART command
debug1           = scalar,     U16,   19,  "%",     1.000,  0.0
debug2           = scalar,     U16,   21,  "%",     1.000,  0.0
debug3           = scalar,     U16,   23,  "%",     1.000,  0.0
kpaix            = scalar,     U08,   25,  "%",     1.000,  0.0
```