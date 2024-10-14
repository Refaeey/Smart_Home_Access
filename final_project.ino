#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <SPI.h>     
#include <MFRC522.h>    
#include <Adafruit_Fingerprint.h> 
#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);

#define SS_PIN 5
#define RST_PIN 4
MFRC522 rfid(SS_PIN, RST_PIN); 

LiquidCrystal_I2C Lcd(0x27, 16, 2);
const byte rows = 4; 
const byte cols = 4; 

char keys[rows][cols] = {
  {'1', '2', '3' , 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[rows] = {33,32,25,26}; 
byte colPins[cols] = {27, 14,12,13}; 
Keypad keypad ( makeKeymap(keys), rowPins, colPins, rows, cols );

const String pass = "1234";
String input_password;
int count = 0;
byte relay=34, wrong_attempt=0;
byte x=0; 

const char* ssid = "WE7EEDBC";
const char* password = "bc028919";
String phoneNumber = "+201094658897";
String apiKey = "3087279";

void sendWAMessage(String message){
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" +           urlEncode(message);
  HTTPClient http;
 http.begin(url);
 http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
 }
  http.end();
}

void setup() {
  finger.begin(57600); 	
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  input_password.reserve(8); 
  Lcd.init();
  Lcd.backlight();
  Lcd.print("Enter Password");

  pinMode(relay,1);
  digitalWrite(relay,0);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  if(x==0){keypad1(); RFID(); FingerPrint();}
  else if(x==1){keypad1(); RFID(); }
  else if(x==2) { FingerPrint(); }
 
 if(wrong_attempt==3){ 
 Lcd.clear();
 Lcd.print("calling admin");
 sendWAMessage("Strange Access Attempt");
 wrong_attempt=0;
 delay(5000);
 Lcd.clear();
 Lcd.print("Enter Password");
 }


}

void keypad1(){
 char key = keypad.getKey();
 if (key){ x=1;
  if (key == '*') 
     { if (count > 0){ 
        count--;
        input_password.remove(count);
       Lcd.setCursor(count, 1);
       Lcd.print (" ");} }
  else if (key == '#') 
    { if (pass == input_password){  x=0;
          digitalWrite(relay,1);
          Lcd.clear();
          Lcd.print("pass is correct");
          delay(2000);
          digitalWrite(relay,0);
          Lcd.clear();
          Lcd.print("Enter Password");
          Lcd.setCursor(0, 1);}
      else { x=0; wrong_attempt++;
              Lcd.clear();
              Lcd.print("Wrong Password");
              delay(2000);
              Lcd.clear();
              Lcd.print("Enter Password");
              Lcd.setCursor(0, 1); 
         }
         input_password = ""; 
         count = 0;
    }
  else{
      input_password += key; 
      Lcd.setCursor(count, 1);
      Lcd.print(key);
      count++; }
  }
}

void RFID()
{
  if (rfid.PICC_IsNewCardPresent() &&
    rfid.PICC_ReadCardSerial()) { x=1;
  String UID="";
  for (byte i=0;i<rfid.uid.size;i++) 
  { UID.concat(String(rfid.uid.uidByte[i]< 0x10 ? " 0" : " "));  
    UID.concat(String(rfid.uid.uidByte[i], HEX));}
  UID.toUpperCase();
  if (UID.substring(1) == "62 C1 C5 1F") //change here
  { x=0;
    digitalWrite(relay,1);
    Lcd.clear();
    Lcd.print("ST Smart");
    delay(2000);
    Lcd.clear();
    digitalWrite(relay,0);
    Lcd.print("Enter Password");
    Lcd.setCursor(0, 1); }
  else{ x=0; wrong_attempt++;
        Lcd.clear();
        Lcd.print("wrong ID card");
        delay(2000);
        Lcd.clear();
        Lcd.print("Enter Password");
        Lcd.setCursor(0, 1);}
     rfid.PICC_HaltA();
     rfid.PCD_StopCrypto1();

  }
}

void FingerPrint() { 
  uint8_t p = finger.getImage();
  switch (p) {
     case FINGERPRINT_OK:
            x=2;
     break;
     default:  
         return;
  }
  p = finger.image2Tz();
  switch (p) {
     case FINGERPRINT_OK: 
        break;
     default:  
        return; 
  }
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {x=0;
          digitalWrite(relay,1);
          Lcd.clear();
          Lcd.print("Access");
          delay(2000);
          Lcd.clear();
          digitalWrite(relay,0);
          Lcd.print("Enter Password");
          Lcd.setCursor(0, 1);
  }
 else if (p == FINGERPRINT_NOTFOUND) {
                wrong_attempt++; x=0;
                Lcd.clear();
                Lcd.print("Try Again");
                delay(2000);
                Lcd.clear();
                Lcd.print("Enter Password");
                Lcd.setCursor(0, 1);
}}