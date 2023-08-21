# 1 "C:\\Users\\miral\\AppData\\Local\\Temp\\tmpro9fwtaa"
#include <Arduino.h>
# 1 "C:/Users/miral/Desktop/hmg_git/music-learning-project/sdcard2/src/I2S_audio_SD.ino"
#include "Arduino.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif


#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26


#define C_TOUCH_PIN 12
#define D_TOUCH_PIN 13

const int threshold = 20;


Audio audio;


unsigned long previousMillis = 0, previousMillis2 = 0;
bool pressed = true;



#define PIN 5
#define NUMPIXELS 3
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


void playTone(const char *file_name, int pixel);


String current_note_string;
int played;
int start = true;
int finished = false;
File current_file;
int current_pixel;
String note_file_name;
void setup();
void loop();
void playTone(const char *file_name,int pixel);
#line 53 "C:/Users/miral/Desktop/hmg_git/music-learning-project/sdcard2/src/I2S_audio_SD.ino"
void setup() {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  SD.begin(SD_CS);
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(12);

  audio.connecttoFS(SD, "123_u8.wav");

  audio.stopSong();

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif


  pixels.begin();
  pixels.clear();
  delay(1000);

}

void loop()
{

  audio.loop();
  unsigned long currentMillis = millis();

  if(start){

    current_file= SD.open("/music_sheets/song1.txt");

    current_note_string = current_file.readStringUntil('\n');


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

  }

  if(!finished){


    int C_touchValue = touchRead(C_TOUCH_PIN);
    int D_touchValue = touchRead(D_TOUCH_PIN);




    if (currentMillis - previousMillis > 2000 ) {
      previousMillis = currentMillis;
      pixels.setPixelColor(current_pixel, pixels.Color(0, 150, 0));
      pixels.show();





    }

    if(currentMillis - previousMillis2 > 200){

      previousMillis2 = currentMillis;

      if(C_touchValue < threshold){
        if(current_pixel==0){
          playTone("C_major.wav",0);
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

        }
        else{
          printf("wrong note\n");
        }
      }

      if(D_touchValue < threshold){

        if(current_pixel==1){
          playTone("D_major.wav",1);
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

        }
        else{
          printf("wrong note\n");
        }
      }
    }

    if (Serial.available()) {
      audio.stopSong();
      Serial.println("audio stopped");
      log_i("free heap=%i", ESP.getFreeHeap());
      Serial.flush();
    }
  }

}


void playTone(const char *file_name,int pixel){
  printf("giong to start playing music\n");

  pixels.setPixelColor(pixel, pixels.Color(0, 0, 0));
  pixels.show();
  audio.connecttoFS(SD,file_name);
  audio.loop();
}