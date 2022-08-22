/*
 * note.h
 *
 *  Created on: Feb 5, 2022
 *      Author: samar
 */

#ifndef NOTE_H_
#define NOTE_H_

typedef struct {
    int pitch;       //stores the frequency of the note
    float duration;  //stores the duration of the note
    char led;        //store the led that needs to light up for that note (1-4)
    int pos;	     //dont mind me, just encoding a little extra information for the animation
} Note;


//Notes, LEDs, and LCD display at the same time
//button corresponds to notes inputted
//then they are compared
						//rests are pos 0
#define  NOTE_A     440                 //1	//pos 1
#define  NOTE_Bb    466    //B flat     //1	//pos 2
#define  NOTE_B     494                 //1	//pos 3

#define  NOTE_C     523                 //2	//pos 4
#define  NOTE_Cs    554    //C sharp    //2	//pos 5
#define  NOTE_D     587                 //2	//pos 6

#define  NOTE_Eb    622    //E flat     //3	//pos 7
#define  NOTE_E     659                 //3	//pos 8
#define  NOTE_F     698                 //3	//pos 9

#define  NOTE_Fs    740    //F sharp    //4	//pos 10
#define  NOTE_G     784                 //4	//pos 11
#define  NOTE_Ab    831 //A flat        //4	//pos 12
#define  NOTE_A2    880                 //4	//pos 13
#define  NOTE_A3    900                 //4	//pos 14
#define  NOTE_C4    920                 //4	//pos 15
#define  NOTE_Alow     400                //4 //pos 16

//Mr.Sandman by The Chordettes size = 78
Note sandman[] = {
                  {NOTE_A, 100, 2, 1}, {NOTE_Cs, 100, 2, 5}, {NOTE_E, 100, 3, 8}, {NOTE_G, 100, 4, 11}, {NOTE_Fs, 100, 4, 10},
                  {NOTE_E, 100, 3, 8}, {NOTE_Cs, 100, 2, 5}, {NOTE_A, 100, 1, 1}, {NOTE_B, 100, 1, 3}, {NOTE_D, 100, 2, 6},
                  {NOTE_Fs, 100, 4, 8}, {NOTE_A2, 100, 4, 8}, {NOTE_G, 100, 4, 8}, {0, 100, 0, 8},

                  {NOTE_A, 100, 2, 1}, {NOTE_Cs, 100, 2, 5}, {NOTE_E, 100, 3, 8}, {NOTE_G, 100, 4, 11}, {NOTE_Fs, 100, 4, 10},
                  {NOTE_E, 100, 2, 8}, {NOTE_Cs, 100, 2, 5}, {NOTE_A, 100, 1, 1}, {NOTE_B, 100, 1, 3}, {NOTE_D, 100, 2, 6},
                  {NOTE_Fs, 100, 4, 10}, {NOTE_A2, 100, 2, 12}, {NOTE_G, 100, 3, 11}, {0, 100, 0, 0},

                  {NOTE_E, 50, 1, 8}, {NOTE_Fs, 50, 4, 10}, {NOTE_G, 50, 3, 11}, {NOTE_Fs, 100, 2, 10}, {0, 100, 0, 0},
                  {NOTE_G, 50, 2, 11}, {NOTE_G, 50, 3, 11}, {NOTE_G, 50, 1, 11}, {NOTE_G, 50, 2, 11}, {0, 50, 0, 0},
                  {NOTE_A, 50, 4, 11}, {NOTE_Bb, 50, 1, 11}, {NOTE_C, 50, 2, 50}, {NOTE_Cs, 50, 2, 11}, {NOTE_A2, 50, 1, 11},
                  {NOTE_A2, 50, 4, 13}, {NOTE_A2, 50, 4, 14}, {NOTE_G, 50, 1, 11}, {NOTE_A2, 50, 4, 13}, {NOTE_A2, 100, 4, 13},

                  {NOTE_E, 50, 1, 8}, {NOTE_Fs, 50, 4, 10}, {NOTE_G, 50, 3, 11}, {NOTE_Fs, 100, 2, 10}, {0, 100, 0, 0},
                  {NOTE_G, 50, 4, 11}, {NOTE_G, 50, 1, 11}, {NOTE_Fs, 50, 2, 10}, {NOTE_G, 150, 2, 11}, {NOTE_Fs, 50, 3, 10},
                  {NOTE_E, 50, 3, 8}, {NOTE_E, 50, 2, 8}, {NOTE_D, 50, 2, 6}, {NOTE_E, 100, 2, 8},

                  {NOTE_E, 50, 1, 8}, {NOTE_Fs, 50, 4, 10}, {NOTE_G, 50, 3, 11}, {NOTE_Fs, 100, 2, 10}, {0, 100, 0, 0},
                  {NOTE_Ab, 50, 4, 12}, {NOTE_Ab, 50, 4, 12}, {NOTE_A2, 50, 4, 13}, {NOTE_Ab, 50, 4, 12}, {NOTE_A2, 50, 4, 13},
                  {NOTE_Ab, 50, 4, 12}, {NOTE_A2, 50, 4, 13}, {NOTE_A3, 120, 4, 14}, {NOTE_A3, 100, 4, 14}, {NOTE_Ab, 50, 4, 12},
                  {NOTE_G, 130, 2, 11}
};

Note electricLove[] = {
                       {NOTE_A2, 50, 4, 13}, {NOTE_G, 50, 2, 11}, {NOTE_Fs, 50, 2, 10}, {NOTE_E, 50, 2, 8}, {NOTE_Fs, 50, 4, 10},
                       {NOTE_Fs, 15, 2, 10}, {NOTE_Fs, 50, 2, 10}, {NOTE_E, 15, 2, 8}, {NOTE_Fs, 50, 4, 10}, {NOTE_E, 15, 3, 8},
                       {0, 100, 0, 0}, {NOTE_Cs, 50, 2, 5}, {NOTE_E, 50, 3, 8}, {NOTE_Fs, 50, 4, 10}, {NOTE_A2, 50, 4, 13},
                       {NOTE_A3, 50, 4, 14}, {NOTE_A3, 15, 4, 14}, {NOTE_A3, 50, 4, 14}, {NOTE_A2, 15, 2, 13}, {NOTE_A3, 50, 4, 14},
                       {NOTE_C4, 15, 4, 15}, {0, 100, 0, 0}, {NOTE_C4, 50, 4, 15}, {NOTE_A3, 50, 4, 14}, {NOTE_A2, 50, 4, 13},
                       {NOTE_G, 50, 2, 11}, {NOTE_Fs, 50, 2, 10}, {NOTE_E, 50, 2, 8}, {NOTE_Cs, 50, 2, 5}, {NOTE_A, 50, 1, 1},
                       {NOTE_B, 100, 1, 3}, {NOTE_Cs, 100, 2, 5}, {NOTE_D, 50, 2, 6}, {NOTE_E, 50, 3, 8}, {NOTE_Fs, 50, 4, 10},
                       {NOTE_G, 50, 2, 11}, {NOTE_C4, 200, 4, 15}, {NOTE_A3, 50, 4, 14}, {NOTE_A2, 300, 4, 13}
};

Note testWin[] = {
                  {NOTE_A, 100, 1, 1}, {NOTE_C, 100, 2, 11}, {NOTE_Fs, 100, 3, 10}, {NOTE_Ab, 100, 4, 8}, {NOTE_A, 100, 1, 10}
};

Note makeYouMine[] = {
                      {NOTE_Cs, 50, 2, 5}, {NOTE_E, 50, 3, 5},{NOTE_Cs, 50, 2, 5},{NOTE_E, 50, 1, 5}, {NOTE_Cs, 150, 2, 5},
                      {NOTE_E, 50, 1, 5}, {NOTE_Cs, 50, 2, 5}, {NOTE_E, 50, 1, 5}, {NOTE_Fs, 50, 1, 5}, {NOTE_E, 50, 3, 5},
                      {NOTE_Cs, 50, 2, 5}, {NOTE_B, 50, 1, 5}, {NOTE_Alow, 50, 4, 5}, {NOTE_Alow, 50, 4, 5},{NOTE_E, 50, 1, 5},
                      {NOTE_E, 50, 3, 5}, {NOTE_Cs, 100, 2, 5}, {NOTE_E, 50, 3, 5},{NOTE_Cs, 50, 2, 5}, {NOTE_E, 50, 3, 5},
                      {NOTE_Fs, 50, 4, 5}, {NOTE_E, 50, 3, 5}, {NOTE_Cs, 50, 2, 5}, {NOTE_B, 50, 1, 5}, {NOTE_A, 50, 1, 5},
                      {NOTE_A, 50, 1, 5}, {NOTE_E, 50, 3, 5}, {NOTE_E, 50, 3, 5}, {NOTE_Cs, 150, 2, 5}, {NOTE_E, 50, 3, 5},
                      {NOTE_Cs, 50, 2, 5}, {NOTE_E, 50, 3, 5}, {NOTE_Ab, 50, 4, 5}, {NOTE_G, 50, 1, 5}, {NOTE_Fs, 50, 4, 5},
                      {NOTE_E, 50, 3, 5}, {NOTE_Alow, 50, 4, 5}, {NOTE_Alow, 50, 4, 5}, {NOTE_E, 50, 1, 5}, {NOTE_E, 50, 3, 5},
                      {NOTE_Cs, 150, 2, 5}, {NOTE_B, 50, 1, 5}, {NOTE_A, 50, 1, 5}, {NOTE_Alow, 50, 4, 5}, {NOTE_A, 50, 1, 5},
                      {NOTE_B, 80, 1, 5}, {NOTE_D, 200, 2, 5},

};
#endif /* NOTE_H_ */
