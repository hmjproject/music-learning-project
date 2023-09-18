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
// const char* ssid = "miral";
// const char* password = "jessyj2772";
// const char* ssid = "Hadeel";
// const char* password = "1234hadeel";

//Telegram BOT
#define BOTtoken "6382002255:AAFPCttqq1v4URJGQbHBJ9fzRpcZedvYxaw"
#define CHAT_ID "1019453346"
WiFiClientSecure client;
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtoken, client);
// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

int index11 = 0;

// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      3
#define I2S_LRC       26

//touch
#define C5_TOUCH_PIN     12
#define B_TOUCH_PIN     13
#define A_TOUCH_PIN     14
#define G_TOUCH_PIN     33
#define F_TOUCH_PIN     32
#define E_TOUCH_PIN     15
#define D_TOUCH_PIN     4
#define C_TOUCH_PIN     27

//touch threshold
const int threshold = 40;
// telegramMessage message = bot.getUpdates();

//touch sensors array
bool touch_sensor_val[8] = {false,false,false,false,false,false,false,false};

//audio
Audio audio;

//millis
unsigned long touch_sensor_millis = 0, touch_sensor_millis_1 = 0, note_read_millis = 0;

//URL - photo
String p0mn12 = "https://i.imgur.com/9aQd3wM.jpg";
String p1mn12 = "https://i.imgur.com/WL7kmln.jpg";
String p2mn12 = "https://i.imgur.com/E6oNOEx.jpg";
String p3mn12 = "https://i.imgur.com/wQCJUfU.jpg";
String p4mn12 = "https://i.imgur.com/JmLg1dr.jpg";
String p5mn12 = "https://i.imgur.com/k8HnRLs.jpg";
String p6mn12 = "https://i.imgur.com/h1jKlm7.jpg";
String p7mn12 = "https://i.imgur.com/UShvin3.jpg";
String p8mn12 = "https://i.imgur.com/kBXbx2c.jpg";
String p9mn12 = "https://i.imgur.com/isPoQZE.jpg";
String p10mn12 = "https://i.imgur.com/Bvd3pzc.jpg";
String p11mn12 = "https://i.imgur.com/a2uKUYN.jpg";
String p12mn12 = "https://i.imgur.com/63HfeT7.jpg";


//neopixel
#define PIN 22
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//funcs
void play_music();
void read_touch_sensors();
int read_user_song();
void check_sd_card_status();
// ----- wifi funcs -----
void check_wifi_connection();
void reconnect_to_wifi();
// ----- note functions -----
void playTone(const char *file_name);
void play_note(int note_number);
bool _is_long_note();
int get_pixel(String note);
// ------ light functions ------
void turn_off_lights();
void turn_lights_red();
void turn_lights_green();
void turn_lights_emerald();
void turn_pixel_red(int pixel_num);
void turn_pixel_blue(int pixel_num);
void turn_pixel_green(int pixel_num);
// ----- Telegram funcs -----
String choosePhoto();
String pickComment();
void bot_print_menu(String chat_id);
void handleNewMessages(int numNewMessages);


//for files
String current_note_string;
String next_note_string;
int current_note_played = 1;
int start = true;
int finished = false;
File current_file;
int current_pixel = 0;
int next_note_pixel = 20;
String file_name = "";
bool pressed = true;

// long note
bool long_note = false;


String my_notes[13] = {"","","","","","","","","","","","",""};
String song_text;

//volume
int volume = 20;

//statistics
double wrong_notes = 0;
double delayed_notes = 0;
int last_played_wrong_note = -1;

String current_chat_id = "";
//states declaration 
// for message sending and recieving
enum bot_states{
  START,
  INSTRUCTION,
  SETTINGS,
  CHOOSE_MUSIC,
  VOLUME,
  STATS,
  STATS_MENU,
  GAME_INSTR 
};

// in order to distinguish between playing music and recieving messages 
enum machine_state{
  PLAYING_SONG,
  WAITING_FOR_COMMANDS,
  PLAY_FREELY
};

//state values
machine_state m_state = WAITING_FOR_COMMANDS;
bot_states b_state = START;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    current_chat_id = chat_id;
    // if (chat_id != CHAT_ID){
    //   bot.sendMessage(chat_id, "Unauthorized user", "");
    //   continue;
    // }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if(b_state == START){
      if (text == "/start") {
        String welcome = "Welcome, " + from_name + ".🙋‍♀️\n";
        welcome += "what would you like to do❓\n\n";
        String keyboardJson = "[[\"Play music 🎼\" ,\"Settings ⚙\" ],[ \"Game Instructions 🎹\"]]";
        bot.sendMessageWithReplyKeyboard(chat_id, welcome, "", keyboardJson, true); 
        b_state = INSTRUCTION;
      }
      else{
        bot.sendMessage(chat_id, "Please type /start to start 🎬", "");
        
      }
    }
    
    else if(b_state == INSTRUCTION)
    {
      if (text == "Settings ⚙") {
        String print_text = "Which settings would like to change❓\n";
        String keyboardJson = "[[\"Volume 🔈\"]]";
        bot.sendMessageWithReplyKeyboard(chat_id, print_text, "", keyboardJson, true); 
        b_state = SETTINGS;
      }
      else if (text == "Play music 🎼") {
        String print_text = "Which song would you like to play❓\n";
        String keyboardJson = "[[\"DoReMi\",\"Happy Birthday\" ,\"Old Macdonalds\",\"my song\"],[ \"Go back 🔙\"]]";
        bot.sendMessageWithReplyKeyboard(chat_id, print_text, "", keyboardJson, true); 
        b_state = CHOOSE_MUSIC;
      }
      else if(text == "Game Instructions 🎹"){
        String print_text = "Welcome to our interactive piano learning game! 🎹 \nTo get started, simply select a song of your choice by clicking on 'Choose Song.'. \n";
        print_text += "Once you've made your selection, \nthe lights will illuminate following these guidelines:\n\n";
        print_text += "🟢 Green: Indicates the note you should play. \n\n🔴 Red: Signals the next note to be played.\n\n";
        print_text += "🔵 Blue: Highlights when the current note and the next note are identical. the light will turn pink for a short time indicating that you must lift your finger.\n\nEnjoy!";
        String keyboardJson = "[[ \"Go back 🔙\"]]";
        bot.sendMessageWithReplyKeyboard(chat_id, print_text, "", keyboardJson, true); 
        b_state = GAME_INSTR;
      }
      else{
        bot.sendMessage(chat_id, "Please insert one of the options: Settings or Play music", "");
      }
    }
    
    else if(b_state == CHOOSE_MUSIC)
    {
      wrong_notes = 0;
      delayed_notes = 0;
      if(text == "DoReMi"){
        m_state = PLAYING_SONG;
        bot.sendMessage(chat_id, "Going to play music💃🏻", "");
        file_name = "/music_sheets/song1.txt";
        start = true;
        finished = false;
        b_state = STATS;
      }
      else if(text == "Happy Birthday"){
        m_state = PLAYING_SONG;
        bot.sendMessage(chat_id, "Going to play music💃🏻", "");
        file_name = "/music_sheets/song2.txt";
        start = true;
        finished = false;
        b_state = STATS;
      }
      else if(text == "Old Macdonalds"){
        m_state = PLAYING_SONG;
        bot.sendMessage(chat_id, "Going to play music💃🏻", "");
        file_name = "/music_sheets/OldMac.txt";
        start = true;
        finished = false;
        b_state = STATS;
      }
      else if (text == "my song"){
        String message = "enter a string of 12 notes with commas in between \n";
        message += "example: A1,B2,NULL,A2,C1,G1,F2,G2,E1,D2,A1,A1! \nwhere 1 stands for a short note\n";
        message += "and 2 stands for a long note \n";
        bot.sendMessage(chat_id, message, "");
        // Check for new messages
        // int num_of_messages = bot.getUpdates(bot.last_message_received + 1);
        // while(!num_of_messages){
        //   num_of_messages = bot.getUpdates(bot.last_message_received + 1);
        // }
        // printf("before if\n ");
        // if(num_of_messages == 1){
        //   song_text = bot.messages[0].text;
        //   FillArray(song_text);
        //   createNewSongFile();
        //   printArray();
        // }
        // else{
        //   bot.sendMessage(chat_id, "invalid input , please enter another one !", "");
        // }
        int ret_val = 0;
        while(!ret_val){
          ret_val = read_user_song();
        }
        if(ret_val == 2){
          bot_print_menu(chat_id);
          b_state = INSTRUCTION;
        }
        else{
          bot.sendMessage(chat_id, "Going to play music💃🏻", "");
          delay(500);
          m_state = PLAYING_SONG;
          file_name = "/music_sheets/example.txt";
          start = true;
          finished = false;
          b_state = STATS;
        }

      }
      else if(text == "Go back 🔙"){
        bot_print_menu(chat_id);
        b_state = INSTRUCTION;
      }
      else{
        bot.sendMessage(chat_id, "Please choose a valid option", "");
      }

    }

    else if(b_state == SETTINGS)
    {
      if(text == "Volume 🔈"){
        String print_text = "Would like to increase or decrease volume❓\n";
        String keyboardJson = "[[\"increase volume🔊\",\"decrease volume🔉\"],[ \"Go back 🔙\"]]";
        bot.sendMessageWithReplyKeyboard(chat_id, print_text, "", keyboardJson, true); 
        b_state = VOLUME;
      }
      else{
        bot.sendMessage(chat_id, "Please choose a valid option", "");
      }
    }

    else if(b_state == VOLUME){
      if(text == "increase volume🔊"){
        if(volume < 21){
          volume++;
          audio.setVolume(volume);
          double vol_percent = ((double)volume)*100/ 21;
          String welcome = "Increased volume to " + String(vol_percent)+"%";
          bot.sendMessage(chat_id, welcome, "");
        }
        else{
          bot.sendMessage(chat_id, "Can't increse the volume anymore 🤷‍♀️", "");
        }
        b_state = VOLUME;
      }

      else if(text == "decrease volume🔉"){
        if(volume > 0){
          volume--;
          audio.setVolume(volume);
          double vol_percent = ((double)volume)*100/ 21;
          String welcome = "Decreased volume to " + String(vol_percent) + "%";
          bot.sendMessage(chat_id, welcome, "");
        }
        else{
          bot.sendMessage(chat_id, "Can't decrease the volume anymore 🤷‍♀️", "");
        }
        b_state = VOLUME;

      }
      else if(text == "Go back 🔙"){
        bot_print_menu(chat_id); 
        b_state = INSTRUCTION;
      }
      else{
        bot.sendMessage(chat_id, "Please choose a valid option", "");
      }
    }

    else if(b_state == STATS_MENU){
      if(text == "get statistics📉")
      {
        double st1=(wrong_notes/12)*100;
        // printf("st1 is: %f\n",st1);
        double st2=(delayed_notes/12)*100;
        if(file_name == "/music_sheets/OldMac.txt"){
          double st2=(delayed_notes/26)*100;
        }
        // printf("st2 is: %f\n",st2);
        String message = "Your stats:\nWrong notes: " + String(st1,3) +" ❌"+"\nDelayed notes: " + String(st2,3) + " ⏰";
        bot.sendMessage(chat_id, message, "");
        String comment = pickComment();
        bot.sendPhoto(chat_id, choosePhoto(), comment);
        bot_print_menu(chat_id);
        b_state = INSTRUCTION;
      }
      else if(text == "Go back to menu🔙"){
        bot_print_menu(chat_id);
        b_state = INSTRUCTION;
      }
      else{
        bot.sendMessage(chat_id, "Please choose a valid option", "");
      }
    }
    else if(b_state == GAME_INSTR){
      if(text == "Go back 🔙"){
        bot_print_menu(chat_id);
        b_state = INSTRUCTION;
      }
      else{
        bot.sendMessage(chat_id, "Please choose a valid option", "");
      }
    }
  }
}

void bot_print_menu(String chat_id){
  String welcome = "What would you like to do❓\n";
  String keyboardJson = "[[\"Play music 🎼\" ,\"Settings ⚙\" ],[ \"Game Instructions 🎹\"]]";
  bot.sendMessageWithReplyKeyboard(chat_id, welcome, "", keyboardJson, true);
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
  check_sd_card_status();

  //--------------- audio ------------------------
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(volume); // range 0...21 - This is not amplifier gain, but controlling the level of output amplitude. 
  // audio.connecttoFS(SD, "123_u8.wav"); // a file with the proper name must be placed in root folder of SD card (formatted FAT32, file name max 8 chars no spaces)
  // audio.stopSong();

  //----------------- Connect to Wi-Fi ----------------------
  WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);
  check_wifi_connection();
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(1000);
  //   Serial.println("Connecting to WiFi..");
  // }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  //----------------------- initialize neo-pexils -------------------------
  pixels.begin(); 
  pixels.clear();

}

void loop()
{
  //audio.loop();
  check_wifi_connection();
  // check_sd_card_status();

  // 
  if(m_state == WAITING_FOR_COMMANDS){
    for(int i = 0; i < 8; i++){
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show();
    }
    
    if (millis() > lastTimeBotRan + botRequestDelay)  {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      // printf("num of new messages is %d\n",numNewMessages);
      while(numNewMessages) {
        Serial.println("got response");
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = millis();
    }
  }

  if(m_state == PLAYING_SONG){
    play_music();
    if(finished){
      bot.sendMessage(current_chat_id, "Done playing! ✅", "");
      String welcome = "What would you like to do❓\n";
      String keyboardJson = "[[\"get statistics📉\" ,\"Go back to menu🔙\" ]]";
      bot.sendMessageWithReplyKeyboard(current_chat_id, welcome, "", keyboardJson, true); 
      b_state = STATS_MENU;
      audio.stopSong();

    }

  }
  /*
  for debug! we can't support this since we can't accept messages when playing music
  */
  // if(m_state == PLAY_FREELY){
  //   unsigned long currentMillis = millis();
  //   if ( (currentMillis - touch_sensor_millis_1 > 20) ) {
  //     touch_sensor_millis = currentMillis;
  //     read_touch_sensors();
  //     // is the right note pressed?
  //     for(int i = 0; i < 8; i++){
  //       if(touch_sensor_val[i]){
  //         // printf("in sensor %d\n",i);
  //         play_note(i);

  //       }
  //     }
  //   }
  // }
  // if(m_state == DONE_PLAYING_SONG){
  //   //ask if he wants to 
  // }
  
}

int read_user_song(){
  // Check for new messages
  int num_of_messages = bot.getUpdates(bot.last_message_received + 1);
  while(!num_of_messages){
    num_of_messages = bot.getUpdates(bot.last_message_received + 1);
  }
  printf("before if\n ");
  if(num_of_messages == 1){
    song_text = bot.messages[0].text;
    if(song_text == "Go back"){
      return 2;
    }
    int filled_arr = FillArray(song_text);
    if(filled_arr == -1){
      return 0;
    }
    createNewSongFile();
    // printArray();
    return 1;
  }
  else{
    bot.sendMessage(current_chat_id, "invalid input, please enter another one!", "");
    return 0;
  }
}

void playTone(const char *file_name){
  // printf("giong to start playing music\n");
  // pressed = true;
  audio.connecttoFS(SD,file_name);
  audio.loop();
}

void play_music(){
  if(!start){
    audio.loop();
  }
    unsigned long currentMillis = millis();
    //start -> get params ready + open file
    if(start){
      //open file
      current_file= SD.open(file_name);  
      if(!current_file){
        Serial.println("couldn't open file!");
      } 
      //update params
      start = false;
      current_note_played = 1;
      next_note_string = current_file.readStringUntil('\n');
      index11 = 0;
    }

    if((currentMillis - note_read_millis > 1200 && !long_note) || (long_note && currentMillis - note_read_millis > 2400) ){
      //reset long note param
      index11++;
      long_note = false;
      //check if prev note was played
      if(!current_note_played && last_played_wrong_note == -1){
        Serial.println("updating wrong notes ----------------> \n");
        wrong_notes++;
      }
      //turn off previous pexil
      turn_off_lights();

      note_read_millis = currentMillis; 
      //read next note
      current_note_string = next_note_string;
      //check if current note is long
      if(_is_long_note()){
        long_note = true;
      }
      // get next note ready
      next_note_string = current_file.readStringUntil('\n');
      next_note_pixel = get_pixel(next_note_string);
      current_pixel = get_pixel(current_note_string);
      // printf("current note: %s\n",current_note_string);
      // printf("next note: %s\n", next_note_string);
      if(current_note_string == "NULL\r"){
        Serial.println("got null");
        turn_off_lights();
        current_pixel = 20;
        
      }
      else if(current_note_string == "END\r"){
        turn_off_lights();
        finished = true;
        m_state = WAITING_FOR_COMMANDS;
        current_pixel = 20;
        int c = millis();
        while(millis() - c < 200);
        
        audio.stopSong();
        current_file.close();
      }

      // if(current_note_string == "End\r"){
      //   turn_off_lights();
      // }
      else if(next_note_pixel == current_pixel){
        // printf("turning pixels blue, index: %d, next_note: %d, current: %d\n",index11, next_note_pixel, current_pixel);
        
        //if current pixel and next pixel are the same turn pixel blue
        turn_pixel_blue(next_note_pixel);
      }
      else{
        // printf("turning current pixel green, %d\n",index11);
        turn_pixel_green(current_pixel);
        if(next_note_string != "END\r"){
          // printf("turning next pixel red, %d\n",index11);
          //turn current pixel green
          turn_pixel_red(next_note_pixel);
        }
      }
      //if current note is null then consider it played
      if(current_pixel == 20){
        current_note_played = 1;
      }
      else{
        current_note_played = 0;
      }

      last_played_wrong_note = -1;

    }

    // if the right sensor is touched within 500ms, the note is considered to be played successfully
    // if the right sensor was touched after 500ms from reading the note, it is considered to be a missed/delayed note
    //if a wrong sensor was touched after 500ms from reading the note, it is considered a fail

    if (!current_note_played && (currentMillis - touch_sensor_millis_1 > 20) && currentMillis - note_read_millis < 500 ) {
      touch_sensor_millis = currentMillis;
      read_touch_sensors();
      // is the right note pressed?
      if(touch_sensor_val[current_pixel]){
        // play note
        play_note(current_pixel);
        // turn_pixel_pink(current_pixel);
        // update that note has been read
        current_note_played = 1;
      }
    }

    // after 500ms, note hasn't been played
    if (!current_note_played && (currentMillis - touch_sensor_millis_1 > 20) && currentMillis - note_read_millis > 500 ) {
      touch_sensor_millis = currentMillis;
      read_touch_sensors();
      // is the right note pressed?
      if(touch_sensor_val[current_pixel]){
        // play note
        play_note(current_pixel);
        // turn_pixel_pink(current_pixel);
        // update that note has been played
        current_note_played = 1;

        delayed_notes++;
      }
      else{
        for(int i = 0; i < 8; i++){
          if(i==current_pixel){
            continue;
          }
          if(touch_sensor_val[i]){
            if(last_played_wrong_note != -1){
              continue;
            }
            // play note that was pressed
            play_note(i);

            // update wrong notes number
            wrong_notes++;
            last_played_wrong_note = i;
          }
        }
      }
    }

    if(currentMillis - note_read_millis > 1100 && next_note_pixel == current_pixel && next_note_string != "NULL\r"){
      turn_pixel_pink(current_pixel);
    }

    //once the note has been played, make sure no wrong note is played every 30ms
    if (current_note_played && currentMillis - touch_sensor_millis > 20 ) {
      touch_sensor_millis = currentMillis;
      read_touch_sensors();

      // is the right note pressed?
      
      for(int i = 0; i < 8; i++){
        if(i == current_pixel){
          if(!touch_sensor_val[i] && currentMillis - note_read_millis > 500){
            Serial.println("finger removed");
            audio.stopSong();
          }
          continue;
        }
        if(touch_sensor_val[i]){
          if(last_played_wrong_note != -1){
            continue;
          }
          if(current_note_string == "NULL\r" && currentMillis - note_read_millis < 500){
            Serial.println("in null wrong note");
            continue;
          }
          if(current_note_string == "NULL\r"){
            Serial.println("playing note in null");
          }
          // play note that was pressed
          play_note(i);

          // update wrong notes number
          wrong_notes++;
          last_played_wrong_note = i;
        }
      }
    }
}

void reconnect_to_wifi(){
  WiFi.begin(ssid, password);
  while (WiFi.status()  != WL_CONNECTED) {
    delay(1000);
    // Serial.print(".");
    Serial.println("reconnecting...\n");
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

void turn_lights_emerald(){
  for(int i = 0; i < 8; i++){
    pixels.setPixelColor(i, pixels.Color(100, 150, 0));
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
  int D5_touchValue = touchRead(C5_TOUCH_PIN);
  touch_sensor_val[7] = (C_touchValue<threshold)? true : false;
  touch_sensor_val[6] = (D_touchValue<threshold)? true : false;
  touch_sensor_val[5] = (E_touchValue<threshold)? true : false;
  touch_sensor_val[4] = (F_touchValue<threshold)? true : false;
  touch_sensor_val[3] = (G_touchValue<threshold)? true : false;
  touch_sensor_val[2] = (A_touchValue<threshold)? true : false;
  touch_sensor_val[1] = (B_touchValue<threshold)? true : false;
  touch_sensor_val[0] = (D5_touchValue<threshold)? true : false;
  
}

void play_note(int note_number){
  
  if(note_number == 7){
    if(long_note){
      playTone("C_long_major.wav");
    }
    else{
      playTone("C_major.wav");
    }
  }
  else if(note_number == 6){
    if(long_note){
      playTone("D_long_major.wav");
    }
    else{
      playTone("D_major.wav");
    }
  }
  else if(note_number == 5){
    if(long_note){
      playTone("E_long_major.wav");
    }
    else{
      playTone("E_major.wav");
    }
  }
  else if(note_number == 4){
    if(long_note){
      playTone("F_long_major.wav");
    }
    else{
      playTone("F_major.wav");
    }
  }
  else if(note_number == 3){
    if(long_note){
      playTone("G_long_major.wav");
    }
    else{
      playTone("G_major.wav");
    }
  }
  else if(note_number == 2){
    if(long_note){
      playTone("A_long_major.wav");
    }
    else{
      playTone("A_major.wav");
    }
  }
  else if(note_number == 1){
    if(long_note){
      playTone("B_long_major.wav");
    }
    else{
      playTone("B_major.wav");
    }
  }
  else if(note_number == 0){
    if(long_note){
      playTone("D5_long_major.wav");
    }
    else{
      playTone("D5_major.wav");
    }
  }
      
}

int get_pixel(String note){
  int pexil_to_ret = 0;
  if(note == "C\r" || note == "LC\r"){
    pexil_to_ret = 7;
  }
  else if(note == "D\r" || note == "LD\r"){
    pexil_to_ret = 6;
  }
  else if(note == "E\r" || note == "LE\r"){
    pexil_to_ret = 5;
  }
  else if(note == "F\r" || note == "LF\r"){
    pexil_to_ret = 4;
  }
  else if(note == "G\r" || note == "LG\r"){
    pexil_to_ret = 3;
  }
  else if(note == "A\r" || note == "LA\r"){
    pexil_to_ret = 2;
  }
  else if(note == "B\r" || note == "LB\r"){
    pexil_to_ret = 1;
  }
  else if(note == "H\r" || note == "LH\r"){
    pexil_to_ret = 0;
  }
  else{
    pexil_to_ret = 20;
    
  }
  return pexil_to_ret;

}

void turn_pixel_red(int pixel_num){
  if(pixel_num == 20){
    return;
  }
  pixels.setPixelColor(pixel_num, pixels.Color(100, 0, 0));
  pixels.show();
}

void turn_pixel_blue(int pixel_num){
  if(pixel_num == 20){
    return;
  }
  pixels.setPixelColor(pixel_num, pixels.Color(0, 0, 150));
  pixels.show();
}

void turn_pixel_green(int pixel_num){
  if(pixel_num == 20){
    return;
  }
  pixels.setPixelColor(pixel_num, pixels.Color(0, 150, 0));
  pixels.show();
}

void turn_pixel_pink(int pixel_num){
  if(pixel_num == 20){
    return;
  }
  pixels.setPixelColor(pixel_num, pixels.Color(255, 182, 193));
  pixels.show();
}

bool _is_long_note(){
  if(current_note_string == "LD\r" || current_note_string == "LE\r" || current_note_string == "LF\r"
   || current_note_string == "LG\r" || current_note_string == "LA\r" || current_note_string == "LB\r" 
   || current_note_string == "LC\r" || current_note_string == "LH\r"){
    return true;
  }
  return false;
}

String choosePhoto(){
  if(file_name == "/music_sheets/OldMac.txt"){
    wrong_notes = (wrong_notes==0)?0:wrong_notes/2-1;
    wrong_notes = (wrong_notes<0)?0:wrong_notes;
  }
  if(wrong_notes == 0){
    return p12mn12;
  }
  else if (wrong_notes == 1){
    return p11mn12;
  }
  else if (wrong_notes == 2){
    return p10mn12;
  }
  else if (wrong_notes == 3){
    return p9mn12;
  }
  else if (wrong_notes == 4){
    return p8mn12;
  }
  else if (wrong_notes == 5){
    return p7mn12;
  }
  else if (wrong_notes == 6){
    return p6mn12;
  }
  else if (wrong_notes == 7){
    return p5mn12;
  }
  else if (wrong_notes == 8){
    return p4mn12;
  }
  else if (wrong_notes == 9){
    return p3mn12;
  }
  else if (wrong_notes == 10){
    return p2mn12;
  }
  else if (wrong_notes == 11){
    return p1mn12;
  }
  else{
    return p0mn12;
  }
}

String pickComment(){
  if(wrong_notes == 0){
    return ("Great job 🏆🎉");
  }
  else if(wrong_notes > 0 && wrong_notes < 7){
    return ("Good job 🤩✨" );
  }
  else if(wrong_notes > 6 && wrong_notes < 12){
    return ("It's OK, you can improve 😃💪🏻");
  }
  else{
    return ("OOPS, life be like that sometimes 😐🔁");
  }
}

void createNewSongFile(){
  // Open a new file for writing (creates the file if it doesn't exist)
  File myFile = SD.open("/music_sheets/example.txt", FILE_WRITE);

  // Write data to the file
  if (myFile) {
    for (int i=0 ; i < 13; i++){
      if(i>0 && my_notes[i-1] == "END"){
        break;
      }
      myFile.println(my_notes[i]);
    }
    myFile.close();
  } 
  else {
    Serial.println("Error opening file.");
  }
  
}

/*void createAndFill(String song_text ){
  File myFile = SD.open("/music_sheets/example.txt", FILE_WRITE);
  //printf("get the massege :%s\n", song_text);
  if(song_text.length() > 0 && song_text.length() < 25 ){  // # define 35 2*12+11 
    //printf("len of song:%d\n",song_text.length() );
    if(myFile){
      for (int k=0 ; k<((song_text.length())-1) ;k=k+2){
        if(song_text[k+1] == '1'){
          myFile.println(song_text[k]);
        }
        else if(song_text[k+1] == '2'){
          myFile.println("L" + song_text[k]);

        }
      }
      myFile.println("END");
    }
    myFile.close();
  }
}*/

int FillArray(String song_text ){
  int j =0;
  // printf("len of song:%d\n",song_text.length() );
  for (int k=0; k<((song_text.length())-1); k=k+3){

    //1.check input
    if((song_text[k]!='A' && song_text[k]!='B' && song_text[k]!='C' 
      && song_text[k]!='D' && song_text[k]!='E' && song_text[k]!='F' 
      && song_text[k]!='G' && song_text[k]!='N') || (k != 0 && song_text[k-1] != ',')){
        bot.sendMessage(current_chat_id, "invalid input, \nplease enter valid input or go back to menu by typing 'Go back'!", "");
        return -1;
    }
    // printf("current index: %d\n", k );
    //if it starts with N then most likely the user inserted NULL
    // 2.update array: there are two options, either the user entered a character A/B/C/..., or NULL, 
    if(song_text[k] != 'N'){
      Serial.println("othen than N");
      // A/B/C/...
      // check if note is long or short
      if(song_text[k+1] == '1'){
        my_notes[j]= String(song_text[k]);
        // printf("current character:%s\n",my_notes[j]);
      }
      else if(song_text[k+1] == '2'){
        my_notes[j]= "L" + String(song_text[k]);
        // printf("current character:%s\n",my_notes[j]);
      }
      else{
        bot.sendMessage(current_chat_id, "invalid input, \nplease enter valid input or go back to menu by typing 'Go back'!", "");
        return -1;
      }
    }
    else{
      //check if it is null
      if(song_text.length() <= (k+4)|| song_text[k+1]!='U' || song_text[k+2]!='L' || song_text[k+3]!='L'){
        //invalid input
        bot.sendMessage(current_chat_id, "invalid input, please enter another one!", "");
        return -1;

      }
      my_notes[j]= "NULL";
      // the for loop increases k by 3 already, thus only increase k by 2
      k+=2;
    }
    j++;
    if(j>12){
      bot.sendMessage(current_chat_id, "Long input, \nplease enter valid input or go back to menu by typing 'Go back'!", "");
      return -1;
    }
  }
  if(j<12){
    bot.sendMessage(current_chat_id, "Short input, \nplease enter valid input or go back to menu by typing 'Go back'!", "");
    return -1;
  }
  my_notes[j] = "END";
  return 0;
}

// void printArray(){
//   for (int i=0 ; i < 13; i++){
//     printf("%s\n", my_notes[i]);
//   }
// }

void check_sd_card_status(){
  bool sd_begin = SD.begin(SD_CS);
  while(sd_begin == 0){
    turn_lights_emerald();
    sd_begin = SD.begin(SD_CS);
  }

}
