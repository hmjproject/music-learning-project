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

const int threshold = 35;

//audio
Audio audio;

//millis
unsigned long previousMillis = 0, previousMillis2 = 0;
bool pressed = true;


//neopixel
#define PIN 4
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
    current_note_string = current_file.readStringUntil('\n');

    //continue to play
    //update start
    start = false;

    if(current_note_string == "C\r"){
      current_pixel = 0;
    }
    else if(current_note_string == "D\r"){
      current_pixel = 1;
    }
    else if(current_note_string == "NULL\r"){
      finished = true;
      printf("done playing \n");
    }
    played = 0;

  }

  if(!finished){

    //read touch val


    if (currentMillis - previousMillis2 > 200 ) {

      int C_touchValue = touchRead(C_TOUCH_PIN);
      int D_touchValue = touchRead(D_TOUCH_PIN);
      if(C_touchValue < threshold){
        if(current_pixel !=0){
          printf("wrong note\n");
        }
        else if(!played){
          playTone("C_major.wav",1);
          played=1;
        }

      }

      if(D_touchValue < threshold){
        if(current_pixel !=1){
          printf("wrong note\n");
        }
        else if(!played){
          playTone("D_major.wav",1);
          played=1;
        }
      }

    }

    if(currentMillis - previousMillis2 > 3000){

      previousMillis2 = currentMillis; 
      pixels.setPixelColor(current_pixel, pixels.Color(0, 0, 0));
      pixels.show();

      //read next note
      current_note_string = current_file.readStringUntil('\n');
      if(current_note_string == "C\r"){
        current_pixel = 0;
      }
      else if(current_note_string == "D\r"){
        current_pixel = 1;
      }
      else if(current_note_string == "NULL\r"){
        finished = true;
        printf("done playing \n");
      }
      pixels.setPixelColor(current_pixel, pixels.Color(0, 200, 150));
      pixels.show();
      played = 0;

    }
  
    if (Serial.available()) { // if any string is sent via serial port
      audio.stopSong();
      Serial.println("audio stopped");
      log_i("free heap=%i", ESP.getFreeHeap()); //available RAM
      Serial.flush();
    }
  }
  if(finished)
  {for(int i = 0; i < 2; i++){
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.show();
  }}
}


void playTone(const char *file_name,int pixel){
  printf("giong to start playing music\n");
  // pressed = true;
  audio.connecttoFS(SD,file_name);
  audio.loop();
}

