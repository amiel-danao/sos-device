#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include "OneButton.h"
// #include <CStringBuilder.h>

#define rxPin 6
#define txPin 5
#define BUTTON1_PIN 2
#define BUTTON2_PIN 8
#define FLASHLIGHT_PIN 7
#define ALARM_PIN 9

SoftwareSerial A9modem(rxPin, txPin);  // Pins D6 Rx and D5 Tx are used as used as software serial pins
TinyGPSPlus gps;

const String defaultMessage = "Please help me, I'm currently located at ";
String incomingData;
String tempData;
const char sample[] = "Please help me this is an emergency! I'm located at www.google.com/maps/place/14.32,120.98";

OneButton button1(BUTTON1_PIN, false);
OneButton button2(BUTTON2_PIN, false);

int flashLightState = LOW;
int alarmState = LOW;

void setup() {

  Serial.begin(115200);   // Baud rate for serial monitor
  A9modem.begin(115200);  // Baud rate for GSM shield

  pinMode(LED_BUILTIN, OUTPUT);  // Use builtin LED for correct GPS status
  pinMode(FLASHLIGHT_PIN, OUTPUT);
  pinMode(ALARM_PIN, OUTPUT);
  button1.attachClick(toggleFlashLight);
  button2.attachClick(toggleAlarm);
  button2.attachLongPressStop(sendLocation);

  // Serial.println("started");
  // A9modem.println("AT+GPS=1");
  // delay(1000);
  // A9modem.println("AT+GPSMD=2\r");   // Change to only GPS mode from GPS+BDS, set to 2 to revert to default.
  // delay(1000);
  // A9modem.println("AT+GPSRD=5\r");

  // delay(1000);
  // // Set SMS mode to text mode
  // A9modem.print("AT+CMGF=1\r");
  // delay(1000);
  
  // // Set GSM module to TP show the output on serial out
  // A9modem.print("AT+CNMI=2,2,0,0,0\r"); 
  // delay(1000);

  // A9modem.println("AT+CREG=2\r");
  // delay(6000);

  // A9modem.println("AT+CGATT=1\r");
  // delay(6000);

  // A9modem.println("AT+CGDCONT=1,\"IP\",\"internet\"\r");
  // delay(6000);

  // A9modem.println("AT+CGACT=1,1\r");
  // delay(6000);
  Serial.println("Done initialization");
}

void toggleFlashLight(){
  flashLightState = !flashLightState;
  Serial.println("Flashlight toggled : " + String(flashLightState));
  digitalWrite(FLASHLIGHT_PIN, flashLightState);
}

void toggleAlarm(){
  alarmState = !alarmState;
  Serial.println("Alarm toggled : " + String(alarmState));
  if(alarmState){
    for (float f=3000;f>40;f=f*0.93){
      tone(ALARM_PIN, f);
      delay(10);
    }
  }
  else{
    noTone(ALARM_PIN);
  }
}

void loop2(){
  

  while (A9modem.available() > 0)
  {
    Serial.print((char)A9modem.read());
    // String tokens = A9modem.readString();    
  }
}

void sendLocation(){
  if(incomingData.length() > 0) {  // If the location string is correct send SMS
    // if (isdigit(gpsData.charAt(0))) {           // Check if the stream starts with a number
      SendSMS();
      digitalWrite(LED_BUILTIN, HIGH);
      delay(300);
      digitalWrite(LED_BUILTIN, LOW);
      delay(300);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(300);
      digitalWrite(LED_BUILTIN, LOW);  
      incomingData = "";
      tempData = "";
    // } 
  }
}


void loop() {

  button1.tick();
  button2.tick();

  delay(10);

  if (A9modem.available() > 0)
  {
    String tokens = A9modem.readString();
    Serial.println(tokens);
    if(tokens.length() > 0 && tokens.indexOf("$G") > 0){
      
      String token;
      float latitude = 0;
      float longitude = 0;
        uint8_t token_idx = 0;
        while (get_token(tokens, token, token_idx, ','))
        {
          // Serial.print("Token[");
          // Serial.print(token_idx);
          // Serial.print("] = \"");
          // Serial.print(token);
          // Serial.println("\"");
          if(token_idx == 2){
           
            if(tokens.indexOf("N,") > 0){
              latitude = GpsToDecimalDegrees(token.c_str(), 'N');
            }
            else{
              latitude = GpsToDecimalDegrees(token.c_str(), 'S');
            }
            tempData += String(latitude, 6);
          }
          if(token_idx == 4){
            tempData += ",";
            
            if(tokens.indexOf("W,") > 0){
              longitude = GpsToDecimalDegrees(token.c_str(), 'W');
            }
            else{
              longitude = GpsToDecimalDegrees(token.c_str(), 'E');
            }
            tempData += String(longitude, 6);
          }
          token_idx++;
        }

        if(latitude == 0 || longitude == 0){
          incomingData = "";
          // memset(incomingData, '', 128);
        }
        else{
          // sb.print(F("www.google.com/maps/dir/"));
          // sb.print(tempData);
          // sb.print(F("/"));
          incomingData = "www.google.com/maps/dir/";
          incomingData += tempData;  // Create the SMS string
        }
      Serial.println(incomingData); 
    }
    // else{
    //   Serial.print(temp);
    // }
    // delay(500); 
  }

  
}


bool get_token(String &from, String &to, uint8_t index, char separator)
{
  uint16_t start = 0, idx = 0;
  uint8_t cur = 0;
  while (idx < from.length())
  {
    if (from.charAt(idx) == separator)
    {
      if (cur == index)
      {
        to = from.substring(start, idx);
        return true;
      }
      cur++;
      while ((idx < from.length() - 1) && (from.charAt(idx + 1) == separator)) idx++;
      start = idx + 1;
    }
    idx++;
  }
  if ((cur == index) && (start < from.length()))
  {
    to = from.substring(start, from.length());
    return true;
  }
  return false;
}

/**
 * Convert NMEA absolute position to decimal degrees
 * "ddmm.mmmm" or "dddmm.mmmm" really is D+M/60,
 * then negated if quadrant is 'W' or 'S'
 */
float GpsToDecimalDegrees(const char* nmeaPos, char quadrant)
{
  float v= 0;
  if(strlen(nmeaPos)>5)
  {
    char integerPart[3+1];
    int digitCount= (nmeaPos[4]=='.' ? 2 : 3);
    memcpy(integerPart, nmeaPos, digitCount);
    integerPart[digitCount]= 0;
    nmeaPos+= digitCount;
    v= atoi(integerPart) + atof(nmeaPos)/60.;
    if(quadrant=='W' || quadrant=='S')
      v= -v;
  }
  return v;
}

void SendSMS()
{                     
  // A9modem.println("AT+CMGF=1\r");
  // delay(1000);
  // A9modem.println("AT+CNMI=2,2,0,0,0\r");
  // delay(1000);
  A9modem.print("AT+CMGS=\"+639954261220\"\r");  // Replace your mobile number here
  delay(1000);

  // A9modem.print("LOL");
  A9modem.print(defaultMessage + tempData);
  // A9modem.print(incomingData);
  // delay(5000);
  // delay(1000);
  A9modem.write(0x1A);
  // A9modem.println((char)26);          // ASCII code of CTRL+Z
  delay(5000);
  Serial.println("message was sent");

  
  // delay(1000);
  // A9modem.println("ATD+639954261220");
}

