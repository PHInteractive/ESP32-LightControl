#include <iostream>

#include <Arduino.h>
#include <WiFi.h>
#include <OneButton.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//WiFi credentials
const char* ssid = "ESP32_LightControl";
const char* password = "supersecurePassword";

//WebServer variables
AsyncWebServer Server(80);
const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";
String processor(const String& var);
String outputState(int room);
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>LightControl</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP32 Light Control</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";


//Setup Inputs
OneButton Room1(32, true);
OneButton Room2(23, true);
OneButton Room3(15, true);
OneButton Room4(1, true);
OneButton Room5(3, true);

//Setup Variables
//timers
bool Room_1_AlwaysOn = false;
bool Room_2_AlwaysOn = false;
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
  Serial.begin(112500);
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
  xTaskCreate(Relay1Controller, "Relay1Controller", 1024, NULL, 2, NULL);
  xTaskCreate(Relay2Controller, "Relay2Controller", 1024, NULL, 2, NULL);

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

  //initialize WiFi and WebServer
  WiFi.persistent(false);
  WiFi.softAP(ssid, password, 1, 1);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Server.on("/", HTTP_GET, [](AsyncWebServerRequest* request){
    request->send_P(200, "text/html", index_html, processor);
  });
  //Handle main WebServer functionality
  Server.on("/update", HTTP_GET, [](AsyncWebServerRequest* request){
    String inputMessage1;
    String inputMessage2;
    if(request->hasParam(PARAM_INPUT_1) and request->hasParam(PARAM_INPUT_2)){
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value().c_str();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value().c_str();
      switch(inputMessage1.toInt()){
        case 1 :
          if(inputMessage2.toInt() == 1){
            Room_1_AlwaysOn = true;
          }else if(inputMessage2.toInt() == 0){
            Room_1_AlwaysOn = false;
          }
          break;

        case 2 :
          if(inputMessage2.toInt() == 1){
            Room_2_AlwaysOn = true;
          }else if(inputMessage2.toInt() == 0){
            Room_2_AlwaysOn = false;
          }
          break;
      }
    }
    request->send(200, "text/plain", "OK");
  });
  Server.begin();
}

void loop() {
  vTaskDelay(50 / portTICK_PERIOD_MS);
  Room1.tick();
  Room2.tick();
}

void Relay1Controller(void * parameters){
  do{
    vTaskDelay(time_wait_loop_ms / portTICK_PERIOD_MS);
    if(Room_1_AlwaysOn == true){
      Room_1_Time_Left_ms = 60000;
      digitalWrite(4, 0);
    }else if(Room_1_Time_Left_ms <= 0){
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
    if(Room_2_AlwaysOn == true){
      Room_2_Time_Left_ms = 60000;
      digitalWrite(4, 0);
    }else if(Room_2_Time_Left_ms <= 0){
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

String processor(const String& var){
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Output - Room 1</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"1\" " + outputState(1) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - Room 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(2) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}

String outputState(int room){
  switch(room){
    case 1 :
      if(Room_1_AlwaysOn == true){
        return "checked";
      }else{
        return "";
      }

    case 2 :
      if(Room_2_AlwaysOn == true){
        return "checked";
      }else{
        return "";
      }
  }return "";
}
