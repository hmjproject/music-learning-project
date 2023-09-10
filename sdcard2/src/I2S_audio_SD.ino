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
// telegramMessage message = bot.getUpdates();

//touch sensors array
bool touch_sensor_val[7] = {false,false,false,false,false,false,false};

//audio
Audio audio;
//millis
unsigned long touch_sensor_millis = 0, touch_sensor_millis_1 = 0, note_read_millis = 0;
bool pressed = true;

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
String next_note_string;
int current_note_played = 1;
bool note_played_in_epsilon_time = false;
int start = true;
int finished = false;
File current_file;
File imageFile;
int current_pixel = 0;
String note_file_name;
int playing_music = false;
String file_name = "";
String photo_name = "";
// long note
bool long_note = false;
//volume
int volume = 20;
//statistics
double wrong_notes = 0;
double delayed_notes = 0;
int last_played_wrong_note = -1;

String current_chat_id = "";
//states declaration 
enum bot_states{
  START,
  INSTRUCTION,
  SETTINGS,
  CHOOSE_MUSIC,
  VOLUME,
  STATS,
  STATS_MENU
};

enum machine_state{
  PLAYING_SONG,
  WAITING_FOR_COMMANDS,
  PLAY_FREELY
};

bool isMoreDataAvailable();
byte getNextByte();

bool isMoreDataAvailable()
{
  return imageFile.available();
}

byte getNextByte()
{
  return imageFile.read();
}

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
        String keyboardJson = "[[\"play music 🎼\" ,\"settings ⚙\" ]]";
        bot.sendMessageWithReplyKeyboard(chat_id, welcome, "", keyboardJson, true); 
        b_state = INSTRUCTION;
      }
      else{
        bot.sendMessage(chat_id, "Please type /start to start 🎬", "");
        
      }
    }
    
    else if(b_state == INSTRUCTION)
    {
      if (text == "settings ⚙") {
        String print_text = "which settings would like to change❓\n";
        String keyboardJson = "[[\"volume 🔈\"]]";
        bot.sendMessageWithReplyKeyboard(chat_id, print_text, "", keyboardJson, true); 
        b_state = SETTINGS;
      }
      else if (text == "play music 🎼") {
        String print_text = "Which song would you like to play❓\n";
        String keyboardJson = "[[\"song1\",\"song2\" ,\"song3\",\"play freely\"],[ \"go back 🔙\"]]";
        bot.sendMessageWithReplyKeyboard(chat_id, print_text, "", keyboardJson, true); 
        b_state = CHOOSE_MUSIC;
      }
      else{
        bot.sendMessage(chat_id, "Please insert one of the options: settings or play music", "");
      }
    }
    
    else if(b_state == CHOOSE_MUSIC)
    {
      wrong_notes = 0;
      delayed_notes = 0;
      if(text == "song1"){
        m_state = PLAYING_SONG;
        bot.sendMessage(chat_id, "going to play music💃🏻", "");
        file_name = "/music_sheets/song1.txt";
        start = true;
        finished = false;
        b_state = STATS;
      }
      else if(text == "song2"){
        m_state = PLAYING_SONG;
        bot.sendMessage(chat_id, "going to play music💃🏻", "");
        file_name = "/music_sheets/song2.txt";
        start = true;
        finished = false;
        b_state = STATS;
      }
      else if(text == "song3"){
        m_state = PLAYING_SONG;
        bot.sendMessage(chat_id, "going to play music💃🏻", "");
        file_name = "/music_sheets/oldMac.txt";
        start = true;
        finished = false;
        b_state = STATS;
      }
      else if(text == "go back 🔙"){
        bot_print_menu(chat_id);
        b_state = INSTRUCTION;
      }
      else if(text == "play freely"){
        b_state = CHOOSE_MUSIC;
        m_state = PLAY_FREELY;

      }
      else{
        bot.sendMessage(chat_id, "Please choose a valid option", "");
      }

    }

    else if(b_state == SETTINGS)
    {
      if(text == "volume 🔈"){
        String print_text = "would like to increase or decrease volume❓\n";
        String keyboardJson = "[[\"increase volume🔊\",\"decrease volume🔉\"],[ \"go back 🔙\"]]";
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
          bot.sendMessage(chat_id, "Increased volume", "");
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
          bot.sendMessage(chat_id, "Decreased volume", "");
        }
        else{
          bot.sendMessage(chat_id, "Can't decrease the volume anymore 🤷‍♀️", "");
        }
        b_state = VOLUME;

      }
      else if(text == "go back 🔙"){
        String welcome = "What would you like to do❓\n";
        String keyboardJson = "[[\"play music 🎼\" ,\"settings ⚙\" ]]";
        bot.sendMessageWithReplyKeyboard(chat_id, welcome, "", keyboardJson, true); 
        b_state = INSTRUCTION;
      }
      else{
        bot.sendMessage(chat_id, "Please choose a valid option", "");
      }
    }

    // else if(b_state == STATS){
    //   String welcome = "What would you like to do?\n";
    //   String keyboardJson = "[[\"get statistics\" ,\"go back to menu\" ]]";
    //   bot.sendMessageWithReplyKeyboard(chat_id, welcome, "", keyboardJson, true); 
    //   b_state = STATS_MENU;
    // }

    else if(b_state == STATS_MENU){
      if(text == "get statistics📉")
      {
       double st1=(wrong_notes/12)*100;
        printf("st1 is: %f\n",st1);
        double st2=(delayed_notes/12)*100;
        printf("st2 is: %f\n",st2);
        String message = "your stats:\nwrong notes: " + String(st1,3) +" ❌"+"\nDelayed notes: " + String(st2,3) + " ⏰";
        bot.sendMessage(chat_id, message, "");
        String comment = pickComment();
        bot.sendPhoto(chat_id, choosePhoto(), comment);
        bot_print_menu(chat_id);
        b_state = INSTRUCTION;
      }
      else if(text == "go back to menu🔙"){
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
  String keyboardJson = "[[\"play music 🎼\" ,\"settings ⚙\" ]]";
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
      String keyboardJson = "[[\"get statistics📉\" ,\"go back to menu🔙\" ]]";
      bot.sendMessageWithReplyKeyboard(current_chat_id, welcome, "", keyboardJson, true); 
      b_state = STATS_MENU;
    }

  }
  if(m_state == PLAY_FREELY){
    unsigned long currentMillis = millis();
    if ( (currentMillis - touch_sensor_millis_1 > 20) ) {
      touch_sensor_millis = currentMillis;
      read_touch_sensors();
      // is the right note pressed?
      for(int i = 0; i < 7; i++){
        if(touch_sensor_val[i]){
          
          play_note(i);

        }
      }
    }
  }
  // if(m_state == DONE_PLAYING_SONG){
  //   //ask if he wants to 
  // }
  
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
    //start -> get params ready + open file
    if(start){
      //open file
      current_file= SD.open(file_name);  
      if(!current_file){
        Serial.print("couldn't open file!");
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
        printf("updating wrong notes ----------------> \n");
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
      int next_note_pixel = get_pixel(next_note_string);
      current_pixel = get_pixel(current_note_string);
      printf("current note: %s\n",current_note_string);
      printf("next note: %s\n", next_note_string);
      if(current_note_string == "NULL\r"){
        turn_off_lights();
        current_pixel = 20;
        
      }
      else if(current_note_string == "END\r"){
        turn_off_lights();
        finished = true;
        m_state = WAITING_FOR_COMMANDS;
        current_pixel = 20;
        audio.stopSong();
        current_file.close();
        // printf("wrong note number ----------------> %f\n",wrong_notes);
        // printf("delayed note number ----------------> %f\n",delayed_notes);

      }

      if(current_note_string == "End\r"){
        turn_off_lights();
      }
      else if(next_note_pixel == current_pixel){
        printf("turning pixels blue, index: %d, next_note: %d, current: %d\n",index11, next_note_pixel, current_pixel);
        
        //if current pixel and next pixel are the same turn pixel blue
        turn_pixel_blue(next_note_pixel);
      }
      else{
        printf("turning current pixel green, %d\n",index11);
        turn_pixel_green(current_pixel);
        if(next_note_string != "END\r"){
          printf("turning next pixel red, %d\n",index11);
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
        printf("current note was played!\n");
        // play note
        play_note(current_pixel);
        printf("if 11111111111111111111111111111111\n");

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
        // printf("if 22222222222222222222222222\n");

        // update that note has been played
        current_note_played = 1;
        delayed_notes++;
        // printf("delayed time: %d\n",currentMillis - note_read_millis);
      }
      else{
        for(int i = 0; i < 7; i++){
          if(i==current_pixel){
            continue;
          }
          if(touch_sensor_val[i]){
            if(last_played_wrong_note != -1){
              continue;
            }
            // printf("got wrong note, %d\n", i);
            // play note that was pressed
            play_note(i);
            // printf("if 22222222222333333333333333333333\n");

            // update wrong notes number
            wrong_notes++;
            last_played_wrong_note = i;
          }
        }
      }
    }


    //once the note has been played, make sure no wrong note is played every 30ms
    if (current_note_played && currentMillis - touch_sensor_millis > 30 ) {
      touch_sensor_millis = currentMillis;
      read_touch_sensors();
      // is the right note pressed?
      
      for(int i = 0; i < 7; i++){
        if(i == current_pixel){
          continue;
        }
        if(touch_sensor_val[i]){
          if(last_played_wrong_note != -1){
            continue;
          }
          // printf("got wrong note, %d\n", i);
          // play note that was pressed
          play_note(i);
          // printf("if 3333333333333333333333333\n");

          // update wrong notes number
          wrong_notes++;
          last_played_wrong_note = i;
        }
      }
    }
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
    if(long_note){
      playTone("C_long_major.wav",1);
    }
    else{
      playTone("C_major.wav",1);
    }
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
    if(long_note){
      playTone("E_long_major.wav",1);
    }
    else{
      playTone("E_major.wav",1);
    }
  }
  else if(note_number == 3){
    if(long_note){
      playTone("F_long_major.wav",1);
    }
    else{
      playTone("F_major.wav",1);
    }
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
    if(long_note){
      playTone("A_long_major.wav",1);
    }
    else{
      playTone("A_major.wav",1);
    }
  }
  else if(note_number == 6){
    if(long_note){
      playTone("B_long_major.wav",1);
    }
    else{
      playTone("E_major.wav",1);
    }
  }
      
}

int get_pixel(String note){
  int pexil_to_ret = 0;
  if(note == "C\r" || note == "LC\r"){
    pexil_to_ret = 0;
  }
  else if(note == "D\r" || note == "LD\r"){
    pexil_to_ret = 1;
  }
  else if(note == "E\r" || note == "LE\r"){
    pexil_to_ret = 2;
  }
  else if(note == "F\r" || note == "LF\r"){
    pexil_to_ret = 3;
  }
  else if(note == "G\r" || note == "LG\r"){
    pexil_to_ret = 4;
  }
  else if(note == "A\r" || note == "LA\r"){
    pexil_to_ret = 5;
  }
  else if(note == "B\r" || note == "LB\r"){
    pexil_to_ret = 6;
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

bool _is_long_note(){
  if(current_note_string == "LD\r" || current_note_string == "LE\r" || current_note_string == "LF\r"
   || current_note_string == "LG\r" || current_note_string == "LA\r" || current_note_string == "LB\r" 
   || current_note_string == "LC\r"){
    return true;
  }
  return false;
}

String choosePhoto(){
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

