#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <mpu6050_esp32.h>
#include<math.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.
uint32_t primary_timer = 0;

//IMU SETUP
const uint8_t LOOP_PERIOD = 500; //milliseconds
float z; //variables for grabbing x,y,and z values
int CW_counter = 0;
int CCW_counter = 0;

int password[10];
int entry[10];

const uint8_t BUTTON1 = 45; //pin connected to button
const uint8_t BUTTON2 = 39; //pin connected to button
MPU6050 imu; //imu object called, appropriately, imu
//float old_acc_mag;  //previous acc mag
//float older_acc_mag;  //previous prevoius acc mag
const float ZOOM = 9.81;

//state vars
const uint8_t UNLOCKED = 0;//start state
uint8_t state = UNLOCKED;  //system state for step counting
const uint8_t PROGRAM1 = 1;
const uint8_t PROGRAM2 = 2;
const uint8_t PROGRAM3 = 3;
//
const uint8_t LOCKED = 4;
const uint8_t ENTRY1 = 5;
const uint8_t CHECK1 = 6;
const uint8_t ENTRY2 = 7;
const uint8_t CHECK2 = 8;
const uint8_t ENTRY3 = 9;
const uint8_t CHECK3 = 10;

// WAITING STATE
const uint8_t W1 = 11;
const uint8_t W2 = 12;
const uint8_t W3 = 13;
const uint8_t W4 = 14;

const uint8_t W5 = 15;
const uint8_t W6 = 16;
const uint8_t W7 = 17;
const uint8_t W8 = 18;
const uint8_t W9 = 19;

// button tracker
uint8_t prev_B1 = 1;
uint8_t prev_B2 = 1;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //begin serial comms
  while (!Serial); // wait for Serial to show up
  Wire.begin();
  delay(50); //pause to make sure comms get set up
  if (imu.setupIMU(1)) {
    Serial.println("IMU Connected!");
  } else {
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(2); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
   
 
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
}


void loop() {
  uint8_t button1 = digitalRead(BUTTON1);
  uint8_t button2 = digitalRead(BUTTON2);
  combo_fsm(button1,button2);
  
  while (millis()-primary_timer<300); //wait for primary timer to increment
  primary_timer =millis();
}


void combo_fsm(uint8_t button1, uint8_t button2){
  switch(state){
    case UNLOCKED:
      Serial.println("UNLOCKED");
      tft.fillScreen(TFT_GREEN);
      tft.setCursor(0, 0, 1);
      tft.setTextSize(2);
      tft.setTextColor(TFT_BLACK, TFT_GREEN);//set color of font to black foreground, green background
      tft.println("UNLOCKED. \n Hello \n World\n");
      if(prev_B1 == 1 && button1 == 0){
        //once on pressing
        prev_B1 = 0;
        state = W1;
        }else if(prev_B2 == 1 && button2 == 0){ //if b2 on press
          prev_B2 = 0;
          state = W5;
          }
      break;

    case W1:
      Serial.println("W1");
      if(prev_B1 == 0 && button1 == 1){
        prev_B1 = 1;
        state = PROGRAM1; //go to program 1
        }
        
      break;

    case PROGRAM1:
      Serial.println("PROG 1");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.println(CW_counter);
      imu.readGyroData(imu.gyroCount);
      z = imu.gyroCount[2] * imu.gRes;
      if (z < -5){
        CW_counter = (CW_counter + 1)%11;
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0, 1);
        tft.println(CW_counter);
        }
      if(prev_B1 == 1 && button1 == 0){
        //once press
        prev_B1 = 0;
        state = W2;
        }
      break;

    case W2:
      // TODO: Store first entry (current angle)
      Serial.println("W2");
      password[0] = CW_counter;
      
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.println(password[0]);
      if(prev_B1 == 0 && button1 == 1){
        prev_B1 = 1;
        state = PROGRAM2; //go to program 2
        CW_counter = 0;
       }
      break;

    case PROGRAM2:
      char output[100];
      Serial.println("PROG 2");
      sprintf(output, "%i %i", password[0], CCW_counter);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.println(output);
      imu.readGyroData(imu.gyroCount);
      z = imu.gyroCount[2] * imu.gRes;
      if (z > 5){ 
        CCW_counter = (CCW_counter + 1) %11;
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0, 1);
        sprintf(output, "%i %i", password[0], CCW_counter);
        tft.println(output);
        }
      if(prev_B1 == 1 && button1 == 0){
        //once on pressing
        prev_B1 = 0;
        state = W3;
        }
      break;

    case W3:
      // TODO: Store second entry
      password[1] = CCW_counter;
      
      Serial.println("W3");
      sprintf(output, "%i %i", password[0], password[1]);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.println(output);
       if(prev_B1 == 0 && button1 == 1){
        prev_B1 = 1;
        state = PROGRAM3; //go to program 1
        CCW_counter = 0;
        }
      break;

    case PROGRAM3:
      Serial.println("PROG 3");
      sprintf(output, "%i %i %i", password[0], password[1], CW_counter);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.println(output);
      imu.readGyroData(imu.gyroCount);
      z = imu.gyroCount[2] * imu.gRes;
      if (z < -5){ 
        CW_counter = (CW_counter + 1) %11;
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0, 1);
        sprintf(output, "%i %i %i", password[0], password[1], CW_counter);
        tft.println(output);
        }
      if(prev_B1 == 1 && button1 == 0){
        //once on pressing
        prev_B1 = 0;
        state = W4;
        }
      break;

    case W4:
      // TODO: Store second entry
      password[2] = CW_counter;
      
      Serial.println("W4");
      sprintf(output, "%i %i %i", password[0], password[1], password[2]);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.println(output);
      if(prev_B1 == 0 && button1 == 1){
        prev_B1 = 1;
        state = UNLOCKED; //go to program 1
        CW_counter = 0;
        }
      break;

     case W5:
      Serial.println("W5");
      if(prev_B2 == 0 && button2 == 1){ //on release
        prev_B2 = 1;
        state = LOCKED;
      }
      break;

     case LOCKED:
//      memset(entry, 0, sizeof(entry));
      tft.fillScreen(TFT_RED);
      tft.setCursor(0, 0, 2);
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE, TFT_RED);
      tft.println("LOCKED");
      
      if(prev_B1 == 1 && button1 == 0){
        prev_B1 = 0;
        state = W6;
        }
      break;

     case W6:
      if(prev_B1 == 0 && button1 == 1){ //on B1 release
        prev_B1 = 1;
        state = ENTRY1;
        }
      break;

     case ENTRY1:
      /* TODO: ENTER FIRST NUM*/
      Serial.println("ENTRY 1");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.println(CW_counter);
      imu.readGyroData(imu.gyroCount);
      z = imu.gyroCount[2] * imu.gRes;
      if (z < -5){
        CW_counter = (CW_counter + 1)%11;
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0, 1);
        tft.println(CW_counter);
        }
      if(prev_B1 == 1 && button1 == 0){ //B1 on press
        prev_B1 = 0;
        state = W7;
        }
      break;

     case W7:
      Serial.println("W7");
      entry[0] = CW_counter;
      
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.println(entry[0]);
      if(prev_B1 == 0 && button1 == 1){
        prev_B1 = 1;
        state = CHECK1;
        CW_counter = 0;
        }
      break;

     case CHECK1:
      /* if correct, go to entry2
       * else, go to LOCKED*/ 
       Serial.println("CHECK1");
      if(entry[0] == password[0]){
        state = ENTRY2;
      }else{
        state = LOCKED;
      }
      break;

     case ENTRY2:
      Serial.println("ENTRY2");
      sprintf(output, "%i %i", entry[0], CCW_counter);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.println(output);
      imu.readGyroData(imu.gyroCount);
      z = imu.gyroCount[2] * imu.gRes;
      if (z > 5){ 
        CCW_counter = (CCW_counter + 1) %11;
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0, 1);
        sprintf(output, "%i %i", entry[0], CCW_counter);
        tft.println(output);
        }
      if(prev_B1 == 1 && button1 == 0){ //B1 on press
        prev_B1 = 0;
        state = W8;
        }
      break;

     case W8:
      entry[1] = CCW_counter;
      
      Serial.println("W8");
      sprintf(output, "%i %i", entry[0], entry[1]);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.println(output);
      if(prev_B1 == 0 && button1 == 1){
        prev_B1 = 1;
        state = CHECK2;
        CCW_counter = 0;
        }
      break;
      
     case CHECK2:
      Serial.println("CHECK2");
     /* TODO: Check if it is correct
       * if correct, go to entry3
       * else, go to LOCKED
       */ 
      if(entry[1] == password[1]){
        state = ENTRY3;
      }else{
        state = LOCKED;
      }
      break;

     case ENTRY3:
      Serial.println("ENTRY 3");
      sprintf(output, "%i %i %i", entry[0], entry[1], CW_counter);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.println(output);
      imu.readGyroData(imu.gyroCount);
      z = imu.gyroCount[2] * imu.gRes;
      if (z < -5){ 
        CW_counter = (CW_counter + 1) %11;
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0, 1);
        sprintf(output, "%i %i %i", entry[0], entry[1], CW_counter);
        tft.println(output);
        }
      if(prev_B1 == 1 && button1 == 0){ //B1 on press
        prev_B1 = 0;
        state = W9;
        }
      break;

     case W9:
      entry[2] = CW_counter;
      
      Serial.println("W9");
      sprintf(output, "%i %i %i", password[0], password[1], password[2]);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.println(output);
     if(prev_B1 == 0 && button1 == 1){
        prev_B1 = 1;
        state = CHECK3;
        CW_counter = 0;
        }
      break;

     case CHECK3:
      Serial.println("check3");
      /* TODO: Check if it is correct
       * if correct, go to UNLOCKED
       * else, go to LOCKED
       */ 
       if(entry[2] == password[2]){
        state = UNLOCKED;
      }else{
        state = LOCKED;
      }
      break;

  }
}
