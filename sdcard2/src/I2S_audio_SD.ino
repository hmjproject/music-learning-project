#include "Arduino.h" //required for PlatformIO
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

//touch
#define C_TOUCH_PIN     12
#define D_TOUCH_PIN     13
#define E_TOUCH_PIN     14
#define F_TOUCH_PIN     33
#define G_TOUCH_PIN     32
#define A_TOUCH_PIN     15
#define B_TOUCH_PIN     4

const int threshold = 35;

//audio
Audio audio;

//millis
unsigned long previousMillis = 0, previousMillis2 = 0;
bool pressed = true;


//neopixel
#define PIN 22
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//funcs
void playTone(const char *file_name, int pixel);
void check_touch_values();

//for files
String current_note_string;
int played = 0;
int start = true;
int finished = false;
File current_file;
int current_pixel = 0;
String note_file_name;

// long note
int note_length = 1200;

void setup() {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  SD.begin(SD_CS);
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21); // range 0...21 - This is not amplifier gain, but controlling the level of output amplitude. 

  audio.connecttoFS(SD, "123_u8.wav"); // a file with the proper name must be placed in root folder of SD card (formatted FAT32, file name max 8 chars no spaces)
  
  audio.stopSong();

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif
  // END of Trinket-specific code.

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();
  delay(1000);

}

void loop()
{
  
  audio.loop();
  unsigned long currentMillis = millis();
  // String prev_note = "G\r";
  if(start){
    //open file
    current_file= SD.open("/music_sheets/song2.txt");
    //update start
    start = false;
    played = 0;

  }

  if(!finished){

    if(currentMillis - previousMillis2 > 1250){
      //turn off previous pexil
      for(int i = 0; i < 8; i++){
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        pixels.show();
      }

      note_length = 1200;
      previousMillis2 = currentMillis; 

      //read next note
      current_note_string = current_file.readStringUntil('\n');
      // printf("prev note %s\n", prev_note);
      // if(prev_note == current_note_string){
      //   printf("got into if, prev note: %s, current note: %s\n",prev_note,current_note_string);
      //   pixels.setPixelColor(4, pixels.Color(100, 0, 0));
      //   pixels.show();
      //   delay(800);
      // }

      if(current_note_string == "C\r"){
        // printf("got c\n");
        current_pixel = 0;
      }
      else if(current_note_string == "D\r"){
        // printf("got D\n");
        current_pixel = 1;
      }
      else if(current_note_string == "E\r"){
        // printf("got E\n");
        current_pixel = 2;
      }
      else if(current_note_string == "F\r"){
        // printf("got F\n");
        current_pixel = 3;
      }
      else if(current_note_string == "G\r"){
        // printf("got G\n");
        current_pixel = 4;
      }
      else if(current_note_string == "A\r"){
        // printf("got A\n");
        current_pixel = 5;
      }
      else if(current_note_string == "B\r"){
        // printf("got B\n");
        current_pixel = 6;
      }
      else if(current_note_string == "LG\r"){
        // printf("got LG\n");
        note_length = 2400;
        current_pixel = 4;
      }
      else if(current_note_string == "LD\r"){
        // printf("got LD\n");
        note_length = 2400;
        current_pixel = 1;
      }
      else if(current_note_string == "NULL\r"){
        // printf("got null\n");
        for(int i = 0; i < 8; i++){
          pixels.setPixelColor(i, pixels.Color(0, 0, 0));
          pixels.show();
        }
      }
      else if(current_note_string == "END\r"){
        finished = true;
        for(int i = 0; i < 8; i++){
          pixels.setPixelColor(i, pixels.Color(0, 0, 0));
          pixels.show();
        }
      }
      if(current_note_string != "NULL\r" && current_note_string != "END\r")
      {
        printf("turn on current pixel \n");
        pixels.setPixelColor(current_pixel, pixels.Color(0, 200, 150));
        pixels.show();
      }
      played = 0;

    }

    if (currentMillis - previousMillis2 > 80 ) {
      check_touch_values();
    }

  
    if (Serial.available()) { // if any string is sent via serial port
      audio.stopSong();
      Serial.println("audio stopped");
      log_i("free heap=%i", ESP.getFreeHeap()); //available RAM
      Serial.flush();
    }

    // prev_note = current_note_string;

  }
  
}


void playTone(const char *file_name,int pixel){
  printf("giong to start playing music\n");
  // pressed = true;
  audio.connecttoFS(SD,file_name);
  audio.loop();
}

void check_touch_values(){

  int C_touchValue = touchRead(C_TOUCH_PIN);
  int D_touchValue = touchRead(D_TOUCH_PIN);
  int E_touchValue = touchRead(E_TOUCH_PIN);
  int F_touchValue = touchRead(F_TOUCH_PIN);
  int G_touchValue = touchRead(G_TOUCH_PIN);
  int A_touchValue = touchRead(A_TOUCH_PIN);
  int B_touchValue = touchRead(B_TOUCH_PIN);
  if(C_touchValue < threshold){
    if(current_pixel !=0){
      printf("C wrong note\n");
    }
    else if(!played){
      playTone("C_major.wav",1);
      played=1;
    }

  }

  if(D_touchValue < threshold){
    if(current_pixel !=1){
      printf("D wrong note\n");
    }
    else if(!played){
      if(current_note_string=="LG\r"){
        playTone("D_long_major.wav",1);
      }
      else{
        playTone("D_major.wav",1);
      }

      played=1;
    }
  }

  if(E_touchValue < threshold){
    if(current_pixel != 2){
      printf("E wrong note\n");
    }
    else if(!played){
      playTone("E_major.wav",1);
      played=1;
    }
  }
  if(F_touchValue < threshold){
    if(current_pixel != 3){
      printf("F wrong note\n");
    }
    else if(!played){
      playTone("F_major.wav",1);
      played=1;
    }
  }
  if(G_touchValue < threshold){
    if(current_pixel != 4){
      printf("G wrong note\n");
    }
    else if(!played){
      if(current_note_string=="LG\r"){
        playTone("G_long_major.wav",1);
      }
      else{
        playTone("G_major.wav",1);
      }

      played=1;
    }
  }
  if(A_touchValue < threshold){
    if(current_pixel !=5){
      printf("A wrong note\n");
    }
    else if(!played){
      playTone("A_major.wav",1);
      played=1;
    }
  }

  if(B_touchValue < threshold){
    if(current_pixel != 6){
      printf("B wrong note\n");
    }
    else if(!played){
      playTone("B_major.wav",1);
      played=1;
    }
  }

}