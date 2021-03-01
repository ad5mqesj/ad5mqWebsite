/*note : EPCI registers are hard-wired here */
EPCI_out(ch)
char ch;
{
#asm

		LDAB  7,Y	*get char
		STAB	$2000	*stuff it to EPCI data reg

TWRT: LDAB	$2001	*hang around to wait for completion
		ANDB	#$01	
		CMPB	#$01	*Done bit set?
		BNE	TWRT	*if not (busy) wait some more
#endasm
}
