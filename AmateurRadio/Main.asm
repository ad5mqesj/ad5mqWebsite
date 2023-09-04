;Main.asm by esj 16 July 2006
;the main synthesizer control is very simple,
;blink the led (RC0) until lockup occurs
;send the setup to the synthesizer chip, if RA1 is low set to 1152 MHz else to
;programmed freq in this file
;when lock happens, RC0 is high - indicating lock, if lock is lost its off

;here are the defined controls for the synthesizer
;we set the prescaler divider to 32/33, and the R divider to 10
;this gives an output freq (in MHz) of 32*B+A where
;B is the lower 5 bits of upper byte and middle byte of reg 1 (13 bits total) and 
;A is the top 6 bits of reg 1 (bottom two bits are reg address)

;control register 03 sets up prescaler, output current
SYNTCTL1H	EQU	H'81'
SYNTCTL1M	EQU	H'00'
SYNTCTL1L	EQU	H'13'
;control register 00 rets up r
SYNTCTL2H	EQU	H'10'
SYNTCTL2M	EQU	H'00'
SYNTCTL2L	EQU	H'28'
;FREQ for 1152 MHz (test mode); B=36, A=0
FRQTSTH		EQU H'00'
FRQTSTM		EQU H'24'	;24
FRQTSTL		EQU H'01'
;FRQ normal mode 1966 MHz B=61, A=14
FRQNORH		EQU H'00'
FRQNORM		EQU H'3D'
FRQNORL		EQU H'39'

;signal line bits Connected to Port C (these are bit nubers for bsf, bcf ops
SYNCE		EQU 4
SYNCLK		EQU 3
SYNDAT		EQU 2
SYNLE		EQU 1

LOCK		EQU 0
;sense bit for mode will be A1, A0 is lock
SENS		EQU 1

	LIST   P=16F57
#include <p16F5X.inc>

	__config (_HS_OSC & _WDT_OFF & _CP_OFF)

	cblock 0x10
Delay1			; Define two file registers for the
Delay2			; delay loop
SynByte			; this is the data for each Synth register
bitcnt			; bit counter
	endc
	
	org 0
Start
	;set port input and output pins, all unused pins are inputs
	movlw H'E0'		; set upper 3 bits, makes C[7:5] in, C[4,0] out
	tris PORTC

	bcf	PORTC, SYNDAT
	bcf	PORTC, SYNCLK	;make sure clock ,data, and le lines are low before we start
	bcf PORTC, SYNLE
	bsf PORTC, SYNCE	;turn on synthesizer
	nop
	nop					
	nop
	nop
	nop					;wait a couple µS
	;now send the various control words
	movlw SYNTCTL1H
	call SendByte
	movlw SYNTCTL1M
	call SendByte
	movlw SYNTCTL1L
	call SendByte
	;now latch data in
	bsf PORTC, SYNLE
	nop
	bcf PORTC, SYNLE
	nop

	;control word 2
	movlw SYNTCTL2H
	call SendByte
	movlw SYNTCTL2M
	call SendByte
	movlw SYNTCTL2L
	call SendByte
	;now latch data in
	bsf PORTC, SYNLE
	nop					;wait a couple µS
	bcf PORTC, SYNLE
	nop

;check port A(1) if low set TEST FRQ (1152) else set command freq
	btfsc PORTA,SENS
	goto DefFrq

	movlw FRQTSTH
	call SendByte
	movlw FRQTSTM
	call SendByte
	movlw FRQTSTL
	call SendByte
	;now latch data in
	bsf PORTC, SYNLE
	nop					;wait a couple µS
	bcf PORTC, SYNLE
	nop
	goto MainLoop
DefFrq
	movlw FRQNORH
	call SendByte
	movlw FRQNORM
	call SendByte
	movlw FRQNORL
	call SendByte
	;now latch data in
	bsf PORTC, SYNLE
	nop					;wait a couple µS
	bcf PORTC, SYNLE
	nop

MainLoop
	bsf	PORTC,LOCK		; turn on LED C0
	movlw	.254		; Delay about 192mS plus whatever was above
	call	Delay
	;if lock is set just go back up, else turn off led for blink
	btfsc PORTA,LOCK
	goto MainLoop
	bcf	PORTC,LOCK	; Turn off LED C0
	movlw	.254		; Delay about 192mS plus whatever was above
	call	Delay


	goto	MainLoop	; Do it again...

;function Delay
; Delay Function.  Enter with number 771uS delays in Wreg
Delay	movwf	Delay2		;
DelayLoop
	nop
	nop
	nop
	nop
	decfsz	Delay1,f	; Waste time.  
	goto	DelayLoop	; The Inner loop takes 3 instructions per loop * 256 loopss = 768 instructions
	decfsz	Delay2,f	; The outer loop takes and additional 3 instructions per lap * 256 loops
	goto	DelayLoop	; (768+3) * 256 = 197376 instructions / 1M instructions per second = 0.197 sec.
				; call it a two-tenths of a second.
	return

;send a byte to synth - byte in W -> SynByte and sent MSB first
SendByte 
	movwf SynByte	;put reg dat in SynByte
	movlw .9
	movwf bitcnt	;number of bits
nextbit
	nop
	decfsz bitcnt,1	;decrement bit counter, exit when done
	goto stillsend
	return
stillsend
	btfsc SynByte,7 ;check MSB if 0 skip next instr
	goto  bitHi
	;send a 0 bit to synth
	bcf	PORTC, SYNCLK		;clock low
	nop
	bcf	PORTC, SYNDAT		;data (0)
	nop
	bsf	PORTC, SYNCLK		;clock high
	nop						;1µS
	bcf	PORTC, SYNCLK		;clock low
	RLF SynByte,1			;move over to next bit
	goto nextbit
bitHi
	;send a 0 bit to synth
	bcf	PORTC, SYNCLK		;clock low
	nop
	bsf	PORTC, SYNDAT		;data (1)
	nop
	bsf	PORTC, SYNCLK		;clock high
	nop						;1µS
	bcf	PORTC, SYNCLK		;clock low
	RLF SynByte,1			;move over to next bit
	goto nextbit
;end of function to write to synthesizer

	end
