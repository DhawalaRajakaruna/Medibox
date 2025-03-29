#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <time.h>
#include <DHTesp.h>


#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET 0
#define UTC_OFFSET_DST 0

#define SCREEN_WIDTH 128  // OLED width
#define SCREEN_HEIGHT 64  // OLED height
#define OLED_RESET    -1  // Reset pin (not used)
#define SCREEN_ADDRESS 0x3C // I2C address for the screen

#define BUZZER 18
#define LED_1 15
#define LED_2 2
#define CANCEL 34
#define UP 35
#define DOWN 32
#define OK 33
#define DHT 12


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

int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

bool alarm_enabled = false;
int n_alarms = 2;
int alarm_hours[] = {0,0};
int alarm_minutes[] = {1,10};
bool alarm_triggered[] = {false,false};

unsigned long timeNow = 0;
unsigned long timeLast = 0;

int current_mode = 0;
int max_modes = 4;
String options[] = {"1 - Set Time","2 - Set Alarm 1","3 - Set Alrm 2","4 - Remove Alarm"};

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

const int LED = 23;
void print_line(String text, int text_size,int row,int column){
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column,row);
  display.println(text);
  display.display();
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

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C is the I2C address
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  
  display.display();
  delay(2000);


  display.clearDisplay();
  print_line("Welcome to Medibox",2,0,0);
  delay(3000);
  display.clearDisplay();

  print_line("Connecting to WiFi",2,0,0);
  display.clearDisplay();
  WiFi.begin("Wokwi-GUEST", "");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    print_line(".",2,0,0);
  }
  print_line("WiFi Connected", 2, 0, 0);
  delay(2000);               // Update screen
}

void loop() {
  // No need to refresh text repeatedly
}
