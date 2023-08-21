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

const int threshold = 20;

//audio
Audio audio;

//millis
unsigned long previousMillis = 0, previousMillis2 = 0;
bool pressed = true;


//neopixel
#define PIN 5
#define NUMPIXELS 3
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


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
  //read touch val
  int C_touchValue = touchRead(C_TOUCH_PIN);
  int D_touchValue = touchRead(D_TOUCH_PIN);
  //


  if (currentMillis - previousMillis > 2000 ) {
    previousMillis = currentMillis; 
    pixels.setPixelColor(0, pixels.Color(0, 150, 0));
    pixels.show();

    previousMillis = currentMillis; 
    pixels.setPixelColor(1, pixels.Color(150, 0, 0));
    pixels.show();
    // pressed = false;
  }

  if(currentMillis - previousMillis2 > 100){

    previousMillis2 = currentMillis; 

    if(C_touchValue < threshold){
      //audio isn't running -> play audio
      printf("giong to start playing music\n");
      // pressed = true;
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      pixels.show();
      audio.connecttoFS(SD, "C_major.wav"); // a file with the proper name must be placed in root folder of SD card (formatted FAT32, file name max 8 chars no spaces)
      audio.loop();
    }

    if(D_touchValue < threshold){

      //audio isn't running -> play audio
      printf("giong to start playing music\n");
      // pressed = true;
      pixels.setPixelColor(1, pixels.Color(0, 0, 0));
      pixels.show();
      audio.connecttoFS(SD, "D_major.wav"); // a file with the proper name must be placed in root folder of SD card (formatted FAT32, file name max 8 chars no spaces)
      audio.loop();

    }
  }
 
  if (Serial.available()) { // if any string is sent via serial port
    audio.stopSong();
    Serial.println("audio stopped");
    log_i("free heap=%i", ESP.getFreeHeap()); //available RAM
    Serial.flush();
  }
  
}


// void playTone()
// 
