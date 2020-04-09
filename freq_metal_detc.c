//ELEC 291 project 2b 
//student number : 43586999
// name : Dhruv Patel
// Frequency metal detection with descrimination
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////////////////////////        PORTS     ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//green leds
//				RB15 //al  <<indicates metal presence        
//				RB6  //fe  <<indicates metal presence       
//				RA2  //cu  <<indicates metal presence       
//				RA3  //ni  <<indicates metal presence       

// multicolored LEDs for signal strength
//				RA0   //red
//				RA1   //red
//				RB0   //yellow
//				RB1   //yellow
//				RB3   //blue

//speaker 
//			    RB14

//calibration led/ mode indication LED white
//				RB2 

//colpitts oscillator square_wave_in
//              RB13

//Sensor coil square_wave_in 
//              RB5

//IO
	// calibrate+rst
	//	pin 1

	//change modes
	//  R12


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////////////////////////        PORTS     ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include <XC.h>
#include <sys/attribs.h>

#pragma config FNOSC = FRCPLL       // Internal Fast RC oscillator (8 MHz) w/ PLL
#pragma config FPLLIDIV = DIV_2     // Divide FRC before PLL (now 4 MHz)
#pragma config FPLLMUL = MUL_20     // PLL Multiply (now 80 MHz)
#pragma config FPLLODIV = DIV_2     // Divide After PLL (now 40 MHz) see figure 8.1 in datasheet for more info
#pragma config FWDTEN = OFF         // Watchdog Timer Disabled

#pragma config FPBDIV = DIV_1       // PBCLK = SYCLK
#pragma config FSOSCEN = OFF        // Secondary Oscillator Enable (Disabled)

// Defines
#define SYSCLK 40000000L
#define FREQ 10000L // 2Hz or 0.5 seconds interrupt rate

#define Baud2BRG(desired_baud)( (SYSCLK / (16*desired_baud))-1)
 
void UART2Configure(int baud_rate)
{
    // Peripheral Pin Select
    U2RXRbits.U2RXR = 4;    //SET RX to RB8
    RPB9Rbits.RPB9R = 2;    //SET RB9 to TX

    U2MODE = 0;         // disable autobaud, TX and RX enabled only, 8N1, idle=HIGH
    U2STA = 0x1400;     // enable TX and RX
    U2BRG = Baud2BRG(baud_rate); // U2BRG = (FPb / (16*baud)) - 1
    
    U2MODESET = 0x8000;     // enable UART2
}

// Needed to by scanf() and gets()
int _mon_getc(int canblock)
{
	char c;
	
    if (canblock)
    {
	    while( !U2STAbits.URXDA); // wait (block) until data available in RX buffer
	    c=U2RXREG;
	    if(c=='\r') c='\n'; // When using PUTTY, pressing <Enter> sends '\r'.  Ctrl-J sends '\n'
		return (int)c;
    }
    else
    {
        if (U2STAbits.URXDA) // if data available in RX buffer
        {
		    c=U2RXREG;
		    if(c=='\r') c='\n';
			return (int)c;
        }
        else
        {
            return -1; // no characters to return
        }
    }
}

// Use the core timer to wait for 1 ms.
void wait_1ms(void)
{
    unsigned int ui;
    _CP0_SET_COUNT(0); // resets the core timer count

    // get the core timer count
    while ( _CP0_GET_COUNT() < (SYSCLK/(2*1000)) );
}

void waitms(int len)
{
	while(len--) wait_1ms();
}

#define PIN_PERIOD (PORTB&(1<<13))

volatile unsigned char pwm_count=0;
volatile unsigned char pwm_control=0;

void __ISR(_TIMER_1_VECTOR, IPL5SOFT) Timer1_Handler(void)
{	
	pwm_count+=10;
	if(pwm_count>100) pwm_count=0;
	
	LATBbits.LATB14 =((pwm_count>(100-pwm_control))?0:1); // Blink led on RB14
	IFS0CLR=_IFS0_T1IF_MASK; // Clear timer 1 interrupt flag, bit 4 of IFS0
}

void SetupTimer1 (void)
{
	// Explanation here:
	// https://www.youtube.com/watch?v=bu6TTZHnMPY
	__builtin_disable_interrupts();
	PR1 =(SYSCLK/(FREQ*256))-1; // since SYSCLK/FREQ = PS*(PR1+1)
	TMR1 = 0;
	T1CONbits.TCKPS = 3; // Pre-scaler: 256
	T1CONbits.TCS = 0; // Clock source
	T1CONbits.ON = 1;
	IPC1bits.T1IP = 5;
	IPC1bits.T1IS = 0;
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;
	
	INTCONbits.MVEC = 1; //Int multi-vector
	__builtin_enable_interrupts();
}
// GetPeriod() seems to work fine for frequencies between 200Hz and 700kHz.
long int GetPeriod (int n)
{
	int i;
	unsigned int saved_TCNT1a, saved_TCNT1b;
	
    _CP0_SET_COUNT(0); // resets the core timer count
	while (PIN_PERIOD!=0) // Wait for square wave to be 0
	{
		if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
	}

    _CP0_SET_COUNT(0); // resets the core timer count
	while (PIN_PERIOD==0) // Wait for square wave to be 1
	{
		if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
	}
	
    _CP0_SET_COUNT(0); // resets the core timer count
	for(i=0; i<n; i++) // Measure the time of 'n' periods
	{
		while (PIN_PERIOD!=0) // Wait for square wave to be 0
		{
			if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
		}
		while (PIN_PERIOD==0) // Wait for square wave to be 1
		{
			if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
		}
	}

	return  _CP0_GET_COUNT();
}
long int GetPeriod_a (int n) // used for calculating the phase difference
{
	int i;
	unsigned int saved_TCNT1a, saved_TCNT1b;
	
    _CP0_SET_COUNT(0); // resets the core timer count
	while ((PORTB&(1<<13))!=0) // Wait for square wave to be 0
	{
		if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
	}

    _CP0_SET_COUNT(0); // resets the core timer count
	while ((PORTB&(1<<5))==0) // Wait for square wave to be 1
	{
		if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
	}
	
    _CP0_SET_COUNT(0); // resets the core timer count
	for(i=0; i<n; i++) // Measure the time of 'n' periods
	{
		while ((PORTB&(1<<13))!=0) // Wait for square wave to be 0
		{
			if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
		}
		while ((PORTB&(1<<5))==0) // Wait for square wave to be 1
		{
			if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
		}
	}

	return  _CP0_GET_COUNT();
}
void SomeDelay (void)
{
	int j;
	for(j=0; j<50000; j++);
}
void main (void)
{	
	ANSELB &= ~(1<<12); // Set RB12 as a digital I/O
    TRISB |= (1<<12);   // configure pin RB12 as input
    CNPUB |= (1<<12);   // Enable pull-up resistor for RB12

	ANSELB &= ~(1<<5); // Set RB5 as a digital I/O
    TRISB |= (1<<5);   // configure pin RB5 as input
    CNPUB |= (1<<5);   // Enable pull-up resistor for RB5

	ANSELB &= ~(1<<13); // Set RB5 as a digital I/O
    TRISB |= (1<<13);   // configure pin RB5 as input
    CNPUB |= (1<<13);   // Enable pull-up resistor for RB5

	DDPCON = 0;
	
	TRISBbits.TRISB14 = 0;
	LATBbits.LATB14 = 0;	
	pwm_control=0;
	////-----------
	CFGCON = 0;
    UART2Configure(115200);  // Configure UART2 for a baud rate of 115200
	////-----------
	volatile unsigned long t=0;
	float cal_f,cal_f_set,phase,phase_set;
	long int count;
	float T, f;
	int flag=0,block_metal=0;
	DDPCON = 0;
	int i;
	//configering pins
	TRISBbits.TRISB6 = 0;
	TRISBbits.TRISB15 = 0;

	TRISAbits.TRISA0 = 0;
	TRISAbits.TRISA1 = 0;
	TRISAbits.TRISA2 = 0;
	TRISAbits.TRISA3 = 0;
	

	TRISBbits.TRISB0 = 0;
	TRISBbits.TRISB1 = 0;
	TRISBbits.TRISB2 = 0;
	TRISBbits.TRISB3 = 0;
	


	//setting pins
			LATBbits.LATB6 = 0; 
			LATBbits.LATB15 = 0; 
			LATBbits.LATB0 = 0;
			LATBbits.LATB1 = 0;
			LATBbits.LATB2 = 0;
			LATBbits.LATB3 = 0;
			LATAbits.LATA0 = 0;	
			LATAbits.LATA1 = 0;	
			LATAbits.LATA2 = 0;	
			LATAbits.LATA3 = 0;	
			

	INTCONbits.MVEC = 1;
	count=GetPeriod(100);
	T=(count*2.0)/(SYSCLK*100.0);
	cal_f_set=1/T;
	//timer

	//RX metal des phase calibration;
	
	{
	count=GetPeriod_a(100);// i know that this 
	if(count>0)
		{
			T=(count*2.0)/(SYSCLK*100.0);
			f=1/T;
			// i know that this  is not the 'phase', but since T is time difference between the two 
			//signals and f is just some linear function of the actual phase 
			// it has all the information of the phase
			phase_set+=f;
		}
	}
	
	// for the speaker pwm 
	// that changes wrt hoe strong the signal is
	SetupTimer1();

	
	while (1)
 {		//////////////////////////////////////////////////////////////////////////////
	 	//////////////////////////////////////////////////////////////////////////////
		//set flag to 5 for debugging it
		//flag=5; 
	
		//printf("phase=%fHz                   offset=hz%fhz          feq=hz%fhz          \r", phase,phase_set,cal_f);
	
		// default is   0 ie for coin detection
		//				1 is metal type detection
		//				2 is metal descimination
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		if((PORTB&(1<<12)?1:0)==0) // mode changing button
		{
			while ((PORTB&(1<<12)?1:0)==0)
			{
				//wait for it to go to 1 again
				// do nothing lol
			}
			
			flag+=1;
			if(flag==3)
			{
				flag=0;
			}
		}

	if(flag==0) // precision coin detection mode {all green LEDs are on }
	{
		
		count=GetPeriod(100);
		if(count>0)
		{	
			
			T=(count*2.0)/(SYSCLK*100.0);
			f=1/T;
			cal_f=f-cal_f_set;

			count=GetPeriod_a(100);
			
			
				LATBbits.LATB15 =1; //al
				LATBbits.LATB6 = 1; //fe
				LATAbits.LATA2 = 1;	//cu
				LATAbits.LATA3 = 1;	//ni	
				//calibration led
				LATBbits.LATB2 = 0;//white

			
			

			if (cal_f<10)
			{	pwm_control=0;
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 1; //red
				LATBbits.LATB0 = 1;	//yellow
				LATBbits.LATB1 = 1;//yellow
				LATBbits.LATB3 = 1; // blue
			}
			else if (cal_f<30)
			{	pwm_control=0;
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 1;	//yellow
				LATBbits.LATB1 = 1;//yellow
				LATBbits.LATB3 = 1; // blue
			}
			else if (cal_f<70)
			{	pwm_control=15;
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 0;	//yellow
				LATBbits.LATB1 = 1;//yellow
				LATBbits.LATB3 = 1; // blue
			}
			else if (cal_f<100)
			{	pwm_control=35;
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 0;	//yellow
				LATBbits.LATB1 = 0;//yellow
				LATBbits.LATB3 = 1; // blue
			}
			else if (cal_f<150)
			{	pwm_control=50;
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 0;	//yellow
				LATBbits.LATB1 = 0;//yellow
				LATBbits.LATB3 = 0; // blue
			}

			else
			{
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 0;	//yellow
				LATBbits.LATB1 = 0;//yellow
				LATBbits.LATB3 = 0; // blue
			}
		
			
			

		}

		else
		{
			//sensor series
			LATAbits.LATA0 = 1;	//red
			LATAbits.LATA1 = 1; //red
			LATBbits.LATB0 = 1;	//yellow
			LATBbits.LATB1 = 1;//yellow
			LATBbits.LATB3 = 1; // blue
			//calibration led
			LATBbits.LATB2 = 0;//white
		}
	}

	else if(flag==1) // 10 cm object detection mode {all green LEDs are off}
	{
			
		count=GetPeriod(100);
		if(count>0)
		{	
			//calibration led
			LATBbits.LATB2 = 1;//white
			T=(count*2.0)/(SYSCLK*100.0);
			f=1/T;
			cal_f=f-cal_f_set;

			count=GetPeriod_a(100);
			

		if(count>0)
		{
			T=(count*2.0)/(SYSCLK*100.0);
			phase=1/T;
			phase=phase_set-phase+1000;

			if(phase>10000)
			{
			//green leds
			LATBbits.LATB15 =1; //al
			LATBbits.LATB6 = 0; //fe
			LATAbits.LATA2 = 1;	//cu
			LATAbits.LATA3 = 1; //ni
			//calibration led
				LATBbits.LATB2 = 1;//white
			}
			else if(phase<-4000)
			{
			//green leds
			LATBbits.LATB15 =0; //al
			LATBbits.LATB6 = 1; //fe
			LATAbits.LATA2 = 1;	//cu
			LATAbits.LATA3 = 1;	//ni
			//calibration led
			LATBbits.LATB2 = 1;//white
			}
			else if((phase<-100)&&(phase>-1000))
			{
				//green leds
				LATBbits.LATB15 =1; //al
				LATBbits.LATB6 = 1; //fe
				LATAbits.LATA2 = 0;	//cu
				LATAbits.LATA3 = 0;	//ni	
				//calibration led
				LATBbits.LATB2 = 1;//white
			}

			else 
			{
				//green leds
				LATBbits.LATB15 =1; //al
				LATBbits.LATB6 = 1; //fe
				LATAbits.LATA2 = 1;	//cu
				LATAbits.LATA3 = 1;	//ni	
				//calibration led
				LATBbits.LATB2 = 0;//white
			}
		}
		else
		{
			//green leds
				LATBbits.LATB15 =1; //al
				LATBbits.LATB6 = 1; //fe
				LATAbits.LATA2 = 1;	//cu
				LATAbits.LATA3 = 1;	//ni	
				//calibration led
				LATBbits.LATB2 = 0;//white
		}
			
			

			if (cal_f<100)
			{	pwm_control=0;
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 1; //red
				LATBbits.LATB0 = 1;	//yellow
				LATBbits.LATB1 = 1;//yellow
				LATBbits.LATB3 = 1; // blue
			}
			else if (cal_f<300)
			{	pwm_control=0;
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 1;	//yellow
				LATBbits.LATB1 = 1;//yellow
				LATBbits.LATB3 = 1; // blue
			}
			else if (cal_f<700)
			{	pwm_control=15;
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 0;	//yellow
				LATBbits.LATB1 = 1;//yellow
				LATBbits.LATB3 = 1; // blue
			}
			else if (cal_f<1000)
			{	pwm_control=35;
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 0;	//yellow
				LATBbits.LATB1 = 0;//yellow
				LATBbits.LATB3 = 1; // blue
			}
			else if (cal_f<1200)
			{	pwm_control=50;
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 0;	//yellow
				LATBbits.LATB1 = 0;//yellow
				LATBbits.LATB3 = 0; // blue
			}

			else
			{
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 0;	//yellow
				LATBbits.LATB1 = 0;//yellow
				LATBbits.LATB3 = 0; // blue
			}
		
			
			

		}

		else
		{
			//sensor series
			LATAbits.LATA0 = 1;	//red
			LATAbits.LATA1 = 1; //red
			LATBbits.LATB0 = 1;	//yellow
			LATBbits.LATB1 = 1;//yellow
			LATBbits.LATB3 = 1; // blue
			//calibration led
			LATBbits.LATB2 = 0;//white
		}
	}

	else if(flag==2) // (alumninium) descrimination mode {all green LEDs except Al's LED(led b6) are off + white LED is on}
	{
			//green leds
			LATBbits.LATB15 =1; //al
			LATBbits.LATB6 = 0; //fe
			LATAbits.LATA2 = 1;	//cu
			LATAbits.LATA3 = 1; //ni
			//calibration led
			LATBbits.LATB2 = 0;//white

		count=GetPeriod(100);
		if(count>0)
		{	
			

			T=(count*2.0)/(SYSCLK*100.0);
			f=1/T;
			cal_f=f-cal_f_set;

			count=GetPeriod_a(100);
			

		if(count>0)
		{
			T=(count*2.0)/(SYSCLK*100.0);
			phase=1/T;
			phase=phase_set-phase+1000;

			if(phase<-4000)
			{
				block_metal=1;
			}
			
			else
			{
				block_metal=0;
			}
			
		}
		else
		{		//error code
			    //green leds
				LATBbits.LATB15 =1; //al
				LATBbits.LATB6 = 0; //fe
				LATAbits.LATA2 = 1;	//cu
				LATAbits.LATA3 = 0;	//ni	
				//calibration led
				LATBbits.LATB2 = 0;//white
		}
			
			

			if (cal_f<100)
			{	
				if (block_metal==1)
				{
					pwm_control=0;
				}
				else
				{
					pwm_control=0;
				}
				
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 1; //red
				LATBbits.LATB0 = 1;	//yellow
				LATBbits.LATB1 = 1;//yellow
				LATBbits.LATB3 = 1; // blue
			}
			else if (cal_f<300)
			{	if (block_metal==1)
				{
					pwm_control=0;
				}
				else
				{
					pwm_control=0;
				}
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 1;	//yellow
				LATBbits.LATB1 = 1;//yellow
				LATBbits.LATB3 = 1; // blue
			}
			else if (cal_f<700)
			{	if (block_metal==1)
				{
					pwm_control=15;
				}
				else
				{
					pwm_control=0;
				}
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 0;	//yellow
				LATBbits.LATB1 = 1;//yellow
				LATBbits.LATB3 = 1; // blue
			}
			else if (cal_f<1000)
			{	if (block_metal==1)
				{
					pwm_control=35;
				}
				else
				{
					pwm_control=0;
				}
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 0;	//yellow
				LATBbits.LATB1 = 0;//yellow
				LATBbits.LATB3 = 1; // blue
			}
			else if (cal_f<1200)
			{	if (block_metal==1)
				{
					pwm_control=50;
				}
				else
				{
					pwm_control=0;
				}
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 0;	//yellow
				LATBbits.LATB1 = 0;//yellow
				LATBbits.LATB3 = 0; // blue
			}

			else
			{
				LATAbits.LATA0 = 0;	//red
				LATAbits.LATA1 = 0; //red
				LATBbits.LATB0 = 0;	//yellow
				LATBbits.LATB1 = 0;//yellow
				LATBbits.LATB3 = 0; // blue
			}
		
			
			

		}

		else
		{
			//sensor series
			LATAbits.LATA0 = 1;	//red
			LATAbits.LATA1 = 1; //red
			LATBbits.LATB0 = 1;	//yellow
			LATBbits.LATB1 = 1;//yellow
			LATBbits.LATB3 = 1; // blue
			//calibration led
			LATBbits.LATB2 = 0;//white
		}
	}

	else
	{	// error code 
		// to help me indentify error states
			LATBbits.LATB15 =1; //al
			LATBbits.LATB6 = 1; //fe
			LATAbits.LATA2 = 1;	//cu
			LATAbits.LATA3 = 0; //ni
			LATBbits.LATB3 = 0; // blue
	}
}
}
		



