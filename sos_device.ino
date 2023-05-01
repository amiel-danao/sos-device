#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <pdulib.h>
// #include <CStringBuilder.h>

#define rxPin 6
#define txPin 5

SoftwareSerial A9modem(rxPin, txPin);  // Pins D7 Rx and D8 Tx are used as used as software serial pins
TinyGPSPlus gps;


const String defaultMessage = "Please help me, this is my current location www.google.com/maps/dir/14.32,120.98/";
// char incomingData[128];  // For storing incoming serial data
// CStringBuilder sb(incomingData, sizeof(incomingData));
String incomingData;
const char *Target = "+639973995372"; 
#define BUFFER_LENGTH 100
PDU mypdu = PDU(BUFFER_LENGTH);
char temp[30];


void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);  // Use builtin LED for correct GPS status
  
  Serial.begin(115200);   // Baud rate for serial monitor
  A9modem.begin(115200);  // Baud rate for GSM shield

  Serial.println("started");
  A9modem.println("AT+GPS=1");
  delay(1000);
  A9modem.println("AT+GPSMD=2\r");   // Change to only GPS mode from GPS+BDS, set to 2 to revert to default.
  delay(1000);
  // A9modem.println("AT+GPSRD=5\r");

  delay(1000);
  // Set SMS mode to text mode
  // A9modem.print("AT+CMGF=1\r");
  // delay(1000);
  
  // Set GSM module to TP show the output on serial out
  A9modem.print("AT+CNMI=2,2,0,0,0\r"); 
  delay(1000);

  A9modem.println("AT+CREG=2\r");
  delay(6000);

  //A9modem.print("AT+CREG?\r");
  A9modem.println("AT+CGATT=1\r");
  delay(6000);

  A9modem.println("AT+CGDCONT=1,\"IP\",\"WWW\"\r");
  delay(6000);

  // A9modem.println("AT+LOCATION=1\r");
  A9modem.println("AT+CGACT=1,1\r");
  delay(6000);
  Serial.println("Done initialization");

  // SendSMS();
}

void loop(){
  if (A9modem.available() > 0)
  {
    String tokens = A9modem.readString();
    Serial.println(tokens);
    A9modem.println("AT+CMGF=1\r");
    delay(1000);
    A9modem.println("AT+CNMI=2,2,0,0,0\r");
    delay(1000);
    // char sample[] = "https://goo.gl/maps/eNVqDopP9cVBC4tM6";
    char sample[] = "Please help me this is an emergency!";
    // char sample[] = "LOL Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry\'s standard dummy text ever since the 1500s";
    
    // int len = mypdu.encodePDU(Target, sample);
    // sprintf(temp,"AT+CMGS=%d\r",len);
    // A9modem.print(temp);
    A9modem.print("AT+CMGS=\"+639954261220\"\r");  // Replace your mobile number here
    delay(1000);

    A9modem.print(sample);
    // A9modem.print("Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry\'s standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum");
    
    
    // A9modem.print(mypdu.getSMS());
    // A9modem.print(incomingData);
    delay(5000);
    // delay(200);
    A9modem.write(0x1A);
    // A9modem.println((char)26);          // ASCII code of CTRL+Z
    delay(10000);
  }
}

void loop2() {

  if (A9modem.available() > 0)
  {
    String tokens = A9modem.readString();
    Serial.println(tokens);
    if(tokens.length() > 0 && tokens.indexOf("$G") > 0){
      
      String token;
      float latitude = 0;
      float longitude = 0;
        uint8_t token_idx = 0;
        String tempData;
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
            tempData += String(latitude);
          }
          if(token_idx == 4){
            tempData += ",";
            
            if(tokens.indexOf("W,") > 0){
              longitude = GpsToDecimalDegrees(token.c_str(), 'W');
            }
            else{
              longitude = GpsToDecimalDegrees(token.c_str(), 'E');
            }
            tempData += String(longitude);
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
          incomingData += "/";
        }
      Serial.println(incomingData); 
    }
    // else{
    //   Serial.print(temp);
    // }
    // delay(500); 
  }

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
    // } 
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
  A9modem.print(defaultMessage);
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

