#!/usr/bin/python

# This script is used to produces tuner software INI config file and wiki documentation from source code varDef.h
# To comply with Megasquirt serial protocol and use associated tuner software, a INI config file shall be used to described mapping of config and monitoring data buffers. 
# This INI file also described how the tuner interface is linked to variables (panel contents, min/max, gauges...). This part is directly part of this script (don't want to have another config file)
# The documentation page output produces a wiki page compliant with Google code format describing the data structure. It is basically a "pretty-print" of the varDef.h file.

# Warning ! The varDef.h file has to be correctly formatted ! This script aims to be fairly robust but it is just a script ;-)

import sys
import re

#-----------------------------------------------------------------------------------------------------------------
license = """;/***************************************************************************
; *   Copyright (C) 2012 by Thibault Bouttevin                              *
; *   thibault.bouttevin@gmail.com                                          *
; *   www.legalethurlant.fr.st                                              *
; *                                                                         *
; *   This file is part of SolexTronic                                      *
; *                                                                         *
; *   SolexTronic is free software; you can redistribute it and/or modify   *
; *   it under the terms of the GNU General Public License as published by  *
; *   the Free Software Foundation; either version 3 of the License, or     *
; *   any later version.                                                    *
; *                                                                         *
; *   This program is distributed in the hope that it will be useful,       *
; *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
; *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
; *   GNU General Public License for more details.                          *
; *                                                                         *
; *   You should have received a copy of the GNU General Public License     *
; *   along with this program; if not, write to the                         *
; *   Free Software Foundation, Inc.,                                       *
; *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
; ***************************************************************************/"""

#-----------------------------------------------------------------------------------------------------------------
megaTuneSection = """[MegaTune]
   MTversion      = 2.25 ; MegaTune itself; needs to match exec version.

   versionInfo    = "S"  ; Put this in the title bar.
   queryCommand   = "Q"  ; Verify against signature.
   signature      = "MSII Rev 2.90500   " ; MS-II sends a null at 20th byte.
                   ; 123456789.123456789."""

#-----------------------------------------------------------------------------------------------------------------
eepromHeaderSection="""[Constants]
   pageActivationDelay =  50 ; Milliseconds delay after burn command.
   blockReadTimeout    = 500 ; Milliseconds total timeout for reading page.

   endianness          = little
   nPages              = 1
   pageSize            = 81
   pageIdentifier      = "\\x00"
   burnCommand         = "b"
   pageReadCommand     = "r%2o%2c"
   pageValueWrite      = "w%2o%2c%v"
   pageChunkWrite      = "w%2o%2c%v"
   interWriteDelay     = 15 

   page = 1
   ;  name            = class,  type, offset,      shape,  units,       scale, translate,    lo,      hi, digits
"""
#-----------------------------------------------------------------------------------------------------------------
menuSection = """[Menu]
   menuDialog = main
      menu = "Injection &Set-Up"
         subMenu = generalSettings, "&General"
         subMenu = injChars,        "I&njector Characteristics",       0,  
         subMenu = std_injection,   "Injection &Control",              0,  
         subMenu = revLimiter,      "&Rev Limiter",                    0,  
         subMenu = std_separator    ;----------------------------------------------
         subMenu = injectionTbl,     "&VE Table (MAP/Baro)",         0,  
         subMenu = std_separator    ;----------------------------------------------
	     subMenu = injector_test,   "&Injector Test Mode",            0,  

   menuDialog = main
       menu = "&Ignition Set-Up"
         subMenu = ignitionOptions, "&Base Ignition Settings",         0,  
         subMenu = ignitionTbl,     "&Spark Advance Table (MAP/Baro)",0,  

   menuDialog = injectionTbl
      menu = "&Edit Bins"
	 subMenu = fmapTableBins,   "Edit Fuel &MAP Bins"
	 subMenu = frpmTableBins,   "Edit Fuel &RPM Bins"

   menuDialog = ignitionTbl
      menu = "&Edit Bins"
	 subMenu = fmapTableBins,   "Edit Spark &MAP Bins"
	 subMenu = frpmTableBins,   "Edit Spark &RPM Bins"		 

   menuDialog = main
      menu = "&Tuning"
         subMenu = std_realtime,    "&Realtime Display",              0,  
         subMenu = std_separator    ;----------------------------------------------
         subMenu = veTable1Map,     "VE &Table (MAP/Baro)",         0, 
         subMenu = ignitionMap,     "I&gnition Advance Table (MAP/Baro)",    0, 
         subMenu = std_separator    ;----------------------------------------------
         subMenu = std_warmup,      "&Warmup Wizard",                 0,  

   menu = "Help"
         subMenu = helpGeneral,     "&MS-II Info"
         subMenu = sensorHelp,      "&Sensor Calibration"
         subMenu = burnHelp,        "&Burning Values to ECU"
"""
#-----------------------------------------------------------------------------------------------------------------
userDefinedMenu="""[UserDefined]

   dialog = generalSettings, "General Settings"
      field = "BTDC sensor offset",       PMHOffset           

   dialog = revLimiter, "Rev Limiter"
      field = "Ignition retard",             igniOverheat
      field = "Rev Limit",                   maxRPM

   dialog = injChars, "Injector Characteristics"
      field = "Injector Flow Rate",         injRate,
      field = "Injector Open Time",         injOpen
      field = "PWM Current Limit",          holdPWM
      field = "PWM Time Threshold",         injStart

   dialog = ignitionOptions, "Base Ignition Settings"
      field = "#General Ignition"
 
   dialog = injector_test, "Injector Test Mode"
      field = "!Injector test mode : set 0 to stop"
      field = "Injector test squirts",  injTestCycles
      field = "Injector test PW",       injTestPW

   dialog = frpmTableBins, "RPM Table Bins for Tables"
      array1D   = "", "Fuel RPM  %INDEX% ", rpmBins
      field = "#Close and re-open Table(s) to see changes"

   dialog = fmapTableBins, "MAP Table Bins for Tables"
      array1D   = "", "Fuel load  %INDEX% ", loadBins
      field = "#Close and re-open Table(s) to see changes"

	  
   help = burnHelp, "Burning Values to ECU"
      webHelp = "http://www.megamanual.com/mt28.htm"
      text = "Occassionally, you may have trouble getting values to 'stick'"
      text = "in MegaTune. There are a few things you can try if this happens:<br><br>"
      text = "\tHit TAB after entering the value. This forces MegaTune to" 
      text = "recognize the changed value.<br>" 
      text = "\tThen 'Burn to ECU'<br><br>"	
      text = "If that doesn't work:<br><br>"	
      text = "\tChange the value (as above) and immediately save the MSQ file ('File/Save').<br>"
      text = "\tThen reload the file you just saved, and burn it when prompted.<br><br>"
      text = "This forces all the values to be burned to both flash and RAM memory, and"
      text = "should solve the problem."	
   
   help = sensorHelp, "Sensor Calibration"
      webHelp = "http://www.megamanual.com/mt28.htm#os"
      text = "MAP Sensor Calibration<br>"
      text = "<br>"
      text = "For the\tMPX4115   use\t10.6 and \t121.7<br>"
      text = "\tMPX4250\t\t10.0\t260.0<br>"
      text = "\tMPXH6300\t1.1\t315.5<br>"
      text = "\tGM 3-BAR\t1.1\t315.5<br>"
      text = "\tMPXH6400\t3.5\t416.5<br>"
      text = "<br>"
      text = "(GM 3-bar data from Dave Hartnell, http://www.not2fast.com/electronics/component_docs/MAP_12223861.pdf)<br>"
      text = "<br>"
      text = "\tSensor type\tvLo\tpLo\tvHi\tpHi\tvRef<br>"
      text = "\tMPX4115 \t0.204 v\t15 kPa\t4.794 v\t115 kPa\t5.100 v<br>"
      text = "\tMPX4250 \t0.204 v\t20 kPa\t4.896 v\t250 kPa\t5.100 v<br>"
      text = "\tMPXH6300\t0.306 v\t20 kPa\t4.913 v\t304 kPa\t5.100 v<br>"
      text = "\tGM 3-BAR\t0.631 v\t40 kPa\t4.914 v\t304 kPa\t5.100 v<br>"
      text = "\tMPXH6400\t0.200 v\t20 kPa\t4.800 v\t400 kPa\t5.000 v<br>"
      text = "<br>"
      text = "In general, use values derived from these equations:<br>"
      text = "<br>"
      text = "\tm = (pHi-pLo)/(vHi-vLo)<br>"
      text = "\tpv1 = pLo - m * vLo<br>"
      text = "\tpv2 = pv1 + m * vRef<br>"
      text = "<br>"
      text = "References:<br>"
      text = "\thttp://www.freescale.com/files/sensors/doc/data_sheet/MPX4115A.pdf<br>"
      text = "\thttp://www.freescale.com/files/sensors/doc/data_sheet/MPX4250A.pdf<br>"
      text = "\thttp://www.freescale.com/files/sensors/doc/data_sheet/MPXH6300A.pdf<br>"
      text = "\thttp://www.freescale.com/files/sensors/doc/data_sheet/MPXH6400A.pdf<br>"
      text = "<br>"
      text = "Barometer Sensor Calibration<br>"
      text = "<br>"
      text = "If your system has an external barometer sensor, separate from the MAP"
      text = "sensor, then use these values to calibrate it properly.  If you have"
      text = "a standard MS installation, then copy your MAP sensor values here.<br>"
      text = "<br>"
      text = "Barometric Correction Calibration<br>"
      text = "<br>"
      text = "Correction for barometric effects is performed using the linear function below.<br>"
      text = "<br>"
      text = "\tcorrection = correction_0 + (rate * barometer) / 100<br>"
      text = "<br>"
      text = "'At total vacuum' contains the total correction at a barometer reading"
      text = "of 0 kPa (you are on the moon).  The 'Rate' contains the percentage per 100"
      text = "kPa to scale the barometer value.  Using the default values of 147 and -47,"
      text = "we see that for a barometer of 100 kPa, we have 100% correction.<br>"
      text = "<br>"
      text = "\tcorrection = 147 + (-47*100) / 100 = 100%"


   help = helpGeneral, "MS-II General Info"
      webHelp = "http://www.megamanual.com/mt28.htm"
      text = "Al Grippo and Bruce Bowling have created MegaSquirt-II, which"
      text = "is a plug-in daughter card that replaces the 8-bit MC68HC908GP32"
      text = "with a 16-bit MC9S12C64 processor."
      text = "<br>"
      text = "It is basically a plug-in processor card that has the MC9S12C64"
      text = "processor plus support hardware as well as a stepper motor chip,"
      text = "and an ignition module controller.  The embedded code is written"
      text = "in C, rather than assembly language, so it should be more accessible"
      text = "to more programmers (Stephane Carrez has ported GCC to the HC12"
      text = "processor so, although it doesn't use the full 9S12 instruction"
      text = "set, we are able to use his version to write code for the MS-II).<br>"
      text = "<br>"
      text = "For current documentation, click the Web Help button below, or for"
      text = "support questions visit http://www.msefi.com/."

;-------------------------------------------------------------------------------
"""
#-----------------------------------------------------------------------------------------------------------------
curveEditorSection="""[CurveEditor]
"""

#-----------------------------------------------------------------------------------------------------------------
tableEditorSection="""
[TableEditor]
   ;       table_id,    map3d_id,    "title",      page
   table = injectionTbl, veTable1Map, "VE Table (MAP)", 1
      ;             constant,   variable
      xBins       = rpmBins, rpm,     readonly
      yBins       = loadBins, kpaix, readonly
      zBins       = injTable

      upDownLabel = "RICHER", "LEANER"
      gridHeight  = 2.0
      gridOrient  = 250,   0, 340 ; Space 123 rotation of grid in degrees.

   table = ignitionTbl, ignitionMap, "Spark Advance Table", 1
      xBins       = rpmBins, rpm, readonly
      yBins       = loadBins, kpaix, readonly
      zBins       = igniTable
      upDownLabel = "ADVANCING", "RETARDING"
      gridHeight  = 3.0
      gridOrient  = 250,   0, 340


;-------------------------------------------------------------------------------
"""
gaugeConfigSection="""[GaugeConfigurations]

   ;-------------------------------------------------------------------------------
   ; Define a gauge's characteristics here, then go to a specific layout
   ; block (Tuning or FrontPage) and use the name you've defined here to
   ; display that gauge in a particular position.
   ;
   ; Name  = Case-sensitive, user-defined name for this gauge configuration.
   ; Var   = Case-sensitive name of variable to be displayed, see the
   ;         OutputChannels block in this file for possible values.
   ; Title = Title displayed at the top of the gauge.
   ; Units = Units displayed below value on gauge.
   ; Lo    = Lower scale limit of gauge.
   ; Hi    = Upper scale limit of gauge.
   ; LoD   = Lower limit at which danger color is used for gauge background.
   ; LoW   = Lower limit at which warning color is used.
   ; HiW   = Upper limit at which warning color is used.
   ; HiD   = Upper limit at which danger color is used.
   ; vd    = Decimal places in displayed value
   ; ld    = Label decimal places for display of Lo and Hi, above.

   ;Name               Var            Title                 Units     Lo     Hi     LoD    LoW   HiW   HiD vd ld

   advdegGauge       = advance,       "Ignition Advance",   "deg",     0,    50,    -1,     -1,  999,  999, 0, 0
   clockGauge        = second,        "Clock",              "Seconds", 0,   255,     10,    10,  245,  245, 0, 0
   dutyCycle1Gauge   = injStart,      "Injection Start",    "deg",     0,   360,     -1,    -1,  999,  999, 0, 0
   mafGauge          = MAP,      "MAF Air flow",       "g/sec",   0,   650,      0,   200,  480,  550, 0, 0
   calcMapGauge      = MAP,      "calcMAP",            "kPa",     0,   255,      0,    20,  200,  245, 0, 0
   pulseWidthGauge   = injPulseWidth, "Pulse Width",        "uSec",    0,  9000,   1000,  1200, 8000, 8500, 0, 0
   tachometer        = rpm,           "Engine Speed",       "RPM",     0, 10000,    300,   500, 8000, 9000, 0, 0
   TPSGauge     = TPS,      "Throttle Position",  "%",       0,   100,     -1,     1,   90,  100, 0, 0
   voltMeter         = battery,       "Battery Voltage",    "v",       7,    15,      9,    10,   14, 14.5, 1, 1
   CLTGauge    = CLT,     "Motor Temp",         "C",       0,   150,    -15,     0,   95,  105, 0, 0
   IATGauge      = IAT,       "Intake Air Temp",    "C",       0,   100,      5,    10,   60,   70, 0, 0
   speedGauge        = speed,         "Speed",              "km/h",    0,   100,     -1,    -1,   90,   95, 0, 0

;-------------------------------------------------------------------------------
"""

frontpageSection="""
[FrontPage]
   ; Gauges are numbered left to right, top to bottom.
   ;
   ;    1  2  3  4
   ;    5  6  7  8
   ; for unknown reason, speedometer has to be in position 1

   gauge6 = speedGauge
   gauge2 = tachometer
   gauge3 = pulseWidthGauge
   gauge4 = advdegGauge
   gauge5 = voltMeter
   gauge6 = TPSGauge
   gauge7 = CLTGauge
   gauge8 = IATGauge


   ;----------------------------------------------------------------------------
   ; Indicators
   ;               expr         off-label          on-label, off-bg, off-fg, on-bg, on-fg
   ;   indicator = { tpsaen }, "Not Accelerating", "AE",     cyan,   white,  red,   black
   ;
   ; Look in the new colorScheme.ini for the basic ones, add more or tell me what to add.

   indicator = { ready              }, "Not ready", "Ready",    red, black, green, black
   indicator = { crank              }, "Not Cranking", "Cranking",    white, black, black, white
   indicator = { startw             }, "ASE off",      "ASE ON",         white, black, cyan,   black
   indicator = { warmup             }, "WUE off",      "WUE ON",         white, black, blue, white
   indicator = { tpsaen             }, "Accel Enrich",   "Accel Enrich",   white, black, green, black
   indicator = { tpsden             }, "Decel Cut",      "Decel Cut", white, black, green, black
   indicator = { revlim             }, "Rev limiter off",  "Rev limiter on", green, black, red,   black
   indicator = { overheat           }, "No overheat",  "Overheating", green, black, red,   black
   indicator = { battery < 10.5 }, "Battery OK",  "Battery LOW", green, black, red,   black

;-------------------------------------------------------------------------------
"""

runTimeSection="""
[RunTime]
   barHysteresis = 2.5 ; Seconds
   coolantBar    = -40,  100
   matBar        = -40,  100
   batteryBar    =   6,   15
   dutyCycleBar  =   0,  100
   gammaEBar     =   0,  200
   mapBar        =   0,  255
   pulseWidthBar =   0, 25.5
   rpmBar        =   0, 8000
   TPSBar   =   0,  100
   baroCorrBar   =   0,  200
   warmupCorrBar =   0,  200
   airdenCorrBar =   0,  200
   veCorrBar     =   0,  200
   accCorrBar    =   0,  100

;-------------------------------------------------------------------------------
"""

tuningSection="""
[Tuning]

;  font            = "Lucida Console", 12
;  font            = "Courier", 14
   spotDepth       =   2 ; 0 = no indicators, 1 = Z only, 2 = XYZ indicators.
   cursorDepth     =   2 ; Same as spot depth.

   ; The four radio buttons on the tuning screen select a "page" of six
   ; gauges.  The gauge numbering is the same as the front page, across
   ; then down.
   ;             1  2
   ;             3  4
   ;             5  6
   ;
   ; gaugeColumns allows you to hide or show gauges in column 2 (i.e.,
   ; gauges 2, 4 and 6).

   gaugeColumns = 2 ; Only 1 or 2 are valid.

   ;              Page 1            Page 2             Page 3            Page 4
   pageButtons  = "&EGO",           "&WUE",            "PW&1",           "PW&2"
   gauge1       = tachometer,       tachometer,        tachometer,       tachometer
   gauge2       = mapGauge,         mapGauge,          mapGauge,         mapGauge
   gauge3       = afr1Gauge,        afr1Gauge,         afr1Gauge,        afr1Gauge
   gauge4       = egoCorrGauge,     warmupEnrichGauge, pulseWidth1Gauge, pulseWidth2Gauge
   gauge5       = veBucketGauge,    veBucketGauge,     veBucketGauge,    veBucketGauge
   gauge6       = accelEnrichGauge, accelEnrichGauge,  dutyCycle1Gauge,  dutyCycle2Gauge

;-------------------------------------------------------------------------------

[AccelerationWizard]
   tpsDotBar = 0, 100
   mapDotBar = 0, 200

;-------------------------------------------------------------------------------
"""
outputChannelHeaderSection="""
[BurstMode]
   getCommand       = "A"
;getCommand       = "a\x00\x06"

[OutputChannels]
   deadValue        = { 0 } ; Convenient unchanging value.

   ochBlockSize     = 26
;ochGetCommand    = "a\x00\x06" ; Lower case so we don't get confused.
   ochGetCommand    = "A" ; Lower case so we don't get confused.

"""
datalogSection="""
[Datalog]
   ;       Channel          Label          Type    Format
   ;       --------------   ----------     -----   ------
   entry = time,            "Time",        float,  "%.3f"
   entry = second,          "SecL",        int,    "%d"
   entry = rpm,             "RPM",         int,    "%d"
   entry = cycleTime,       "ms",          float,  "%.1f"
   entry = speed,           "km/h",        int,    "%d"
   entry = MAP,        "MAP",         int,    "%d"
   entry = TPS,        "TPS",         int,    "%d"
   entry = battery,         "vBatt",       float,  "%.1f"
   entry = CLT,       "temp motor",  int,    "%d"
   entry = IAT,         "Temp air",    int,    "%d"
   entry = engine,          "Engine",      int,    "%d"
   entry = injPulseWidth,   "PW",          int,    "%d"
   entry = injStart,        "Inj adv",     int,    "%d"
   entry = advance,         "Spark Adv",   int,    "%d"
   
;-------------------------------------------------------------------------------
"""

# FATAL : print error and abort
def FATAL(lineToPrint):
    print lineToPrint
    sys.exit("FATAL ERROR !")

# WARNING : print warning only
def WARNING(lineToPrint):
    print "WARNING : %s" % lineToPrint
        
###################################################################################     
class OutputFile():
    def __init__(self, filename=None):
        self.filehandle = open(filename, "wb")
        self.output = [] # buffer to be written in file
    
        
    def close(self):
        # Actually write buffer to the file
        for line in self.output:
            self.filehandle.write("%s\n" % line)
        self.filehandle.close()
        
        
#################################
## Tuner file => Megatune INI  format
#################################
class TunerFile(OutputFile):
    def __init__(self, filename=None):
        OutputFile.__init__(self,filename)
        self.eepromSize = 0
        self.eepromSection = []
        self.outputChannelSize = 0
        self.outputChannelSection = []

    def computeSize(self, lineContent):
        size = 0
        if (lineContent['class'] in ('scalar','bits')):
            typeMember = re.sub("[a-zA-Z]", "", lineContent['type'])
            if(typeMember.isdigit()):
                size = int(typeMember) / 8
            else:
                FATAL("Type is wrong ! %s" % lineContent) 
        elif (lineContent['class'] == 'array'):
            typeMember = re.sub("[a-zA-Z]", "", lineContent['type'])
            if(typeMember.isdigit()):
                size = int(typeMember) / 8
                shapeMember = re.sub("\[|\]", "", lineContent['shape'])
                shapeList = shapeMember.split("x")  
                for item in shapeList:
                    if(item.isdigit()):
                        size *= int(item)
                    else:
                        FATAL("Shape is wrong ! %s" % lineContent)
            else:
                FATAL("Type is wrong ! %s" % lineContent) 

        return size

    def addEepromLine(self, lineContent):
        if (lineContent['class'] == 'scalar'):
            self.eepromSection.append("\t%16s\t= %s,\t%s,\t%d,\t\t\"%s\",\t%.5f,\t%.5f,\t%.2f,\t%.2f,\t%d ;\t%s" % (lineContent['name'], lineContent['class'], lineContent['type'], self.eepromSize, lineContent['units'], lineContent['scale'], lineContent['translate'], lineContent['lo'], lineContent['hi'], lineContent['digits'], lineContent['comment']))

        elif (lineContent['class'] == 'array'):
            self.eepromSection.append("\t%16s\t= %s,\t%s,\t%d,\t[%s],\t\"%s\",\t%.5f,\t%.5f,\t%d,\t%d,\t%d ;\t%s" % (lineContent['name'], lineContent['class'], lineContent['type'], self.eepromSize, lineContent['shape'], lineContent['units'], lineContent['scale'], lineContent['translate'], lineContent['lo'], lineContent['hi'], lineContent['digits'], lineContent['comment']))

        elif (lineContent['class'] == 'bits'): # TODO : manage more than 1 bit
            self.eepromSection.append("\t%16s\t= %s,\t%s,\t%d, [%d:%d], \"%s\", \"%s\";\t%s" % (lineContent['name'], lineContent['class'], lineContent['type'], self.eepromSize, lineContent['start'], lineContent['stop'], lineContent['state0'], lineContent['state1'], lineContent['comment']))
        else:
            FATAL("EEPROM class not supported %s" % lineContent)

        self.eepromSize += self.computeSize(lineContent)
    
    def addOutputChannelLine(self, lineContent):
        if (lineContent['class'] == 'scalar'):
            self.eepromSection.append("\t%16s\t= %s,\t%s,\t%d,\t\t\"%s\",\t%.5f,\t%.5f,\t%.2f,\t%.2f,\t%d ;\t%s" % (lineContent['name'], lineContent['class'], lineContent['type'], self.eepromSize, lineContent['units'], lineContent['scale'], lineContent['translate'], lineContent['lo'], lineContent['hi'], lineContent['digits'], lineContent['comment']))

        elif (lineContent['class'] == 'array'):
            self.eepromSection.append("\t%16s\t= %s,\t%s,\t%d,\t[%s],\t\"%s\",\t%.5f,\t%.5f,\t%d,\t%d,\t%d ;\t%s" % (lineContent['name'], lineContent['class'], lineContent['type'], self.eepromSize, lineContent['shape'], lineContent['units'], lineContent['scale'], lineContent['translate'], lineContent['lo'], lineContent['hi'], lineContent['digits'], lineContent['comment']))

        elif (lineContent['class'] == 'bits'): # TODO : manage more than 1 bit
            self.eepromSection.append("\t%16s\t= %s,\t%s,\t%d, [%d:%d], \"%s\", \"%s\";\t%s" % (lineContent['name'], lineContent['class'], lineContent['type'], self.eepromSize, lineContent['start'], lineContent['stop'], lineContent['state0'], lineContent['state1'], lineContent['comment']))
        else:
            FATAL("EEPROM class not supported %s" % lineContent)

        self.outputChannelSize += self.computeSize(lineContent)

    def close(self):
        # Assemble and update sections to output buffer
        # update version in MegaTune section
        global megaTuneSection
        self.output.append(megaTuneSection)

        # update EEPROM size in eepromHeaderSection (called "Constants" in TunerStudio)
        global eepromHeaderSection
        eepromHeaderSection = re.sub("   pageSize            = 81","   pagesize            = %d" % self.eepromSize, eepromHeaderSection)  
        self.output.append(eepromHeaderSection)
        # EEPROM content
        for item in self.eepromSection:
            self.output.append(item)
        
        # Menu section
        global menuSection
        self.output.append(menuSection)

        # User defined menu
        global userDefinedMenu
        self.output.append(userDefinedMenu)

        # Curve editor
        global curveEditorSection
        self.output.append(curveEditorSection)

        # Table editor
        global tableEditorSection
        self.output.append(tableEditorSection)

        # Gauge configuration
        global gaugeConfigSection
        self.output.append(gaugeConfigSection)

        # Front page
        global frontpageSection
        self.output.append(frontpageSection)

        # Runtime
        global runTimeSection
        self.output.append(runTimeSection)

        # Tuning 
        global tuningSection
        self.output.append(tuningSection)
        
        # Output channel header : update size of the block
        global outputChannelHeaderSection
        outputChannelHeaderSection = re.sub("   ochBlockSize\s*= 26","   ochBlockSize = %d" % self.outputChannelSize, outputChannelHeaderSection)  
        self.output.append(outputChannelHeaderSection)
        # Output channel content
        for item in self.outputChannelSection:
            self.output.append(item)

        # Datalog Section
        global datalogSection
        self.output.append(datalogSection)

        # Actual write to file
        OutputFile.close(self)

#################################
## Doc file => Markdown format
#################################
class DocFile(OutputFile):
    def __init__(self, filename=None):
        OutputFile.__init__(self,filename)


#################################
## Source file => C header format
#################################
class SourceFile:
    def __init__(self, filename=None):
        self.fileObj = open(filename, "rb")
        # read the whole content in buffer and preprocess it
        self.content = self.fileObj.readlines()
        self.eepromContent = [] # list of EEPROM parameters
        self.outputChannelContent = [] # list of output channel parameters
        
        # get table size define 
        self.tabsize = self.getTabsize()
    
        # Extract EEPROM section
        self.processEeprom()

        # Extract output channel section
        self.processOutputChannel()


    def close(self):
        self.fileObj.close()

    def dbgPrint(self):
        print self.content
    
    def findTabsize(self, line):
        if "define TABSIZE" in line:
            return True
        else:
            return False

    def getTabsize(self):
        found = filter(self.findTabsize, self.content)
        if(len(found) == 0): # hum strange
            WARNING("Tabsize not found")
            return 0
        elif(len(found) > 1):
            FATAL("Several definition of TABSIZE")
        else:
            # TODO
            return 10

    def processEeprom(self):
        # Search for EEPROM section: start contains "typedef struct", end contains "}eeprom_data_t;"
        startList = []
        eepromBound = {}
        for i,line in enumerate(self.content):
            if "typedef struct" in line: #we will have several match
                startList.append(i)
            elif "}eeprom_data_t" in line:
                if len(startList) == 0:
                    FATAL("EEPROM section malformed")
                else:
                    eepromBound['start'] = startList[-1]+1
                    eepromBound['end'] = i

        # extract content line by line
        for line in self.content[eepromBound['start']: eepromBound['end']]:
            if "scalar" in line:
                # sorry for this... easier debug on regex101.com
                p = re.compile(ur"""(?P<ctype>[u|U|s|S][0-9]+)\s*(?P<name>.+);\s*(\/\*)\s*(?P<class>scalar),\s*(?P<type>[U|S][0-9]+),\s*(?P<offset>..),\s*"(?P<units>.*)",\s*(?P<scale>[0-9|\.]+),\s*(?P<translate>[0-9|\.]+),\s*(?P<lo>[0-9|\.]+),\s*(?P<hi>[0-9|\.]+),\s*(?P<digits>[0-9|\.]+)\s*;\s*(?P<comment>.*)\*\/""", re.VERBOSE)
                mo = re.search(p, line)
                if mo:
                    # affect the values
                    decodedLine = {'name' : mo.group("name"), 'class' : "scalar", 'type' : mo.group("type"), 'offset' : 0, 'shape' : "", 'units' : mo.group("units"), 'scale' : float(mo.group("scale")), 'translate' : float(mo.group("translate")), 'lo' : float(mo.group("lo")), 'hi' : float(mo.group("hi")), 'digits' : int(mo.group("digits")), 'comment' :mo.group("comment")};
                    self.eepromContent.append(decodedLine)
                else:
                    FATAL("Line not decodable %s" % line) 
            
            elif "array" in line:
                # replace TABSIZE
                lineclean = str(line).replace("TABSIZE", str(self.getTabsize()))
                # sorry for this... easier debug on regex101.com
                pSingleDim = re.compile(ur"""(?P<ctype>[u|U|s|S][0-9]+)\s*(?P<name>.+)\[(?P<tabsize>[0-9]+)\];\s*(\/\*)\s*(?P<class>array),\s*(?P<type>[U|S][0-9]+),\s*(?P<offset>..),\s*\[(?P<shape>[0-9]+)\],\s*"(?P<units>.*)",\s*(?P<scale>[0-9|\.]+),\s*(?P<translate>[0-9|\.]+),\s*(?P<lo>[0-9|\.]+),\s*(?P<hi>[0-9|\.]+),\s*(?P<digits>[0-9|\.]+)\s*;\s*(?P<comment>.*)\*\/""", re.VERBOSE)
                pDualDim = re.compile(ur"""(?P<ctype>[u|U|s|S][0-9]+)\s*(?P<name>.+)\[(?P<tabsize>[0-9]+)\]\[(?P<tabsize2>[0-9]+)\];\s*(\/\*)\s*(?P<class>array),\s*(?P<type>[U|S][0-9]+),\s*(?P<offset>..),\s*\[(?P<shape>.+)\],\s*"(?P<units>.*)",\s*(?P<scale>[0-9|\.]+),\s*(?P<translate>[0-9|\.]+),\s*(?P<lo>[0-9|\.]+),\s*(?P<hi>[0-9|\.]+),\s*(?P<digits>[0-9|\.]+)\s*;\s*(?P<comment>.*)\*\/""", re.VERBOSE)
                mo = re.search(pSingleDim, lineclean)
                moDual = re.search(pDualDim, lineclean)
                if mo:
                    # affect the values
                    decodedLine = {'name' : mo.group("name"), 'class' : "array", 'type' : mo.group("type"), 'offset' : 0, 'shape' : mo.group("shape"), 'units' : mo.group("units"), 'scale' : float(mo.group("scale")), 'translate' : float(mo.group("translate")), 'lo' : float(mo.group("lo")), 'hi' : float(mo.group("hi")), 'digits' : int(mo.group("digits")), 'comment' :mo.group("comment")};
                    self.eepromContent.append(decodedLine)
                elif moDual:
                    # affect the values
                    decodedLine = {'name' : moDual.group("name"), 'class' : "array", 'type' : moDual.group("type"), 'offset' : 0, 'shape' : moDual.group("shape"), 'units' : moDual.group("units"), 'scale' : float(moDual.group("scale")), 'translate' : float(moDual.group("translate")), 'lo' : float(moDual.group("lo")), 'hi' : float(moDual.group("hi")), 'digits' : int(moDual.group("digits")), 'comment' :moDual.group("comment")};
                    self.eepromContent.append(decodedLine)
                else:
                    FATAL("Line not decodable %s" % line) 

            elif "bits" in line:
                # sorry for this... easier debug on regex101.com
                # TODO : More than 1 bit bitfields are not yet managed
                p = re.compile(ur"""(?P<ctype>[u|U|s|S][0-9]+)\s*(?P<name>.+);\s*(\/\*)\s*(?P<class>bits),\s*(?P<type>[U|S][0-9]+),\s*(?P<offset>..),\s*\[(?P<start>[0-9]):(?P<stop>[0-9])\],\s*"(?P<state0>.*)",\s*"(?P<state1>.*)"\s*;\s*(?P<comment>.*)\*\/""", re.VERBOSE)
                mo = re.search(p, line)
                if mo:
                    # affect the values
                    decodedLine = {'name' : mo.group("name"), 'class' : "bits", 'type' : mo.group("type"), 'offset' : 0, 'start' : int(mo.group("start")), 'stop' : int(mo.group("stop")), 'state0': mo.group("state0"), 'state1': mo.group("state1"),  'comment' :mo.group("comment")};
                    self.eepromContent.append(decodedLine)
                else:
                    FATAL("Line not decodable %s" % line) 

            else:
                FATAL("Line not decodable %s" % line) 




    def processOutputChannel(self):
        # Search for output channel section: start contains "typedef struct", end contains "}Current_Data_t;"
        startList = []
        outcBound = {}
        for i,line in enumerate(self.content):
            if "typedef struct" in line: #we will have several match
                startList.append(i)
            elif "}Current_Data_t" in line:
                print "end !"
                if len(startList) == 0:
                    FATAL("Output Channel/current Data section malformed")
                else:
                    outcBound['start'] = startList[-1]+1
                    outcBound['end'] = i
        # extract content line by line
        for line in self.content[outcBound['start']: outcBound['end']]:
            pass





def main(argv):
    if(len(argv) != 1):
        print "Input parameters is only the varDef.h file !"
        exit(1)

    # open files
    varDefFile = SourceFile(argv[0])
    tunerFile = TunerFile("solextronic.ini")
    docFile = DocFile("commands.wiki")

    #varDefFile.dbgPrint()
    
    # Process EEPROM memory structure
    lineContent = {'name' : "toto", 'class' : "scalar", 'type' : "U08", 'offset' : 0, 'shape' : "", 'units' : "%", 'scale' : 1.0, 'translate' : 0.0, 'lo' : 0, 'hi' : 255, 'digits' : 0, 'comment' :"un super exemple"};
    for item in varDefFile.eepromContent:
        tunerFile.addEepromLine(item)

    # Process outputchannel structure
    for item in varDefFile.outputChannelContent:
        tunerFile.addOutputChannelLine(item)

    # close files : this will actually write to output files
    varDefFile.close()
    tunerFile.close()
    docFile.close()


# Starting point
if __name__ == "__main__":
        main(sys.argv[1:])
