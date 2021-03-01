/*#define _DEBUG_ON*/
/*code starts at A000, data above internal control registers, stack below internal registers*/

#code  0xA000
#data  0x0000
#stack 0x0fff

#include <startup.c>

/*Internal control registers*/
#define  REG_BASE 0x1000

/*external hardware */
#define  EPCI_DAT	0x2000
#define  EPCI_STS 0x2001
#define  EPCI_MOD	0x2002
#define  EPCI_CMD	0x2003

#define AZ_LOW		0xE000	/*Azimuth encoder low byte*/
#define EL_LOW		0xE001	/*Elevation encoder low byte*/
#define ENC_HI		0xE002	/*Encoder hi bits*/
#define DRV_DIR	0xE003	/*DIRECTION latches*/
#define DRV_AZ		0xE004	/*Azimuth PWM PLD*/
#define DRV_EL		0xE005	/*Elevation PWM PLD*/
#define ENC_STRB	0xE006   /*Encoder strobe bit*/
#define HPADDLE	0xE003	/*hand paddle read latch*/

/*defines for hand paddle bits*/
#define	EL_UP		0x01
#define	EL_DOWN	0x02
#define	AZ_CW		0x04
#define	AZ_CCW	0x08
#define	SWFAST	0x10


#include <68hc11.h>
#include <stdio.h>

/*Global variable declarations*/
int CBufSize;

char SCI_RBuf[80];
int  SCI_rbIx;
int  SCI_Msg;

int  EchoOn;

rom char ComErr[] = "Bad Character in input.\r\n";


/*these are used in the SCI handler*/
char SCI_status;
FILE stdout;

#ifdef _DEBUG_ON
FILE dbgOUT;

/*these are for the EPCI debug port*/	
char EPCI_RBuf[80];
int  EPCI_rbIx;
int  EPCI_Msg;
char EPCI_status;
int  EPCI_echo;
int  EPCI_mon;
#endif

/*these are used by timer system if enabled*/
int	TimerOn;	/*send position continuously */
unsigned int   TimeTick;
int	scale;	/*pre-scaler factor for timer interrupt*/
/*16 ~= 0.85 sec, set only 1 bit this is a shift divisor*/
int   temp;		/*used in get data, called by timer interrupt*/

/*these variables hold the position, AZ, and EL*/
/*the commanded Az, El velocities, and directions*/
/*and a utility string for transmission*/
unsigned int	AzPos;
unsigned int	ElPos;

unsigned int	OldAzPos;
unsigned int	OldElPos;

int	changesOnly;

unsigned int	AzVel;
unsigned int  ElVel;
int	AzDir;
int	ElDir;

/*variables for hand paddle state*/
int	HPState;
int	LastState;
int	ElUp;
int	ElDown;
int	AzCW;
int	AzCCW;
int	Fast;

char  PosString[10];

#define AZ 1
#define EL 0

/* Link in the following libraries (nearly all needed by fprintf() )*/
#include <fopen.c>
#include <fputs.c>
#include <fputc.c>
#include <itoa.c>
#include <atoi.c>
#include <is.c>
#include <fprintf.c>
#include <itoab.c>
#include <reverse.c>
#include <strlen.c>

/* Link in the device driver for the Motorola 68HC11 */
#include <SER_out.c>
/*and the EPCI we use for a debug port*/
#ifdef _DEBUG_ON
#include <epci_out.c>
#endif

/*interrupt handlers*/
interrupt sci ()
	{
	SCI_status = peekb (REG_BASE+SCSR);
	if ((SCI_status & 0x2E) == 0x20)
		{
   	SCI_RBuf[SCI_rbIx] = peekb (REG_BASE+SCDR);
		/*force to capital letter*/
		if ((SCI_RBuf[SCI_rbIx] > 0x60) && (SCI_RBuf[SCI_rbIx] < 0x7B))
			SCI_RBuf[SCI_rbIx] = SCI_RBuf[SCI_rbIx] - 0x20;
		SCI_rbIx++;

		if (SCI_Msg != 0)
			{
			fprintf (stdout, "Error - input too fast\n");
			SCI_Msg = 0;
			SCI_rbIx = 0;		
			return;
			}

		/*circular buffer*/
		if (SCI_rbIx < 0)
			SCI_rbIx = 0;

		if (SCI_rbIx > CBufSize)
			SCI_rbIx = 0;

		if ((SCI_RBuf[SCI_rbIx-1] == 0x0D))
			 {
			 SCI_Msg = 1;
			 /*debug stuff*/
			 SCI_RBuf[SCI_rbIx] = '\0';

#ifdef _DEBUG_ON
			 if (EPCI_mon)
				 fprintf (dbgOUT, SCI_RBuf);
#endif
			 }/*end of message*/
		}/*good char received*/
	else
	/*if we get here an error was indicated by SCI*/
		{
		/*reset COMM buffer*/
		SCI_rbIx = 0;

		peekb (REG_BASE+SCDR);
#ifdef _DEBUG_ON
		if (EPCI_mon)
			fprintf (dbgOUT, "SCI error - %s", SCI_RBuf);
#endif
		fprintf (stdout, "SCI error - %s", SCI_RBuf);
		}/*bad char handler*/
	}/*SCI com receive interrupt*/

#ifdef _DEBUG_ON
interrupt irq()
	{
	EPCI_status = peekb (EPCI_STS);
	if ((EPCI_status & 0x3A) == 2)
		{
		/*get EPCI character*/
		EPCI_RBuf[EPCI_rbIx] = peekb (EPCI_DAT);
		if (EPCI_echo)
			fputc (EPCI_RBuf[EPCI_rbIx], dbgOUT);

		EPCI_rbIx = EPCI_rbIx + 1;
		if (EPCI_rbIx > CBufSize)
			EPCI_rbIx = 0;
		if ((EPCI_RBuf[EPCI_rbIx-1] == 0x0D))
			 {
			 EPCI_Msg = 1;
			 }
		}/*EPCI char OK*/
	else
		{
		/*reset COMM buffer*/
		EPCI_rbIx = 0;
		fprintf(dbgOUT, ComErr);
		}/*EPCUI bad char*/ 
	}/*EPCI interrupt*/
#endif

interrupt toi()
	{
	/*reset overflow flag to prevent re-interrupt*/
   bit_set (REG_BASE+TFLG2, 0x80);
	HPState = peekb (HPADDLE);
	/*decode state*/
	if ((HPState & EL_UP) == EL_UP)
		ElUp = 0;
	else
		ElUp = 1;
	if ((HPState & EL_DOWN) == EL_DOWN)
		ElDown = 0;
	else
		ElDown = 1;
	if ((HPState & AZ_CW) == AZ_CW)
		AzCW = 0;
	else
		AzCW = 1;
	if ((HPState & AZ_CCW) == AZ_CCW)
		AzCCW = 0;
	else
		AzCCW = 1;
	if ((HPState & SWFAST) == SWFAST)
		Fast = 0;
	else 
		Fast = 1;


	/*if selected then send out position*/
	if (TimerOn)
		{
		TimeTick++;
		if ((TimeTick / scale) > 1)
			{
			TimeTick = 0;
			get_data();
			send_data();
			}
		}
	}/*main timer time out interrupt*/

main ()
   {
	int i;
/* Set the PreScale bits before the time protect timer expires */
/*fastest time*/
#asm
      LDX  #$1024
      LDAB #$00 
      STAB 0,X
#endasm
	/*no interrupts*/
	d_int();
	/*set up various registers*/
   pokeb (REG_BASE+TEST1, 0x00);		/*no illegal op code interrupts*/
   pokeb (REG_BASE+BAUD,  0x20);		/*19.2KBaud if 4.9152MHz clock*/
   pokeb (REG_BASE+SCCR1, 0x00);		/*No parity, 1 stop bit*/
   pokeb (REG_BASE+SCCR2, 0x2C);		/*Turn on SCI xmit and RCV*/
	pokeb (REG_BASE+SPCR,  0x04);		/*Pre-scaler division ration for BAUD clock*/
	/*set up EPCI*/
	pokeb (EPCI_MOD,0x4E);				/*1 stop no parity*/
	pokeb (EPCI_MOD,0x3f);				/*9600 baud*/
	pokeb (EPCI_CMD,0x27);				/*enable xcvr*/

   /* Clear the Timer Overflow Flag */
   bit_set(REG_BASE+TFLG2,0x80);
   /*  Enable the Timer Overflow Interrupt */
   bit_set(REG_BASE+TMSK2,0x80);

	SCI_rbIx = 0;
	SCI_Msg = 0;
	EchoOn = 1;


#ifdef _DEBUG_ON
	EPCI_rbIx = 0;
	EPCI_Msg = 0;
	EPCI_echo = 0;
	EPCI_mon = 0;
#endif
	
	scale = 0x01;
	TimerOn = 1;
	TimeTick = 0;

	AzVel = 0;
	ElVel = 0;
	AzDir = 1;
	ElDir = 1;

	for (i = 0; i < 80; i = i+1)
		{
		SCI_RBuf[i] = '\0';
#ifdef _DEBUG_ON
		EPCI_RBuf[i] = '\0';
#endif
		}

	HPState = 0;
	LastState = 0;
	ElUp = 0;
	ElDown = 0;
	AzCCW = 0;
	AzCW = 0;
	Fast = 0;
	changesOnly = 1;

	stdout = fopen (SER_out);

#ifdef _DEBUG_ON
	dbgOUT = fopen (EPCI_out);
#endif

	fprintf (stdout, "Telescope Microcontroller R 3.0 initialization complete.\n");

#ifdef _DEBUG_ON
	fprintf (dbgOUT, "Telescope Microcontroller R 3.0 initialization complete.\n");
#endif

	SetDir();
	SetVel (AZ);
	SetVel (EL);
			
	CBufSize = 80; 

	/*now we are set up so enable interrupts*/
	e_int();

	/*for ever*/
	while (1)
		{
		if (SCI_Msg)
			{
			SCI_RBuf[SCI_rbIx] = '\0';
			switch (SCI_RBuf[0])
				{
				case 'S':
					if (SCI_RBuf[1] == 'O')
						{
						/*force data to transmit*/
						i = changesOnly;
						changesOnly = 0;
						send_data();
						changesOnly = i;
						}
					else
						{
						SCI_RBuf[SCI_rbIx] = '\0';
						i = atoi (&SCI_RBuf[1]);
						scale = i;
						}
					if (EchoOn)
						fprintf (stdout, "- %s : scale = %d\n",SCI_RBuf,scale);
					break;

				case 'T':
					if (SCI_RBuf[1] == '1')
						TimerOn = 1;
					else
						TimerOn = 0;
					if (EchoOn)
						fprintf (stdout, "- %s : timer %s\n",SCI_RBuf,(TimerOn?"on":"off"));
					break;

				case 'A':
					if (SCI_RBuf[1] == 'D')	
						{
						if (SCI_RBuf[2] == '1')
							AzDir = 1;
						else
							AzDir = 0;	
						SetDir();
						if (EchoOn)
							fprintf (stdout, "- %s : AzDir = %d\n",SCI_RBuf,AzDir);
						}
					else if (SCI_RBuf[1] == 'V')
						{
						SCI_RBuf[SCI_rbIx] = '\0';
						i = atoi (&SCI_RBuf[2]);
						AzVel = i & 0x00ff;
						SetVel(AZ);
						if (EchoOn)
							fprintf (stdout, "- %s : AzVel = %d\n",SCI_RBuf,AzVel);
						}
					else
						{
						SCI_RBuf[SCI_rbIx] = '\0';
						fprintf (stdout,"Unrecognized Command %s\n",SCI_RBuf);
						}
					break;
				case 'E':
					if (SCI_RBuf[1] == 'D')	
						{
						if (SCI_RBuf[2] == '1')
							ElDir = 1;
						else
							ElDir = 0;	
						SetDir();
						if (EchoOn)
							fprintf (stdout, "- %s : ElDir = %d\n",SCI_RBuf, ElDir);
						}
					else if (SCI_RBuf[1] == 'V')
						{
						SCI_RBuf[SCI_rbIx] = '\0';
						i = atoi (&SCI_RBuf[2]);
						ElVel = i & 0x00ff;
						SetVel(EL);
						if (EchoOn)
							fprintf (stdout, "- %s : ElVel = %d\n",SCI_RBuf, ElVel);
						}
					else if (SCI_RBuf[1] == 'C')
						{
						if (SCI_RBuf[6] == 'N')
							EchoOn = 1;
						else
							EchoOn = 0;
						if (EchoOn)
							fprintf (stdout, "- %s\n",SCI_RBuf, ElVel);
						}	
					else
						{
						SCI_RBuf[SCI_rbIx] = '\0';
						fprintf (stdout,"Unrecognized Command %s\n",SCI_RBuf);
						}
					break;

				case 'H':
					if (EchoOn)
						fprintf (stdout, "- %s\n",SCI_RBuf);
					fprintf (stdout, "HPState = %d\n",HPState);
					fprintf (stdout, "LastState = %d\n",LastState);
					fprintf (stdout, "ElUp = %d\n",ElUp);
					fprintf (stdout, "ElDown = %d\n",ElDown);
					fprintf (stdout, "AzCW = %d\n",AzCW);
					fprintf (stdout, "AzCCW = %d\n",AzCCW);
					fprintf (stdout, "Fast = %d\n",Fast);
					fprintf (stdout, "ElVel = %d\n",ElVel);
					fprintf (stdout, "AzVel = %d\n",AzVel);
					fprintf (stdout, "ElDir = %d\n",ElDir);
					fprintf (stdout, "AzDir = %d\n",AzDir);
					break;

				case 'R':
					if (EchoOn)
						fprintf (stdout, "- %s : reset all values.\n",SCI_RBuf);
					ElVel = 0;
					SetVel(EL);
					AzVel = 0;
					SetVel(AZ);
					ElDir = 1;
					AzDir = 1;
					SetDir();
					HPState = 0;
					LastState = 0;
					ElUp = 0;
					ElDown = 0;
					AzCCW = 0;
					AzCW = 0;
					Fast = 0;
					scale = 0x01;
					TimerOn = 1;
					TimeTick = 0;
					break;				/*RESET*/

				case 'C':
					if (changesOnly)
						changesOnly = 0;
					else
						changesOnly = 1;

					if (EchoOn)
						fprintf (stdout, "- %s : changes only %s\n",SCI_RBuf,(changesOnly?"on":"off"));
					break;

				default:
					SCI_RBuf[SCI_rbIx] = '\0';
					fprintf (stdout,"Unrecognized Command %s\n",SCI_RBuf);
					break;
				}/*SCI command switch*/
			SCI_Msg = 0;
			SCI_rbIx = 0;
			}/*handle SCI message*/
#ifdef _DEBUG_ON
		if (EPCI_Msg)
			{
			switch (EPCI_RBuf[0])
				{
				case 'E':
					if (EPCI_RBuf[1] == '1')
						EPCI_echo = 1;
					else
						EPCI_echo = 0;
					break;
				case 'M':
					if (EPCI_RBuf[1] == '1')
						EPCI_mon = 1;
					else
						EPCI_mon = 0;
					break;
				default:
					EPCI_RBuf[SCI_rbIx] = '\0';
					fprintf (stdout,"Unrecognized Command %s\n",EPCI_RBuf);
					break;
				}/*EPCI command switch*/
			EPCI_Msg = 0;
			EPCI_rbIx = 0;
			}/*EPCI msg*/
#endif
	
		/*Check Hand Paddle*/
		if (HPState != LastState)
			{
			if (ElUp)
				{
				ElDir = 0;
				SetDir ();
				if (Fast)
					ElVel = 150;
				else
					ElVel = 75;
				SetVel (EL);
				}
			else if (ElDown)
				{
				ElDir = 1;
				SetDir();
				if (Fast)
					ElVel = 150;
				else 
					ElVel = 75;
				SetVel (EL);
				}
			else
				{
				ElDir = 1;
				ElVel = 0;
				SetDir();
				SetVel (EL);
				}	
			if (AzCCW)
				{
				AzDir = 0;
				SetDir ();
				if (Fast)
					AzVel = 150;
				else
					AzVel = 75;
				SetVel (AZ);
				}
			else if (AzCW)
				{
				AzDir = 1;
				SetDir();
				if (Fast)
					AzVel = 150;
				else 
					AzVel = 75;
				SetVel (AZ);
				}
			else
				{
				AzDir = 1;
				AzVel = 0;
				SetDir();
				SetVel (AZ);
				}	
			LastState = HPState;
			}/*if hand paddle state changed*/
		}/*for ever loop*/
	}/*end of main*/

/*get data from encoders and format as hex*/
void get_data()
	{
	/*store old values*/
	OldAzPos = AzPos;
	OldElPos = ElPos;

	/*strobe encoder latches*/
	pokeb (ENC_STRB, 0x7f);
	/*get azimuth from encoders*/
	AzPos = peekb (AZ_LOW);
	temp = peekb (ENC_HI);
	temp = temp & 0x0003;		/*bottom 2 bits AZ, next 2 bits EL*/
	AzPos |= ((temp << 8)&0x0300);
	AzPos &= 0x03ff;	
	/*now get elevation*/
	ElPos = peekb (EL_LOW);
	temp = peekb (ENC_HI);
	temp = temp & 0x000C;		/*bottom 2 bits AZ, next 2 bits EL*/
	ElPos |= ((temp << 6)&0x0300);
	ElPos &= 0x03ff;	
	}/*end of get_data()*/

void send_data()
	{
	if ((OldAzPos != AzPos) || (OldElPos != ElPos) || changesOnly == 0)
		fprintf (stdout,"%d,%d\n",AzPos,ElPos);
	}/*end of send_data*/

void SetDir ()
	{
	int c;
	c = AzDir;
	c |= (ElDir << 1) & 0x02;
	pokeb (DRV_DIR, c);
	}/*Set Direction regiser*/

void SetVel (ae)
	int ae;
	{
	if (ae == AZ)
		pokeb (DRV_AZ, AzVel);
	else if (ae == EL)
		pokeb (DRV_EL, ElVel);
	}/*set velocity register*/