/************** ECE2049 PRE LAB 1 Code ******************/
/**************  24 Jan 2022   ******************/
/***************************************************/

#include <msp430.h>
#include <stdlib.h>
#include <stdbool.h>
/* Peripherals.c and .h are where the functions that implement
 * the LEDs and keypad, etc are. It is often useful to organize
 * your code by putting like functions together in files.
 * You include the header associated with that file(s)
 * into the main file of your project. */
#include "peripherals.h"

// Function Prototypes
void swDelay(char numLoops);
void welcome();
void endgame();
void displayKey(char currKey);
void playSequence(int level);
void countdown();
void input(int level);
bool inputCorrect(int a[], int b[]);

// Declare globals here

// Main
void main(void)
{
    // Define some local variables
    char sequence[32]; //generate sequence
    char playerInput[32];  //inputted sequence
    unsigned char currKey=0, dispSz = 3;
    unsigned char dispThree[3];


    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
                                 // You can then configure it properly, if desired

    // Useful code starts here
    initLeds();

    configDisplay();
    configKeypad();

    // *** Intro Screen ***
    Graphics_clearDisplay(&g_sContext); // Clear the display


    //SIMON SAYS BEGINS HERE
//------------------------------------
    //There should be a state for welcome, playing a sequence, accepting input, and end of game
    int state = 0; //Start at the welcome state
    int level = 1; //start at level 1
    while (1)    // Forever loop
    {
        switch(state) {
            //welcome to the game
            case 0:
                welcome();
                currKey = getKey();
                if (currKey == '*') //check for start
                    state++;
                break;
            case 1:
                countdown();
                state++;
                break;
            //play sequence
            case 2:
                playSequence(level);
                state++;
                break;
            //check input
            case 3:
                input(level);
                //if correct, increase level, go to state 2 to play next level
                if (inputCorrect(playerInput, sequence)) //if input matches, go to playSequence
                {
                    level++;
                    state = 2;
                }
                else
                    state = 4;  //else go to endgame
                break;
            //end of game
            case 4:
                endgame();
                swDelay(1);
                state = 0; //go back to welcome
                break;
            default:
                welcome();
                break;
        }

    }  // end while (1)
}

//The welcome screen for the game SIMON
void welcome()
{
    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, "Welcome", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "to", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "SIMON", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, your_name, AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);

    // Draw a box around everything because it looks nice
    Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
    Graphics_drawRectangle(&g_sContext, &box);

    //refresh
    Graphics_flushBuffer(&g_sContext);

    dispThree[0] = ' ';
    dispThree[2] = ' ';
}

//function to keep track of the player input
void input(int level)
{
    //for each level, get the current key, display it, and add it to the array
    for (int x = 0; x < level; x++)
    {
        currKey = getKey();
        displayKey(currKey);
        playerInput[x] == currKey;
    }
}

//function to check input
void inputCorrect(int a[], int b[])
{
    bool correct = true;
    for (int i = 0; i < b.length(); a++)
    {
        if (a[i] == b[i])
            correct = true;
        else
        {
            correct = false;
            break;
        }
    }
    return correct;
}

//the function to play a random sequence
void playSequence(int level)
{
    char key = '0';
    float delay = 0;
    for (int x = 0; x < level; x++)
    {
        key = ceil(rand() * 4) + 48;
        BuzzerOn();
        displayKey(key)
        setLeds(key); //fix to set leds 1 to 4
        swDelay(1);
        BuzzerOff();
        //delay decreases as level increases TODO
        swDelay(1);
    }
}
//The countdown screen for the game SIMON
void countdown()
{
    for (int i = 3; i>0; i--)
    {
        // Write some text to the display
        Graphics_drawStringCentered(&g_sContext, i, AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);

        // Draw a box around everything because it looks nice
        Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
        Graphics_drawRectangle(&g_sContext, &box);

        //refresh
        Graphics_flushBuffer(&g_sContext);

        dispThree[0] = ' ';
        dispThree[2] = ' ';

        //delay before displaying next umber
        swDelay(1);
    }

}

//The ENDGAME screen for the game SIMON
void endgame()
{
    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, "YOU", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "LOST", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "SIMON", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, your_name, AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);

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
}

//Function to display key press on LCD
void displayKey(char currKey)
{
    // Write some text to the display depending on what number is pressed
    Graphics_drawStringCentered(&g_sContext, currKey, AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);

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
}


// This function is a software delay. It performs
    // useless loops to waste a bit of time
    //
    // Input: numLoops = number of delay loops to execute
    // Output: none
    //
    // smj, ECE2049, 25 Aug 2013
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
