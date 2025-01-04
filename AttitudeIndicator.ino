#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library
//PIN is defined at: "C:\Users\LocalAdmin\Documents\Arduino\libraries\TFT_eSPI\User_Setup.h"
//Here is my round LCD pins
//BLK Backlight       3V3
//CS  ChipSelect      I022
//DC  DataCommand     I016
//RES Reset I016      I017 change from default
//SDA SPI MOSI(Data)  I023
//SCL SPI CLK(Clock)  I018
//VCC 3.3V Power3V3   3V3
//GND Ground          GND

// Refer https://github.com/Bodmer/TFT_eSPI
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
// Color is defined in TFT_eSPI.h
// TFT_SKYBLUE     0x867D 135, 206, 235 
// TFT_BROWN       0x9A60 150,  75,   0
// TFT_WHITE       0xFFFF 255, 255, 255

// Set parms
//#define WAITTIME 16 // delay in milliseconds
#define MAXX 240 // size of screen X
#define MAXY 240 // size of screen Y
#define DEG2RAD 0.0174532925

//#define HOR 172    // Horizon vector line length
int HLENGTH = MAXX + MAXY;  //Horizon line length. It is enough if LCD X+Y is used..  
// Declare
int CX = MAXX/2;
int CY = MAXY/2;
int PSW = MAXX/16;   //line length of Pitch Scale
int PSY  =MAXY/16;   //line position of Pitch Scale

int last_roll = 0;
int last_pitch = 0;
int last_px = CX;
int last_py = 5;
int newroll = 0;
int newpitch = 0;
bool stopped = true;
int p_size = 5;                 //Top Center indicator radial size
int p_x_last = CX;              //Top Center indicator x(Center of lcd) 
int p_y_last = p_size;           //Top Center indicator y( initial is size of indicator)

int g_color = TFT_JET;
int s_color = TFT_SKYBLUE;
int p_lastcolor = TFT_SKYBLUE;  //Saves color
int p_color =  TFT_WHITE;       //Top Center indicator color
int last_pcol = s_color;
//https://github.com/newdigate/rgb565_colors?tab=readme-ov-file


//serial communication string
String inString = "";  // string to hold input


// #################################################################
// Setup
void setup(void) {
  Serial.begin(9600);

  tft.begin();
  tft.setRotation(3);

  // Draw background
  tft.fillRect(0,  0, MAXX, CY, s_color);
  tft.fillRect(0, CY, MAXX, CY, g_color );
  // Draw the horizon graphic
  //tft.fillTriangle(0, 0, 200, 0, -100, 200, TFT_WHITE);
  drawHorizon(0, 0);
  drawplaneicon(); 
  drawInit();
  //delay(1000); // Wait to permit visual check
}


// ##################################################################
// Loop
void loop() {

  // Refresh the display at regular intervals
  int i;
  int j;
  int inChar;
  if (Serial.available() >0 ){
    inChar = Serial.read();
    // if you get a newline, process command.
    if (inChar == '\n') {
      Serial.println("Command recieved :" + inString);
      i=inString.indexOf(',');
      j=inString.indexOf(',',i+1);
      if( i>0 && j>0 ){
        Process( inString.substring(0,i),inString.substring(i+1,j),inString.substring(j+1) );
      } else if ( i>0 ){
        Process( inString.substring(0,i),inString.substring(i+1), "0" );
      } else {
        Serial.println("WARNING. Invalid command.");
      }
      inString = "";
    } else{
      inString += (char)inChar;
    }
  }
    updateHorizon();

  //delay(50);  //delay 50ms
}
void Process(String c, String r, String p ){
  c.toLowerCase();
  if(c=="move" ){
    newroll = r.toInt();
    newpitch = p.toInt();
    stopped = false;
    Serial.println("move to roll [" + c +"] pitch:[" + p +"]");
  } else if(c=="reset"){
    Serial.println("WARNING. Reset is not implemented yet.");
  } else {
    Serial.println("WARNING. Invalid parmeter.");
    return;
  }
}


void updateHorizon()
{
  if(stopped == true)
  {
    return;
  }

  if ((last_pitch == newpitch) && (last_roll == newroll))
  {
    stopped=true;
    return;
  }
  bool draw = 1;
  int delta_pitch = 0;
  int pitch_error = 0;
  int delta_roll  = 0;
    delta_pitch = 0;
    delta_roll  = 0;

    if (last_pitch < newpitch) {
      delta_pitch = 1;
      pitch_error = newpitch - last_pitch;
    }

    if (last_pitch > newpitch) {
      delta_pitch = -1;
      pitch_error = last_pitch - newpitch;
    }

    if (last_roll < newroll) delta_roll  = 1;
    if (last_roll > newroll) delta_roll  = -1;

    if (delta_roll == 0) {
      if (pitch_error > 1) delta_pitch *= 2;
    }

    drawHorizon(last_roll + delta_roll, last_pitch + delta_pitch);
    drawplaneicon();
}
// ###################################################################
// Draw horizon line.
void drawHorizon(int roll, int pitch)
{
  // Calculate coordinates for line start
  float sx = cos(roll * DEG2RAD);
  float sy = sin(roll * DEG2RAD);

  int16_t x0 = sx * HLENGTH;
  int16_t y0 = sy * HLENGTH;
  int16_t xd = 0;
  int16_t yd = 1;
  int16_t xdn  = 0;
  int16_t ydn = 0;

  if (roll > 45 && roll <  135) {
    xd = -1;
    yd =  0;
  }
  if (roll >=  135)             {
    xd =  0;
    yd = -1;
  }
  if (roll < -45 && roll > -135) {
    xd =  1;
    yd =  0;
  }
  if (roll <= -135)             {
    xd =  0;
    yd = -1;
  }

  if ((roll != last_roll) && ((abs(roll) > 35)  || (pitch != last_pitch)))
  {
    xdn = 5 * xd;
    ydn = 5 * yd;
    tft.drawLine(CX - x0 - xdn, CY - y0 - ydn - pitch, CX + x0 - xdn, CY + y0 - ydn - pitch, s_color);
    tft.drawLine(CX - x0 + xdn, CY - y0 + ydn - pitch, CX + x0 + xdn, CY + y0 + ydn - pitch, g_color);
    xdn = 4 * xd;
    ydn = 4 * yd;
    tft.drawLine(CX - x0 - xdn, CY - y0 - ydn - pitch, CX + x0 - xdn, CY + y0 - ydn - pitch, s_color);
    tft.drawLine(CX - x0 + xdn, CY - y0 + ydn - pitch, CX + x0 + xdn, CY + y0 + ydn - pitch, g_color);
  }
  xdn = 3 * xd;
  ydn = 3 * yd;
  tft.drawLine(CX - x0 - xdn, CY - y0 - ydn - pitch, CX + x0 - xdn, CY + y0 - ydn - pitch, s_color);
  tft.drawLine(CX - x0 + xdn, CY - y0 + ydn - pitch, CX + x0 + xdn, CY + y0 + ydn - pitch, g_color);
  xdn = 2 * xd;
  ydn = 2 * yd;
  tft.drawLine(CX - x0 - xdn, CY - y0 - ydn - pitch, CX + x0 - xdn, CY + y0 - ydn - pitch, s_color);
  tft.drawLine(CX - x0 + xdn, CY - y0 + ydn - pitch, CX + x0 + xdn, CY + y0 + ydn - pitch, g_color);

  tft.drawLine(CX - x0 - xd, CY - y0 - yd - pitch, CX + x0 - xd, CY + y0 - yd - pitch, s_color);
  tft.drawLine(CX - x0 + xd, CY - y0 + yd - pitch, CX + x0 + xd, CY + y0 + yd - pitch, g_color);

  // Draw horizon line
  tft.drawLine(CX - x0, CY - y0 - pitch,   CX + x0, CY + y0 - pitch,   TFT_WHITE);
  // Clear previous top center indicator  
  tft.fillEllipse(last_px, last_py, 10,10, last_pcol);
  last_px = 120+sy*115;
  last_py = 120-sx*115;
  //last_pcol = tft.readPixel(last_px, last_py);
  last_pcol = s_color;
  // Draw top center indicator
  tft.fillEllipse(last_px, last_py, 8, 8, TFT_WHITE);

  last_roll = roll;
  last_pitch = pitch;
drawcenterindicator(sx,sy);
}

void drawcenterindicator(int sx,int sy)
{
    //Draw top center indicator
  tft.fillEllipse(p_x_last, p_y_last, p_size, p_size, p_lastcolor);   // Restore pervious location with saved color. 
  p_x_last = CX+sy*115;
  p_y_last = CY-sx*115;
  p_lastcolor = tft.readPixel(p_x_last, p_y_last);          // Save color of top center indicator location 
  tft.fillEllipse(p_x_last, p_y_last, p_size, p_size, p_color);   // Draw top center indicator
}



// #########################################################################
// Draw the information
// #########################################################################

void drawplaneicon(void)
{
  // Draw airplace scale
  tft.fillEllipse(CX,CY, PSY*4, 4 , TFT_WHITE); //body
  tft.fillEllipse(CX,CY-10 ,4 ,16, TFT_WHITE);    //tail 
  tft.fillEllipse(CX+12,CY+6, 3 ,8, TFT_WHITE); //right landing gear
  tft.fillEllipse(CX-12,CY+6, 3 ,8, TFT_WHITE); //ledt landing gear
}

void drawInit(void)
{
  newpitch = 0;
  stopped = false;
  int myroll = 60;
  newroll = myroll;
  while(myroll != 0){
    updateHorizon();
    if(stopped){
      myroll = myroll * -0.7;
      newroll = myroll;
      stopped = false;
    }
 }
}



