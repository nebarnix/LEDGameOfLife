#include <TimerOne.h>

/*
  Scrounged LED sign found at ER 2 by HeatSync Labs.

  The sign uses differential inverting amplifiers LM2901 to drive Schmitt−Trigger
  inverters (74HCT14) and utimately drive two parallel rows of ST2221A LED drivers.

  Top and bottom display the SAME data -- so you must blank them alternately using the top and bottom enable lines!
  Don't do this too slowly or your user will get a flicker headache

  When running on an arduino nano (16Mhz) it takes about 380 microseconds to shift out data to the display (usually this will be 1/2 of the display)
*/
//uncomment below to output highlife instead of Conway (highlife is more active and feeds fed less often)
//#define HIGHLIFE

#define TOP 0
#define BOT 1

#define bit0 B00000001
#define bit1 B00000010
#define bit2 B00000100
#define bit3 B00001000
#define bit4 B00010000
#define bit5 B00100000
#define bit6 B01000000
#define bit7 B10000000

//VCC J1 pin13
//GND J1 pin15

#define DL_N 3 //data and latch reference (data and latch are tied on the negative side, positive sides MUST transition together, and oppose this) //PD3
#define DLP_N bit3 //PD3 //J1 pin6

#define DATA_P 7 //PD7 //J1 pin5
#define DATAP_P bit7 //PD7

#define LATCH_P 2 //PD2 //J1 pin7
#define LATCHP_P bit2 //PD2

#define CLOCK_P 5 //PD5
#define CLOCK_N 6 //PD6

#define CLOCKP_P bit5 //PD5  //J1 pin3
#define CLOCKP_N bit6 //PD6 //J1 pin2

#define BOT_EN 8 //PB0 //J1 pin9 //Might have a differential friend, but this signal doesn't need to be fast so it works as-is
#define TOP_EN 9 //PB1 //J1 pin11 //Might have a differential friend, but this signal doesn't need to be fast so it works as-is

#define BOTP_EN bit0
#define TOPP_EN bit1

#define CLK_US 0
// the setup function runs once when you press reset or power the board

int VisDuration = 1000; //this controls the overall brightness, don't go above 7500 or it gets flickery
const int Height = 14; //top bit of each byte half is not shown!
const int Width = 36;
unsigned int fbuffer[Width]; //LEDs -- high byte is bottom low byte is top. bit0 is the very top.
unsigned long time1=0;
unsigned long time2=0;
char timerState=BOT;

void setup() {
  randomSeed(analogRead(0));
  memset(fbuffer, 0, Width * 2);
  
  //we need some random pixels to set the board
  /*
  putPixel(0,0);    
  putPixel(1,1);    
  putPixel(2,2);    
  putPixel(3,3);    
  putPixel(4,4);    
  putPixel(5,5);    
  putPixel(6,6);    
  putPixel(7,7);    
  putPixel(8,8);    
  putPixel(9,9);    
  putPixel(10,10);    
  putPixel(11,11);    
  putPixel(12,12);    
  putPixel(13,13);    
  putPixel(14,14);    
  */
  
  for(int x=1; x < Width-1; x++)
  {
    for(int y=1; y < Height-1; y++)
      {
      if(random(100) > 50) 
        putPixel(x,y);
      }
  }
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(DATA_P, OUTPUT);
  pinMode(DL_N, OUTPUT);
  pinMode(CLOCK_P, OUTPUT);
  pinMode(CLOCK_N, OUTPUT);
  pinMode(LATCH_P, OUTPUT);

  pinMode(BOT_EN, OUTPUT);
  pinMode(TOP_EN, OUTPUT);

  digitalWrite(DATA_P, LOW);
  digitalWrite(LATCH_P, LOW);
  digitalWrite(DL_N, HIGH);

  digitalWrite(CLOCK_P, HIGH);
  //BOTTOM ROW
  digitalWrite(BOT_EN, HIGH);   digitalWrite(TOP_EN, LOW);
  //TOP ROW
  digitalWrite(BOT_EN, LOW);    digitalWrite(TOP_EN, HIGH);
  //TOP ROW
  digitalWrite(BOT_EN, HIGH);    digitalWrite(TOP_EN, HIGH);
  Timer1.initialize(VisDuration);
  Timer1.attachInterrupt(timerCallback);  
}

void timerCallback()
{
  //BLANK DISPLAY
  PORTB &= ~(BOTP_EN | TOPP_EN);

  //SHIFT OUT TOP OR BOTTOM DATA
  if(timerState == BOT)
    {
    timerState = TOP;
    writeFrameBuffer(BOT);
    //SHOW BOTTOM HALF
    PORTB |= TOPP_EN;  PORTB &= ~BOTP_EN;
    }
  else
    {
    timerState = BOT;
    writeFrameBuffer(TOP);
    //SHOW TOP HALF
    PORTB |= BOTP_EN;  PORTB &= ~TOPP_EN;
    } 
}

// the loop function runs over and over again forever
void loop() {
  //time = micros();
  //reset frame buffer
  //memset(fbuffer, 0, Width * 2); //don't clear the framebuffer every time maybe? we will have to call dead lifeforms by clearpixel
  
  //BLANK DISPLAY
  //PORTB &= ~(BOTP_EN | TOPP_EN);

  //Write to framebuffer here
  

  //if(micros() - time1 > 20000)
    // {
     
     //time1 = micros();
     //}
  
  //feed the cells every second
  //HIGHLIFE every 30 seconds
  #ifndef HIGHLIFE
  if(micros() - time2 > 2000000)
  #endif
  #ifdef HIGHLIFE
  if(micros() - time2 > 30000000)
  #endif
     {
     time2 = micros();
     for(int x=1; x < Width-1; x++)
       {
       for(int y=1; y < Height-1; y++)
        {
        
        if(random(100) > 90) 
          putPixel(x,y);
        }
       }
     }
  AdvanceGameOfLife(); 
          
  //SHIFT BOTTOM DATA    
  //writeFrameBuffer(BOT);
  //SHOW BOTTOM HALF
  //PORTB |= BOTP_EN;  PORTB &= ~TOPP_EN;

  //delayMicroseconds(VisDuration);

  //BLANK DISPLAY
  //PORTB &= ~(BOTP_EN | TOPP_EN);

  //SHIFT OUT TOP DATA
  //writeFrameBuffer(TOP);
  //SHOW TOP HALF
  //PORTB |= TOPP_EN;  PORTB &= ~BOTP_EN;
  //delayMicroseconds(VisDuration);
}

void writeFrameBuffer(char TopOrBot)
{
  unsigned char RegShadow;
  unsigned char Byte;
  if (TopOrBot == 1) //high byte, top
  {
    for (int col = 0; col < Width; col++)
    {
      Byte = (fbuffer[col] >> 8);
      for (long row = 0; row < 7; row++)
      {
        if (Byte & 0x1 == 0x1)
        {
          //Clock in a differential 1
          RegShadow = PORTD | DATAP_P | LATCHP_P;
          PORTD = RegShadow & ~(DLP_N);
        }
        else
        {
          //Clock in a differential 0
          RegShadow = PORTD | DLP_N;
          PORTD = RegShadow & ~(DATAP_P | LATCHP_P);
        }

        //Clock in data, but set and clear the differential
        //clock bits at the same time using a shadow reg
        RegShadow = PORTD | CLOCKP_P;
        PORTD = RegShadow & ~CLOCKP_N;

        //clear data lines back to zero (we don't really need this)
        //RegShadow = PORTD | DLP_N;
        //PORTD = RegShadow & ~(DATAP_P | LATCHP_P);

        //Reset Clock, but set and clear the differential
        //clock bits at the same time using a shadow reg
        RegShadow = PORTD | CLOCKP_N;
        PORTD = RegShadow & ~CLOCKP_P;

        Byte = Byte >> 1; //Get the next bit ready
      }
    }
  }
  else
  {
    for (int col = 0; col < Width; col++)
    {
      Byte = fbuffer[col];
      for (long row = 0; row < 7; row++)
      {
        if (Byte & 0x1 == 0x1)
        {
          //Clock in a differential 1
          RegShadow = PORTD | DATAP_P | LATCHP_P;
          PORTD = RegShadow & ~(DLP_N);
        }
        else
        {
          //Clock in a differential 0
          RegShadow = PORTD | DLP_N;
          PORTD = RegShadow & ~(DATAP_P | LATCHP_P);
        }

        //Clock in data, but set and clear the differential
        //clock bits at the same time using a shadow reg
        RegShadow = PORTD | CLOCKP_P;
        PORTD = RegShadow & ~CLOCKP_N;

        //clear data lines back to zero (we don't really need this)
        //RegShadow = PORTD | DLP_N;
        //PORTD = RegShadow & ~(DATAP_P | LATCHP_P);

        //Reset Clock, but set and clear the differential
        //clock bits at the same time using a shadow reg
        RegShadow = PORTD | CLOCKP_N;
        PORTD = RegShadow & ~CLOCKP_P;

        Byte = Byte >> 1; //Get the next bit ready
      }
    }
  }
}

//checks commented out for speed (but doesn't appear to matter much)
void clearPixel(int x, int y)
{
//if(x < Width && y < Height)
//  {
  if(y > 6) 
    y++;
  fbuffer[x] &= ~(1 << y);
//  }
}

bool getPixel(int x, int y)
{
//if(x < Width && y < Height)
//   {
   if( y > 6) //top bit isn't output (7 rows each half), we have to shift up
      y++; 
   if( (fbuffer[x] & (1<<y)) > 0) 
     return true;
   else 
     return false;
//   }
//return false;
}

void putPixel(int x, int y)
{
//if(x < Width && y < Height)
  //{
  if(y > 6) //top bit isn't output (7 rows each half), we have to shift up
     y++;
  fbuffer[x] |= (1 << y);
  //}
}

void writeWordBuffer(unsigned int Word, int col)
{
  if (col < Width)
    fbuffer[col] |= Word;
}

void writeByte(char Byte)
{
  unsigned char RegShadow;
  for (long row = 0; row < 7; row++)
  {
    if (Byte & 0x1 == 0x1)
    {
      //Clock in a differential 1
      RegShadow = PORTD | DATAP_P | LATCHP_P;
      PORTD = RegShadow & ~(DLP_N);
    }
    else
    {
      //Clock in a differential 0
      RegShadow = PORTD | DLP_N;
      PORTD = RegShadow & ~(DATAP_P | LATCHP_P);
    }

    //Clock in data, but set and clear the differential
    //clock bits at the same time using a shadow reg
    RegShadow = PORTD | CLOCKP_P;
    PORTD = RegShadow & ~CLOCKP_N;

    //clear data lines back to zero (we don't really need this)
    //RegShadow = PORTD | DLP_N;
    //PORTD = RegShadow & ~(DATAP_P | LATCHP_P);

    //Reset Clock, but set and clear the differential
    //clock bits at the same time using a shadow reg
    RegShadow = PORTD | CLOCKP_N;
    PORTD = RegShadow & ~CLOCKP_P;

    Byte = Byte >> 1; //Get the next bit ready
  }
}

void AdvanceGameOfLife()
{
  //commented code wraps around edges, but is way too slow on the arduino as-is. Maybe faster to not modulo??
char board[Width][Height];
  char total = 0;
  //int cellValue;
  for(int x = 1; x < Width-1; x++)
    {
    for(int y = 1; y < Height-1; y++)
      {
      total = 0;
      //cellValue = (fbuffer[x] & (1<<y)) >> y; current cell state is not actually used!
      //get cell neighbors by looping around the cell
      //left 3
      /*
      if(getPixel((x-1+Width)%Width, y) == true) total++;
      if(getPixel((x-1+Width)%Width, (y-1+Height)%Height) == true) total++;
      if(getPixel((x-1+Width)%Width, (y+1)%Height) == true) total++;
      */

      if(getPixel(x-1, y) == true) total++;
      if(getPixel(x-1, y-1) == true) total++;
      if(getPixel(x-1, y+1) == true) total++;

      //right 3
      /*
      if(getPixel((x+1)%Width, y) == true) total++;
      if(getPixel((x+1)%Width, (y-1+Height)%Height) == true) total++;
      if(getPixel((x+1)%Width, (y+1)%Height) == true) total++;
      */
      if(getPixel(x+1, y) == true) total++;
      if(getPixel(x+1, y-1) == true) total++;
      if(getPixel(x+1, y+1) == true) total++;

      //above
      //if(getPixel(x, (y+1)%Height) == true) total++;
      if(getPixel(x, y+1) == true) total++;
      //below
      //if(getPixel(x, (y-1+Height)%Height) == true) total++;
      if(getPixel(x, y-1) == true) total++;

      //1.STASIS : If, for a given cell, the number of on neighbours is 
      //exactly two, the cell maintains its status quo into the next 
      //generation. If the cell is on, it stays on, if it is off, it stays off.

      //2.GROWTH : If the number of on neighbours is exactly three, the cell 
      //will be on in the next generation. This is regardless of the cell's     current state.

      //3.DEATH : If the number of on neighbours is 0, 1, 4-8, the cell will 
      //be off in the next generation.
      
     //CONWAY
     #ifndef HIGHLIFE
      if(total == 2) //do nothing
         board[x][y] = getPixel(x,y);
      else if(total == 3) //live!
        board[x][y] = 1;
        //putPixel(x,y);
        
      else  //died to death
        //clearPixel(x,y);
        board[x][y] = 0;
      #endif

      //HIGHLIFE  
      #ifdef HIGHLIFE
      if(total == 2); //do nothing
         //board[x][y] = getPixel(x,y);
      else if(total == 6 || total == 3) //live!
        putPixel(x,y);
        //board[x][y] = 1;        
      else  //died to death
        clearPixel(x,y);
        //board[x][y] = 0;
      
      #endif
      }
    }
  //Output Board  
  
  for(int x = 1; x < Width-1; x++)
    {
    for(int y = 1; y < Height-1; y++)
      {
      if(board[x][y] == 1)
         putPixel(x,y);
      else
         clearPixel(x,y);
      }
    }
    
}
