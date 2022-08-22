/************** ECE2049 LAB 2 Code ******************/
/**************  2 Feb 2022   ******************/
/***************************************************/

#include <msp430.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "peripherals.h"


#define CALADC12_15V_30C  *((unsigned int *)0x1A1A)
#define CALADC12_15V_85C  *((unsigned int *)0x1A1C)
float degC_per_bit;
unsigned int bits30, bits85;
// Function Prototypes
void swDelay(char numLoops); //delay function
void welcome(); //display welcome screen for MSP430 Hero

void displayButton(char inbits);
void configUserLED(char inbits);
void runtimerA2(void);
void stoptimerA2(int reset);
char btnState();
void square();
void triangle();
void sawtooth();
void DC();
void ADCInit();
int bitToLed(char inbits);
float getScroll(void);

unsigned int dacCodes[] = {0, 4095};
unsigned int in_value;
unsigned int code = 0;
unsigned int waveNum = 0;
unsigned char dispThree[3];
float delayTime = 2;
unsigned long time_start = 0;
unsigned long timer_cnt = 0;
char globalBtnState;
int state = 0; //Start at the welcome state
int btnPressed = 0;

// Main
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!

    // Useful code starts here
    initLeds();
    configDisplay();
    configKeypad();
    initButtons();   //initializes S1 (1.1) and S2 (2.1) on the board and configure S1 thru S4 on the board
    configNotif();  //Configure correct and incorrect notification lights
    _BIS_SR(GIE);
    setupSPI_DAC();
    DACInit();
    ADCInit();

    // *** Intro Screen ***
    Graphics_clearDisplay(&g_sContext); // Clear the display

    while (1)    // Forever loop
    {
        switch(state) {
        case 0:
        {
            //welcome to the game
            welcome();
            break;
        }
        case 1: //DC
        {
            btnPressed = 0;
            globalBtnState = 0x08; //used for timer later
            Graphics_clearDisplay(&g_sContext); // Clear the display
            runtimerA2();
            time_start = timer_cnt;
            DC();
            stoptimerA2(1);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            state = 0;
            break;
        }
        case 2: //Square
        {
            btnPressed = 0;
            globalBtnState = 0x04; //used for timer later
            Graphics_clearDisplay(&g_sContext); // Clear the display
            runtimerA2();
            time_start = timer_cnt;
            square();
            stoptimerA2(1);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            state = 0;
            break;
        }
        case 3: //sawtooth
        {
            btnPressed = 0;
            globalBtnState = 0x02; //used for timer later
            Graphics_clearDisplay(&g_sContext); // Clear the display
            runtimerA2();
            sawtooth();
            stoptimerA2(1);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            state = 0;
            break;
        }
        case 4: //triangle
        {
            btnPressed = 0;
            globalBtnState = 0x01; //used for timer later
            Graphics_clearDisplay(&g_sContext); // Clear the display
            runtimerA2();
            triangle();
            stoptimerA2(1);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            state = 0;
            break;
        }
        default:                    //default to welcome display
        {
            Graphics_clearDisplay(&g_sContext); // Clear the display
            globalBtnState = 0x00; //used for timer later
            state = 0;
            break;
        }
        }

    }  // end while (1)
}

//Function to generate DC voltage
void DC()
{
    Graphics_drawStringCentered(&g_sContext, "DC Voltage", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    //refresh
    Graphics_flushBuffer(&g_sContext);
    while (1)
    {
        //Scroll goes from
        code = getScroll(); //uses getScroll() to control voltage
        //float scrollDegrees = (float) (code * 0.01343) + 30.0;
        //float scale = ((scrollDegrees - 30.0) / (84.0 - 30.0)) * (4095 - 0) + 0;
        //unsigned int scale = (unsigned int) ((float) (((scrollDegrees) - 30)/ (84 - 30)) * (4095 - 0) + 0);
        DACSetValue(code); //takes in ADC values
        if (btnState() == 0x08) //if you press the button again, it breaks from loop
        {
            break;
        }
    }

}

void square()
{
    Graphics_drawStringCentered(&g_sContext, "Square Wave", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    //refresh
    Graphics_flushBuffer(&g_sContext);
    while (1)
    {
        code = getScroll();
        //float scale = ((84 - 30) / (84 - 30)) * (4095 - 0) + 0;
        DACSetValue(dacCodes[(timer_cnt % 2)]*code); //fluctuates between 0 and 1
        if (btnState() == 0x04) //if you press the button again, it breaks from loop
        {
            break;
        }
    }
}

void sawtooth()
{
    Graphics_drawStringCentered(&g_sContext, "Sawtooth Wave", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    //refresh
    Graphics_flushBuffer(&g_sContext);
    unsigned int numSteps = 12;
    unsigned int currStep = 0;
    unsigned int current;
    float voltsPerStep = 0.275; //3.3/12
    unsigned int code = 4095; //2^k-1
    unsigned int step = code*voltsPerStep/3.3;
    while (1)
    {
        unsigned int temp2 = timer_cnt % 12;
        if (temp2 == 0)
        {
            code = step*temp2;
            DACSetValue(code);
        }
        else
        {
            code = 4095-(temp2)*step;
            DACSetValue(code);
        }
        if (btnState() == 0x02) //if you press the button again, it breaks from loop
        {
            break;
        }
    }
}


void triangle()
{
    Graphics_drawStringCentered(&g_sContext, "Triangle Wave", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    //refresh
    Graphics_flushBuffer(&g_sContext);
    int currStep = 0;
    unsigned int current;
    float voltsPerStep = 0.275; //3.3/12
    unsigned int code = 4095; //2^k-1
    unsigned int step = code*voltsPerStep/3.3;
    while(1)
    {
        unsigned int temp2 = timer_cnt % 24;
        if (temp2 < 12)
        {
            code = step*temp2;
            DACSetValue(code);
        }
        else
        {
            code = 4095-(temp2-12)*step;
            DACSetValue(code);
        }
        if (btnState() == 0x01) //if you press the button again, it breaks from loop
        {
            break;
        }
    }
}


/*
void triangle()
    {
    //CODE ATTEMPT AT TRIANGLE WAVE ADJUSTABLE
    Graphics_drawStringCentered(&g_sContext, "Triangle Wave",
    AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);
    long newVal;
unsigned int freq = 150;
    while (true)
    {

    //unsigned int freq = (unsigned int) ((float) ((getScroll() - 1.0) / (4095.0 - 1.0)) * (1000.0 - 100.0) + 100.0);
    unsigned int val = (timer_cnt % freq) * (freq + 2);

    if (timer_cnt % freq != 0)
        {
        if ((timer_cnt % freq) <= (freq / 2))
        {
        newVal = ((float) ((val - freq)
            / ((long) ((freq * freq) / 2) + 0.0)) * (4095.0 - 1.0)
            + 1.0);
        }
        if (timer_cnt % freq)
        {
        newVal = 2.0 * 4095.0
            - ((float) ((val - freq)
                / ((long) (((freq * freq) - 1.0) / 2) + 0.0))
                * (4095.0 - 1.0) + 1.0);
        }

        }
    else
        {
        newVal = 0;
        }

    DACSetValue(newVal);

    if (btnState() == 0x01) //if you press the button again, it breaks from loop
        {
        break;
        }
    }
    }
*/


/*
 * Function to return the state of the lab board buttons with 1 = pressed
 */
char btnState()
{
    if ((P7IN & BIT0) == 0)     //S1 is pressed
        return 0x08;
    else if ((P3IN & BIT6) == 0)    //S2 is pressed
        return 0x04;
    else if ((P2IN & BIT2) == 0)    //S3 is pressed
        return 0x02;
    else if ((P7IN & BIT4) == 0)    //S4 is pressed
        return 0x01;
    else
        return 0x00;
}

/*
 * Function to do a software delay using useless loops to waste a bit of time
 * Parameter(s): char numLoops => number of delay loops to execute
 * Return: none
 */
void swDelay(char numLoops)
{
	volatile unsigned int i,j;	// volatile to prevent removal in optimization
			                    // by compiler. Functionally this is useless code

	for (j=0; j<numLoops; j++)
    {
    	i = 50000;					// SW Delay
   	    while (i > 0)				// could also have used while (i)
	       i--;
    }
}

/*
 * Function to display welcome screen
 * Parameter(s): none
 * Return: none
 */
void welcome()
{
    //Initialize

    setLeds(0);
    while(btnPressed == 0)
    {

        // Write some text to the display
        Graphics_drawStringCentered(&g_sContext, "Btn 1 = DC", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
        Graphics_drawStringCentered(&g_sContext, "Btn 2 = Square", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
        Graphics_drawStringCentered(&g_sContext, "Btn 3 = Sawtooth", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
        Graphics_drawStringCentered(&g_sContext, "Btn 4 = Triangle", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);

        if (btnState() == 0x08)
        {
            state = 1;  //DC
            btnPressed = 1;
            break;
        }
        else if (btnState() == 0x04)
        {
            state = 2;  //square
            btnPressed = 1;
            break;
        }
        else if (btnState() == 0x02)
        {
            state = 3;  //sawtooth
            btnPressed = 1;
            break;
        }
        else if (btnState() == 0x01)
        {
            state = 4;      //triangle
            btnPressed = 1;
            break;
        }

        //refresh
        Graphics_flushBuffer(&g_sContext);
    }
}

void ADCInit(void)
    {
    P8SEL &= ~BIT0;
    P8DIR |= BIT0;
    P8OUT |= BIT0;
    REFCTL0 &= ~REFMSTR;
    ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON;
    ADC12CTL1 = ADC12SHP;
    ADC12CTL0 &= ~ADC12SC;
    ADC12MCTL0 = ADC12SREF_0 + ADC12INCH_0; // scroll sensor
    __delay_cycles(1000); // delay to allow Ref to settle

    P6SEL |= BIT0;

    ADC12CTL0 |= ADC12SC + ADC12ENC; // Start conversion
    }

/*
 * returns the volts code value
 */
float getScroll(void)
{
    ADC12CTL0 &= ~ADC12SC;
    ADC12CTL0 |= ADC12SC;               // Sampling and conversion start
        // Single conversion (single channel)
        // Poll busy bit waiting for conversion to complete
        while (ADC12CTL1 & ADC12BUSY)
            __no_operation();
        in_value = ADC12MEM0;  // Read results if conversion done
        //float res = 3.3/4096;
        //float volts = ((float) res * in_value);
        //__no_operation();                       // SET BREAKPOINT HERE
    //}
    //return volts;
        return in_value;
}


/*
 * Function to start the timer
 */
void runtimerA2(void)
{
    //ACLK frequency = 32768Hz
    //if 327.68 ticks = 0.01 seconds, to get to 0.005
    TA2CTL = TASSEL_1 | MC_1 | ID_0; //ACLK selected, Up mode, divider = 1
    if (globalBtnState == 0x08) //DC voltage timer setup
        TA2CCR0 = 327;
    else if (globalBtnState == 0x04) //square wave timer setup  (100Hz)
        TA2CCR0 = 163;      //327 divided by two because wave is square
    else if (globalBtnState == 0x02) //sawtooth wave timer setup (75Hz)
    {
        TA2CTL = TASSEL_2 | MC_1 | ID_0;
        TA2CCR0 = 1162;
    }
    else if (globalBtnState == 0x01) //triangle wave timer setup (150Hz)
    {
        TA2CTL = TASSEL_2 | MC_1 | ID_0;
        TA2CCR0 = 1162; //218 cut in half because triangle wave
    }
    else   //default timer setup
        TA2CCR0 = 327;
    TA2CCTL0 = CCIE; //TA2CCR0 interrupt enabled
}

/*
 * Function to stop timer and reset the global count
 */
void stoptimerA2(int reset)
{
   if (reset == 1)
   {
       TA2CTL = MC_0; //stop timer
       TA2CCTL0 &= ~CCIE; //TA2CCR0 interrupt disabled
       //timer count to 0?
       timer_cnt = 0;
       time_start = 0;
   }
}

/*
 * Timer A2 Interrupt
 */
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMER2_A2_ISR (void)
{
        timer_cnt++;
}

int bitToLed(char inbits)
{
    switch(inbits)
    {
    case 0x08:
        return 1;
    case 0x04:
        return 2;
    case 0x02:
        return 3;
    case 0x01:
        return 4;
    default:
        return 0;
    }
}



