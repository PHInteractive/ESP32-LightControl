#include <iostream>

#include <Arduino.h>
#include <WiFi.h>
#include <OneButton.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>

//WiFi credentials
const char* ssid = "";
const char* password = "";
IPAddress staticIP(192, 168, 1, 71);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);

bool NTPstarted = false;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
int restartHour = 23;
int restartMinute = 59;
String startupTime = "0";
int rebootCounter = 0;


//WebServer variables
AsyncWebServer Server(80);
const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";
String processor(const String& var);
String outputState(int room);
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Lichtsteuerung Keller</title>
  <meta name="viewport" content="width=device-width, initial-scale=1", charset="utf-8" http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate">
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
    input:checked+.slider {background-color: #008000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body onload = "JavaScript:AutoRefresh(5000);">
  <h2>Lichtsteuerung Keller</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var a = confirm("Are you sure?");
  if (a == false){
  }else{
    var xhr = new XMLHttpRequest();
    if (element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
    else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
    xhr.send();
  }
}
function AutoRefresh( t ) {
  setTimeout("location.reload(true);", t);
}
</script>
</body>
</html>
)rawliteral";


//Setup Inputs
OneButton Room1(32, true, true);
OneButton Room2(14, true, true);
OneButton Room3(15, true, true);
OneButton Room4(1, true, true);
OneButton Room5(3, true, true);
OneButton Room6(5, true, true);

//Setup Variables
//timers
bool Room_1_AlwaysOn = false;
bool Room_2_AlwaysOn = false;
bool Room_3_AlwaysOn = false;
bool Room_4_AlwaysOn = false;
bool Room_5_On = false;
bool Room_6_On = false;
int Room_1_Time_Left_ms = 0;
int Room_2_Time_Left_ms = 0;
int Room_3_Time_Left_ms = 0;
int Room_4_Time_Left_ms = 0;

//functions
void RelayController(void * parameters);
void WiFi_connection_handler(void * parameters);
void Room1Click();
void Room1DoubleClick();
void Room1LongPress();  
void Room2Click();
void Room2DoubleClick();
void Room2LongPress();
void Room3Click();
void Room3DoubleClick();
void Room3LongPress();
void Room4Click();
void Room4DoubleClick();
void Room4LongPress();
void Room5Click();
void Room6Click();

//global
int time_wait_loop_ms = 500;

void setup() {
  //Setup Outputs
  //Serial.begin(115200);
  pinMode(32, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
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
  //setup Threads
  xTaskCreate(RelayController, "RelayController", 4096, NULL, 2, NULL);
  xTaskCreate(WiFi_connection_handler, "WiFi_connection_handler", 1024, NULL, 3, NULL);

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

  Room3.attachClick(Room3Click);
  Room3.attachDoubleClick(Room3DoubleClick);
  Room3.attachDuringLongPress(Room3LongPress);
  Room3.attachLongPressStop(Room3LongPress);
  Room3.attachLongPressStart(Room3LongPress);

  Room4.attachClick(Room4Click);
  Room4.attachDoubleClick(Room4DoubleClick);
  Room4.attachDuringLongPress(Room4LongPress);
  Room4.attachLongPressStop(Room4LongPress);
  Room4.attachLongPressStart(Room4LongPress);

  Room5.attachClick(Room5Click);
  
  Room6.attachClick(Room6Click);
  
  //initialize WiFi and WebServer
  if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
  }
  WiFi.begin(ssid, password);

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

        case 3 :
          if(inputMessage2.toInt() == 1){
            Room_3_AlwaysOn = true;
          }else if(inputMessage2.toInt() == 0){
            Room_3_AlwaysOn = false;
          }
          break;

        case 4 :
          if(inputMessage2.toInt() == 1){
            Room_4_AlwaysOn = true;
          }else if(inputMessage2.toInt() == 0){
            Room_4_AlwaysOn = false;
          }
          break;

        case 5 :
          if(inputMessage2.toInt() == 1){
            Room_5_On = true;
          }else if(inputMessage2.toInt() == 0){
            Room_5_On = false;
          }
          break;

        case 6 :
          if(inputMessage2.toInt() == 1){
            Room_6_On = true;
          }else if(inputMessage2.toInt() == 0){
            Room_6_On = false;
          }
          break;
      }
    }
    request->send(200, "text/plain", "OK");
  });
  Server.begin();
}

void loop() {
  vTaskDelay(10 / portTICK_PERIOD_MS);
  Room1.tick();
  Room2.tick();
  Room3.tick();
  Room4.tick();
  Room5.tick();
  Room6.tick();
}

void RelayController(void * parameters){
  do{
    vTaskDelay(time_wait_loop_ms / portTICK_PERIOD_MS);
    if(Room_1_AlwaysOn == true){
      Room_1_Time_Left_ms = 60000;
      digitalWrite(2, 0);
    }else if(Room_1_Time_Left_ms <= 0){
      digitalWrite(2, 1);
    }else if(Room_1_Time_Left_ms == 60000 or Room_1_Time_Left_ms == 59500){
      digitalWrite(2,1);
      Room_1_Time_Left_ms = Room_1_Time_Left_ms - time_wait_loop_ms;
    }else{
      digitalWrite(2, 0);
      Room_1_Time_Left_ms = Room_1_Time_Left_ms - time_wait_loop_ms;
    }
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
    if(Room_3_AlwaysOn == true){
      Room_3_Time_Left_ms = 60000;
      digitalWrite(12, 0);
    }else if(Room_3_Time_Left_ms <= 0){
      digitalWrite(12, 1);
    }else if(Room_3_Time_Left_ms == 60000 or Room_3_Time_Left_ms == 59500){
      digitalWrite(12,1);
      Room_3_Time_Left_ms = Room_3_Time_Left_ms - time_wait_loop_ms;
    }else{
      digitalWrite(12, 0);
      Room_3_Time_Left_ms = Room_3_Time_Left_ms - time_wait_loop_ms;
    }
    if(Room_4_AlwaysOn == true){
      Room_4_Time_Left_ms = 60000;
      digitalWrite(13, 0);
    }else if(Room_4_Time_Left_ms <= 0){
      digitalWrite(13, 1);
    }else if(Room_4_Time_Left_ms == 60000 or Room_4_Time_Left_ms == 59500){
      digitalWrite(13,1);
      Room_4_Time_Left_ms = Room_4_Time_Left_ms - time_wait_loop_ms;
    }else{
      digitalWrite(13, 0);
      Room_4_Time_Left_ms = Room_4_Time_Left_ms - time_wait_loop_ms;
    }
    if(Room_5_On == false){
      digitalWrite(33, 1);
    }else if(Room_5_On == true){
      digitalWrite(33, 0);
    }
    if(Room_6_On == true and Room_5_On == true){
      digitalWrite(25, 0);
    }else{
      Room_6_On = false;
      digitalWrite(25, 1);
    }
  }while(true);
  }

void Room1Click(){
  if(Room_1_Time_Left_ms <= 420000){
    Room_1_Time_Left_ms = 420000;
  }
}

void Room1DoubleClick(){
  if(Room_1_Time_Left_ms <= 1200000){
    Room_1_Time_Left_ms = 1200000;
  }
}

void Room1LongPress(){
  Room_1_Time_Left_ms = 0;
  Room_1_AlwaysOn = false;
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
  Room_2_AlwaysOn = false;
}

void Room3Click(){
  if(Room_3_Time_Left_ms <= 180000){
    Room_3_Time_Left_ms = 180000;
  }
}

void Room3DoubleClick(){
  if(Room_3_Time_Left_ms <= 900000){
    Room_3_Time_Left_ms = 900000;
  }
}

void Room3LongPress(){
  Room_3_Time_Left_ms = 0;
  Room_3_AlwaysOn = false;
}

void Room4Click(){
  if(Room_4_Time_Left_ms <= 180000){
    Room_4_Time_Left_ms = 180000;
  }
}

void Room4DoubleClick(){
  if(Room_4_Time_Left_ms <= 900000){
    Room_4_Time_Left_ms = 900000;
  }
}

void Room4LongPress(){
  Room_4_Time_Left_ms = 0;
  Room_4_AlwaysOn = false;
}

void Room5Click(){
  if(Room_5_On == true){
    Room_5_On = false;
  }else{
    Room_5_On = true;
  }
}

void Room6Click(){
  if(Room_6_On == true){
    Room_6_On = false;
  }else{
    Room_6_On = true;
  }
}

String processor(const String& var){
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Großer Keller (Sekunden verbleiben: " + String(Room_1_Time_Left_ms/1000) +")</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"1\" " + outputState(1) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Heizraum (Sekunden verbleiben: " + String(Room_2_Time_Left_ms/1000) +")</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(2) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Getränkeraum (Sekunden verbleiben: " + String(Room_3_Time_Left_ms/1000) +")</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"3\" " + outputState(3) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Außenbereich (Sekunden verbleiben: " + String(Room_4_Time_Left_ms/1000) +")</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Werkstatt</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"5\" " + outputState(5) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Trainingsraum</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"6\" " + outputState(6) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Startup Time: " + startupTime + "</h4>";
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
      break;

    case 2 :
      if(Room_2_AlwaysOn == true){
        return "checked";
      }else{
        return "";
      }
      break;
      
    case 3 :
      if(Room_3_AlwaysOn == true){
        return "checked";
      }else{
        return "";
      }break;

    case 4 :
      if(Room_4_AlwaysOn == true){
        return "checked";
      }else{
        return "";
      }      
      break;

    case 5 :
      if(Room_5_On == true){
        return "checked";
      }else{
        return "";
      }
      break;

    case 6 :
      if(Room_6_On == true){
        return "checked";
      }else{
        return "";
      }
      break;
  }return "";
}

void WiFi_connection_handler(void * parameters){
  do{
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    if(WiFi.status() != WL_CONNECTED){
      WiFi.disconnect();
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      WiFi.begin();
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      rebootCounter++;
      if(rebootCounter >= 20 and Room_1_Time_Left_ms == 0 and Room_2_Time_Left_ms == 0 and Room_3_Time_Left_ms == 0 and Room_4_Time_Left_ms == 0 and Room_5_On == false and Room_6_On == false){
        esp_restart();
      }
    }else{
      if(NTPstarted == false){
        timeClient.begin();
        NTPstarted = true;
        vTaskDelay(1000);
        timeClient.update();
        startupTime = timeClient.getFormattedTime();
      }
      rebootCounter = 0;
      //Serial.println(timeClient.getFormattedTime());  
      //timeClient.update();
      if(timeClient.getHours() == restartHour and timeClient.getMinutes() == restartMinute and Room_1_Time_Left_ms == 0 and Room_2_Time_Left_ms == 0 and Room_3_Time_Left_ms == 0 and Room_4_Time_Left_ms == 0 and Room_5_On == false and Room_6_On == false){
        esp_restart();
      }
    }
  }while(true);
}