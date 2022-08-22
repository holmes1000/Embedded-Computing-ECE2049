/************** ECE2049 LAB 2 Code ******************/
/**************  2 Feb 2022   ******************/
/***************************************************/

#include <msp430.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "peripherals.h"
#include "animation.h"
#include "note.h"

// Function Prototypes
void swDelay(char numLoops); //delay function
void welcome(); //display welcome screen for MSP430 Hero
void endGame(); //display loss of game (including blinking leds)
void countdown(); //countdown to start of game
void winGame(); //display win of game (including blinking leds)

void displayButton(char inbits);
void configUserLED(char inbits);
char btnState();
void playGame(Note song[]);
char ledToBit(int ledNum);
int bitToLed(char inbits);
void runtimerA2(void);
void stoptimerA2(int reset);
unsigned char selectSong(void);
float correct;

unsigned char dispThree[3];
float delayTime = 2;
volatile unsigned long timer_cnt = 0;
volatile unsigned long time_start = 0;
int playerInput[] = {};
unsigned char songSelection;
float sizeOfSong;

// Main
void main(void)
{
    // Define some local variables
    unsigned char currKey = 0;

    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!

    // Useful code starts here
    initLeds();
    configDisplay();
    configKeypad();
    initButtons();   //initializes S1 (1.1) and S2 (2.1) on the board and configure S1 thru S4 on the board
    configNotif();  //Configure correct and incorrect notification lights
    _BIS_SR(GIE);

    // *** Intro Screen ***
    Graphics_clearDisplay(&g_sContext); // Clear the display

/*
 * MSP HERO BEGINS HERE
 * ---------------------------------------------------
 * Use a state machine to create MSP HERO with the following state configuration:
 * state 0 => welcome
 * state 1 => countdown
 * state 2 => play game
 * state 3 => check for input and check if it matches sequence
 * state 4 => endGame
 * state 5 => winGame
 * default => welcome
 */
    int state = 0; //Start at the welcome state
    while (1)    // Forever loop
    {
        switch(state) {
            case 0:                     //welcome to the game
                welcome();
                correct = 0; //reset the variables to 0
                currKey = getKey();
                int q = 0;
                songSelection = 0;
                if (currKey == '*')     //check for start
                {
                    while (q == 0)
                    {
                        songSelection = selectSong();
                        if (songSelection == 1 || songSelection == 2 || songSelection == 3 || songSelection == 4)
                        {
                            q = 1;
                        }
                    }
                    state = 1;            //go to state 1 => begin countdown
                }
                break;
            case 1:
                Graphics_clearDisplay(&g_sContext); // Clear the display
                runtimerA2();        //start the timer
                countdown();
                stoptimerA2(1);             //stop the timer and reset before the game starts
                state = 2;                //go to state 2 => playGame
                break;
            case 2:                     //play song
                runtimerA2();        //start the timer again
                setLeds(0);             //start LEDs off
                switch(songSelection)
                {
                case 1:
                    sizeOfSong = 78;
//                    graphicsInit(sandman, 40);
                    playGame(sandman);
                    break;
                case 2:
                    sizeOfSong = 39;
//                    graphicsInit(electricLove, 40);
                    playGame(electricLove);
                    break;
                case 3:
                    sizeOfSong = 47;
                    playGame(makeYouMine);
                    break;
                case 4:
                    sizeOfSong = 5;
                    playGame(testWin);
                    break;
                }

                stoptimerA2(1);         //stop the timer
                state = 3;               //go to state 3 => calculate the score
                break;
            case 3:                    //calculate score
                setLeds(0);
                float score = (correct/sizeOfSong)*100.0;
                float toWin = 70.0;
                if (score <= toWin)  //if the score is less than or equal to 70
                    state = 4;          //you lose
                else
                    state = 5;          //else you win
                break;
            case 4:
                endGame();              //end of game
                swDelay(1);             //delay before going back to welcome screen
                currKey = getKey();
                while (currKey != '#')
                {
                    Graphics_drawStringCentered(&g_sContext, "Waiting", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "for reset", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "Press '#'", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
                    Graphics_flushBuffer(&g_sContext);
                    currKey = getKey();
                }
                Graphics_clearDisplay(&g_sContext); //clear the display before going back to welcome screen
                state = 0;              //go back to state 0 => welcome
                break;
            case 5:
                winGame();              //win
                currKey = getKey();
                while (currKey != '#')
                {
                    Graphics_drawStringCentered(&g_sContext, "Waiting", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "for reset", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "Press '#'", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
                    Graphics_flushBuffer(&g_sContext);
                    currKey = getKey();
                }
                Graphics_clearDisplay(&g_sContext); // Clear the display
                state = 0;              //go back to state 0 => welcome
                break;
            default:                    //default to welcome display
                state = 0;
                break;
        }

    }  // end while (1)
}

/*
 * Function to choose song based on keypad
 */
unsigned char selectSong(void)
{
    unsigned char key;
    char key2;
    int pressed  = 0;
    while(getKey() == 0 && pressed == 0)
    {
        key = getKey();
        if (key == 1 || key == 2 || key == 3 || key == 4)
        {
            pressed = 1;
        }
    }
    key2 = key;
    return key2-0x30;
}


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
 * Function to configure and light user LEDs based on the char passed
 */
void configUserLED(char inbits)
{
    setLeds(inbits);        //sets the leds to turn on based on the bits
    displayButton(inbits);  //displays what button to press on the LCD
}

/*
 * Function to display button press on LEDS
 */
void displayButton(char inbits)
{
    Graphics_clearDisplay(&g_sContext); // Clear the display

    switch(inbits)
    {
    case 0x08:
        Graphics_drawStringCentered(&g_sContext, "Press S1", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
        break;
    case 0x04:
        Graphics_drawStringCentered(&g_sContext, "Press S2", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
        break;
    case 0x02:
        Graphics_drawStringCentered(&g_sContext, "Press S3", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
        break;
    case 0x01:
        Graphics_drawStringCentered(&g_sContext, "Press S4", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
        break;
    case 0x00:
        Graphics_drawStringCentered(&g_sContext, "Don't Press", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
        break;
    default:
        Graphics_drawStringCentered(&g_sContext, "Error", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
        break;
    }

    Graphics_flushBuffer(&g_sContext); //refresh
}

/*
 * Function to play game
 * loops through the song playing the corresponding note using the buzzerOn(int freq) function for the corresponding duration,
 * lighting the corresponding LEDs, and displaying the corresponding button to be pressed.
 * This also needs to keep track of the errors and correct buttons pressed to be used later for calculation.
 * If there is an error, it should play a bad note and display a red led.
 *
 */
void playGame(Note song[])
{
    int i = 0;
    char x;
    int btnPressed = 0; //0 is not pressed
    unsigned char currKey = 0;
    currKey = getKey();
    for (i = 0; i<sizeOfSong && currKey != '#'; i++) //try to get size variable later
    {
        x = ledToBit(song[i].led);  //converts the led number to bit

	//animate();

        configUserLED(x); //displays what button to press on the LCD and turns on the corresponding led
        //reset variables
        time_start = timer_cnt;
        btnPressed = 0;
        while(timer_cnt - time_start < song[i].duration)    //while time elapsed is less than note duration
        {
            BuzzerOn(song[i].pitch);
            if (bitToLed(btnState()) == song[i].led && btnPressed == 0)      //if the button pressed matches the LED and you haven't pressed yet
            {
                P4OUT |= BIT7;        // lights led 4.7 (green)
                correct++;          //increment correct count
                btnPressed = 1;     //you have pressed a button
            }
            else if ((bitToLed(btnState()) != 0x00) && (bitToLed(btnState()) != song[i].led) && btnPressed == 0)    //if the button pressed doesn't match and you haven't pressed yet
            {
                P1OUT |= BIT0;        // lights led 1.0 (red)
                btnPressed = 1;
            }
        }
        if (btnPressed == 0) //you didn't press in time
        {
            BuzzerOff();
            while (timer_cnt - time_start < song[i].duration + 10)
            {
                BuzzerOn(128);
                P1OUT |= BIT0;        // lights led 1.0 (red)
            }
        }

        BuzzerOff();
        P1OUT = P1OUT & ~BIT0;    //turns off led 1.0
        P4OUT = P4OUT & ~BIT7;    //turns off led 4.7
        currKey = getKey();

    }
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
 * Function to display welcome screen for SIMON
 * Parameter(s): none
 * Return: none
 */
void welcome()
{
    //Initialize
    setLeds(0);

    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, "Welcome", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "to", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "MSP430 Hero", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "Press '*'", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "to start", AUTO_STRING_LENGTH, 48, 55, TRANSPARENT_TEXT);

    // Draw a box around everything because it looks nice
    Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
    Graphics_drawRectangle(&g_sContext, &box);

    //refresh
    Graphics_flushBuffer(&g_sContext);

    dispThree[0] = ' ';
    dispThree[2] = ' ';
}


/*
 * Function to display the endgame on the LCD
 * Parameter(s): none
 * Return: none
 */
void endGame()
{
    Graphics_clearDisplay(&g_sContext); // Clear the display

    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, "YOU", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "LOST", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);

    Graphics_flushBuffer(&g_sContext);  //refresh

    dispThree[0] = ' ';
    dispThree[2] = ' ';
    int i;

    //play sound repetitively
    for (i = 0; i < 2; i++)
    {
        BuzzerOn(800);
        swDelay(1);
        BuzzerOff();
        setLeds(0x0F);
        swDelay(1);
        setLeds(0x0E);
        swDelay(1);
        setLeds(0x0C);
        swDelay(1);
        setLeds(0x08);
        swDelay(1);
        setLeds(0);
    }
    Graphics_clearDisplay(&g_sContext); // Clear the display
}


/*
 * Function to display the win of the game on the LCD
 * Parameter(s): none
 * Return: none
 */
void winGame()
{
    Graphics_clearDisplay(&g_sContext); // Clear the display

    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, "YOU", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "ARE A", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "ROCKSTAR", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);

    Graphics_flushBuffer(&g_sContext);  //refresh

    dispThree[0] = ' ';
    dispThree[2] = ' ';
    int i;
    //play sound repetitively 10 times for win
    for (i = 0; i < 2; i++)
    {
        BuzzerOn(300);
        swDelay(1);
        BuzzerOff();
        setLeds(0x0F);
        swDelay(2);
    }
    Graphics_clearDisplay(&g_sContext); // Clear the display

}

/*
 * Function to display the countdown on the LCD
 * Parameter(s): none
 * Return: none
 */
void countdown()
{
    int i;
    int done = 0;
    int x = 14;
    unsigned char check[1];

    for (i = 3; i>0; i--)
    {
        done = 0;
        time_start = timer_cnt;
        while(timer_cnt - time_start < 200)
        {
        check[0] = i+48;
        if (done == 0)
        {
            x -= 2*(3-i);
            setLeds(x);
            // Write some text to the display
            Graphics_drawStringCentered(&g_sContext, check, AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);

            //refresh
            Graphics_flushBuffer(&g_sContext);

            dispThree[0] = ' ';
            dispThree[2] = ' ';

            done = 1;
        }


        //delay before displaying next number
        //swDelay(1);

        }
        Graphics_clearDisplay(&g_sContext); // Clear the display
    }
    time_start = timer_cnt;
    while(timer_cnt - time_start < 200)
    {
        Graphics_drawStringCentered(&g_sContext, "GO!!!", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
        Graphics_flushBuffer(&g_sContext);
    }

    Graphics_clearDisplay(&g_sContext); // Clear the display
}

/*
 * Function to turn the bits to an LED number in the note struct
 */
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
        return 0x00;
    }
}

/*
 * Function to turn the LED number in the note struct into a char to be used to set the correct LEDs
 */
char ledToBit(int ledNum)
{
    switch(ledNum)
    {
    case 1:
        return 0x08;
    case 2:
        return 0x04;
    case 3:
        return 0x02;
    case 4:
        return 0x01;
    default:
        return 0x00;

    }
}


/*
 * Function to start the timer
 */
void runtimerA2(void)
{
    //ACLK frequency = 32768Hz
    //if 327.68 ticks = 0.01 seconds, to get to 0.005
    TA2CTL = TASSEL_1 | MC_1 | ID_0; //ACLK selected, Up mode, divider = 1
    TA2CCR0 = 163; //163+1 ACLK ticks = ~5/1000 seconds
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
__interrupt void TIMER2_A0_ISR (void)
{
    timer_cnt++;
}


