/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0
// -- The core to run FastLED.show()
#define FASTLED_SHOW_CORE 0


#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <EEPROM.h>
#include <FastLED_NeoMatrix.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <iterator>
#include <mutex>

AsyncWebServer webServer(80);
WebSocketsServer webSocketsServer = WebSocketsServer(81);

const int led = 32; //Boards led
const int LED_BUILTIN = 2;

uint8_t power = 1;
uint8_t displayframes = 0;
uint8_t displayScrollText = 0;
uint8_t bitmapButton = 0;
ulong fps = 5;
uint8_t breaktrigger = 0;
String serverstring = "";
uint8_t brightness = 150;
uint16_t bitmap[30] = {
  0xFFFF, 0x000F, 0x00F0, 0x0F00, 0xF000, 0xAB10,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0x000F,  
};
uint16_t valueBuffer[10000];

uint16_t bitmap36[36] = { //some warm orange
  0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 
  0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3,
  0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 
  0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3,
  0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 
  0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3, 0xBCE3
};

uint16_t bitmapBLACK[36] = { //all black
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 
};

String array = "";

uint16_t bitmapNframes[60] = {
  0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x003C,  
};
std::vector<uint16_t> singleFrame = {
  0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};  

std::vector <uint16_t> frameData  = { 
  0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,//Frame2
  0x0000, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,//Frame3        
};
std::mutex frameData_mutex;

uint16_t* vectorTest = &frameData[0];

std::vector<uint32_t> bitmapData = { 
  0xFFFFFF, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,  
  0xFFFFFF, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,    
  0xFFFFFF, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,  
  0xFFFFFF, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,  
  0xFFFFFF, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,  
 };

std::vector<uint16_t> bitmapData16 = {
  0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#define DATA_PIN 26 
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB //GRB
#define NUM_STRIPS 1
#define NUM_LEDS_PER_STRIP 36 //32
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS
#define MILLI_AMPS 4000 //IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
#define FRAMES_PER_SECOND 3 //
#define MATRIX_TILE_WIDTH   6 // width of EACH NEOPIXEL MATRIX (not total display)
#define MATRIX_TILE_HEIGHT  6 // height of each matrix
#define MATRIX_TILE_H       1  // number of matrices arranged horizontally
#define MATRIX_TILE_V       1  // number of matrices arranged vertically
#define mw (MATRIX_TILE_WIDTH *  MATRIX_TILE_H)
#define mh (MATRIX_TILE_HEIGHT * MATRIX_TILE_V)
#define NUMMATRIX (mw*mh)
#define LED_GREEN_HIGH 		(63 << 5)
#define LED_BLACK		0

CRGB matrixleds[NUMMATRIX];
/*
   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
    Position of the FIRST LED in the FIRST MATRIX; pick two, e.g.
    NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.

  NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs WITHIN EACH MATRIX are
    arranged in horizontal rows or in vertical columns, respectively;
    pick one or the other.

  NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns WITHIN
    EACH MATRIX proceed in the same order, or alternate lines reverse
    direction; pick one.

  NEO_TILE_TOP, NEO_TILE_BOTTOM, NEO_TILE_LEFT, NEO_TILE_RIGHT:
    Position of the FIRST MATRIX (tile) in the OVERALL DISPLAY; pick
    two, e.g. NEO_TILE_TOP + NEO_TILE_LEFT for the top-left corner.

  NEO_TILE_ROWS, NEO_TILE_COLUMNS: the matrices in the OVERALL DISPLAY
    are arranged in horizontal rows or in vertical columns, respectively;
    pick one or the other.

  NEO_TILE_PROGRESSIVE, NEO_TILE_ZIGZAG: the ROWS/COLUMS OF MATRICES
    (tiles) in the OVERALL DISPLAY proceed in the same order for every
    line, or alternate lines reverse direction; pick one.  When using
    zig-zag order, the orientation of the matrices in alternate rows
    will be rotated 180 degrees (this is normal -- simplifies wiring).
    */
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(matrixleds, MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT,
MATRIX_TILE_H, MATRIX_TILE_V,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG +
    NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE);

#include "field.h"
#include "fields.h"
#include "secrets.h"
#include "wifi_local.h"
#include "web.h"

// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;

void makeFileonSpiffs() {
  
   File file = SPIFFS.open("/test.txt", FILE_WRITE);
   
    if (!file) {
      Serial.println("There was an error opening the file for writing");
      return;
    }
    if (file.print("TEST")) {
      Serial.println("File was written");
    } else {
      Serial.println("File write failed");
    }
}

/** show() for ESP32
    Call this function instead of FastLED.show(). It signals core 0 to issue a show,
    then waits for a notification that it is done.
*/
void FastLEDshowESP32() {
  if (userTaskHandle == 0) {
    // -- Store the handle of the current task, so that the show task can
    //    notify it when it's done
    userTaskHandle = xTaskGetCurrentTaskHandle();

    // -- Trigger the show task
    xTaskNotifyGive(FastLEDshowTaskHandle);

    // -- Wait to be notified that it's done
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200);
    ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    userTaskHandle = 0;
  }
}

/** show Task
    This function runs on core 0 and just waits for requests to call FastLED.show()
*/
void FastLEDshowTask(void *pvParameters) {
  // -- Run forever...
  for (;;) {
    // -- Wait for the trigger
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // -- Do the show (synchronously)
    FastLED.show();

    // -- Notify the calling task
    xTaskNotifyGive(userTaskHandle);
  }
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

//displays text which scrolls through screen
void display_scrollText(String text) {

    //calculates font size according to mw(matrix_width)
    uint8_t size = max(int(mw/8), 1);
    matrix->clear();
    matrix->setTextWrap(false);  //stops text from standing still
    matrix->setTextSize(size);
    matrix->setRotation(0);  //2 +90 degrees;3 -90 degres; 0 default;1 -180 degrees
    int8_t length = serverstring.length();
    // Serial.println(length);
    int16_t looplength = length * mw;
    // Serial.println(looplength);
    for (int8_t x=0; x>=-looplength; x--) {
        //used to be: x=7, x>=-42, x--
	    yield();
	    matrix->clear();
	    matrix->setCursor(x,0);
	    matrix->setTextColor(LED_GREEN_HIGH);
	    matrix->print(text);
	    FastLEDshowESP32();
      delay(100); //controls tim intervall between iteration's (how fast text scrolls)
    }
}

//generates a well formed bitmaps for Adafruits GFX Backend Konversion von BGR 4/4/4 bit Pixelmap zu ein er 5/6/5 RGB Bitmap (https://github.com/marcmerlin/FastLED_NeoMatrix/blob/master/examples/MatrixGFXDemo/MatrixGFXDemo.ino) line 325
void fixdrawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h) {

    uint16_t RGB_bmp_fixed[w * h];
    for (uint16_t pixel=0; pixel<w*h; pixel++) {
	    uint8_t r,g,b;
	    uint16_t color = pgm_read_word(bitmap + pixel);

	    r = (color & 0xF00) >> 8; //b
	    b = (color & 0x0F0) >> 4; //g
	    g = color & 0x00F;        //r

	    //expand from 4/4/4 bits per color to 5/6/5
    	b = map(b, 0, 15, 0, 31);
    	g = map(g, 0, 15, 0, 63);
    	r = map(r, 0, 15, 0, 31);

	    RGB_bmp_fixed[pixel] = (r << 11) + (g << 5) + b;

    }
    matrix->drawRGBBitmap(x, y, RGB_bmp_fixed, w, h);
    matrix->show();
}

//displays Bitmaps, after colors and bits per color has been corrected
void display_rgbBitmap(uint16_t* bitmapinput) { 
    static uint16_t bmx,bmy;
    //clears all Leds could also try matrix_clear();
    matrix->fillRect(bmx,bmy, bmx+8,bmy+8, LED_BLACK);
    fixdrawRGBBitmap(0, 0, bitmapinput, mw, mh);
}

void matrix_clear() {
    // FastLED.clear does not work properly with multiple matrices connected via parallel inputs
    memset(matrixleds, 0, NUMMATRIX*3);
}

void allLedsBlack(){
  
  matrix_clear();
  display_rgbBitmap(bitmapBLACK);
  delay(500);
}

void displayFrames() {

  singleFrame.clear();  

  std::vector<uint16_t>::iterator it;  

  int i = 0;  //data counter
  
  for(it = frameData.begin(); it != frameData.end(); it++,i++ ) {

    if((i % NUMMATRIX)==0) { 
      for(int z=0;z<NUMMATRIX;z++) {   
        singleFrame.push_back(frameData[i+z]);
      } 
      matrix->clear(); 
      matrix->drawRGBBitmap(0, 0, &singleFrame[0], mw, mh);
      singleFrame.clear();
      FastLED.delay(1000 / fps);
      }
    }
}
//test method to get independet from global frameData vector, but does basicly the same as displayFrames
void displayInput(std::vector<uint16_t> vector) {

  singleFrame.clear();  

  std::vector<uint16_t>::iterator it;  
  std::vector<uint16_t>::iterator it2;  
  int i = 0;  //data counter
  int f = 0;  //frame counter

  for(it = vector.begin(); it != vector.end(); it++,i++ ) {
    if((i % NUMMATRIX)==0) { 
      f++;
      for(int z=0;z<NUMMATRIX;z++) {   
        // frameData_mutex.try_lock();  
        singleFrame.push_back(vector[i+z]);

      } 
      
      matrix->clear(); 
      matrix->drawRGBBitmap(0, 0, &singleFrame[0], mw, mh);
      singleFrame.clear();
      FastLED.delay(1000 / fps);
      }
    }
}

void setup() {
  delay(5000);
  pinMode(led, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(led, 1);

  Serial.begin(9600);

  SPIFFS.begin(); 
  listDir(SPIFFS, "/", 1); //lists SPIFF directory in Serial Monitor String parameter defines path

  // restore from memory
  loadFieldsFromEEPROM(fields, fieldCount); 
  
  setupWifi();
  setupWeb();

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(matrixleds, NUMMATRIX).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  FastLED.setBrightness(brightness);

  Serial.print("Matrix Size: ");
  Serial.print(mw);
  Serial.print(" ");
  Serial.println(mh);

  matrix->begin();
  matrix->setTextWrap(false);

  int core = xPortGetCoreID();
  Serial.print("Main code running on core ");
  Serial.println(core);

  // -- Create the FastLED show task
  xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);


}

void loop() {

  handleWeb();
  // Serial.print("Free Heapsize in bytes: ");
  // Serial.println(xPortGetFreeHeapSize());

  if (power == 0) {
     allLedsBlack();
  }
  else {
    // if(displayframes==1) {
      displayFrames();
    // }
    // if(displayScrollText==1) {
      // display_scrollText(serverstring);
    // }
  }

}
