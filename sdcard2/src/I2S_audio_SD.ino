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
void bot_print_menu(String chat_id);


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


String my_notes[13] = {"","","","","","","","","","","","",""};
String song_text;

//volume
int volume = 18;

//statistics
int wrong_notes = 0;
int delayed_notes = 0;
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
        String welcome = "Welcome, " + from_name + ".\n";
        welcome += "what would you like to do?.\n\n";
        String keyboardJson = "[[\"play music\" ,\"settings\" ]]";
        bot.sendMessageWithReplyKeyboard(chat_id, welcome, "", keyboardJson, true); 
        b_state = INSTRUCTION;
      }
      else{
        bot.sendMessage(chat_id, "Please type /start to start", "");
        
      }
    }
    
    else if(b_state == INSTRUCTION)
    {
      if (text == "settings") {
        String print_text = "which settings would like to change?.\n";
        String keyboardJson = "[[\"volume\"]]";
        bot.sendMessageWithReplyKeyboard(chat_id, print_text, "", keyboardJson, true); 
        b_state = SETTINGS;
      }
      else if (text == "play music") {
        String print_text = "Which song would you like to play?.\n";
        String keyboardJson = "[[\"song1\",\"song2\" ,\"song3\",\"my song\",\"play freely\"],[ \"go back\"]]";
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
        bot.sendMessage(chat_id, "going to play music!", "");
        file_name = "/music_sheets/song1.txt";
        start = true;
        finished = false;
        b_state = STATS;
      }
      else if(text == "song2"){
        m_state = PLAYING_SONG;
        bot.sendMessage(chat_id, "going to play music!", "");
        file_name = "/music_sheets/song2.txt";
        start = true;
        finished = false;
        b_state = STATS;
      }
      else if(text == "song3"){
        m_state = PLAYING_SONG;
        bot.sendMessage(chat_id, "going to play music!", "");
        file_name = "/music_sheets/oldMac.txt";
        start = true;
        finished = false;
        b_state = STATS;
      }
      else if (text == "my song"){
        String message = "enter a string of 12 notes with apostrophes in between \n";
        message += "example: A1,B2,NULL,A2,C1,G1,F2,G2,E1,D2,A1,A1! \n where 1 stands for a short note\n";
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
        while(!read_user_song());
        m_state = PLAYING_SONG;
        file_name = "/music_sheets/example.txt";
        start = true;
        finished = false;
        b_state = STATS;

      }
      else if(text == "go back"){
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
      if(text == "volume"){
        String print_text = "would like to increase or decrease volume?.\n";
        String keyboardJson = "[[\"increase volume\",\"decrease volume\"],[ \"go back\"]]";
        bot.sendMessageWithReplyKeyboard(chat_id, print_text, "", keyboardJson, true); 
        b_state = VOLUME;
      }
      else{
        bot.sendMessage(chat_id, "Please choose a valid option", "");
      }
    }

    else if(b_state == VOLUME){
      if(text == "increase volume"){
        if(volume < 21){
          volume++;
          audio.setVolume(volume);
          bot.sendMessage(chat_id, "Increased volume", "");
        }
        else{
          bot.sendMessage(chat_id, "Can't increse the volume anymore :(", "");
        }
        b_state = VOLUME;
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
        b_state = VOLUME;

      }
      else if(text == "go back"){
        String welcome = "What would you like to do?\n";
        String keyboardJson = "[[\"play music\" ,\"settings\" ]]";
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
      if(text == "get statistics"){
        double st1=(wrong_notes/12)*100;
        printf("st1 is: %f\n",st1);

        double st2=(delayed_notes/12)*100;
        printf("st2 is: %f\n",st2);

        String message = "your stats:\nwrong notes: " + String(st1,3) +"\nDelayed notes: " + String(st2,3);
        bot.sendMessage(chat_id, message, "");
        bot_print_menu(chat_id);
        b_state = INSTRUCTION;
      }
      else if(text == "go back to menu"){
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
  String welcome = "What would you like to do?\n";
  String keyboardJson = "[[\"play music\" ,\"settings\" ]]";
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
      if(numNewMessages) {
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
      bot.sendMessage(current_chat_id, "Done playing!", "");
      String welcome = "What would you like to do?\n";
      String keyboardJson = "[[\"get statistics\" ,\"go back to menu\" ]]";
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

int read_user_song(){
  // Check for new messages
  int num_of_messages = bot.getUpdates(bot.last_message_received + 1);
  while(!num_of_messages){
    num_of_messages = bot.getUpdates(bot.last_message_received + 1);
  }
  printf("before if\n ");
  if(num_of_messages == 1){
    song_text = bot.messages[0].text;
    int filled_arr = FillArray(song_text);
    if(filled_arr == -1){
      return 0;
    }
    createNewSongFile();
    printArray();
    return 1;
  }
  else{
    bot.sendMessage(current_chat_id, "invalid input, please enter another one!", "");
    return 0;
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
      if(!current_note_played && last_played_wrong_note == -1){
        wrong_notes++;
      }
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
        m_state = WAITING_FOR_COMMANDS;
        current_pixel = 20;
        audio.stopSong();
        current_file.close();
        printf("wrong note number ----------------> %d\n",wrong_notes);
        printf("delayed note number ----------------> %d\n",delayed_notes);

      }
      if(current_note_string != "NULL\r" && current_note_string != "END\r")
      {
        printf("turn on current pixel %d \n", current_pixel);
        pixels.setPixelColor(current_pixel, pixels.Color(0, 200, 150));
        pixels.show();
      }
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
        printf("if 22222222222222222222222222\n");

        // update that note has been played
        current_note_played = 1;
        delayed_notes++;
        printf("delayed time: %d\n",currentMillis - note_read_millis);
      }
      else{
        for(int i = 0; i < 7; i++){
          if(i==current_pixel){
            continue;
          }
          if(touch_sensor_val[i]){
            if(last_played_wrong_note == i){
              continue;
            }
            printf("got wrong note, %d\n", i);
            // play note that was pressed
            play_note(i);
            printf("if 22222222222222222222222222\n");

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
          if(last_played_wrong_note == i){
            continue;
          }
          printf("got wrong note, %d\n", i);
          // play note that was pressed
          play_note(i);
          printf("if 3333333333333333333333333\n");

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
  printf("len of song:%d\n",song_text.length() );
  for (int k=0; k<((song_text.length())-1); k=k+3){

    //1.check input
    if(song_text[k]!='A' && song_text[k]!='B' && song_text[k]!='C' 
      && song_text[k]!='D' && song_text[k]!='E' && song_text[k]!='F' 
      && song_text[k]!='G' && (song_text[k]!='N') && (k != 0 && song_text[k-1] != ',')){
        bot.sendMessage(current_chat_id, "invalid input, please enter another one!", "");
        return -1;
    }
    printf("current index: %d\n", k );
    //if it starts with N then most likely the user inserted NULL
    // 2.update array: there are two options, either the user entered a character A/B/C/..., or NULL, 
    if(song_text[k] != 'N'){
      // A/B/C/...
      // check if note is long or short
      if(song_text[k+1] == '1'){
        my_notes[j]= String(song_text[k]);
        //printf("current character:%s\n",my_notes[j]);
      }
      else if(song_text[k+1] == '2'){
        my_notes[j]= "L" + String(song_text[k]);
        //printf("current character:%s\n",my_notes[j]);
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
  }
  my_notes[j] = "END";
  return 0;
  //printf("after end");
}

void printArray(){
  for (int i=0 ; i < 13; i++){
    printf("%s\n", my_notes[i]);
  }
}