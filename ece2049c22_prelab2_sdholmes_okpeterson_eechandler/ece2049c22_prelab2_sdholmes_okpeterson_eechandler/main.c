/************** ECE2049 PRELAB 2 Code ******************/
/**************  2 Feb 2022   ******************/
/***************************************************/

#include <msp430.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "peripherals.h"

//Notes, LEDs, and LCD display at the same time
//button corresponds to notes inputted
//then they are compared
#define  NOTE_A     440                 //1
#define  NOTE_Bb    466    //B flat     //1
#define  NOTE_B     494                 //1
#define  NOTE_C     523                 //2
#define  NOTE_Cs    554    //C sharp    //2
#define  NOTE_D     587                 //2
#define  NOTE_Eb    622    //E flat     //3
#define  NOTE_E     659                 //3
#define  NOTE_F     698                 //3
#define  NOTE_Fs    740    //F sharp    //4
#define  NOTE_G     784                 //4
#define  NOTE_Ab    831 //A flat        //4
#define  NOTE A2    880                 //4

// Function Prototypes
void swDelay(char numLoops); //delay function
void welcome(); //display welcome screen for MSP430 Hero
void endGame(); //display loss of game (including blinking leds)
void countdown(); //countdown to start of game
void winGame(); //display win of game (including blinking leds)
void displayKey(char currKey);  //display key on LCD and leds

void configUserLED(char inbits);
char btnState();
void playGame(int[], int[]);

double score;
double error;
double percent; //if player gets less than 70%, they lose
unsigned char dispThree[3];
float delayTime = 2;
int duration[] = {1,1,1,1,1,
                  1,1,1,1,1,
                  1,1,1,1,1,
                  1,1,1,1,1,
                  1,1,1,1,1};
int pitch[] = {NOTE_C, NOTE_E, NOTE_G, NOTE_B, NOTE_A,
               NOTE_G,NOTE_E, NOTE_C,NOTE_D, NOTE_F,
               NOTE_A, NOTE_Cs, NOTE_B,
               NOTE_C, NOTE_E, NOTE_G, NOTE_B, NOTE_A,
               NOTE_G,NOTE_E, NOTE_C,NOTE_D, NOTE_F,
               NOTE_A, NOTE_Cs, NOTE_B};
int sequence[] = {0};
int playerInput[] = {0};
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
    initButtons();   //initializes S1 (1.1) and S2 (2.1) on the board
    configUserBtns();    //configure S1 thru S4 on the board
    configNotif();  //Configure correct and incorrect notification lights

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
                while(true)
                {
                    char x = btnState();
                    configUserLED(x);
                }

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
            case 2:                     //play song
                setLeds(0);             //start LEDs off
                playGame(pitch, duration);
                state = 3;               //go to state 3 => check for input
                break;
            case 3:                    //check input
                setLeds(0);
                break;
            case 4:
                endGame();              //end of game
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
                winGame();              //win
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
 * Function to return the state of the lab board buttons with 1 = pressed
 */
char btnState()
{
    if ((P7IN & BIT0) == 0 && (P3IN & BIT6) == 0 && (P2IN & BIT2) == 0 && (P7IN & BIT4) == 0)
        return 0x0F;
    else if ((P7IN & BIT0) == 0 && (P3IN & BIT6) == 0 && (P2IN & BIT2) == 0)   //S1 + S2 + S3 is pressed
        return 0x0E;
    else if ((P3IN & BIT6) == 0 && (P2IN & BIT2) == 0 && (P7IN & BIT4) == 0)   //S2 + S3 + S4 is pressed
        return 0x07;
    else if ((P7IN & BIT0) == 0 && (P2IN & BIT2) == 0 && (P7IN & BIT4) == 0)   //S1 + S3 + S4 is pressed
        return 0x0B;
    else if ((P7IN & BIT0) == 0 && (P3IN & BIT6) == 0 && (P7IN & BIT4) == 0)   //S1 + S2 + S4 is pressed
        return 0x0D;
    else if ((P7IN & BIT0) == 0 && (P3IN & BIT6) == 0)   //S1 + S2 is pressed
        return 0x0C;
    else if ((P7IN & BIT0) == 0 && (P2IN & BIT2) == 0)   //S1 + S3 is pressed
        return 0x0A;
    else if ((P7IN & BIT0) == 0 && (P7IN & BIT4) == 0)   //S1 + S4 is pressed
        return 0x09;
    else if ((P3IN & BIT6) == 0 && (P2IN & BIT2) == 0)   //S2 + S3 is pressed
        return 0x06;
    else if ((P3IN & BIT6) == 0 && (P7IN & BIT4) == 0)   //S2 + S4 is pressed
        return 0x05;
    else if ((P2IN & BIT2) == 0 && (P7IN & BIT4) == 0)   //S3 + S4 is pressed
        return 0x03;
    else if ((P7IN & BIT0) == 0)     //S1 is pressed
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
    setLeds(inbits);
}

/*
 * Function to display button press on LEDS
 */
void displayButton(char inbits)
{
    unsigned char disp[1];
    switch(inbits)
    {
    case 0x0F:
        disp[0] = "Press S1 \t S2 \t S3 \t S4";
        break;
    case 0x0E:
        disp[0] = "Press S1 \t S2 \t S3";
        break;
    case 0x07:
        disp[0] = "Press S2 \t S3 \t S4";
        break;
    case 0x0B:
        disp[0] = "Press S1 \t S3 \t S4";
        break;
    case 0x0D:
        disp[0] = "Press S1 \t S2 \t S4";
        break;
    case 0x0C:
        disp[0] = "Press S1 \t S2";
        break;
    case 0x0A:
        disp[0] = "Press S1 \t S3";
        break;
    case 0x09:
        disp[0] = "Press S1 \t S4";
        break;
    case 0x06:
        disp[0] = "Press S2 \t S3";
        break;
    case 0x05:
        disp[0] = "Press S2 \t S4";
        break;
    case 0x03:
        disp[0] = "Press S3 \t S4";
        break;
    case 0x08:
        disp[0] = "Press S1";
        break;
    case 0x04:
        disp[0] = "Press S2";
        break;
    case 0x02:
        disp[0] = "Press S3";
        break;
    case 0x01:
        disp[0] = "Press S4";
        break;
    case 0x00:
        disp[0] = "Don't Press";
        break;
    default:
        disp[0] = "inbit not found";
        break;
    }

    Graphics_drawStringCentered(&g_sContext, disp, AUTO_STRING_LENGTH, 15, 45, TRANSPARENT_TEXT);

    // Draw a box around everything because it looks nice
    Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
            Graphics_drawRectangle(&g_sContext, &box);
            Graphics_flushBuffer(&g_sContext);
}

/*
 * Function to play game
 */
void playGame(int pitch[], int duration[])
{
    int i;
    int size = *(&pitch + 1) - pitch;
    for (i = 0; i<size; i++)
    {
        BuzzerOn(pitch[i]);
        swDelay(duration[i]/2.0);
        BuzzerOff();
        swDelay(1);
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
}


/*
 * Function to do a software delay using useless loops to waste a bit of time
 * Parameter(s): char numLoops => number of delay loops to execute
 * Return: none
 */
void swDelay(char numLoops)
{
    volatile unsigned int i,j;  // volatile to prevent removal in optimization
                                // by compiler. Functionally this is useless code

    for (j=0; j<numLoops; j++)
    {
        i = 50000;                  // SW Delay
        while (i > 0)               // could also have used while (i)
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

/*
 * Function to display the countdown on the LCD
 * Parameter(s): none
 * Return: none
 */
void countdown()
{
    int i;
    int x = 14;
    unsigned char check[1];

    for (i = 3; i>0; i--)
    {
        check[0] = i+48;
        x -= 2*(3-i);
        setLeds(x);

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
    swDelay(1);
    Graphics_drawStringCentered(&g_sContext, "GO!!!", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);
    swDelay(1);
    Graphics_clearDisplay(&g_sContext); // Clear the display
}



