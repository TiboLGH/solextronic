;***************************************************************************
; *   Copyright (C) 2012 by Thibault Bouttevin                              *
; *   thibault.bouttevin@gmail.com                                          *
; *   www.legalethurlant.fr.st                                              *
; *                                                                         *
; *   This file is part of Solextronic                                      *
; *                                                                         *
; *   Solextronic is free software; you can redistribute it and/or modify   *
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
; ***************************************************************************/
; -----------------------------------------------------------------------
; Source file for test board RPM and speed simulator
; -----------------------------------------------------------------------
; Pinout
; Pin 1 : Vdd
; Pin 2 : GP5 | OUT | logic    | Top motor
; Pin 3 : GP4 | OUT | logic    | Top Wheel
; Pin 4 : GP3 | NC  
; Pin 5 : GP2 | NC  
; Pin 6 : GP1 | IN  | analog   | potentiometer for RPM
; Pin 7 : GP0 | IN  | analog   | potentiometer for wheel speed
; Pin 8 : Vss

    #include <p12f675.inc>

; -----------------------------------------------------------------------
; Configuration bits
    __CONFIG _INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _BODEN_OFF & _CP_OFF & _CPD_OFF

; -----------------------------------------------------------------------
; Constants
    #define RELOAD		D'9'    ; Load of timer0 to overflow once per 1ms
    #define PRECISION   2       ; byte size for registers
    #define WHEEL_ON    0       ; if bit is 0, keep low output
    #define MOTOR_ON    1       ; if bit is 0, keep low output
    #define WHEEL_OUT   4
    #define MOTOR_OUT   5
    #define WHEEL_ADC   0
    #define RPM_ADC     1
    #define DEBUG       3
    #define MIN_VAL     D'5'    ; if ADC is lower than this threshold, output keeps stopped
    #define HIGH_DUR    D'1'    ; time is high state in ms
    #define MAX_RPM     D'5'    ; period for max RPM : 5ms => 12000rpm
    #define MIN_RPM     D'200'  ; period for min RPM : 200ms => 300rpm
    #define M_MOTOR     D'199' ; for interpolation : M = (200-5)/(5-255) * 256 = -199 
    #define B_MOTOR     D'52198'; for interpolation : B = (5*5-255*200)/(5-255) * 256 = 52198 
    #define MAX_SPEED   D'65'   ; period for max speed : 65ms => 100km/h
    #define MIN_SPEED   D'1310' ; period for min speed : 1310ms => 5km/h
    #define M_SPEED     D'5'   ; for interpolation : M = (1310-65)/(5-255) = -5 
    #define B_SPEED     D'1349' ; for interpolation : B = (5*64-255*1310)/(5-255) = 1349 
; -----------------------------------------------------------------------
; Macros 
BANK0   macro               ; switch to bank0
        bcf STATUS,RP0
    endm

BANK1   macro               ; switch to bank1
        bsf STATUS,RP0
    endm

; -----------------------------------------------------------------------
; Variables
	cblock 	0x20
	w_saved         ; context saving
	status_saved    ; context saving

	state	    ; (0x22) 0 period off, 1 period on. 1 bit for wheel, one for motor
    motorAdc    ; (0x23) ADC value on 8 bits 
    wheelAdc    ; (0x24) ADC value on 8 bits
    motorLow    ; (0x25) duration is low state for motor signal
    wheelLow    ; (0x26) duration in low state for wheel signal (16 bits)
    wheelLowH   ; (0x27) 
    motorCur    ; (0x28) motor loop current counter
    wheelCur    ; (0x29) wheel loop current counter (16 bits)
    wheelCurH   ; (0x2A) 
	
    ; Registers for 16-bit computation
	REG_X        ; (0x2B/2C) 2 bytes
	REG_X_H
	REG_Y        ; (0x2D/2E) 2 bytes
	REG_Y_H
	REG_Z        ; (0x2F/30) 2 bytes
	REG_Z_H
	REG_COUNTER          ; (0x31) 1 byte
	REG_STATUS           ; (0x32) 1 byte
	REG_T1               ; (0x33) 1 byte
	REG_T2               ; (0x34) 1 byte
	REG_ROT_COUNTER      ; (0x35) 1 byte  ==> total 11 bytes
    ;Temp Regs
    TEMP1        ; (0x36/37) 2 bytes
    TEMP1_H    
    TEMP2        ; (0x38/39) 2 bytes
    TEMP2_H    
    TEMP3        ; (0x3A/3B) 2 bytes
    TEMP3_H    
	endc
; -----------------------------------------------------------------------
; reboot vector
;STARTUP CODE 0x000
	org	0x0000
    nop                    ; for ICD2 debug
    movlw   high start
    movwf   PCLATH
    goto    start

; interrupt vector
;INT_VECTOR CODE 0x004
	org	0x0004
    goto    interrupt

;PROG CODE
interrupt
    movwf   w_saved
    swapf   STATUS,w
    movwf   status_saved
timer0int
    BANK0
    btfss   INTCON,T0IF     ; test of timer0 interrupt
    goto    restoreg        ; no -> exit isr
    bcf     INTCON,T0IF     ; clear interrupt flag
    movlw   RELOAD
    movwf   TMR0            ; reload timer for next ovf
wheelFSM
    btfsc   state, WHEEL_ON ; check if wheel signal is enable
    goto    wheelRun        ; yes => generate signal
    bcf     GPIO, WHEEL_OUT ; no => for to low
    goto    motorFSM
wheelRun
    movlw   wheelCur        ; decrement current counter
    call    M_DEC
    movlw   wheelCur
    call    M_IF_ZERO
    btfss   STATUS,Z        ; 0 ? timeout 
    goto    motorFSM          ; no, let's check RPM
    btfss   state,WHEEL_OUT ; wheel state is high ?
    goto    wheelLOW        ; no -> jump to low state
wheelHIGH                   ; high state
switchToLow                 ; time to switch to low state
    bcf     state,WHEEL_OUT ; set state to low
    bcf     GPIO, WHEEL_OUT ; set ouput pin
    movf    wheelLowH, 0    ; reload current count
    movwf   wheelCurH 
    movf    wheelLow, 0
    movwf   wheelCur 
    goto    motorFSM
wheelLOW                    ; low state
switchToHigh                ; time to switch to high state
    bsf     state,WHEEL_OUT ; set state to high
    bsf     GPIO, WHEEL_OUT ; set ouput pin
    clrf    wheelCurH       ; reload current count
    movlw   HIGH_DUR
    movwf   wheelCur 

; Motor signal management
motorFSM
    btfsc   state, MOTOR_ON ; check if motor signal is enable
    goto    motorRun        ; yes => generate signal
    bcf     GPIO, MOTOR_OUT ; no => for to low
    goto    restoreg
motorRun
    decfsz  motorCur,f      ; decrement current counter
    goto    restoreg        ; no, exit isr
    btfss   state,MOTOR_OUT ; motor state is high ?
    goto    motorLOW        ; no -> jump to low state
motorHIGH                   ; high state
motorSwitchToLow            ; time to switch to low state
    bcf     state,MOTOR_OUT ; set state to low
    bcf     GPIO, MOTOR_OUT ; set ouput pin
    movf    motorLow, 0    ; reload current count
    movwf   motorCur 
    goto    restoreg
motorLOW                    ; low state
motorSwitchToHigh           ; time to switch to high state
    bsf     state,MOTOR_OUT ; set state to high
    bsf     GPIO, MOTOR_OUT ; set ouput pin
    movlw   LOW(HIGH_DUR)   ; reload current count
    movwf   motorCur 

restoreg
    swapf   status_saved,w ; restauration du contexte
    movwf   STATUS
    swapf   w_saved,f
    swapf   w_saved,w
    retfie


; main program
start
; Init
; Oscillator calibration. Disable for Simulation !
    BANK1
    call    0x3FF
    movwf   OSCCAL
; I/O settings
    BANK1
    movlw   B'00000011'
    movwf   TRISIO
    clrf    WPU
    BANK0
    clrf    GPIO
; Timer0 settings : prescaler 1/4, initial value 5 : ovf each 1ms
    BANK0
    movlw   RELOAD
    movwf   TMR0
    BANK1
    movlw   B'10000001'
    movwf   OPTION_REG
; ADC settings
    BANK1
    movlw   B'00010011' ; conversion time Fosc/2, adc on GP0/GP1
    movwf   ANSEL
    BANK0
    movlw   B'00000001' ; ADC on, left justified (8bits only) ref as Vdd
    movwf   ADCON0
; interrupts config
    BANK0 
    movlw   B'10000000' ; disable timer interrupt, will be started after 1st conversion
    movwf   INTCON
; variables init
	clrf    state
    clrf    motorAdc
    clrf    wheelAdc
    movlw   D'10'
    movwf   motorCur
    movwf   wheelCur
    clrf    wheelCurH
    clrf    motorLow
    clrf    wheelLow
    clrf    wheelLowH
    ;bsf     GPIO, DEBUG ;DBG

main ; in background task : ADC conversions and period computations run in loop
    ; Conversion on GP0 : wheel speed
    BANK0
    movlw   B'00000011' ; ADC on, left justified (8bits only) ref as Vdd
    movwf   ADCON0
waitConv0
    btfsc   ADCON0, 1   ; wait for conversion end
    goto    waitConv0
    movf    ADRESH,0
    ;movlw   D'50'       ; debug
    movwf   wheelAdc
    ; Compute high and low periods : 
    ; if adc < MIN_VAL => totally off, else
    ; high = 1ms fixed 
    ; low : 
    ; Max => 255     <=> 64ms low period <=> 100km/h 
    ; Min => MIN_VAL <=> 1310ms low period <=> 5km/h 
    movlw   MIN_VAL
    subwf   wheelAdc,w
    btfss  STATUS,C    ; Test for Carry Flag Clear
    goto   wheelTooLow ; wheelAdc < MIN_VAL
wheelComputeSignal
    clrf    TEMP1_H
    movf    wheelAdc,w
    movwf   TEMP1
    movlw   HIGH(M_SPEED)
    movwf   TEMP2_H
    movlw   LOW(M_SPEED)
    movwf   TEMP2
    movlw   HIGH(B_SPEED)
    movwf   TEMP3_H
    movlw   LOW(B_SPEED)
    movwf   TEMP3
    call    interpolation ; Y = M.X + B => X/Y in TEMP1, M in TEMP2, B in TEMP3
    movf    TEMP1_H,w
    movwf   wheelLowH
    movf    TEMP1,w
    movwf   wheelLow
    bsf     state, WHEEL_ON
    goto    conv1
wheelTooLow
    bcf     state, WHEEL_ON

; Conversion on GP1 : Motor
conv1
    movlw   B'00000111' ; ADC on, left justified (8bits only) ref as Vdd
    movwf   ADCON0
waitConv1
    btfsc   ADCON0, 1   ; wait for conversion end
    goto    waitConv1
    movf    ADRESH,0
    movwf   motorAdc
    ; Compute high and low periods : 
    ; if adc < MIN_VAL => totally off, else
    ; high = 1ms fixed 
    ; low : 
    ; Max => 255     <=> 5ms low period <=> 12000rpm 
    ; Min => MIN_VAL <=> 200ms low period <=> 300rpm 
    movlw   MIN_VAL
    subwf   motorAdc,w
    btfss   STATUS,C    ; Test for Carry Flag Clear
    goto    motorTooLow ; motorAdc < MIN_VAL
motorComputeSignal
    clrf    TEMP1_H
    movf    motorAdc,w
    movwf   TEMP1
    movlw   HIGH(M_MOTOR)
    movwf   TEMP2_H
    movlw   LOW(M_MOTOR)
    movwf   TEMP2
    movlw   HIGH(B_MOTOR)
    movwf   TEMP3_H
    movlw   LOW(B_MOTOR)
    movwf   TEMP3
    call    interpolation ; Y = M.X + B => X/Y in TEMP1, M in TEMP2, B in TEMP3
    movf    TEMP1_H,w   ; we get only high byte to dived by 256
    movwf   motorLow
    bsf     state, MOTOR_ON
    goto    endConv
motorTooLow
    bcf     state, MOTOR_ON

; Conversion done, we can run signal generation
endConv
    BANK0 
    movlw   B'10100000' ; enable timer interrupt
    movwf   INTCON

    goto    main  

; -----------------------------------------------------------------------
; Small 16-bit math lib generated by  
; http://avtanski.net/projects/math/
; -----------------------------------------------------------------------

M_STOR_STATUS macro WHERE
    movf    STATUS,w
    movwf   WHERE
    endm

M_RETR_STATUS macro WHERE
    movf    WHERE,w
    movwf   STATUS
    endm

    
M_CLR                           ; clear a register
    movwf   FSR
    movlw   PRECISION
    movwf   REG_COUNTER
M_CLR_loop
    clrf    INDF
    incf    FSR,f
    decf    REG_COUNTER,f
    btfss   STATUS,Z
    goto    M_CLR_loop
    return

M_INC                           ; increment a register
    movwf   FSR
    movlw   PRECISION
    movwf   REG_COUNTER
M_INC_loop
    incf    INDF,f
    btfss   STATUS,Z
    return
    incf    FSR,f
    decf    REG_COUNTER,f
    btfss   STATUS,Z
    goto    M_INC_loop
    return


M_DEC                           ; decrement a register
    movwf   FSR
    movlw   PRECISION
    movwf   REG_COUNTER
M_DEC_loop
    decf    INDF,f
    movlw   0xFF
    subwf   INDF,w
    btfss   STATUS,Z
    return
    incf    FSR,f
    decf    REG_COUNTER,f
    btfss   STATUS,Z
    goto    M_DEC_loop
    return


M_ROL                           ; rotate a register to the left
    movwf   FSR
    M_STOR_STATUS REG_STATUS
    clrf    REG_COUNTER
M_ROL_loop
    M_RETR_STATUS REG_STATUS
    rlf     INDF,f
    M_STOR_STATUS REG_STATUS
    incf    FSR,f
    incf    REG_COUNTER,f
    movlw   PRECISION
    subwf   REG_COUNTER,w
    btfss   STATUS,Z
    goto    M_ROL_loop
    return


M_ROR                           ; rotates a register to the right
    movwf   FSR
    movlw   PRECISION-1
    addwf   FSR,f
    M_STOR_STATUS REG_STATUS
    clrf    REG_COUNTER
M_ROR_loop
    M_RETR_STATUS REG_STATUS
    rrf     INDF,f
    M_STOR_STATUS REG_STATUS
    decf    FSR,f
    incf    REG_COUNTER,f
    movlw   PRECISION
    subwf   REG_COUNTER,w
    btfss   STATUS,Z
    goto    M_ROR_loop
    return


M_CMP                           ; Z <=> X -> STATUS(C,Z)
                                ; STATUS,C set if Z => X;
                                ; STATUS,Z set if Z == X
    clrf    REG_COUNTER
M_CMP_loop
    movf    REG_COUNTER,w
    sublw   REG_Z+PRECISION-1
    movwf   FSR
    movf    INDF,w
    movwf   REG_T1
    movf    REG_COUNTER,w
    sublw   REG_X+PRECISION-1
    movwf   FSR
    movf    INDF,w
    subwf   REG_T1,f
    btfss   STATUS,Z
    return
    incf    REG_COUNTER,f
    movlw   PRECISION
    subwf   REG_COUNTER,w
    btfss   STATUS,Z
    goto    M_CMP_loop
    return


M_ADD                           ; Z + X -> Z
    bcf     STATUS,C
    clrf    REG_STATUS
    clrf    REG_COUNTER
M_ADD_loop
    clrf    REG_T1
    btfsc   REG_STATUS,C
    incf    REG_T1,f
    clrf    REG_STATUS
    movlw   REG_X
    addwf   REG_COUNTER,w
    movwf   FSR
    movf    INDF,w
    addwf   REG_T1,f
    btfsc   STATUS,C
    bsf     REG_STATUS,C
    movlw   REG_Z
    addwf   REG_COUNTER,w
    movwf   FSR
    movf    INDF,w
    addwf   REG_T1,f
    btfsc   STATUS,C
    bsf     REG_STATUS,C
    movf    REG_T1,w
    movwf   INDF
    incf    REG_COUNTER,f
    movlw   PRECISION
    subwf   REG_COUNTER,w
    btfss   STATUS,Z
    goto    M_ADD_loop
    return


M_SUB                           ; Z - X -> Z
    clrf    REG_COUNTER
    bsf     REG_STATUS,C
M_SUB_loop
    bsf     REG_T2,C
    movlw   REG_Z
    addwf   REG_COUNTER,w
    movwf   FSR
    movf    INDF,w
    movwf   REG_T1
    movlw   REG_X
    addwf   REG_COUNTER,w
    movwf   FSR
    movf    INDF,w
    subwf   REG_T1,f
    btfss   STATUS,C
    bcf     REG_T2,C
    btfsc   REG_STATUS,C
    goto    M_SUB_no_carry
    movlw   0x01
    subwf   REG_T1,f
    btfss   STATUS,C
    bcf     REG_T2,C
M_SUB_no_carry
    movlw   REG_Z
    addwf   REG_COUNTER,w
    movwf   FSR
    movf    REG_T1,w
    movwf   INDF
    bsf     REG_STATUS,C
    btfss   REG_T2,C
    bcf     REG_STATUS,C
    incf    REG_COUNTER,f
    movlw   PRECISION
    subwf   REG_COUNTER,w
    btfss   STATUS,Z
    goto    M_SUB_loop
    btfss   REG_STATUS,C
    bcf     STATUS,C
    return


M_MUL                           ; X * Y -> Z
    movlw   REG_Z
    call    M_CLR
    movlw   PRECISION*8+1
    movwf   REG_ROT_COUNTER
M_MUL_loop
    decf    REG_ROT_COUNTER,f
    btfsc   STATUS,Z
    return
    btfsc   REG_Y,0
    call    M_ADD
    bcf     STATUS,C
    movlw   REG_Y
    call    M_ROR
    bcf     STATUS,C
    movlw   REG_X
    call    M_ROL
    goto    M_MUL_loop


M_DIV                           ; Z / X -> Y;  remainder -> Z
    movlw   REG_Y
    call    M_CLR
    movlw   PRECISION*8
    movwf   REG_ROT_COUNTER
M_DIV_rot_loop
    btfsc   REG_X+PRECISION-1,7
    goto    M_DIV_loop
    movlw   REG_X
    bcf     STATUS,C
    call    M_ROL
    decf    REG_ROT_COUNTER,f
    btfss   STATUS,Z
    goto    M_DIV_rot_loop
    bsf     STATUS,Z
    return
M_DIV_loop
    call    M_CMP
    M_STOR_STATUS REG_T2
    movlw   REG_Y
    call    M_ROL
    M_RETR_STATUS REG_T2
    btfsc   STATUS,C
    call    M_SUB
    bcf     STATUS,Z
    bcf     STATUS,C
    movlw   REG_X
    call    M_ROR
    incf    REG_ROT_COUNTER,f
    movlw   PRECISION*8+1
    subwf   REG_ROT_COUNTER,w
    btfss   STATUS,Z
    goto    M_DIV_loop
    return    

; Added by Thibault, only for 16 bits
M_IF_ZERO                   ; check if REG is zero
                            ; STATUS,Z set if REG == 0
    movwf   FSR
    movlw   PRECISION
    movwf   REG_COUNTER
M_ZERO_loop
    movf    INDF,f
    btfss   STATUS,Z
    return
    incf    FSR,f
    decf    REG_COUNTER,f
    btfss   STATUS,Z
    goto    M_DEC_loop
    bsf     STATUS,Z        ; all 0 
    return

; interpolation Y = M.X + B
; as M is negative => Y = B - M.X
; X in TEMP1
; M in TEMP2
; B in TEMP3
interpolation
    ; 1st step M.X
    movf    TEMP1_H,w
    movwf   REG_X_H
    movf    TEMP1,w
    movwf   REG_X
    movf    TEMP2_H,w
    movwf   REG_Y_H
    movf    TEMP2,w
    movwf   REG_Y
    call    M_MUL
    ; 2nd step : B-M.X
    movf    REG_Z_H,w
    movwf   REG_X_H
    movf    REG_Z,w
    movwf   REG_X
    movf    TEMP3_H,w
    movwf   REG_Z_H
    movf    TEMP3,w
    movwf   REG_Z
    call    M_SUB
    movf    REG_Z_H,w
    movwf   TEMP1_H
    movf    REG_Z,w
    movwf   TEMP1
    return
    
	end
