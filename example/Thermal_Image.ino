/*********************
 show Thermal image 63*47
 32*24=768 are real pixels
 32+31  24+23
 the 31*23 are virtual pixels.their value equal the average of around real pixels.
 ********************/
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h>     
#include <SPI.h>

#include <Wire.h>

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640
#define TA_SHIFT 8 //Default shift for MLX90640 in open air
float mlx90640To[768];    //32*24 real pixels
float mlx90640_virtual[63][47];   //all
paramsMLX90640 mlx90640;


#define TFT_CS 5 //chip select pin for the TFT screen
#define TFT_RST 33  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to 0!
#define TFT_DC 32
#define TFT_SCLK 15   // set these to be whatever pins you like!
#define TFT_MOSI 4 
  

 //low range of the sensor (this will be blue on the screen)
float MINTEMP = 10.0;    //first run,the lower temperature.In loop fuction,the limit will be set the largest tem value.  

//high range of the sensor (this will be red on the screen)
float MAXTEMP = 30.0;           //The temperature limit  at first.In loop fuction,the limit will be set the largest tem value.                         


//the colors we will be using
const uint16_t camColors[] = {0x480F,
0x400F,0x400F,0x400F,0x4010,0x3810,0x3810,0x3810,0x3810,0x3010,0x3010,
0x3010,0x2810,0x2810,0x2810,0x2810,0x2010,0x2010,0x2010,0x1810,0x1810,
0x1811,0x1811,0x1011,0x1011,0x1011,0x0811,0x0811,0x0811,0x0011,0x0011,
0x0011,0x0011,0x0011,0x0031,0x0031,0x0051,0x0072,0x0072,0x0092,0x00B2,
0x00B2,0x00D2,0x00F2,0x00F2,0x0112,0x0132,0x0152,0x0152,0x0172,0x0192,
0x0192,0x01B2,0x01D2,0x01F3,0x01F3,0x0213,0x0233,0x0253,0x0253,0x0273,
0x0293,0x02B3,0x02D3,0x02D3,0x02F3,0x0313,0x0333,0x0333,0x0353,0x0373,
0x0394,0x03B4,0x03D4,0x03D4,0x03F4,0x0414,0x0434,0x0454,0x0474,0x0474,
0x0494,0x04B4,0x04D4,0x04F4,0x0514,0x0534,0x0534,0x0554,0x0554,0x0574,
0x0574,0x0573,0x0573,0x0573,0x0572,0x0572,0x0572,0x0571,0x0591,0x0591,
0x0590,0x0590,0x058F,0x058F,0x058F,0x058E,0x05AE,0x05AE,0x05AD,0x05AD,
0x05AD,0x05AC,0x05AC,0x05AB,0x05CB,0x05CB,0x05CA,0x05CA,0x05CA,0x05C9,
0x05C9,0x05C8,0x05E8,0x05E8,0x05E7,0x05E7,0x05E6,0x05E6,0x05E6,0x05E5,
0x05E5,0x0604,0x0604,0x0604,0x0603,0x0603,0x0602,0x0602,0x0601,0x0621,
0x0621,0x0620,0x0620,0x0620,0x0620,0x0E20,0x0E20,0x0E40,0x1640,0x1640,
0x1E40,0x1E40,0x2640,0x2640,0x2E40,0x2E60,0x3660,0x3660,0x3E60,0x3E60,
0x3E60,0x4660,0x4660,0x4E60,0x4E80,0x5680,0x5680,0x5E80,0x5E80,0x6680,
0x6680,0x6E80,0x6EA0,0x76A0,0x76A0,0x7EA0,0x7EA0,0x86A0,0x86A0,0x8EA0,
0x8EC0,0x96C0,0x96C0,0x9EC0,0x9EC0,0xA6C0,0xAEC0,0xAEC0,0xB6E0,0xB6E0,
0xBEE0,0xBEE0,0xC6E0,0xC6E0,0xCEE0,0xCEE0,0xD6E0,0xD700,0xDF00,0xDEE0,
0xDEC0,0xDEA0,0xDE80,0xDE80,0xE660,0xE640,0xE620,0xE600,0xE5E0,0xE5C0,
0xE5A0,0xE580,0xE560,0xE540,0xE520,0xE500,0xE4E0,0xE4C0,0xE4A0,0xE480,
0xE460,0xEC40,0xEC20,0xEC00,0xEBE0,0xEBC0,0xEBA0,0xEB80,0xEB60,0xEB40,
0xEB20,0xEB00,0xEAE0,0xEAC0,0xEAA0,0xEA80,0xEA60,0xEA40,0xF220,0xF200,
0xF1E0,0xF1C0,0xF1A0,0xF180,0xF160,0xF140,0xF100,0xF0E0,0xF0C0,0xF0A0,
0xF080,0xF060,0xF040,0xF020,0xF800,};

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);  


uint16_t displayPixelWidth, displayPixelHeight;

void setup() {
      Wire.begin();
  Wire.setClock(400000); //Increase I2C clock speed to 400kHz

  Serial.begin(115200); //Fast serial as possible

  while (!Serial); //Wait for user to open terminal
  //Serial.println("MLX90640 IR Array Example");

  if (isConnected() == false)
  {
    Serial.println("MLX90640 not detected at default I2C addres. Please check wiring. Freezing.");
    while (1);
  }

  //Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0)
    Serial.println("Failed to load system parameters");

  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0)
    Serial.println("Parameter extraction failed");

  //Once params are extracted, we can release eeMLX90640 array

  //MLX90640_SetRefreshRate(MLX90640_address, 0x02); //Set rate to 2Hz
  MLX90640_SetRefreshRate(MLX90640_address, 0x04); //Set rate to 8Hz
  //MLX90640_SetRefreshRate(MLX90640_address, 0x05); //Set rate to 16Hz
  //MLX90640_SetRefreshRate(MLX90640_address, 0x07); //Set rate to 64Hz


    tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab
    tft.fillScreen(ST7735_BLACK);

    displayPixelWidth = tft.width() / 63;
    displayPixelHeight = tft.width() / 63; //Keep pixels square 

    tft.setRotation(0);

}

void loop() {
  
  //read all the pixels
  for (byte x = 0 ; x < 2 ; x++)
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    float emissivity = 0.95;

    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
  }
  mlx90640To[126] = mlx90640To[125];  //because the No.126 pixels is broken.The Melexis think if the sensor broken pixels'number below 4,it is qualified.You can output all data,check the broken point(the value is nan)
 
  for(int i = 0; i < 47; i++ ){     //the virtual pixels are created by the real pixels.
    for(int j = 0; j < 63; j++ ){

    int colorTemp;
    if(i % 2 == 0 && j % 2 == 0)
    {
      mlx90640_virtual[i][j] = mlx90640To[(i/2) * 32 + (j/2)];
    if( mlx90640To[(i/2) * 32 + (j/2)] > max_data ) {
      max_data = mlx90640To[(i/2) * 32 + (j/2)] ;
      max_num = i ; }
    if( mlx90640To[(i/2) * 32 + (j/2)] < min_data ) {
      min_data = mlx90640To[(i/2) * 32 + (j/2)] ;
      min_num = i ; }
    }
    else if(i % 2 == 0 && j % 2 != 0)
    mlx90640_virtual[i][j] = ( mlx90640To[(i/2) * 32 + (j/2)] + mlx90640To[(i/2) * 32 + (j/2)] + 1) / 2;  //virtual pixels are created by 2 pixels.
    else if(i % 2 != 0 && j % 2 == 0)
    mlx90640_virtual[i][j] = ( mlx90640To[(i/2) * 32 + (j/2)] + mlx90640To[(i/2 + 1) * 32 + (j/2)]) / 2; //virtual pixels are created by 2 pixels.
    else 
    mlx90640_virtual[i][j] = ( mlx90640To[(i/2-1) * 32 + (j/2+1)] + mlx90640To[(i/2-1) * 32 + (j/2-1)]+ mlx90640To[(i/2+1) * 32 + (j/2-1)]+ mlx90640To[(i/2+1) * 32 + (j/2+1)]) / 4;
     //virtual pixels are created by 4 pixels.
    if(mlx90640_virtual[i][j] >= MAXTEMP) colorTemp = MAXTEMP;
    else if(mlx90640_virtual[i][j] <= MINTEMP) colorTemp = MINTEMP;
    else colorTemp = mlx90640_virtual[i][j];
    
    uint8_t colorIndex = map(colorTemp, MINTEMP, MAXTEMP, 0, 255);
    
    colorIndex = constrain(colorIndex, 0, 255);
    //draw the pixels!
    tft.fillRect(displayPixelHeight * i,displayPixelWidth * j,displayPixelHeight, displayPixelWidth, camColors[colorIndex]);
    }    
  } 
    MAXTEMP = max_data;  //self-adaption.
  MINTEMP = min_data;
}
//Returns true if the MLX90640 is detected on the I2C bus
boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}
