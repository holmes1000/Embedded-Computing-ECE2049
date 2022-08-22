/************** ECE2049 LAB 1 Code ******************/
/**************  24 Jan 2022   ******************/
/***************************************************/

#include <msp430.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "peripherals.h"


// Function Prototypes
void swDelay(char numLoops); //delay function
void increaseSpeed(int level);
void welcome(); //display welcome screen for SIMON
void endgame(); //display loss of game (including blinking leds)
void wingame(); //display win of game (including blinking leds)
void displayKey(char currKey);  //display key on LCD
void playSequence(int level, char b[]);   //play sequence of random numbers from 1-4 (including displayKey() and leds)
void countdown(); //countdown to start of game
void input(int level, char a[]);  //checking for input (adding chars to array)
int inputCorrect(char a[], char b[]); //check for matching input

unsigned char dispThree[3];
float delayTime = 2;
char sequence[32]; //generate sequence
char playerInput[32];  //inputted sequence
int level; //start at level 0 (better for looping later)

// Main
void main(void)
{
    // Define some local variables
    unsigned char currKey = 0, dispSz = 3;

    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!

    // Useful code starts here
    initLeds();
    configDisplay();
    configKeypad();
    initButtons();

    // *** Intro Screen ***
    Graphics_clearDisplay(&g_sContext); // Clear the display

/*
 * SIMON BEGINS HERE
 * ---------------------------------------------------
 * Use a state machine to create SIMON with the following state configuration:
 * state 0 => welcome
 * state 1 => countdown
 * state 2 => play sequence of random chars 1-4
 * state 3 => check for input and check if it matches sequence
 * state 4 => endgame
 * state 5 => wingame
 * default => welcome
 */
    int state = 0; //Start at the welcome state
    while (1)    // Forever loop
    {
        switch(state) {
            case 0:                     //welcome to the game
                welcome();
                currKey = getKey();
                if (currKey == '*')     //check for start
                {
                    state = 1;            //go to state 1 => begin countdown
                }
                break;
            case 1:
                Graphics_clearDisplay(&g_sContext); // Clear the display
                countdown();
                state = 2;                //go to state 2 => playSequence
                break;
            case 2:                     //play sequence
                setLeds(0);
                playSequence(level, sequence);
                state = 3;               //go to state 3 => check for input
                break;
            case 3:                    //check input
                setLeds(0);
                input(level, playerInput);
                //if correct, increase level, go to state 2 to play next level
                if (inputCorrect(playerInput, sequence) == 1) //if input matches, go to playSequence
                {
                    swDelay(1);
                    level++;            //increment level
                    state = 2;          //go to state 2 => playSequence again
                }
                else if (level == 31 && inputCorrect(playerInput, sequence) == 1)
                {
                    state = 5;
                }
                else
                {
                    state = 4;          //else go to state 4 => endgame
                }
                break;
            case 4:
                endgame();              //end of game
                swDelay(1);             //delay before going back to welcome screen
                while ((P1IN & BIT1) != 0 && (P2IN & BIT1) != 0)
                {
                    Graphics_drawStringCentered(&g_sContext, "Waiting", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "for reset", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "Press s1+s2", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
                    Graphics_flushBuffer(&g_sContext);
                }
                Graphics_clearDisplay(&g_sContext);
                state = 0;              //go back to state 0 => welcome
                break;
            case 5:
                wingame();              //win
                swDelay(1);             //delay before going back to welcome screen
                state = 0;              //go back to state 0 => welcome
                break;
            default:                    //default to welcome display
                welcome();
                break;
        }

    }  // end while (1)
}


/*
 * Function to display welcome screen for SIMON
 * Parameter(s): none
 * Return: none
 */
void welcome()
{
    //Initialize
    level = 0;
    setLeds(0);
    int x;
    for (x = 0; x < 31; x++)
    {
        sequence[x] = ' ';
        playerInput[x] = ' ';
    }

    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, "Welcome", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "to", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "SIMON", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);

    // Draw a box around everything because it looks nice
    Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
    Graphics_drawRectangle(&g_sContext, &box);

    //refresh
    Graphics_flushBuffer(&g_sContext);

    dispThree[0] = ' ';
    dispThree[2] = ' ';
}

/*
 * Function to keep track of the player input
 * Parameter(s): int level => what level the player is on, determines how many key presses are allowed
 *               char a[] => the array to keep track of player input
 * Return: none
 */
void input(int level, char a[])
{

    unsigned char currKey;
    char key;
    int i;
    //for each level, get the current key, display it, and add it to the array
    for (i = 0; i < level+1; i++)
    {
        currKey = getKey();
        while (currKey == 0)
        {
            currKey = getKey();
            //key = currKey;
        }
        if (currKey != 1 || currKey != 2 || currKey != 3 || currKey != 4)
        {
            displayKey(currKey);
            break;
        }
        displayKey(currKey);
        a[i] = currKey;
    }
}

/*
 * function to compare the input of the player to the sequence that was played
 * Parameter(s): char a[] => array of chars of player input
 *               char b[] => array of chars of random sequence played
 * Return: 1 or 0
 */

int inputCorrect(char a[], char b[])
{
    int i;
    int correct = 1;
    for (i = 0; i < sizeof(b); i++)
    {
        if (a[i] == b[i])
            correct = 1;
        else
        {
            correct = 0;
            break;
        }
    }
    return correct;

}

/*
 * function to play a random sequence of chars 1-4 and add on a char
 * Parameter(s): int level => what level the player is on, determines where to add the random char to the array
 *               char b[] => the current sequence array
 * Return: none
 */
void playSequence(int level, char b[])
{
    int x;
    char key;
    key = (rand() % 4) + 49;
    b[level] = key;             //set key to certain part of array

    //for loop to display sequence
    for (x = 0; x<level+1; x++)
    {
        displayKey(b[x]);
        increaseSpeed(level);
        setLeds(0);
    }
}

/*
 * Function to display the countdown on the LCD
 * Parameter(s): none
 * Return: none
 */
void countdown()
{
    int i;
    unsigned char check[1];

    for (i = 3; i>0; i--)
    {
        check[0] = i+48;

        // Write some text to the display
        Graphics_drawStringCentered(&g_sContext, check, AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);

        // Draw a box around everything because it looks nice
        Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
        Graphics_drawRectangle(&g_sContext, &box);

        //refresh
        Graphics_flushBuffer(&g_sContext);

        dispThree[0] = ' ';
        dispThree[2] = ' ';

        //delay before displaying next number
        swDelay(1);
        Graphics_clearDisplay(&g_sContext); // Clear the display
    }

}


/*
 * Function to display the endgame on the LCD
 * Parameter(s): none
 * Return: none
 */
void endgame()
{
    Graphics_clearDisplay(&g_sContext); // Clear the display

    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, "YOU", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "LOST", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "SIMON", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);

    // Draw a box around everything because it looks nice
    Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
    Graphics_drawRectangle(&g_sContext, &box);

    // We are now done writing to the display.  However, if we stopped here, we would not
    // see any changes on the actual LCD.  This is because we need to send our changes
    // to the LCD, which then refreshes the display.
    // Since this is a slow operation, it is best to refresh (or "flush") only after
    // we are done drawing everything we need.
    Graphics_flushBuffer(&g_sContext);

    dispThree[0] = ' ';
    dispThree[2] = ' ';
    int i;
    //play sound repetitively
    for (i = 0; i < 5; i++)
    {
        BuzzerOn(3);
        swDelay(1);
        BuzzerOff();
        setLeds(8);    //8 = 1000
        swDelay(1);
        setLeds(0);
        swDelay(1);
    }
    Graphics_clearDisplay(&g_sContext); // Clear the display
}


/*
 * Function to display key press at certain positions on LCD AND have different buzzer sounds
 * Parameter(s): char currkey => takes current key and uses that to determine position on LCD
 * Return: none
 */
void displayKey(char currKey)
{
    int posx;
    int posy;
    unsigned char check[1];
    check[0] = currKey;
    int key;
    // Set digital IO control registers for  Port 1 Pin 0
        P1SEL = P1SEL & ~BIT0;          // Select P1.0 for digital IO
        P1DIR |= BIT0;                  // Set P1.0 to output direction
        __disable_interrupt();          // Not using interrupts so disable them

    switch (currKey)
    {
    case '1':
        posx = 15;
        posy = 35;
        setLeds(8);    //8 = 1000
        //set buzzer sound
        key = 1;
        break;
    case '2':
        posx = 25;
        posy = 35;
        setLeds(4);    //4 = 0100
        //set buzzer sound
        key = 2;
        break;
    case '3':
        posx = 35;
        posy = 35;
        setLeds(2);    //2 = 0010
        //set buzzer sound
        key = 3;
        break;
    case '4':
        posx = 45;
        posy = 35;
        setLeds(1);    //1 = 0001
        //set buzzer sound
        key = 3;
        break;
    default:
        break;
    }


    Graphics_clearDisplay(&g_sContext); // Clear the display

    if(currKey == '1' || currKey == '2' || currKey == '3' || currKey == '4')
    {
        BuzzerOn(key);


        // Write some text to the display depending on what number is pressed
        Graphics_drawStringCentered(&g_sContext, check, AUTO_STRING_LENGTH, posx, posy, TRANSPARENT_TEXT);

        // Draw a box around everything because it looks nice
        Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
        Graphics_drawRectangle(&g_sContext, &box);

        // Since this is a slow operation, it is best to refresh (or "flush") only after
        // we are done drawing everything we need.
        Graphics_flushBuffer(&g_sContext);

        dispThree[0] = ' ';
        dispThree[2] = ' ';

        BuzzerOff();
    }
    else
    {
        BuzzerOn(1);


        // Write some text to the display depending on what number is pressed
        Graphics_drawStringCentered(&g_sContext, "Error", AUTO_STRING_LENGTH, posx, posy, TRANSPARENT_TEXT);

        // Draw a box around everything because it looks nice
        Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
        Graphics_drawRectangle(&g_sContext, &box);

        // Since this is a slow operation, it is best to refresh (or "flush") only after
        // we are done drawing everything we need.
        Graphics_flushBuffer(&g_sContext);

        dispThree[0] = ' ';
        dispThree[2] = ' ';

        BuzzerOff();
    }
    increaseSpeed(level);
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
    	i = 50000 ;					// SW Delay
   	    while (i > 0)				// could also have used while (i)
	       i--;
    }
}

/*
 * Function to do a delay based on level
 * Parameter(s): int level => level of game we are on
 * Return: none
 */
void increaseSpeed(int level)
{
    volatile unsigned int i,j;  // volatile to prevent removal in optimization
                                // by compiler. Functionally this is useless code
    level *= 3000;
    for (j=0; j<1; j++)
    {
        i = 50000 - level;          // SW Delay
        while (i > 0)               // could also have used while (i)
           i--;
    }
}
/*
 * Function to display the win of the game on the LCD
 * Parameter(s): none
 * Return: none
 */
void wingame()
{
    Graphics_clearDisplay(&g_sContext); // Clear the display

    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, "YOU", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "WON", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "SIMON", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);

    // Draw a box around everything because it looks nice
    Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
    Graphics_drawRectangle(&g_sContext, &box);

    // We are now done writing to the display.  However, if we stopped here, we would not
    // see any changes on the actual LCD.  This is because we need to send our changes
    // to the LCD, which then refreshes the display.
    // Since this is a slow operation, it is best to refresh (or "flush") only after
    // we are done drawing everything we need.
    Graphics_flushBuffer(&g_sContext);

    dispThree[0] = ' ';
    dispThree[2] = ' ';
    int i;
    //play sound repetitively 10 times for win
    for (i = 0; i < 2; i++)
    {
        BuzzerOn(1);
        swDelay(1);
        BuzzerOff();
        setLeds(2);    //8 = 1000
        swDelay(2);
    }
    Graphics_clearDisplay(&g_sContext); // Clear the display

}

