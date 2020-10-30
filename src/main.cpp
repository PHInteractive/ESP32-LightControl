#include <Arduino.h>
#include <WiFi.h>
#include <OneButton.h>
#include <WiFi.h>
#include <WebServer.h>

  //WiFi credentials
  const char* ssid = "ESP32_LightControll";
  const char* password = "supersecurePassword";

  //Setup Inputs
  OneButton Room1(32, true);
  OneButton Room2(23, true);
  OneButton Room3(15, true);
  OneButton Room4(1, true);
  OneButton Room5(3, true);

  //Setup Variables
  //timers
  int Room_1_Time_Left_ms = 0;
  int Room_2_Time_Left_ms = 0;
  int Room_3_Time_Left_ms = 0;
  int Room_4_Time_Left_ms = 0;
  int Room_5_Time_Left_ms = 0;
  //functions
  void Relay1Controller(void * parameters);
  void Relay2Controller(void * parameters);
  void Room1Click();
  void Room1DoubleClick();
  void Room1LongPress();  
  void Room2Click();
  void Room2DoubleClick();
  void Room2LongPress();
  //global
  int time_wait_loop_ms = 500;

void setup() {
  //Setup Outputs
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  //turn off outputs
  digitalWrite(2, 1);
  digitalWrite(4, 1);
  digitalWrite(12, 1);
  digitalWrite(13, 1);
  digitalWrite(33, 1);
  digitalWrite(25, 1);
  digitalWrite(26, 1);
  digitalWrite(27, 1);
  //Setup MosFet Outputs
  pinMode(22, OUTPUT);
  pinMode(21, OUTPUT);
  //setup Threads
  xTaskCreate(Relay1Controller, "Relay1Controller", 1024, NULL, NULL, NULL);
  xTaskCreate(Relay2Controller, "Relay2Controller", 1024, NULL, NULL, NULL);

  Room1.attachClick(Room1Click);
  Room1.attachDoubleClick(Room1DoubleClick);
  Room1.attachDuringLongPress(Room1LongPress);
  Room1.attachLongPressStop(Room1LongPress);
  Room1.attachLongPressStart(Room1LongPress);
  
  Room2.attachClick(Room2Click);
  Room2.attachDoubleClick(Room2DoubleClick);
  Room2.attachDuringLongPress(Room2LongPress);
  Room2.attachLongPressStop(Room2LongPress);
  Room2.attachLongPressStart(Room2LongPress);
}

void loop() {
  vTaskDelay(50 / portTICK_PERIOD_MS);
  Room1.tick();
  Room2.tick();
}

void Relay1Controller(void * parameters){
  do{
    vTaskDelay(time_wait_loop_ms / portTICK_PERIOD_MS);
    if(Room_1_Time_Left_ms <= 0){
      digitalWrite(2, 1);
    }else if(Room_1_Time_Left_ms == 60000 or Room_1_Time_Left_ms == 59500){
      digitalWrite(2,1);
      Room_1_Time_Left_ms = Room_1_Time_Left_ms - time_wait_loop_ms;
    }else{
      digitalWrite(2, 0);
      Room_1_Time_Left_ms = Room_1_Time_Left_ms - time_wait_loop_ms;
    }
  }while(true);
}

void Relay2Controller(void * parameters){
  do{
    vTaskDelay(time_wait_loop_ms / portTICK_PERIOD_MS);
    if(Room_2_Time_Left_ms <= 0){
      digitalWrite(4, 1);
    }else if(Room_2_Time_Left_ms == 60000 or Room_2_Time_Left_ms == 59500){
      digitalWrite(4,1);
      Room_2_Time_Left_ms = Room_2_Time_Left_ms - time_wait_loop_ms;
    }else{
      digitalWrite(4, 0);
      Room_2_Time_Left_ms = Room_2_Time_Left_ms - time_wait_loop_ms;
    }
  }while(true);
}

void Room1Click(){
  if(Room_1_Time_Left_ms <= 300000){
    Room_1_Time_Left_ms = 300000;
  }
}

void Room1DoubleClick(){
  if(Room_1_Time_Left_ms <= 1200000){
    Room_1_Time_Left_ms = 1200000;
  }
}

void Room1LongPress(){
  Room_1_Time_Left_ms = 0;
}

void Room2Click(){
  if(Room_2_Time_Left_ms <= 180000){
    Room_2_Time_Left_ms = 180000;
  }
}

void Room2DoubleClick(){
  if(Room_2_Time_Left_ms <= 900000){
    Room_2_Time_Left_ms = 900000;
  }
}

void Room2LongPress(){
  Room_2_Time_Left_ms = 0;
}

void Room3Click(){}

void Room3DoubleClick(){}

void Room3LongPress(){}

void Room4Click(){}

void Room4DoubleClick(){}

void Room4LongPress(){}

void Room5Click(){}

void Room5DoubleClick(){}

void Room5LongPress(){}