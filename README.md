# ** Music Learning Project **

By: Jessica Baba, Manar Taha, Hadeel Hamodi.

The Music Learning Project is an engaging interactive piano learning game designed to teach users how to play the piano.  
Users have the option to select a song from a menu or even input their own compositions.  
This game incorporates eight touch sensors, with each sensor corresponding to one of the following notes: C4, D4, E4, F4, G4, A4, B4, and C5. 
Following each song, players receive detailed statistics, highlighting the number of incorrect notes played and any timing delays in their performance.  
## Game Instruction:
To get started, simply select a song of your choice by clicking on 'Choose Song'.Once you've made your selection, the lights will illuminate following these guidelines:  
        🟢 Green: Indicates the note you should play.   
        🔴 Red: Signals the next note to be played.   
        🔵 Blue: Highlights when the current note and the next note are identical. the light will turn pink for a short time indicating that you must lift your finger.
  
  ### Enjoy!
  
  
     
## Features List:  
* Reading eight touch sensors
* Increase/Decrease Volume
* Read WAV file from SD card
* Read note sheets from SD card
* Connection to Internet
* Telegram Bot
* Statistics


## Folder Description:  
* Documentation: Wiring diagram.
* sdcard_files: has WAV files and music sheets in sdcard_files/music_sheets/ directory.
* music_learning_code/code/: has the source code.


## Arduino/ESP32 libraries used:
* Audio.h
* SPI.h
* SD.h
* Adafruit_NeoPixel.h
* WiFi.h
* WiFiClientSecure.h
* UniversalTelegramBot.h

![pins](https://github.com/hmjproject/music-learning-project/assets/118805669/fcdfb49d-445b-4f38-883d-a486d6b061e2)


