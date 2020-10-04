#include <Arduino.h>
#include <WiFi.h>
#include <OneButton.h>

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
  void RelayController(void * parameters);
  void Room1Click();
  void Room1DoubleClick();
  void Room1LongPress();
  //global
  int time_wait_loop_ms = 500;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ...");
  
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
  xTaskCreate(RelayController, "RelayController", 1024, NULL, 2, NULL);

  Room1.attachClick(Room1Click);
  Room1.attachDoubleClick(Room1DoubleClick);
  Room1.attachDuringLongPress(Room1LongPress);
  Room2.attachClick(Room1Click);
  Room2.attachDoubleClick(Room1DoubleClick);
  Room2.attachDuringLongPress(Room1LongPress);
}

void loop() {
  Room1.tick();

}

void RelayController(void * parameters){
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
  if(Room_1_Time_Left_ms <= 180000){
    Room_1_Time_Left_ms = 180000;
  }
}

void Room1DoubleClick(){
  if(Room_1_Time_Left_ms <= 1800000){
    Room_1_Time_Left_ms = 1800000;
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
  if(Room_2_Time_Left_ms <= 1800000){
    Room_2_Time_Left_ms = 1800000;
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