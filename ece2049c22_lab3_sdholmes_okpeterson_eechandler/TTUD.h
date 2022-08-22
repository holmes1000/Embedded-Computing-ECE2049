/*
 * TTUD.h
 *
 *  Created on: Feb 2, 2022
 *      Authors: Ethan, Samara, Olivia
 */

// Time and Temperature Utility Display
#ifndef TTUD_H_
#define TTUD_H_
#include "peripherals.h"

#define CALADC12_15V_30C  *((unsigned int *)0x1A1A)
#define CALADC12_15V_85C  *((unsigned int *)0x1A1C)
#define  NOTE_C3     131
#define  NOTE_E3     165
#define  NOTE_G3     208
#define  NOTE_C4     262
#define  NOTE_F4     349

// Temperature Sensor Calibration = Reading
// at 85 degrees C is stored at address 1A1Ch
typedef struct
    {
    unsigned int *array;
    size_t used;
    size_t size;
    } Array;

typedef struct
    {
    float *data;
    int dataSize;
    int filterIndex;
    } Filter;

// an array of the last FILTER_SIZE data points; this will be used as a circular buffer
// with the index being updated for each new data point, until it rolls over

unsigned int months[] =
    {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    }; // this can easily be made compatible with leap years, but its not really necessary

unsigned int notes[] =
    {
    NOTE_C3, NOTE_E3, NOTE_G3, NOTE_C4, NOTE_F4
    };

const char monthsName[13][4] =
    {
	    "JAN",
	    "FEB",
	    "MAR",
	    "APR",
	    "MAY",
	    "JUN",
	    "JUL",
	    "AUG",
	    "SEP",
	    "OCT",
	    "NOV",
	    "DEC"
    };

Array time; // stores months, days, hours, minutes, seconds in that order

Filter avFilteredTemp;
Filter mFilteredTemp;

Filter avFilteredScroll;
Filter mFilteredScroll;

enum TTUDState
    {
    TTUD_HAS_A_PROBLEM,
    TTUD_IDLE,
    TTUD_MEASURING,
    TTUD_EDITING
    } ttud;

enum TTUDState TTUDStatePrev = TTUD_IDLE;
enum TTUDState TTUDState = TTUD_IDLE;

unsigned int editFlags = 0;

unsigned char stringNumber[3];
unsigned char currKey = 0;
unsigned char currBtn = 0;
unsigned char asn[24];

unsigned long long dispMillis = (unsigned long long) 86400000 * 1 + 3600000 * 11
	+ (unsigned long long) 60000 * 59 + (unsigned long long) 1000 * 59;
unsigned long long tempMillis = 0;
unsigned long long millis = 0;

unsigned long long debugMessageTime = 0;
unsigned long long graphicsTimer = 0;
unsigned long long buzzerTime = 0;
unsigned long long badKeyTime = 0;
unsigned long long keyTime = 0;

float temperatureDegC;
float temperatureDegF;
float degC_per_bit;

unsigned int bits30, bits85;

unsigned int debugMessagePeriod = 3000;
unsigned int prev_adc_in_scroll = 0;
unsigned int prev_adc_in_temp = 0;
unsigned int badKeyHysteresis = 3000; // needs 3 seconds for debouncing because holy crap these keys are bad
unsigned int keyHysteresis = 250;
unsigned int newStateCounter = 1;
unsigned int adc_in_scroll = 0;
unsigned int refreshRate = 45; // you get 22 fps suck it up
unsigned int adc_in_temp = 0;
unsigned int measMonth = 0;
unsigned int measDays = 0;
unsigned int measHour = 0;
unsigned int currDay = 1;
unsigned int prevDay = 1;
unsigned int measMin = 0;
unsigned int measSec = 0;

bool clearBuffer = false;
bool flushBuffer = false;
bool newState = true;

Graphics_Rectangle box =
    {
    .xMin = 3, .xMax = 93, .yMin = 3, .yMax = 93
    };

void swapArray(float *sourceArr, int i, float *destArr, int j);
bool getScroll(unsigned int *scroll, unsigned int *prevScroll);
float addAndReturnAverage(Filter *avFilter, float value);
bool getTemp(unsigned int *temp, unsigned int *prevTemp);
void ftoa(float n, unsigned char *res, int afterpoint);
float addAndReturnMedian(Filter *mFilter, float value);
void insertArray(Array *a, unsigned int element);
int intToStr(int x, unsigned char str[], int d);
void initArray(Array *a, size_t initialSize);
void initFilter(Filter *filter, size_t size);
void handleScrollWheel(unsigned int scroll);
void addDatum(Filter *filter, float value);
void reversed(unsigned char *str, int len);
void handleTempSensor(unsigned int temp);
void freeFilterBuffer(Filter *filter);
void itoa(int n, unsigned char s[]);
void reverseStr(unsigned char s[]);
void handleMeasurements(void);
void handleKeyPress(void);
void handleGraphics(void);
void freeArray(Array *a);
void handleProblem(void);
int concat(int a, int b);
void checkWDTIFG(void);
void timerA0Init(void);
void timerA2Init(void);
void setupClock(void);
char btnState(void);
void getTime(void);
void ADCInit(void);
void init(void);
void loop(void);

void init(void)
    {

    WDTCTL = WDTPW + WDTHOLD;
    _BIS_SR(GIE);
    setupClock();
    initLeds();
    configDisplay();
    configKeypad();
    Graphics_clearDisplay(&g_sContext);
    timerA0Init();
    timerA2Init();
    ADCInit();
    WDTCTL = WDT_ARST_250;
    }

void loop(void)
    {

    WDTCTL = WDT_ARST_250; // WD system reset timer of 250 ms
    // note that this is clocked by ACLK, which is assumed 32kHz.
    // the loop is significantly faster than this, so the WDT will only trigger
    // if the program gets stuck in a state via blocking

    checkWDTIFG();

    if (TTUDStatePrev != TTUDState)
	{

	setLeds(0);
	Graphics_clearDisplay(&g_sContext);
	BuzzerOff();
	newState = true;
	newStateCounter++;
	}

    currKey = getKey();
    currBtn = btnState();
    TTUDStatePrev = TTUDState;

    switch (TTUDState)
	{

    // You can never leave this state once you enter it. this state is purely for debugging
    // It can only be entered if the loop takes an absurdly long time
    case TTUD_HAS_A_PROBLEM:
	{
	handleProblem(); // just displays a message

	break;
	}
    case TTUD_IDLE:
	{
	handleKeyPress();
	handleMeasurements();

	break;
	}
    case TTUD_MEASURING:
	{
	handleKeyPress();
	handleMeasurements();

	break;
	}
    case TTUD_EDITING:
	{
	handleKeyPress();
	handleMeasurements();
	}
    default:

	break;

	}

    }

void handleKeyPress(void)
    {

    if (currKey == '#')
	{

	TTUDState = TTUD_IDLE;
	}

    if (currKey == '*')
	{

	TTUDState = TTUD_MEASURING;
	}
    if (currKey == '0')
	{

	TTUDState = TTUD_EDITING;
	}

    switch (TTUDState)
	{
    case TTUD_IDLE:
	{
	if (newState)
	    {
	    setLeds(0);
	    Graphics_drawStringCentered(&g_sContext, "Welcome to",
	    AUTO_STRING_LENGTH, 48, 16, TRANSPARENT_TEXT);
	    Graphics_drawStringCentered(&g_sContext, "Temp. Timer",
	    AUTO_STRING_LENGTH, 48, 33, TRANSPARENT_TEXT);
	    Graphics_drawStringCentered(&g_sContext, "Utility Disp.",
	    AUTO_STRING_LENGTH, 48, 50, TRANSPARENT_TEXT);
	    Graphics_drawRectangle(&g_sContext, &box);
	    Graphics_flushBuffer(&g_sContext);
	    newState = false;

	    if (newStateCounter > 999)
		{

		Graphics_drawStringCentered(&g_sContext, "Invalid Number",
		AUTO_STRING_LENGTH, 48, 48, TRANSPARENT_TEXT);
		Graphics_flushBuffer(&g_sContext);
		}

	    else
		{

		itoa(newStateCounter, stringNumber);
		Graphics_drawStringCentered(&g_sContext, "New States:",
		AUTO_STRING_LENGTH, 48, 67, TRANSPARENT_TEXT);
		Graphics_drawStringCentered(&g_sContext, stringNumber,
		AUTO_STRING_LENGTH, 48, 84, TRANSPARENT_TEXT);
		Graphics_flushBuffer(&g_sContext);
		}

	    }

	break;
	}

    case TTUD_MEASURING:
	{
	if (currBtn == 0x08)
	    {

	    TTUDState = TTUD_EDITING;
	    }
	if (currKey == '0')
	    {

	    TTUDState = TTUD_EDITING;
	    }

	break;
	}
    case TTUD_EDITING:
	{
	// note- new state is not set to false until handleMeasurements is called
	// this was mostly a design decision in that i did not want to initialize filter arrays
	// and such in a function that was meant to handle button presses
	if (newState)
	    {
	    badKeyTime = millis;
	    keyTime = millis;
	    tempMillis = 0;
	    editFlags = 1;
	    measMonth = 0;
	    measDays = 1;
	    measHour = 0;
	    measMin = 0;
	    measSec = 0;
	    }

	if (millis > buzzerTime + 5 * keyHysteresis)
	    {

	    BuzzerOff();
	    setSeqLeds(0);
	    }

	if (millis > (badKeyTime + badKeyHysteresis))
	    {
	    switch (currBtn)
		{

	    case 0x08:
		{
		editFlags++;
		badKeyTime = millis;

		break;
		}
	    case 0x04:
		{
		TTUDState = TTUD_MEASURING;

		break;
		}
	    default:

		break;
		}
	    }

	if (millis > (keyTime + keyHysteresis))
	    {

	    if (currKey >= 0x31 && currKey <= 0x35)
		{
		char tmp[2];
		tmp[0] = currKey;
		tmp[1] = '\0';
		editFlags = atoi(tmp);
		setSeqLeds(tmp[0]);
		BuzzerOn(notes[atoi(tmp) - 1]);
		buzzerTime = millis;
		keyTime = millis;
		}
	    }

	}
	break;

    default:

	break;
	}

    }

bool getScroll(unsigned int *scroll, unsigned int *prevScroll)
    {

// only measure changes greater than 1 degree (for use in editing state)
    if (scroll != prevScroll)
	{
	prevScroll = scroll;
	return true;
	}

    return false;
    }

void handleScrollWheel(unsigned int scroll)
    {

    unsigned int scrollADC = scroll;
    float scrollDegrees = (float) (scrollADC * 0.01343) + 30.0;
    float avgDeg = addAndReturnAverage(&avFilteredScroll, scrollDegrees);
//    float medDeg = addAndReturnAverage(&mFilteredScroll, scrollDegrees);
// filters aren't really needed on the scroll wheel, but nice to have. works well enough as is

    unsigned char scrollTxt[] = "Pot.";
    Graphics_drawStringCentered(&g_sContext, scrollTxt,
    AUTO_STRING_LENGTH, 48, 12, TRANSPARENT_TEXT);

    unsigned char scrollStr[3];
    itoa(scrollDegrees, scrollStr);
    Graphics_drawStringCentered(&g_sContext, scrollStr,
    AUTO_STRING_LENGTH, 48, 24, TRANSPARENT_TEXT);

    if (TTUDState == TTUD_EDITING)
	{

	switch (editFlags)
	    {

	case 1:
	    { // editing months
	    tempMillis = 0;
	    // recall that for a linear conversion between two distinct ranges,
	    // new_value = ((old_value - old_min) / (old_max - old_min)) * (new_max - new_min) + new_min
	    measMonth = (unsigned int) ((float) (((scrollDegrees) - 30)
		    / (84 - 30)) * (11 - 0) + 0); // simple linear scaling

	    unsigned int invMonths;
	    if (measMonth > 0)
		{
		for (invMonths = 0; invMonths < measMonth; invMonths++)
		    {

		    tempMillis += months[invMonths] * (unsigned long) 86400000;
		    }
		}
	    dispMillis = tempMillis;

	    unsigned char monthsTxt[] = "Months:";

	    Graphics_drawStringCentered(&g_sContext, monthsTxt,
	    AUTO_STRING_LENGTH, 24, 48, TRANSPARENT_TEXT);

	    Graphics_drawStringCentered(&g_sContext, monthsName[measMonth],
	    AUTO_STRING_LENGTH, 72, 48, TRANSPARENT_TEXT);

	    break;
	    }
	case 2: // editing days
	    {
	    measDays =
		    (unsigned int) ((float) ((scrollDegrees - 30) / (84 - 30))
			    * (months[measMonth] - 1) + 1); // simple linear scaling

	    dispMillis = (unsigned long long) tempMillis
		    + (unsigned long) 86400000 * measDays;

	    unsigned char daysTxt[] = "Days:";

	    Graphics_drawStringCentered(&g_sContext, daysTxt,
	    AUTO_STRING_LENGTH, 24, 48, TRANSPARENT_TEXT);

	    unsigned char daysStr[3];

	    itoa(measDays, daysStr);

	    Graphics_drawStringCentered(&g_sContext, daysStr,
	    AUTO_STRING_LENGTH, 72, 48, TRANSPARENT_TEXT);

	    break;
	    }
	case 3: // editing hours
	    {
	    measHour =
		    (unsigned int) ((float) ((scrollDegrees - 30) / (84 - 30))
			    * (23 - 0) + 0);  // simple linear scaling

	    dispMillis = (unsigned long long) tempMillis
		    + (unsigned long) 86400000 * measDays
		    + (unsigned long) 3600000 * measHour;

	    unsigned char hoursTxt[] = "Hours:";

	    Graphics_drawStringCentered(&g_sContext, hoursTxt,
	    AUTO_STRING_LENGTH, 24, 48, TRANSPARENT_TEXT);

	    unsigned char hoursStr[3];

	    itoa(measHour, hoursStr);

	    Graphics_drawStringCentered(&g_sContext, hoursStr,
	    AUTO_STRING_LENGTH, 72, 48, TRANSPARENT_TEXT);

	    break;
	    }
	case 4: // editing minutes
	    {
	    measMin = (unsigned int) ((float) ((scrollDegrees - 30) / (84 - 30))
		    * (60 - 0) + 0);  // simple linear scaling

	    if (measMin >= 60)
		{
		measMin = 59;
		}

	    dispMillis = (unsigned long long) tempMillis
		    + (unsigned long) 86400000 * measDays
		    + (unsigned long) 3600000 * measHour
		    + (unsigned long) 60000 * measMin;

	    unsigned char minsTxt[] = "Mins:";

	    Graphics_drawStringCentered(&g_sContext, minsTxt,
	    AUTO_STRING_LENGTH, 24, 48, TRANSPARENT_TEXT);

	    unsigned char minsStr[3];

	    itoa(measMin, minsStr);

	    Graphics_drawStringCentered(&g_sContext, minsStr,
	    AUTO_STRING_LENGTH, 72, 48, TRANSPARENT_TEXT);

	    break;
	    }
	case 5: // editing seconds
	    {
	    measSec = (unsigned int) ((float) ((scrollDegrees - 30) / (84 - 30))
		    * (60 - 0) + 0);  // simple linear scaling

	    if (measSec >= 60)
		{
		measSec = 59;
		}

	    dispMillis = (unsigned long long) tempMillis
		    + (unsigned long) 86400000 * measDays
		    + (unsigned long) 3600000 * measHour
		    + (unsigned long) 60000 * measMin
		    + (unsigned int) 1000 * measSec;

	    unsigned char sescTxt[] = "Secs:";

	    Graphics_drawStringCentered(&g_sContext, sescTxt,
	    AUTO_STRING_LENGTH, 24, 48, TRANSPARENT_TEXT);

	    unsigned char secsStr[3];

	    itoa(measSec, secsStr);

	    Graphics_drawStringCentered(&g_sContext, secsStr,
	    AUTO_STRING_LENGTH, 72, 48, TRANSPARENT_TEXT);

	    break;
	    }
	case 6: // if you press it again when editing seconds, circle back to months. press S2 to confirm edits.
	    {
	    editFlags = 1;

	    break;
	    }
	default:

	    break;

	    }
	}
    }

bool getTemp(unsigned int *temperature, unsigned int *prevTemperature)
    {

    if (temperature != prevTemperature)
	{
	prevTemperature = temperature;
	return true;
	}

    return false;
    }

void handleTempSensor(unsigned int temp)
    {

    unsigned int temperature = temp;
    float temperatureDegC = (float) ((long) temperature - CALADC12_15V_30C)
	    * degC_per_bit + 30.0;
    float temperatureDegF = temperatureDegC * 9.0 / 5.0 + 32.0;
    float avg = addAndReturnAverage(&avFilteredTemp, temperatureDegF);
    float med = addAndReturnMedian(&mFilteredTemp, temperatureDegF);
// because it is a circular buffer that resizes until it hits its target size,
// it is okay if it tries to filter before there are 25 values. It will still be correct
    unsigned char avgTxt[] = "Avg.";

    Graphics_drawStringCentered(&g_sContext, avgTxt,
    AUTO_STRING_LENGTH, 24, 36, TRANSPARENT_TEXT);

    unsigned char temp1Txt[] = "Temp.";
    Graphics_drawStringCentered(&g_sContext, temp1Txt,
    AUTO_STRING_LENGTH, 24, 48, TRANSPARENT_TEXT);

    unsigned char avgStr[7];

    ftoa(avg, avgStr, 2);

    Graphics_drawStringCentered(&g_sContext, avgStr,
    AUTO_STRING_LENGTH, 24, 60, TRANSPARENT_TEXT);

    unsigned char medTxt[] = "Med.";

    Graphics_drawStringCentered(&g_sContext, medTxt,
    AUTO_STRING_LENGTH, 72, 36, TRANSPARENT_TEXT);

    unsigned char temp2Txt[] = "Temp.";
    Graphics_drawStringCentered(&g_sContext, temp2Txt,
    AUTO_STRING_LENGTH, 72, 48, TRANSPARENT_TEXT);

    unsigned char medStr[7];

    ftoa(med, medStr, 2);

    Graphics_drawStringCentered(&g_sContext, medStr,
    AUTO_STRING_LENGTH, 72, 60, TRANSPARENT_TEXT);
    }

void getTime(void)
    {

    freeArray(&time);
    initArray(&time, 5);

// this series of calculations will take less than a second, so it is fine if we assume that millis is unchanging
    unsigned long secondsLeft = (unsigned long) (dispMillis / 1000);

    unsigned long days = (unsigned long) (secondsLeft / 86400); //rounds down

    secondsLeft -= (86400 * days);

    unsigned long hours = secondsLeft / 3600; //rounds down
    bool leadingZHours = false;
    // leading hours is implemented after AMPM determination
    secondsLeft -= (3600 * hours);

    unsigned long minutes = secondsLeft / 60; //rounds down
    bool leadingZMinutes = false;
    if (minutes < 10)
	{
	leadingZMinutes = true;
	}
    secondsLeft -= (60 * minutes);

    unsigned long tSeconds = secondsLeft; //assign a new variable the leftover seconds
    bool leadingZSeconds = false;
    if (tSeconds < 10)
	{
	leadingZSeconds = true;
	}

// now deal with months

    unsigned long months_num = 1;
    unsigned long day1 = days;
    bool done = false;
    int mVar;
    for (mVar = 0; mVar < 12 && !done; mVar++)
	{

	if (day1 <= months[mVar])
	    {
	    days = day1 + 1;
	    months_num = mVar + 1;
	    if (months_num > 12)
		{
		months_num = 1;
		}
	    done = 1;
	    }
	else
	    {
	    day1 -= months[mVar];
	    }
	}

    days = day1;

    if (hours == 0)
	{
	dispMillis += 3600000 * 12;
	hours = 12;
	}

    bool AMPM = true; // true for AM, false for PM
    if (hours > 12)
	{
	AMPM = false;
	hours -= 12;
	}

    if (hours < 10)
	{
	leadingZHours = true;
	}

    bool leadingZDays = false;
    if (days < 10)
	{
	leadingZDays = true;
	}

    bool leadingZMonths = false;
    if (months_num < 10)
	{
	leadingZMonths = true;
	}

    if (leadingZMonths)
	{
	insertArray(&time, 0);
	insertArray(&time, months_num % 10);
	}
    else
	{
	unsigned long temp = months_num % 10;
	insertArray(&time, (months_num - temp) / 10);
	insertArray(&time, temp);
	}

    if (leadingZDays)
	{
	insertArray(&time, 0);
	insertArray(&time, days % 10);
	}
    else
	{
	unsigned long temp = days % 10;
	insertArray(&time, (days - temp) / 10);
	insertArray(&time, temp);
	}

    if (leadingZHours)
	{
	insertArray(&time, 0);
	insertArray(&time, hours % 10);
	}
    else
	{
	unsigned long temp = hours % 10;
	insertArray(&time, (hours - temp) / 10);
	insertArray(&time, temp);
	}

    if (leadingZMinutes)
	{
	insertArray(&time, 0);
	insertArray(&time, minutes % 10);

	}
    else
	{
	unsigned long temp = minutes % 10;
	insertArray(&time, (minutes - temp) / 10);
	insertArray(&time, temp);
	}

    if (leadingZSeconds)
	{
	insertArray(&time, 0);
	insertArray(&time, tSeconds % 10);
	}
    else
	{
	unsigned long temp = tSeconds % 10;
	insertArray(&time, (tSeconds - temp) / 10);
	insertArray(&time, temp);
	}

	{
	char mn[4];

	if (leadingZMonths)
	    {
	    strcpy(mn, monthsName[time.array[1] - 1]);

	    int cpy;
	    for (cpy = 0; cpy < 3; cpy++)
		{
		asn[cpy] = mn[cpy];
		}
	    }
	else
	    {

	    int con = concat(time.array[0], time.array[1]);
	    strcpy(mn, monthsName[con - 1]);

	    int cpy;
	    for (cpy = 0; cpy < 3; cpy++)
		{
		asn[cpy] = mn[cpy];
		}
	    }
	}

    asn[3] = ' ';

    unsigned char strrr1[1];
    unsigned char strrr2[1];

    itoa(time.array[2], strrr1);
    asn[(4)] = strrr1[0];

    itoa(time.array[3], strrr2);
    asn[(5)] = strrr2[0];
    asn[(6)] = ',';

    int i;
    int j = 4;

    for (i = 7; i < time.used + 4; i += 3)
	{

	unsigned char strrr1[1];
	itoa(time.array[j], strrr1);
	j++;
	asn[i] = strrr1[0];

	unsigned char strrr2[1];
	itoa(time.array[j], strrr2);
	j++;
	asn[i + 1] = strrr2[0];

	asn[i + 2] = ':';
	}

    asn[time.used + 5] = ' ';

    if (AMPM)
	{
	asn[time.used + 6] = 'A';
	asn[time.used + 7] = 'M';
	}
    else
	{
	asn[time.used + 6] = 'P';
	asn[time.used + 7] = 'M';
	}

    asn[time.used + 8] = '\0'; // cuts off the last colon

    unsigned char hms[12];
    unsigned char md[8];

    for (i = 0; i < time.used - 3; i++)
	{

	md[i] = asn[i];
	}
    md[time.used - 3] = '\0';

    for (i = time.used - 3; i < time.used + 8; i++)
	{

	hms[i - (time.used - 3)] = asn[i];
	}
    hms[time.used + 1] = '\0';

    Graphics_drawStringCentered(&g_sContext, md,
    AUTO_STRING_LENGTH, 48, 72, TRANSPARENT_TEXT);

    Graphics_drawStringCentered(&g_sContext, hms,
    AUTO_STRING_LENGTH, 48, 84, TRANSPARENT_TEXT);

    }

void handleMeasurements(void)
    {

    switch (TTUDState)
	{

    case TTUD_MEASURING:
	{
	if (newState)
	    {

	    // not overkill in the slightest
	    Graphics_drawRectangle(&g_sContext, &box);
	    Graphics_flushBuffer(&g_sContext);
	    freeArray(&time);
	    freeFilterBuffer(&avFilteredTemp);
	    freeFilterBuffer(&mFilteredTemp);
	    freeFilterBuffer(&avFilteredScroll);
	    freeFilterBuffer(&mFilteredScroll);

	    initArray(&time, 5);
	    initFilter(&avFilteredTemp, 25);
	    initFilter(&mFilteredTemp, 25);
	    initFilter(&avFilteredScroll, 25);
	    initFilter(&mFilteredScroll, 25);

	    graphicsTimer = millis;
	    flushBuffer = true;
	    clearBuffer = false;
	    newState = false;
	    }

	Graphics_drawRectangle(&g_sContext, &box);

	getTime();

	if (getScroll(&adc_in_scroll, &prev_adc_in_scroll))
	    {
	    handleScrollWheel(adc_in_scroll);
	    }

	if (getTemp(&adc_in_temp, &prev_adc_in_temp))
	    {
	    handleTempSensor(adc_in_temp);
	    }

	if ((millis >= (graphicsTimer + refreshRate))
		&& (millis < (graphicsTimer + (refreshRate * 2)))
		|| flushBuffer)
	    {

	    Graphics_flushBuffer(&g_sContext);
	    flushBuffer = false;
	    clearBuffer = true;
	    }

	if ((millis >= (graphicsTimer + (refreshRate * 2)))
		&& (millis < (graphicsTimer + (refreshRate * 3)))
		&& clearBuffer)
	    {

	    graphicsTimer = millis;
	    Graphics_clearDisplay(&g_sContext);
	    flushBuffer = true;
	    clearBuffer = false;
	    }

	break;
	}
    case TTUD_EDITING:
	{
	if (newState)
	    {

	    // not overkill in the slightest
	    Graphics_drawRectangle(&g_sContext, &box);
	    Graphics_flushBuffer(&g_sContext);
	    freeArray(&time);
	    freeFilterBuffer(&avFilteredTemp);
	    freeFilterBuffer(&mFilteredTemp);
	    freeFilterBuffer(&avFilteredScroll);
	    freeFilterBuffer(&mFilteredScroll);

	    initArray(&time, 5);
	    initFilter(&avFilteredTemp, 25);
	    initFilter(&mFilteredTemp, 25);
	    initFilter(&avFilteredScroll, 25);
	    initFilter(&mFilteredScroll, 25);

	    graphicsTimer = millis;
	    flushBuffer = true;
	    clearBuffer = false;
	    newState = false;
	    }

	Graphics_drawRectangle(&g_sContext, &box);
	if (getScroll(&adc_in_scroll, &prev_adc_in_scroll))
	    {
	    handleScrollWheel(adc_in_scroll);
	    }

	if ((millis >= (graphicsTimer + refreshRate))
		&& (millis < (graphicsTimer + (refreshRate * 2)))
		|| flushBuffer)
	    {

	    Graphics_flushBuffer(&g_sContext);
	    flushBuffer = false;
	    clearBuffer = true;
	    }

	if ((millis >= (graphicsTimer + (refreshRate * 2)))
		&& (millis < (graphicsTimer + (refreshRate * 3)))
		&& clearBuffer)
	    {

	    graphicsTimer = millis;
	    Graphics_clearDisplay(&g_sContext);
	    flushBuffer = true;
	    clearBuffer = false;
	    }

	break;
	}
    default:

	break;

	}

    }

void checkWDTIFG(void)
    {

    if (SFRIFG1 & WDTIFG)
	{

	TTUDState = TTUD_HAS_A_PROBLEM;
	SFRIFG1 &= WDTIFG;
	}
    }

void handleProblem(void)
    {

    if (newState)
	{

	Graphics_clearDisplay(&g_sContext);
	Graphics_drawStringCentered(&g_sContext, "WE HAVE A",
	AUTO_STRING_LENGTH, 48, 21,
	TRANSPARENT_TEXT);
	Graphics_drawStringCentered(&g_sContext, "PROBLEM!!!",
	AUTO_STRING_LENGTH, 48, 39,
	TRANSPARENT_TEXT);
	Graphics_flushBuffer(&g_sContext);
	debugMessageTime = millis;
	}

    else
	{

	if (millis > debugMessageTime + debugMessagePeriod)
	    {

	    Graphics_clearDisplay(&g_sContext);
	    Graphics_drawStringCentered(&g_sContext, "FIGURE IT",
	    AUTO_STRING_LENGTH, 48, 21,
	    TRANSPARENT_TEXT);
	    Graphics_drawStringCentered(&g_sContext, "OUT!!!!!",
	    AUTO_STRING_LENGTH, 48, 39,
	    TRANSPARENT_TEXT);
	    Graphics_flushBuffer(&g_sContext);
	    }

	}
    }

void ADCInit(void) // all adc stuffs
    {

    P8SEL &= ~BIT0;
    P8DIR |= BIT0;
    P8OUT |= BIT0;
// Reset REFMSTR to hand over control of internal reference
// voltages to ADC12_A control registers
    REFCTL0 &= ~REFMSTR;
// Initialize control register ADC12CTL0 = 0000 1001 0111 0000
// SHT0x=9h (384 clk cycles), MCS=1=burst thru selected chans.,
// REFON = 1 = use internal reference volts
// and ADC12ON = 1 = turn ADC on
    ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON |
    ADC12MSC;
// Initialize control register ADC12CTL1 = 0000 0010 0000 0010
// ADC12CSTART ADDx = 0000 =  start conversion with ADC12MEM0,
// ADC12SHSx = 00 = use SW conversion trigger, ADC12SC bits
// ADC12SHP = 1 = SAMPCON signal sourced from sampling timer,
// ADC12ISSH = 0 = sample input signal not inverted,
// ADC12DIVx = 000 = divide ADC12CLK by 1,
// ADC12SSEL=00= ADC clock ADC12OSC (~5 MHz),
// ADC12CONSEQx = 01 = sequence of channels converted once
// ADC12BUSY = 0 = no ADC operation active
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1;
// Set conversion memory control registers for the 2 channels
// ADC12MCTL0: EOS = 0, SREF =001 = voltage refs = GND to Vref+
// INCHx = 0000
// Using ADC12MEM0 to store reading
    ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10; // ADC i/p ch A10 = temperature sensor
// ACD12SREF_1 = internal ref = 1.5v
    ADC12IE |= BIT0;    // enable interrupt on ADC12MEM0
    ADC12MCTL1 = ADC12SREF_0 + ADC12INCH_0 + ADC12EOS;    // scroll sensor
    ADC12IE |= BIT1;    // enable interrupt on ADC12MEM1

    __delay_cycles(1000);    // delay to allow Ref to settle

// Use calibration data stored in info memory
    bits30 = CALADC12_15V_30C;
    bits85 = CALADC12_15V_85C;
    degC_per_bit = ((float) (85.0 - 30.0)) / ((float) (bits85 - bits30));

    P6SEL |= BIT0;

    ADC12CTL0 |= ADC12SC + ADC12ENC;    // Start conversion
    }

void timerA0Init(void)
    {

    TA0CCTL0 = CCIE; // CCR0 interrupt enabled
    TA0CCR0 = 12000 - 1; // 12000 cycles = 1ms at 12 MHz
    TA0CTL = TASSEL_2 + MC_1; // SMCLK, upmode
    }

void timerA2Init(void)
    {

    TA2CCTL0 = CCIE;
    TA2CCR0 = 1024 - 1; // 1024 cycles = 0.03125 seconds at 32 kHz
// this update rate happens to be pretty close to the refresh rate,
// so every now and then some values will show up twice. but quite frankly I could care less

// recall that ACLK is set to REF0, so still 32 kHz,
// even though the other clocks are boosted to 12 MHz
    TA2CTL = TASSEL_1 + MC_1;    // ACLK, upmode
    }

// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
    {

    millis++;
    if (TTUDState != TTUD_EDITING)
	{
	dispMillis++;
	}
    }

// Timer A2 interrupt service routine
#pragma vector = TIMER2_A0_VECTOR
__interrupt void Timer_A2(void)
    {

    ADC12CTL0 |= ADC12SC + ADC12ENC;  // Start conversion
    }

#pragma vector = ADC12_VECTOR // move adc values to memory
__interrupt void ADC12ISR(void)
    {

    adc_in_temp = ADC12MEM0;
    adc_in_scroll = ADC12MEM1;
    }

char btnState()
    {
    if ((P1IN & BIT1) == 0)     // Launchpad S2 is pressed
	return 0x04;
    else if ((P2IN & BIT1) == 0)    // Launchpad S1 is pressed
	return 0x08;
    else
	return 0x00;
    }

float addAndReturnAverage(Filter *avFilter, float value) // averaging filter
    {

    addDatum(avFilter, value);
    float average = 0;

    int i;
    for (i = 0; i < avFilter->dataSize; i++)
	{

	average += avFilter->data[i];
	}
    return average / avFilter->dataSize;
    }

// for funsies (median filter)
float addAndReturnMedian(Filter *mFilter, float value)
    {

    addDatum(mFilter, value);
//we're going to sort the last dataSize data points, but note
//that we don't sort the original array, as that would mess up our time series
//this is a brute force method -- there are better ways

//first copy data into a temporary array
    float *tempArray;
// allocate zeroed memory to tempArray
    tempArray = calloc(sizeof(*tempArray), mFilter->dataSize);
	{
	int i;
	for (i = 0; i < mFilter->dataSize; i++)
	    {
	    tempArray[i] = mFilter->data[i];
	    }
	}

//now brute force (bubble) sort the temporary array
	{
	int i;
	for (i = 0; i < mFilter->dataSize; i++)
	    {
	    int j;
	    for (j = i + 1; j < mFilter->dataSize; j++)
		{
		if (tempArray[j] < tempArray[i])
		    swapArray(tempArray, i, tempArray, j);
		}
	    }
	}
// free memory
    free(tempArray);

    return tempArray[mFilter->dataSize / 2];

    }

void swapArray(float *sourceArr, int i, float *destArr, int j) // swaps the values of two indices in an array
    {
    float tmp = sourceArr[i];
    sourceArr[i] = destArr[i];
    destArr[i] = tmp;
    }

void initArray(Array *a, size_t initialSize)
    {

    a->array = malloc(initialSize * sizeof(unsigned int));
    a->used = 0;
    a->size = initialSize;
    }

// a generic function handler would be really nice right about now but oh well
void initFilter(Filter *filter, size_t size)
    {

    filter->data = malloc(size * sizeof(float));
    filter->filterIndex = 0;
    filter->dataSize = size;
    }

void insertArray(Array *a, unsigned int element)
    {

// a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
// Therefore a->used can go up to a->size

    if (a->used == a->size)
	{

	a->size++;
	a->array = realloc(a->array, a->size * sizeof(unsigned int));
	}

    a->array[a->used++] = element;
    }

void addDatum(Filter *filter, float value)
    {
    filter->data[filter->filterIndex++] = value;
    if (filter->filterIndex == filter->dataSize)
	filter->filterIndex = 0;
    }

void freeArray(Array *a)
    {

    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
    }

void freeFilterBuffer(Filter *filter)
    {
    free(filter->data);
    filter->data = NULL;
    filter->dataSize = filter->filterIndex = 0;
    }

// the six conversion functions below are found in most normal c libraries but i could not find them in the one provided by Ti
// so i had them implemented here:

// itoa:  convert n to characters in s
void itoa(int n, unsigned char s[])
    {

    int i, sign;

    if ((sign = n) < 0) // record sign
	n = -n; // make n positive
    i = 0;
    do
	{ // generate digits in reverse order

	s[i++] = n % 10 + '0'; // get next digit
	}

    while ((n /= 10) > 0); // delete it

    if (sign < 0)
	s[i++] = '-';
    s[i] = '\0';
    reverseStr(s);
    }

// for itoa
// reverse:  reverse string s in place
void reverseStr(unsigned char s[])
    {

    int i, j;
    unsigned char c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--)
	{

	c = s[i];
	s[i] = s[j];
	s[j] = c;
	}

    }

// for intToStr (very slightly different)
// Reverses a string 'str' of length 'len'
void reversed(unsigned char *str, int len)
    {
    int i = 0, j = len - 1, temp;
    while (i < j)
	{
	temp = str[i];
	str[i] = str[j];
	str[j] = temp;
	i++;
	j--;
	}
    }

// different from itoa in that it adds leading zeros
// Converts a given integer x to string str[].
// d is the number of digits required in the output.
// If d is more than the number of digits in x,
// then 0s are added at the beginning.
int intToStr(int x, unsigned char str[], int d)
    {
    int i = 0;
    while (x)
	{
	str[i++] = (x % 10) + '0';
	x = x / 10;
	}

// If number of digits required is more, then
// add 0s at the beginning
    while (i < d)
	str[i++] = '0';

    reversed(str, i);
    str[i] = '\0';
    return i;
    }

// Converts a floating-point/double number to a string.
void ftoa(float n, unsigned char *res, int afterpoint)
    {
// Extract integer part
    int ipart = (int) n;

// Extract floating part
    float fpart = n - (float) ipart;

// convert integer part to string
    int i = intToStr(ipart, res, 0);

// check for display option after point
    if (afterpoint != 0)
	{
	res[i] = '.'; // add dot

// Get the value of fraction part up to given no.
// of points after dot. The third parameter
// is needed to handle cases like 233.007
	fpart = fpart * pow(10, afterpoint);

	intToStr((int) fpart, res + i + 1, afterpoint);
	}
    }

int concat(int a, int b)
    {

    char s1[20];
    char s2[20];

    // Convert both the integers to string
    sprintf(s1, "%d", a);
    sprintf(s2, "%d", b);

    // Concatenate both strings
    strcat(s1, s2);

    // Convert the concatenated string
    // to integer
    int c = atoi(s1);

    // return the formed integer
    return c;
    }

void setupClock(void) // SUPER SPEEEEEEEED (12 MHz
    {

    UCSCTL3 |= SELREF_2;             // Set DCO FLL reference = REFO

    UCSCTL4 |= SELA_2;             // Set ACLK = REFO

    __bis_SR_register(SCG0);             // Disable the FLL control loop

    UCSCTL0 = 0x0000;             // Set lowest possible DCOx, MODx

    UCSCTL1 = DCORSEL_5;             // Select DCO range 24MHz operation

    UCSCTL2 = FLLD_1 + 374;             // Set DCO Multiplier for 12MHz

// (N + 1) * FLLRef = Fdco

// (374 + 1) * 32768 = 12MHz

// Set FLL Div = fDCOCLK/2

    __bic_SR_register(SCG0);             // Enable the FLL control loop

// Worst-case settling time for the DCO when the DCO range bits have been

// changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx

// UG for optimization.

// 32 x 32 x 12 MHz / 32,768 Hz = 375000 = MCLK cycles for DCO to settle

    __delay_cycles(375000);    // Loop until XT1,XT2 & DCO fault flag is cleared

    do

	{

	UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);

// Clear XT2,XT1,DCO fault flags

	SFRIFG1 &= ~OFIFG;             // Clear fault flags

	}

    while (SFRIFG1 & OFIFG);         // Test oscillator fault flag

    }

#endif // TTUD_H_
