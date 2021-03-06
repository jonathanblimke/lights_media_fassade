/*
   ESP32 FastLED WebServer: https://github.com/jasoncoon/esp32-fastled-webserver
   Copyright (C) 2017 Jason Coon

   Built upon the amazing FastLED work of Daniel Garcia and Mark Kriegsman:
   https://github.com/FastLED/FastLED

   ESP32 support provided by the hard work of Sam Guyer:
   https://github.com/samguyer/FastLED

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


String getPower() {
  return String(power);
}

String setPower(String value) {
  power = value.toInt();
  power = power == 0 ? 0 : 1;
  return String(power);
}

String getDisplaybitmap() {
  return String(displayframes);
}

String setDisplaybitmap(String value) {
  displayframes = value.toInt();
  displayframes = displayframes == 0 ? 0 : 1;
  return String(displayframes);
}

String getDisplaytext() {
  return String(power);
}

String setDisplaytext(String value) {
  displayScrollText = value.toInt();
  displayScrollText = displayScrollText == 0 ? 0 : 1;
  return String(displayScrollText);
}

String getBrightness() {
  return String(brightness);
}

String setBrightness(String value) {
  brightness = value.toInt();
  FastLED.setBrightness(brightness);
  return String(brightness);
}

String getText() {
  return String(serverstring);
}

String setText(String value) {
  String result ="\"";
  result += value;
  result += "\"";
  serverstring = result;
  return serverstring;
}

String decToHex(int dec_value) {
  std::stringstream strs;
  strs << std::hex << dec_value;
  std::string hexstr(strs.str());
  // Serial.println(hexstr.c_str());
  
  return hexstr.c_str();
}

//converts an array of integers into an JSON conform string
String arrayToString(std::vector<uint16_t> frames) {
  String returnvalue; 
  returnvalue +="[";
  std::vector<uint16_t>::iterator iter;  
  int i = 0;  //counter

  for(iter = frames.begin(); iter != frames.end(); iter++,i++ ) {
    returnvalue += *iter;
    if( i < frames.size()-1) {
      returnvalue += ",";
    }
  }
    returnvalue +="]";
    return returnvalue;
}

void clearValueBuffer() {
 for(int i = 0; i < 10000; i++){
   valueBuffer[i] = 0;
 } 
}

void clearFrameData() {
 std::vector<uint16_t>::iterator iter;  
  int i = 0;  //data counter

  for(iter = frameData.begin(); iter != frameData.end(); ++iter,i++ ) {
    frameData.erase(iter); //last change
  }
}

void stringToArray(String str) {   
  //  clearFrameData();
   frameData.clear(); 
   std::stringstream sstr(str.c_str());
   std::string token;
   size_t size = 0;
   
  // frameData_mutex.lock();
   while(std::getline(sstr, token, ',')) {
    //  Serial.print(token.c_str());
     std::stringstream ss(token.c_str());
     uint16_t intvalue;
     ss >> intvalue;
    //  Serial.print(intvalue, HEX);
    // Serial.print("Free Heapsize in bytes: ");
    // Serial.println(xPortGetFreeHeapSize());
    //  frameData_mutex.lock();
     frameData.push_back(intvalue);
    //  frameData_mutex.unlock();
  
   } 
  //  frameData_mutex.unlock(); 
    // printTHATArray(frameData);  
      
} 

void stringToHexArray(String str) {   
  //  clearFrameData();
   frameData.clear(); 
   std::stringstream sstr(str.c_str());
   std::string token;
   size_t size = 0;
   

   while(std::getline(sstr, token, ',')) {
    //  Serial.print(token.c_str());
     std::stringstream ss(token.c_str());
     uint16_t intvalue;
     ss >> std::hex >> intvalue;

    //  std::stringstream strs;
    //  strs << std::hex << intvalue;

    // Serial.print(intvalue);
    // frameData_mutex.try_lock();
    frameData.push_back(intvalue);
    // frameData_mutex.unlock();
  
   }  
    // printTHATArray(frameData);  
} 

String getBitmapArray() {
  return arrayToString(frameData);
  
}

String setBitmapArray(String value) {
 stringToArray(value);
 return arrayToString(frameData); 
}

String getBitmapHexArray() {
  // frameData_mutex.try_lock();
  return arrayToString(frameData);
  // frameData_mutex.unlock();
}

String setBitmapHexArray(String value) {
  // breaktrigger = 1;
  stringToHexArray(value);
  //  frameData_mutex.try_lock();
  // displayFrames(); 
 return arrayToString(frameData); 
  //  frameData_mutex.unlock();
}

String getFps() {
  return (String)fps;
}

String setFps(String value) {
  fps = value.toInt();
  return String(fps);
}

uint16_t RGB888toRGB565(const char *rgb32_str_) {
 long rgb32=strtoul(rgb32_str_, 0, 16);
 return (rgb32>>8&0xf800)|(rgb32>>5&0x07e0)|(rgb32>>3&0x001f);
}

inline byte hexToNibble(char hex) {
 if (hex & _BV(6)){
   hex += 9;
 }
 return hex;
}

//like the other method with similar name only a lot faster and smaller
uint16_t RGB888toRGB565_2(const char *rgb32_str_) {
 typedef union {
   uint16_t integer;
   struct{
     uint8_t low;
     uint8_t high;
     };
  } Byter;
 
 byte red;
 byte green;
 Byter rgb16;
                         
 green = hexToNibble(rgb32_str_[2])<<4;
 green |= hexToNibble(rgb32_str_[3])&0xC;
 rgb16.low = hexToNibble(rgb32_str_[4])<<4;
 rgb16.low |= hexToNibble(rgb32_str_[5])&0x8;
 rgb16.high = hexToNibble(rgb32_str_[0])<<4;
 rgb16.high |= hexToNibble(rgb32_str_[1])&0x8;
 rgb16.low >>= 3;
 rgb16.integer |= (green << 3);
 
 return rgb16.integer;
}


String bitmapDataToString(std::vector<uint32_t> frame) {
  String returnValue; //return String in JSON array syntax
  returnValue +="[";
  std::vector<uint32_t>::iterator itera;  
  int ii = 0;  //counter

  for(itera = frame.begin(); itera != frame.end(); itera++,ii++ ) {
    // Serial.println("   itera: ");
    // Serial.print(*itera);
    returnValue += *itera;
    if( ii < frame.size()-1) {
      returnValue += ",";
    }
  }
    returnValue +="]";
    return returnValue;
}

String getBitmapPixelData() { 
  return bitmapDataToString(bitmapData);
}

void convertVectorFromUint32toUint16(std::vector<uint32_t> vector) {
  std::vector<uint32_t>::iterator iterat;  
  // bitmapData16.clear(); //set to save mode
  frameData.clear();
  for(iterat = vector.begin(); iterat != vector.end(); iterat++ ) {
    // Serial.print(*iterat);
    // Serial.print(", ");
    frameData.push_back(*iterat); //used to be pushed into bitmapData16
    // Serial.print(*iterat);
    // Serial.print(,);
  }
}

String setBitmapPixelData(String value) {
   bitmapData.clear(); 
   std::stringstream sstr(value.c_str());
   std::string token;

   while(std::getline(sstr, token, ',')) {
     std::stringstream ss(token.c_str());
     uint16_t rgb565 = RGB888toRGB565_2(token.c_str());
     bitmapData.push_back(rgb565);
   }  
   convertVectorFromUint32toUint16(bitmapData);
  return bitmapDataToString(bitmapData);
}

String getBitmapButton(){
   return String(bitmapButton);
}

String setBitmapButton(String value) {
  bitmapButton = value.toInt();
  bitmapButton = bitmapButton == 0 ? 0 : 1;
  return String(bitmapButton);
}

FieldList fields = {
  { "power", "Power", BooleanFieldType, 0, 1, getPower, NULL, setPower },
  { "brightness", "Brightness", NumberFieldType, 1, 255, getBrightness, NULL, setBrightness },
  { "displaytext", "Display Text", BooleanFieldType, 0, 1, getDisplaytext, NULL, setDisplaytext },
  { "text", "Text", TextFieldType, 0, 1, getText, NULL, setText },
  { "rgb565_DEC", "Matrixdata", ArrayFieldType, 0, 1, getBitmapArray, NULL, setBitmapArray},
  { "displaybitmap", "Display Bitmap", BooleanFieldType, 0, 1, getDisplaybitmap, NULL, setDisplaybitmap },
  { "fps", "Frames per Second", NumberFieldType, 1, 60, getFps, NULL, setFps },
  { "rgb888A_HEX", "Bitmap Data", ArrayFieldType, 0, 1, getBitmapPixelData, NULL, setBitmapPixelData }
};

uint8_t fieldCount = ARRAY_SIZE(fields);
