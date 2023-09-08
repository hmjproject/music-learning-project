#include "Arduino.h" //required for PlatformIO
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

//Telegram
// network credentials
const char* ssid = "CS_conference";
const char* password = "openday23";
// const char* ssid = "baba";
// const char* password = "0502214066";

//Telegram BOT
#define BOTtoken "6382002255:AAFPCttqq1v4URJGQbHBJ9fzRpcZedvYxaw"
#define CHAT_ID "1019453346"
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;


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

//touch threshold
const int threshold = 35;

//touch sensors array
bool touch_sensor_val[7] = {false,false,false,false,false,false,false};

//audio
Audio audio;

//millis
unsigned long touch_sensor_millis = 0, touch_sensor_millis_1 = 0, note_read_millis = 0;
bool pressed = true;


//neopixel
#define PIN 22
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//funcs
void playTone(const char *file_name, int pixel);
void check_touch_values();
void turn_off_lights();
void play_music();
void reconnect_to_wifi();
void turn_off_lights();
void turn_lights_red();
void turn_lights_green();

//for files
String current_note_string;
int current_note_played = 0;
bool note_played_in_epsilon_time = false;
int start = true;
int finished = false;
File current_file;
int current_pixel = 0;
String note_file_name;
int playing_music = false;
String file_name = "";

// long note
bool long_note = false;

//volume
int volume = 18;

//statistics
int wrong_notes = 0;
int delayed_notes = 0;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "what would you like to do?.\n\n";
      String keyboardJson = "[[\"play music\" ,\"settings\" ]]";
      bot.sendMessageWithReplyKeyboard(chat_id, welcome, "", keyboardJson, true); 
    }

    if (text == "settings") {
      String print_text = "which settings would like to change?.\n";
      String keyboardJson = "[[\"volume\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, print_text, "", keyboardJson, true); 
    }

    if (text == "play music") {
      String print_text = "Which song would you like to play?.\n";
      String keyboardJson = "[[\"song1\",\"song2\" ,\"song3\"],[ \"go back\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, print_text, "", keyboardJson, true); 
    }

    
    if(text == "song1"){
      playing_music = true;
      bot.sendMessage(chat_id, "going to play music!", "");
      file_name = "/music_sheets/song1.txt";
      start = true;
      finished = false;
    }

    if(text == "song2"){
      playing_music = true;
      bot.sendMessage(chat_id, "going to play music!", "");
      file_name = "/music_sheets/song2.txt";
      start = true;
      finished = false;
    }

    if(text == "song3"){
      playing_music = true;
      bot.sendMessage(chat_id, "going to play music!", "");
      file_name = "/music_sheets/oldMac.txt";
      start = true;
      finished = false;
    }

    if(text == "volume"){
      String print_text = "would like to increase or decrease volume?.\n";
      String keyboardJson = "[[\"increase volume\",\"decrease volume\"],[ \"go back\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, print_text, "", keyboardJson, true); 
    }

    if(text == "increase volume"){
      if(volume < 21){
        volume++;
        audio.setVolume(volume);
        bot.sendMessage(chat_id, "Increased volume", "");
      }
      else{
        bot.sendMessage(chat_id, "Can't increse the volume anymore :(", "");
      }
    }

    if(text == "decrease volume"){
      if(volume > 0){
        volume--;
        audio.setVolume(volume);
        bot.sendMessage(chat_id, "Decreased volume", "");
      }
      else{
        bot.sendMessage(chat_id, "Can't decrease the volume anymore :(", "");
      }
    }

    if(text == "go back"){
      String welcome = "What would you like to do?\n";
      String keyboardJson = "[[\"play music\" ,\"settings\" ]]";
      bot.sendMessageWithReplyKeyboard(chat_id, welcome, "", keyboardJson, true); 
    }

  }
}


void setup() {

  // ------------------- sd pin setup --------------------
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  //-------------------- spi setup --------------------
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);

  //------------------- serial --------------------
  Serial.begin(115200);


  SD.begin(SD_CS);

  //--------------- audio ------------------------
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(volume); // range 0...21 - This is not amplifier gain, but controlling the level of output amplitude. 
  // audio.connecttoFS(SD, "123_u8.wav"); // a file with the proper name must be placed in root folder of SD card (formatted FAT32, file name max 8 chars no spaces)
  // audio.stopSong();

  //----------------- Connect to Wi-Fi ----------------------
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  //----------------------- initialize neo-pexils -------------------------
  pixels.begin(); 
  pixels.clear();

}


void loop()
{
  audio.loop();
  

  if(!playing_music){
    for(int i = 0; i < 8; i++){
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show();
    }
    
    if (millis() > lastTimeBotRan + botRequestDelay)  {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      printf("num of new messages is %d\n",numNewMessages);
      if(numNewMessages) {
        Serial.println("got response");
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = millis();
    }
  }
  if(playing_music){
    play_music();

    if(finished){
      bot.sendMessage(CHAT_ID, "Done playing!", "");
    }

  }
  
}


void playTone(const char *file_name,int pixel){
  printf("giong to start playing music\n");
  // pressed = true;
  audio.connecttoFS(SD,file_name);
  audio.loop();
}

void play_music(){
  audio.loop();
    unsigned long currentMillis = millis();
    // String prev_note = "G\r";
    if(start){
      printf("at start, opening file\n");
      //open file
      current_file= SD.open(file_name);  
      if(!current_file){
        Serial.print("couldn't open file!");
      } 
      //update start
      start = false;
      current_note_played = 0;
      // note_played_in_epsilon_time = true;
    }

    if(currentMillis - note_read_millis > 1200){
      
      //turn off previous pexil
      turn_off_lights();

      note_read_millis = currentMillis; 
      //read next note
      current_note_string = current_file.readStringUntil('\n');

      if(current_note_string == "C\r"){
        current_pixel = 0;
      }
      else if(current_note_string == "D\r"){
        current_pixel = 1;
      }
      else if(current_note_string == "E\r"){
        current_pixel = 2;
      }
      else if(current_note_string == "F\r"){
        current_pixel = 3;
      }
      else if(current_note_string == "G\r"){
        current_pixel = 4;
      }
      else if(current_note_string == "A\r"){
        current_pixel = 5;
      }
      else if(current_note_string == "B\r"){
        current_pixel = 6;
      }
      else if(current_note_string == "LG\r"){
        current_pixel = 4;
      }
      else if(current_note_string == "LD\r"){
        current_pixel = 1;
      }
      else if(current_note_string == "NULL\r"){
        turn_off_lights();
        current_pixel = 20;
      }
      else if(current_note_string == "END\r"){
        turn_off_lights();
        finished = true;
        playing_music = false;
        current_pixel = 20;
        audio.stopSong();
        current_file.close();

      }
      if(current_note_string != "NULL\r" && current_note_string != "END\r")
      {
        printf("turn on current pixel %d \n", current_pixel);
        pixels.setPixelColor(current_pixel, pixels.Color(0, 200, 150));
        pixels.show();
      }
      current_note_played = 0;

    }

    // if the right sensor is touched within 300ms, the note is considered to be played successfully
    // if the right sensor was touched after 300ms from reading the note, it is considered to be a missed/delayed note
    //if a wrong sensor was touched after 300ms from reading the note, it is considered a fail

    if (!current_note_played && (currentMillis - touch_sensor_millis_1 > 30) && currentMillis - note_read_millis < 300 ) {
      touch_sensor_millis = currentMillis;
      read_touch_sensors();
      // is the right note pressed?
      if(touch_sensor_val[current_note_played]){
        printf("current note was played!\n");
        // play note
        play_note(current_pixel);
        // update that note has been read
        current_note_played = 1;
      }
    }

    // after 300ms, note hasn't been played
    if (!current_note_played && (currentMillis - touch_sensor_millis_1 > 30) && currentMillis - note_read_millis > 300 ) {
      touch_sensor_millis = currentMillis;
      read_touch_sensors();
      // is the right note pressed?
      if(touch_sensor_val[current_note_played]){
        // play note
        play_note(current_pixel);
        // update that note has been played
        current_note_played = 1;
        delayed_notes++;
      }
      else{
        for(int i = 0; i < 7; i++){
          if(touch_sensor_val[i]){
            // play note that was pressed
            play_note(i);
            // update wrong notes number
            wrong_notes++;
          }
        }
      }
    }


    //once the note has been played, make sure no wrong note is played every 30ms
    if (current_note_played && currentMillis - touch_sensor_millis > 30 ) {
      touch_sensor_millis = currentMillis;
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
        else if(!current_note_played){
          playTone("C_major.wav",1);
          current_note_played=1;
        }

      }

      if(D_touchValue < threshold){
        if(current_pixel !=1){
          printf("D wrong note\n");
        }
        else if(!current_note_played){
          if(current_note_string=="LG\r"){
            playTone("D_long_major.wav",1);
          }
          else{
            playTone("D_major.wav",1);
          }

          current_note_played=1;
        }
      }

      if(E_touchValue < threshold){
        if(current_pixel != 2){
          printf("E wrong note\n");
        }
        else if(!current_note_played){
          playTone("E_major.wav",1);
          current_note_played=1;
        }
      }
      if(F_touchValue < threshold){

        if(current_pixel != 3){
          printf("F wrong note\n");
        }
        else if(!current_note_played){
          playTone("F_major.wav",1);
          current_note_played=1;
        }
      }
      if(G_touchValue < threshold){
        if(current_pixel != 4){
          printf("G wrong note\n");
        }
        else if(!current_note_played){
          if(current_note_string=="LG\r"){
            playTone("G_long_major.wav",1);
          }
          else{
            playTone("G_major.wav",1);
          }

          current_note_played=1;
        }
      }
      if(A_touchValue < threshold){
        if(current_pixel !=5){
          printf("A wrong note\n");
        }
        else if(!current_note_played){
          playTone("A_major.wav",1);
          current_note_played=1;
        }
      }

      if(B_touchValue < threshold){
        if(current_pixel != 6){
          printf("B wrong note\n");
        }
        else if(!current_note_played){
          playTone("B_major.wav",1);
          current_note_played=1;
        }
      }

    }

    // if (Serial.available()) { // if any string is sent via serial port
    //   audio.stopSong();
    //   Serial.println("audio stopped");
    //   log_i("free heap=%i", ESP.getFreeHeap()); //available RAM
    //   Serial.flush();
    // }
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
      bot.sendMessage(CHAT_ID, "touch sensor was touched, ouch!", "");
      printf("C wrong note\n");
    }
    else if(!current_note_played){
      bot.sendMessage(CHAT_ID, "touch sensor was touched, ouch!", "");
      playTone("C_major.wav",1);
      current_note_played=1;
    }

  }

  if(D_touchValue < threshold){
    if(current_pixel !=1){
      bot.sendMessage(CHAT_ID, "touch sensor was touched, ouch!", "");

      printf("D wrong note\n");
    }
    else if(!current_note_played){
      bot.sendMessage(CHAT_ID, "touch sensor was touched, ouch!", "");

      if(current_note_string=="LG\r"){
        playTone("D_long_major.wav",1);
      }
      else{
        playTone("D_major.wav",1);
      }

      current_note_played=1;
    }
  }

  if(E_touchValue < threshold){
    if(current_pixel != 2){
      bot.sendMessage(CHAT_ID, "touch sensor was touched, ouch!", "");

      printf("E wrong note\n");
    }
    else if(!current_note_played){
      bot.sendMessage(CHAT_ID, "touch sensor was touched, ouch!", "");

      playTone("E_major.wav",1);
      current_note_played=1;
    }
  }
  if(F_touchValue < threshold){

    if(current_pixel != 3){
      bot.sendMessage(CHAT_ID, "touch sensor was touched, ouch!", "");

      printf("F wrong note\n");
    }
    else if(!current_note_played){
      bot.sendMessage(CHAT_ID, "touch sensor was touched, ouch!", "");

      playTone("F_major.wav",1);
      current_note_played=1;
    }
  }
  if(G_touchValue < threshold){
    if(current_pixel != 4){
      bot.sendMessage(CHAT_ID, "touch sensor was touched, ouch!", "");

      printf("G wrong note\n");
    }
    else if(!current_note_played){
      bot.sendMessage(CHAT_ID, "touch sensor was touched, ouch!", "");

      if(current_note_string=="LG\r"){
        playTone("G_long_major.wav",1);
      }
      else{
        playTone("G_major.wav",1);
      }

      current_note_played=1;
    }
  }
  if(A_touchValue < threshold){
    bot.sendMessage(CHAT_ID, "touch sensor was touched, ouch!", "");

    if(current_pixel !=5){
      printf("A wrong note\n");
    }
    else if(!current_note_played){
      playTone("A_major.wav",1);
      current_note_played=1;
    }
  }

  if(B_touchValue < threshold){
      bot.sendMessage(CHAT_ID, "touch sensor was touched, ouch!", "");

    if(current_pixel != 6){
      printf("B wrong note\n");
    }
    else if(!current_note_played){
      playTone("B_major.wav",1);
      current_note_played=1;
    }
  }

}


void reconnect_to_wifi(){
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, password);
    // Serial.print(".");
    printf("reconnecting...\n");
    delay(300);
  }
}

void check_wifi_connection(){
  if (WiFi.status() != WL_CONNECTED){
    turn_lights_red();
    reconnect_to_wifi();
    turn_lights_green();
    delay(200);
    turn_off_lights();
  }
}

void turn_off_lights(){
  for(int i = 0; i < 8; i++){
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.show();
  }
}

void turn_lights_red(){
  for(int i = 0; i < 8; i++){
    pixels.setPixelColor(i, pixels.Color(100, 0, 0));
    pixels.show();
  }
}

void turn_lights_green(){
  for(int i = 0; i < 8; i++){
    pixels.setPixelColor(i, pixels.Color(0, 100, 0));
    pixels.show();
  }
}

void read_touch_sensors(){
  int C_touchValue = touchRead(C_TOUCH_PIN);
  int D_touchValue = touchRead(D_TOUCH_PIN);
  int E_touchValue = touchRead(E_TOUCH_PIN);
  int F_touchValue = touchRead(F_TOUCH_PIN);
  int G_touchValue = touchRead(G_TOUCH_PIN);
  int A_touchValue = touchRead(A_TOUCH_PIN);
  int B_touchValue = touchRead(B_TOUCH_PIN);
  touch_sensor_val[0] = (C_touchValue<threshold)? true : false;
  touch_sensor_val[1] = (D_touchValue<threshold)? true : false;
  touch_sensor_val[2] = (E_touchValue<threshold)? true : false;
  touch_sensor_val[3] = (F_touchValue<threshold)? true : false;
  touch_sensor_val[4] = (G_touchValue<threshold)? true : false;
  touch_sensor_val[5] = (A_touchValue<threshold)? true : false;
  touch_sensor_val[6] = (B_touchValue<threshold)? true : false;
  
}

void play_note(int note_number){
  
  if(note_number == 0){
    playTone("C_major.wav",1);
  }
  else if(note_number == 1){
    if(long_note){
      playTone("D_long_major.wav",1);
    }
    else{
      playTone("D_major.wav",1);
    }
  }
  else if(note_number == 2){
    playTone("E_major.wav",1);
  }
  else if(note_number == 3){
    playTone("F_major.wav",1);
  }
  else if(note_number == 4){
    if(long_note){
      playTone("G_long_major.wav",1);
    }
    else{
      playTone("G_major.wav",1);
    }
  }
  else if(note_number == 5){
    playTone("A_major.wav",1);
  }
  else if(note_number == 6){
    playTone("B_major.wav",1);
  }
      

      
}
