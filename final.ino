#include <LiquidCrystal.h>
#include "HX711.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#define STASSID "xxx"
#define STAPSK "xxx"
#include <string>
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
const int backlight = 8;
const int LOADCELL_DOUT_PIN = 19;
const int LOADCELL_SCK_PIN = 18;
const int BTN1_PIN = 13;
const int BTN2_PIN = 14;
const int BTN3_PIN = 15;
const char *ssid = STASSID;
const char *pass = STAPSK;
int buttonOneState = 0;
int buttonTwoState = 0;
int buttonThreeState = 0;
int lastButtonThreeState = 0;
int onoff = 0;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
WiFiMulti WiFiMulti;
HX711 scale;
String x = "4056489647126";
float weight = 0;
float protein = 0;
float fat = 0;
float carbs = 0;


void setup() {
  Serial.begin(115200);
    for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, pass);
    if (WiFiMulti.run() != WL_CONNECTED) {
    Serial.println("Unable to connect to network, rebooting in 10 seconds...");
    delay(10000);
    rp2040.reboot();
  }
  pinMode(BTN1_PIN, INPUT);
  pinMode(BTN2_PIN, INPUT);
  pinMode(BTN3_PIN, INPUT);
  pinMode(backlight, OUTPUT);
  lcd.begin(16, 2);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.tare();
  scale.set_scale(410.f);
  lcd.setCursor(0, 0);
  lcd.setCursor(0, 1);
  
  
}

void loop() {
  buttonOneState = digitalRead(BTN1_PIN);
  buttonTwoState = digitalRead(BTN2_PIN);
  buttonThreeState = digitalRead(BTN3_PIN);
  
  if(buttonOneState==HIGH){
    JsonDocument values = getValues(x);
    protein = values["product"]["nutriments"]["proteins_100g"];
    carbs = values["product"]["nutriments"]["carbohydrates_100g"];
    fat = values["product"]["nutriments"]["fat_100g"];
    Serial.println(protein);
    Serial.println(carbs);
    Serial.println(fat);
    // scale.tare();
    lcd.clear();
    weight = scale.get_units(10);
    Serial.println(weight);
    protein = (protein/100)*weight;
    carbs = (carbs/100)*weight;
    fat = (fat/100)*weight;
    fat = round(fat*10)/10;
    carbs = round(carbs*10)/10;
    protein = round(protein*10)/10;
    Serial.println(protein);
    Serial.println(carbs);
    Serial.println(fat);
    lcd.print(weight);
    lcd.setCursor(5, 0);
    lcd.print("g");
    lcd.setCursor(7, 0);
    lcd.print(protein);
    lcd.setCursor(14, 0);
    lcd.print("P");
    lcd.setCursor(0, 1);
    lcd.print(carbs);
    lcd.setCursor(5, 1);
    lcd.print("C");
    lcd.setCursor(7, 1);
    lcd.print(fat);
    lcd.setCursor(13, 1);
    lcd.print("F");
    // lcd.setCursor(0,1); 
    delay(200);
  }
  
  


  if(buttonTwoState==HIGH){
    scale.tare();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write("0");
    delay(200);
    
  }
    if(lastButtonThreeState == 0 && buttonThreeState==HIGH){
      if(onoff==0)
      {
        digitalWrite(backlight, HIGH);
        onoff=1;
      }
      else{
        digitalWrite(backlight, LOW);
        onoff = 0;
      }
    
  }
  lastButtonThreeState = buttonThreeState;
  delay(200);


}
JsonDocument getValues(String barcode){
  HTTPClient client;
  JsonDocument doc;
  String address = "https://world.openfoodfacts.net/api/v2/product/" + barcode;
  client.setInsecure();
  if (client.begin(address)) {  // HTTPS

    Serial.print("[HTTPS] GET...\n");
    // start connection and send HTTP header
    int httpCode = client.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = client.getString();
        
        deserializeJson(doc,payload);
        return doc;

      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", client.errorToString(httpCode).c_str());
      rp2040.reboot();
    }
    
    client.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
    rp2040.reboot();
    exit(0);
  }


  exit(0);
}
