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

//for files
String current_note_string;
int played = 0;
int start = true;
int finished = false;
File current_file;
int current_pixel = 0;
String note_file_name;


void setup() {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  SD.begin(SD_CS);
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(12); // range 0...21 - This is not amplifier gain, but controlling the level of output amplitude. 

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

  if(start){
    //open file
    current_file= SD.open("/music_sheets/song1.txt");
    //get first note

    //continue to play
    //update start
    start = false;
    played = 0;

  }

  if(!finished){

    if(currentMillis - previousMillis2 > 3000){

      previousMillis2 = currentMillis; 
      pixels.setPixelColor(current_pixel, pixels.Color(0, 0, 0));
      pixels.show();

      //read next note
      current_note_string = current_file.readStringUntil('\n');
      if(current_note_string == "C\r"){
        printf("got c\n");
        current_pixel = 0;
      }
      else if(current_note_string == "D\r"){
        printf("got D\n");

        current_pixel = 1;
      }
      else if(current_note_string == "E\r"){
        printf("got E\n");

        current_pixel = 2;
      }
      else if(current_note_string == "F\r"){
        printf("got F\n");

        current_pixel = 3;
      }
      else if(current_note_string == "G\r"){
        printf("got G\n");

        current_pixel = 4;
      }
      else if(current_note_string == "A\r"){
        printf("got A\n");

        current_pixel = 5;
      }
      else if(current_note_string == "B\r"){
        printf("got B\n");

        current_pixel = 6;
      }
      else if(current_note_string == "NULL\r"){
        printf("got null\n");
      }
      else if(current_note_string == "END\r"){
        finished = true;
      }
      pixels.setPixelColor(current_pixel, pixels.Color(0, 200, 150));
      pixels.show();
      played = 0;

    }

    if (currentMillis - previousMillis2 > 200 ) {

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
          playTone("D_major.wav",1);
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
          playTone("G_major.wav",1);
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

  
    if (Serial.available()) { // if any string is sent via serial port
      audio.stopSong();
      Serial.println("audio stopped");
      log_i("free heap=%i", ESP.getFreeHeap()); //available RAM
      Serial.flush();
    }
  }
  if(finished)
  {
    for(int i = 0; i < 2; i++){
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show();
    }
  }
}


void playTone(const char *file_name,int pixel){
  printf("giong to start playing music\n");
  // pressed = true;
  audio.connecttoFS(SD,file_name);
  audio.loop();
}

