#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <time.h>
#include <DHTesp.h>
#include <pubsubclient.h>
#include <ESP32Servo.h>

#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET 0
#define UTC_OFFSET_DST 0

#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  
#define OLED_RESET    -1  
#define SCREEN_ADDRESS 0x3C 

#define BUZZER 18
#define LED_1 15
#define LED_2 2
#define CANCEL 34
#define UP 35
#define DOWN 32
#define OK 33
#define DHT 12
#define LDR 39
#define SERVO 4


int n_notes =8;
int C=262;
int D=294;
int E=330;  
int F=349;
int G=392;
int A=440;
int B=494;
int C_H=523;
int notes[]={C,D,E,F,G,A,B,C_H};

int year = 0;
int month = 0;
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

int tita = 0;
int titaoffset = 30;
float gammaVal = 0.75;
float temp = 0;
float tempmed = 30;

int intensitycount = 0;
float intensity = 0;

bool alarm_enabled = false;
int n_alarms = 2;
bool enable_Alrm[] = {false,false};
int alarm_hours[] = {0,0};
int alarm_minutes[] = {0,0};
bool alarm_triggered[] = {false,false};

bool alarm_snoozed[] = {false,false};

unsigned long timeNow = 0;
unsigned long timeLast = 0;
unsigned long timelastmin = 0;

int Ts = 5;
int Tu = 2;

int current_mode = 1;
int max_modes = 4;
String options[] = {"1 - Set Time","2 - Set Alarm 1","3 - Set Alrm 2","4 - View Alarms"};
float cumuIntensity = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;
Servo servo;

const char* mqtt_server = "broker.hivemq.com";
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Print the payload as characters
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Create a null-terminated string from payload
  char message[length + 1];           // +1 for null terminator
  memcpy(message, payload, length);   // Copy the payload
  message[length] = '\0';             // Null terminate the string

  // Process the topic
  if (strcmp(topic, "Medibox/intensitySampling") == 0) {
    Ts = atoi(message);
    //Serial.print("Sampling Time (Ts) : ");
    //Serial.println(Ts);
  }
  if (strcmp(topic, "Medibox/intensitySending") == 0) {
    Tu = atoi(message);
    //Serial.print("Sending Time (Tu) : ");
    //Serial.println(Tu);
  }
  if (strcmp(topic, "Medibox/titaoffset") == 0) {
    titaoffset = atoi(message);
    //Serial.print("Tita Offset : ");
    //Serial.println(titaoffset);
  }
  if (strcmp(topic, "Medibox/controlFact") == 0) {
    gammaVal = atof(message);
    //Serial.print("Gamma : ");
    //Serial.println(gammaVal);
  }
  if (strcmp(topic, "Medibox/tempmed") == 0) {
    tempmed = atof(message);
    //Serial.print("Temp Med : ");
    //Serial.println(tempmed);
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);  // Unique client ID

    // Attempt to connect with Last Will message
    if (client.connect(clientId.c_str(), "Medibox/status", 0, true, "ESP32 Disconnected")) {
      Serial.println("connected");

      // Publish initial online status
      client.publish("Medibox/status", "ESP32 Connected to MQTT", true);

      // Subscribe to relevant topics
      client.subscribe("Medibox/temperature");
      client.subscribe("Medibox/intensity");
      client.subscribe("Medibox/intensitySampling");
      client.subscribe("Medibox/intensitySending");
      client.subscribe("Medibox/titaoffset");
      client.subscribe("Medibox/controlFact");
      client.subscribe("Medibox/tempmed");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}




void print_line(String text, int text_size, int row, int column) {
  //Complete
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column, row);
  display.println(text);
  display.display();
}
void check_temp(){
  //Complete
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  temp = data.temperature;
  client.publish("Medibox/temperature", String(temp).c_str(), true);
  bool all_good = true;
  if(data.temperature>32){
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("TEMP HIGH ", 1, 55, 0);
  }
  else if(data.temperature<25){
    all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line("TEMP LOW ", 1, 55, 0);
  }
  if(data.humidity>80){
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("HUMD HIGH",1,45,0);
  }
  else  if(data.humidity<65){
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("HUMD LOW", 1, 45, 0);
  }
  if(all_good){
    digitalWrite(LED_2,LOW); 
    print_line("All Good",1,50,0);
  }
}

void print_time_now(void) {
  //Complete
  display.clearDisplay();

  String dateString = String(year) + "-" + (month < 10 ? "0" : "") + String(month) + "-" + (days < 10 ? "0" : "") + String(days);
  print_line(dateString, 1, 0, 60);

  String timeString = (hours < 10 ? "0" : "") + String(hours) + ":" + 
                      (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
                      (seconds < 10 ? "0" : "") + String(seconds);
  print_line(timeString, 2, 25, 20);

  display.display();
}

void snoozeAlarm(int i) {
    noTone(BUZZER);  
    delay(200);
    digitalWrite(LED_1, LOW);
    display.clearDisplay();
    print_line("Snoozed Alarm... "+String(i), 1, 40, 0);
    Serial.println("Snoozed Alarm... "+String(i));
    int temphour=hours;
    int tempminute=minutes;
    if(tempminute+5>=60){
      temphour+=1;
      tempminute = (tempminute+5)%60;
    }
    else{
      tempminute+=2;
    }
    alarm_hours[i] = temphour;
    alarm_minutes[i] = tempminute;
    alarm_triggered[i] = false;
    enable_Alrm[i] = true;
    alarm_enabled = true;
    alarm_snoozed[i] = true;
    delay(1000);
    display.clearDisplay();
    return;
}

void ring_alarm(int alarm) {
  //
  alarm_triggered[alarm] = true;
  display.clearDisplay();
  print_line("Medicine Time", 1, 40, 0);

  digitalWrite(LED_1, HIGH);
  int count = 0;
  while (digitalRead(CANCEL) == HIGH) {
    for (int i = 0; i < n_notes; i++) {
      if (digitalRead(CANCEL) == LOW) {
        Serial.println("Alarm off");
        alarm_enabled = false;
        enable_Alrm[alarm] = false;
        alarm_triggered[alarm] = true;
        noTone(BUZZER);  
        delay(200);
        digitalWrite(LED_1, LOW);
        return;
      }
      tone(BUZZER, notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(2);
      if(!alarm_snoozed[alarm]){
        if(count>5){
          snoozeAlarm(alarm);
          digitalWrite(LED_1, LOW);
        return;
        }
      }
    }
    Serial.println("Alarm on"+String(count));
    count++;
  }

}

void update_time(void) {
    //Complete
    struct tm timeinfo;
    getLocalTime(&timeinfo);

    char year_str[8];
    char month_str[8];
    char day_str[8];
    char hour_str[8];
    char min_str[8];
    char sec_str[8];

    strftime(year_str, 8, "%Y", &timeinfo);  
    strftime(month_str, 8, "%m", &timeinfo); 
    strftime(day_str, 8, "%d", &timeinfo);   
    strftime(sec_str, 8, "%S", &timeinfo);   
    strftime(hour_str, 8, "%H", &timeinfo);  
    strftime(min_str, 8, "%M", &timeinfo);   

    year = atoi(year_str);
    month = atoi(month_str);
    days = atoi(day_str);
    hours = atoi(hour_str);
    minutes = atoi(min_str);
    seconds = atoi(sec_str);

    if (minutes >= 60) {
      minutes -= 60;
      hours += 1;
    }
    if (hours >= 24) {
      hours -= 24;
      days += 1; 
    }
}

void update_time_with_check_alarm(){
  update_time();
  if(enable_Alrm[0]==false && enable_Alrm[1]==false){
    alarm_enabled == false;
  }
  if(alarm_enabled){
    for (int i = 0; i < n_alarms; i++)
    {
      if(alarm_triggered[i]==false && hours == alarm_hours[i] && minutes == alarm_minutes[i]){
        Serial.println("Alram  ON");
        ring_alarm(i); 
      }
    }
  }
}

int wait_for_button_press(){
  //Complete
  while (true){
    if(digitalRead(UP)==LOW){
      delay(200);
      return UP;
    }
    else if(digitalRead(DOWN)==LOW){
      delay(200);
      return DOWN;
    }
    else if(digitalRead(CANCEL)==LOW){
      delay(200);
      return CANCEL;
    }
    else if(digitalRead(OK)==LOW){
      delay(200);
      return OK;
    }
    update_time();
  }
}


void set_time(){
  //Complete
  int temp_hour = 5;
  while(true){
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour),0,0,2);

    int pressed = wait_for_button_press();
    if(pressed == UP){
      delay(20);
      temp_hour ++;
      if(temp_hour>12){
        temp_hour = -12;
      }
    }
    else if(pressed == DOWN){
      delay(20);
      temp_hour -= 1;
      if(temp_hour<-12){
        temp_hour = 12;
      }
    }
    else if(pressed == OK){
      delay(200);
      break;
    }
    else if(pressed = CANCEL){
      delay(200);
      return;
    }
  }
  int temp_minute = 30;
  while(true){
    display.clearDisplay();
    print_line("Enter minute: "+String(temp_minute),0,0,2);

    int pressed = wait_for_button_press();
    if(pressed == UP){
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }
    else if(pressed == DOWN){
      delay(200);
      temp_minute -= 1;
      if(temp_minute<0){
        temp_minute = 59;
      }
    }
    else if(pressed == OK){
      delay(200);
      break;
    }
    else if(pressed == CANCEL){
      delay(200);
      return;
    }
  }
  int offSec = 0;
  if(temp_hour<0){
    offSec=temp_hour*60*60-temp_minute*60;
  }
  else{
    offSec=temp_hour*60*60+temp_minute*60;
  }
  configTime(offSec, UTC_OFFSET_DST, NTP_SERVER);
  update_time();
  display.clearDisplay();
  print_line("Time is set",0,0,2);
  delay(1000);
  display.clearDisplay();
  print_time_now();
  delay(1000);
  display.clearDisplay();
  return;
}

void set_alarm(int alarm){
  //Complete
  int temp_hour = hours;//Has to change
  while(true){
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour),0,0,2);

    int pressed = wait_for_button_press();
    if(pressed == UP){
      delay(200);
      temp_hour ++;
      temp_hour = temp_hour % 24;
    }
    else if(pressed == DOWN){
      delay(200);
      temp_hour -= 1;
      if(temp_hour<0){
        temp_hour = 23;
      }
    }
    else if(pressed == OK){
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;
    }
    else if(pressed = CANCEL){
      display.clearDisplay();
      print_line("Set Alarm Canceled! ",0,0,2);
      delay(200);
      return;
    }
  }
  int temp_minute = minutes;//has to change
  while(true){
    display.clearDisplay();
    print_line("Enter minute: "+String(temp_minute),0,0,2);

    int pressed = wait_for_button_press();
    if(pressed ==UP){
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }
    else if(pressed == DOWN){
      delay(200);
      temp_minute -= 1;
      if(temp_minute<0){
        temp_minute = 59;
      }
    }
    else if(pressed == OK){
      delay(200);
      alarm_minutes[alarm] = temp_minute;
      break;
    }
    else if(pressed == CANCEL){
      display.clearDisplay();
      print_line("Set Time Canceled! ",0,0,2);
      delay(200);
      return;
    }
  }
  enable_Alrm[alarm] = true;
  alarm_enabled = true;
  alarm_triggered[alarm] = false;
  alarm_snoozed[alarm] = false;
  display.clearDisplay();
  Serial.println("Alarm "+String(alarm+1)+" "+alarm_hours[alarm]+":"+alarm_minutes[alarm]+" SET");
  print_line("Alarm  "+String(alarm)+" "+alarm_hours[alarm]+":"+alarm_minutes[alarm]+" SET",0,0,2);
  delay(1000);
  display.clearDisplay();
}
void viewAlarms() {
  while (true) {
    display.clearDisplay();
 
    for (int i = 0; i < n_alarms; i++) {
      String alarmString;
      if (enable_Alrm[i]) {
        alarmString = "Alarm " + String(i + 1) + ": " + String(alarm_hours[i]) + ":" + String(alarm_minutes[i]);
      } else {
        alarmString = "Alarm " + String(i + 1) + ": OFF";
      }
      print_line(alarmString, 1, i * 10, 0);
    }

    if (digitalRead(OK) == LOW) {
      delay(200);
      display.clearDisplay();
      print_line("Exiting Alarm View", 1, 40, 0);
      delay(1000);
      return;
    }

    if (digitalRead(CANCEL) == LOW) {
      delay(200);
      display.clearDisplay();
      print_line("Delete Alarms !", 1, 10, 0);

      if (enable_Alrm[0]) {
        print_line("Alarm 1 - Press Up to Delete", 1, 25, 0);
      }
      if (enable_Alrm[1]) {
        print_line("Alarm 2 - Press Down to Delete", 1, 45, 0);
      }
      if (!enable_Alrm[0] && !enable_Alrm[1]) {
        print_line("No Alarms to Delete.", 1, 40, 0);
      }

      int press = wait_for_button_press(); 

      if (press == CANCEL) {
        display.clearDisplay();
        print_line("Exiting Alarm View", 1, 40, 0);
        return;
      }

      if (press == UP && enable_Alrm[0]) {
        display.clearDisplay();
        enable_Alrm[0] = false;
        alarm_snoozed[0] = false;
        print_line("Alarm 1 Deleted", 1, 10, 0);
        delay(1000);
      }
      if (press == DOWN && enable_Alrm[1]) {
        display.clearDisplay();
        enable_Alrm[1] = false;
        alarm_snoozed[1] = false;
        print_line("Alarm 2 Deleted", 1, 30, 0);
        delay(1000);
      }
    }
    delay(5);
  }
}



void run_mode(int mode){
if(mode ==0){
    Serial.println("Set Time");
    set_time();
  }
  else if(mode == 1 || mode ==2){
    set_alarm(mode - 1);
  }
  else if(mode == 3){
    viewAlarms();
  }
}

void go_to_menu() {
  //Complete
  while (digitalRead(CANCEL) == HIGH) {
    update_time_with_check_alarm();
    display.clearDisplay();
    print_line(options[current_mode-1], 1, 0, 0); 

    int pressed = wait_for_button_press();

    if (pressed == UP) {
      current_mode++;  
      if (current_mode > max_modes) {
        current_mode = 1; 
      }
      delay(20);  
    } 
    else if (pressed == DOWN) {
      current_mode--;  
      if (current_mode < 1) {
        current_mode = max_modes;
      }
      delay(20);
    } 
    else if (pressed == OK) {
      Serial.print("Selected Mode: ");
      Serial.println(current_mode);  
      delay(200);
      run_mode(current_mode-1);
    }
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(CANCEL, INPUT);
  pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(OK, INPUT);

  dhtSensor.setup(DHT,DHTesp::DHT22);


  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  
  display.display();
  delay(2000);


  display.clearDisplay();
  print_line("Welcome to Medibox",1,40,10);
  delay(3000);


  
  display.clearDisplay();

  WiFi.begin("Wokwi-GUEST", "");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    print_line("Connecting to WiFi...",1,0,0);
  }
  display.clearDisplay();
  print_line("WiFi Connected", 1, 0, 0);
  delay(2000);

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);

  client.setServer(mqtt_server, 1883);  // Default MQTT port
  client.setCallback(callback);
  client.publish("Medibox/intensity", String(0).c_str());
  servo.attach(SERVO);
  servo.write(tita);
}

bool Sample_intensity(){
  //complete
  timeNow = millis();
  //Serial.println("Time Now: "+String(timeNow-timeLast)+"Ts : "+String(Ts*1000));
  if (timeNow - timeLast >= Ts*1000) {
    timeLast = timeNow;
    //Serial.println("Time to sample intensity");
    return true;
  }
  return false;
}

bool Send_avg_intensity(){
  //complete
  timeNow = millis();
  int thresholdTime = Tu*60000;
  //Serial.println(" Time Now: "+String(timeNow-timelastmin)+" Tu : "+String(thresholdTime));
  if (timeNow - timelastmin >= thresholdTime) {
    timelastmin = timeNow;
    //Serial.println("Time to send avg intensity");
    return true;
  }
  return false;
}

void check_intensity(){
  //complete
  if(Send_avg_intensity()){
    //display in NODE RED
    //Serial.println("Intensity Count: "+String(intensitycount));
    float avgIntensity = cumuIntensity/intensitycount;
    client.publish("Medibox/intensity", String(avgIntensity,2).c_str());
    intensitycount = 0;
    cumuIntensity = 0;
  }
  if(Sample_intensity()){
    //Serial.println("Sampling Intensity");
    intensity = float((analogRead(LDR)-4063))/float(-4031);
    //Serial.println("Intensity: "+String(intensity));
    cumuIntensity += intensity;
    intensitycount++;
  }
}
void control_servo(){
  //Complete
  Serial.println("tita offset: "+String(titaoffset));
  Serial.println("intensity: "+String(intensity));
  Serial.println("temp: "+String(temp));
  Serial.println("tempmed: "+String(tempmed));
  Serial.println("gammaVal: "+String(gammaVal));
  Serial.println("Ts: "+String(Ts));
  Serial.println("Tu: "+String(Tu));
  Serial.println("tita: "+String(tita));
  int rawtita = titaoffset + (180 - titaoffset) *intensity*gammaVal*log(float(Ts)/float(Tu*60))*(float(temp)/float(tempmed));
  tita = constrain(rawtita, 0, 180);
  Serial.println("Servo Position: "+String(tita));
  
  servo.attach(SERVO);
  servo.write(tita);
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  update_time_with_check_alarm();//cant we use a thred here
  print_time_now();
  check_intensity();
  check_temp();
  control_servo();
  if(enable_Alrm[0]){
    print_line("Alarm 1 ON ",1,45,60);
     }
  if(enable_Alrm[1]){
    print_line("Alarm 2 ON",1,55,60);
  }
  if(digitalRead(CANCEL)==LOW){
    Serial.println("MENU");
    delay(1000);
    go_to_menu();
  }
  delay(20);
  client.loop(); 
}
