//**********************************************************************************************************
//* based on audioI2S-- I2S audiodecoder for ESP32, https://github.com/schreibfaul1/ESP32-audioI2S/wiki    *
//**********************************************************************************************************

#include "Arduino.h" //required for PlatformIO

#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26
#define TOUCH_PIN     12

const int threshold = 20;

Audio audio;

// void setup() {
//   pinMode(SD_CS, OUTPUT);
//   digitalWrite(SD_CS, HIGH);
//   SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
//   SPI.setFrequency(1000000);
//   Serial.begin(115200);
//   delay(1000);
//   SD.begin(SD_CS);
//    // a file with the proper name must be placed in root folder of SD card (formatted FAT32, file name max 8 chars no spaces)
  
//   //audio.setFileLoop(true); //this causes the file to play in an endless loop
// }

// void loop()
// {
//   //audio.loop();
//   int state = 1;
//   int touchValue = touchRead(TOUCH_PIN);
//   //Serial.print(touchValue);

//   if(touchValue<threshold){
//     // printf("in touchpad read\n");
//     if(!audio.isRunning()){
//       printf("not running \n");
//       // audio.setFileLoop(true);
//       audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
//       audio.setVolume(21); // range 0...21 - This is not amplifier gain, but controlling the level of output amplitude. 

//       audio.connecttoFS(SD, "123_u8.wav");
//       audio.loop();
//       printf("started playing sound\n");
//     }
//     else{
//       printf("running \n");
//       //audio.pauseResume();
//       // audio.setFileLoop(false);
//       audio.stopSong();
//       Serial.println(" - song  off");

//     }

//   }
 
//   // if (Serial.available()) { // if any string is sent via serial port
//   //   audio.stopSong();
//   //   Serial.println("audio stopped");
//   //   log_i("free heap=%i", ESP.getFreeHeap()); //available RAM
//   //   Serial.flush();
//   // }

//   delay(1000);
// }

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
  
  //audio.setFileLoop(true); //this causes the file to play in an endless loop
  audio.stopSong();
  delay(1000);
}

void loop()
{
  audio.loop();
  int touchValue = touchRead(TOUCH_PIN);

  if(touchValue<threshold){
    if(audio.isRunning()){
      printf("going to stop running audio\n");
      //audio.pauseResume();
      // audio.setFileLoop(false);
      audio.stopSong();
      Serial.println(" - song  off");
      delay(1000);
      printf("finished delay!\n");
    }
    else{
      printf("giong to start playing music\n");
      audio.connecttoFS(SD, "123_u8.wav"); // a file with the proper name must be placed in root folder of SD card (formatted FAT32, file name max 8 chars no spaces)
      //audio.setFileLoop(true); 
      audio.loop();
      delay(1000);
    }
  }
 
  if (Serial.available()) { // if any string is sent via serial port
    audio.stopSong();
    Serial.println("audio stopped");
    log_i("free heap=%i", ESP.getFreeHeap()); //available RAM
    Serial.flush();
  }
  //delay(1000);
  
}

