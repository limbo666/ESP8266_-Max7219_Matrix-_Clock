
// Original code taken from techlogics.net
// https://techlogics.net/esp8266-clock-with-max7219-matrix-display-date-time-display/
// The code was heavily changed to meet personal requirements and changed in order to be remotely manageable.


#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>


#define EEPROM_SIZE 512
#define TEXT_SIZE 20

WiFiClient client;


// Define the UDP port
const int udpPort = 19744;

// Create a UDP instance
WiFiUDP udp;

String date;

WiFiUDP udpServer;
char udpBuffer[255];

int myIntensity = 5;

long clockInterval = 30000;
long holdInterval = 7000;

#define NUM_MAX 4
#define LINE_WIDTH 16
#define ROTATE  0

// for NodeMCU 1.0
#define DIN_PIN 15  // D8
#define CS_PIN  13  // D7
#define CLK_PIN 12  // D6

#include "max7219.h"
#include "fonts.h"


// =======================================================================
// CHANGE YOUR CONFIG HERE:
// =======================================================================
const char* ssid1 = "SSID_1";  // SSID of the second Wi-Fi network
const char* password1 = "Pass_1"; // Password for the second Wi-Fi network
const char* ssid2 = "SSID2";       // SSID of the first Wi-Fi network
const char* password2 = "Pass_2"; // Password for the first Wi-Fi network
float utcOffset = 3; // Time Zone setting
// =======================================================================

void setup()
{
    Serial.begin(115200);

    EEPROM.begin(EEPROM_SIZE);
    // Save a text value
    // saveText(0, "Test EEPROM");

    initMAX7219();
    sendCmdAll(CMD_SHUTDOWN, 1);
    // sendCmdAll(CMD_INTENSITY, 10); // Adjust the brightness between 0 and 15
    Serial.println("Startup ");
    printStringWithShift("ver 1.2", 16);
    delay(1500);

    Serial.print("Connecting WiFi ");

    // Attempt to connect to the first Wi-Fi network
    WiFi.begin(ssid1, password1);
    printStringWithShift("   WiFi  ", 16);

    // Wait for the first connection attempt to complete
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
        Serial.print(".");
    }

    // If the first connection attempt failed or the connection was not established within 10 seconds
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nConnection to the first Wi-Fi network failed. Trying the second network.");
        WiFi.disconnect(); // Disconnect from the first network
        delay(1000); // Wait for the disconnect to complete
        WiFi.begin(ssid2, password2); // Attempt to connect to the second Wi-Fi network
        startTime = millis(); // Reset the start time for the second connection attempt

        // Wait for the second connection attempt to complete
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
            delay(500);
            Serial.print(".");
        }
    }

    Serial.println("");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("Connected: "); Serial.println(WiFi.localIP());
    }
    else {
        Serial.println("Connection to the second Wi-Fi network failed as well.");
    }


  udpServer.begin(19744);
  Serial.println("UDP server started on port 19744");
  
 



  //// Read and print the text value
  //char textValue[TEXT_SIZE];
  //readText(0, textValue);
  //Serial.begin(9600);
  //Serial.print("Text value read from EEPROM: ");
  //Serial.println(textValue);


// Read the text from EEPROM and extract the value of myIntensity
  char textValue[TEXT_SIZE];
  readText(0, textValue);

  // Check if the text starts with "Intensity:"
  String textString2(textValue);
  if (textString2.startsWith("Intensity:")) {
      // Extract the value after the prefix
      String numString = textString2.substring(10);
      int extractedNum = numString.toInt();

      // Assign the extracted value to myIntensity
      myIntensity = extractedNum;
      Serial.println("myIntensity is set to " + String(myIntensity));
 
      delay(1500);

  }


  // Check if the text starts with "clockInterval:"
  readText(100, textValue);
  String textString(textValue);
  if (textString.startsWith("clockInterval:")) {
      // Extract the value after the prefix
      String numString = textString.substring(14);
      long extractedNum = numString.toInt();

      // Assign the extracted value to clockInterval
      clockInterval = extractedNum;
      Serial.println("clockInterval is set to " + String(clockInterval));
  }


  // Check if the text starts with "holdInterval:"
  readText(200, textValue);
  String textString3(textValue);
  if (textString3.startsWith("holdInterval:")) {
      // Extract the value after the prefix
      String numString = textString3.substring(13);
      long extractedNum = numString.toInt();

      // Assign the extracted value to clockInterval
      holdInterval = extractedNum;
      Serial.println("holdInterval is set to " + String(holdInterval));
  }

  // Check if the text starts with "utcOffset"
  readText(300, textValue);
  String textString4(textValue);
  if (textString4.startsWith("utcOffset:")) {
      // Extract the value after the prefix
      String numString = textString4.substring(10);
      long extractedNum = numString.toInt();

      // Assign the extracted value to clockInterval
      utcOffset = extractedNum;
      Serial.println("utcOffset is set to " + String(utcOffset));
      if (utcOffset == 3)     {
          Serial.println("Summer time ");
      
      }
      else if (utcOffset == 2) {
          Serial.println("Winter time ");
      }
  }


  // the UDP client
  udp.begin(udpPort);
  // Send the broadcast message
  udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
  String ipAddressString = WiFi.localIP().toString();
  udp.print("System is up and running. IP " + ipAddressString);
  udp.endPacket();
  // Print the broadcast message to Serial (optional)

  //Serial.println("Broadcasted value");
     
  
      Serial.println("System is up and running ");

}
// =======================================================================
#define MAX_DIGITS 16
byte dig[MAX_DIGITS]={0};
byte digold[MAX_DIGITS]={0};
byte digtrans[MAX_DIGITS]={0};
int updCnt = 0;
int dots = 0;
long dotTime = 0;
long clkTime = 0;
int dx=0;
int dy=0;
byte del=0;
int h,m,s;
long localEpoc = 0;
long localMillisAtUpdate = 0;

// =======================================================================
void loop()
{
  if(updCnt<=0) { // every 10 scrolls, ~450s=7.5m
    updCnt = 10;
    Serial.println("Getting data ...");
    printStringWithShift("  Wait...",15);
   
    getTime();
    Serial.println("Got internet data");
    clkTime = millis();
  }
 
  if(millis()-clkTime > clockInterval && !del && dots) { // clock for 15s, then scrolls for about 30s
    printStringWithShift(date.c_str(),40);
   delay(7000);
    updCnt--;
    clkTime = millis();
  }
  if(millis()-dotTime > 500) {
    dotTime = millis();
    dots = !dots;
  }
  updateTime();
  showAnimClock();

  // Check for UDP messages
  int packetSize = udpServer.parsePacket();
  if (packetSize) {
      int bytesRead = udpServer.read(udpBuffer, sizeof(udpBuffer) - 1);
      udpBuffer[bytesRead] = '\0'; // Null-terminate the received data
      Serial.print("Received UDP command: ");
      Serial.println(udpBuffer);
      processUDPCommand(udpBuffer);
  }

    
  // Adjusting LED intensity.
  // 12am to 6am, lowest intensity 0
  if ( (h == 0) || ((h >= 1) && (h <= 6)) ) sendCmdAll(CMD_INTENSITY, myIntensity);
  // 6pm to 11pm, intensity = 2
  else if ( (h >=18) && (h <= 23) ) sendCmdAll(CMD_INTENSITY, myIntensity);
  // max brightness during bright daylight
  else sendCmdAll(CMD_INTENSITY, myIntensity);
  
}

// =======================================================================

void showSimpleClock()
{
  dx=dy=0;
  clr();
  //showDigit(h/10,  4, dig7x16);
  //showDigit(h%10,  12, dig7x16);
  showDigit(h / 10, 4, dig7x16);
showDigit(h % 10, 12, dig7x16);
if (h < 10) {
  showDigit(0, 4, dig7x16);  // Display leading zero for single-digit hours
}
  showDigit(m/10, 21, dig7x16);
  showDigit(m%10, 29, dig7x16);
  showDigit(s/10, 38, dig7x16);
  showDigit(s%10, 46, dig7x16);
  setCol(19,dots ? B00100100 : 0);
  setCol(36,dots ? B00100100 : 0);
  refreshAll();
}

// =======================================================================

void showAnimClock()
{
  
  byte digPos[4]={1,8,17,25};
  int digHt = 12;
  int num = 4; 
  int i;
  if(del==0) {
    del = digHt;
    for(i=0; i<num; i++) digold[i] = dig[i];
  //  dig[0] = h/10 ? h/10 : 10;
  //  dig[1] = h%10;
  dig[0] = h / 10 ? h / 10 : 0;  // Assign leading zero for single-digit hours
dig[1] = h % 10;
    dig[2] = m/10;
    dig[3] = m%10;
    for(i=0; i<num; i++)  digtrans[i] = (dig[i]==digold[i]) ? 0 : digHt;
  } else
    del--;
  
  clr();
  for(i=0; i<num; i++) {
    if(digtrans[i]==0) {
      dy=0;
      showDigit(dig[i], digPos[i], dig6x8);
    } else {
      dy = digHt-digtrans[i];
      showDigit(digold[i], digPos[i], dig6x8);
      dy = -digtrans[i];
      showDigit(dig[i], digPos[i], dig6x8);
      digtrans[i]--;
    }
  }
  dy=0;
  setCol(15,dots ? B00100100 : 0);
  refreshAll();
 delay(30);
}

// =======================================================================

void showDigit(char ch, int col, const uint8_t *data)
{
  if(dy<-8 | dy>8) return;
  int len = pgm_read_byte(data);
  int w = pgm_read_byte(data + 1 + ch * len);
  col += dx;
  for (int i = 0; i < w; i++)
    if(col+i>=0 && col+i<8*NUM_MAX) {
      byte v = pgm_read_byte(data + 1 + ch * len + 1 + i);
      if(!dy) scr[col + i] = v; else scr[col + i] |= dy>0 ? v>>dy : v<<-dy;
    }
}

// =======================================================================

void setCol(int col, byte v)
{
  if(dy<-8 | dy>8) return;
  col += dx;
  if(col>=0 && col<8*NUM_MAX)
    if(!dy) scr[col] = v; else scr[col] |= dy>0 ? v>>dy : v<<-dy;
}

// =======================================================================

int showChar(char ch, const uint8_t *data)
{
  int len = pgm_read_byte(data);
  int i,w = pgm_read_byte(data + 1 + ch * len);
  for (i = 0; i < w; i++)
    scr[NUM_MAX*8 + i] = pgm_read_byte(data + 1 + ch * len + 1 + i);
  scr[NUM_MAX*8 + i] = 0;
  return w;
}

// =======================================================================

void printCharWithShift(unsigned char c, int shiftDelay) {
  
  if (c < ' ' || c > '~'+25) return;
  c -= 32;
  int w = showChar(c, font);
  for (int i=0; i<w+1; i++) {
    delay(shiftDelay);
    scrollLeft();
    refreshAll();
  }
}

// =======================================================================

void printStringWithShift(const char* s, int shiftDelay){
  while (*s) {
    printCharWithShift(*s, shiftDelay);
    s++;
  }
}

// =======================================================================



void getTime()
{
  WiFiClient client;
  if (!client.connect("www.google.com", 80)) {
    Serial.println("connection to google failed");
    return;
  }

  client.print(String("GET / HTTP/1.1\r\n") +
               String("Host: www.google.com\r\n") +
               String("Connection: close\r\n\r\n"));
  int repeatCounter = 0;
  while (!client.available() && repeatCounter < 10) {
    delay(500);
    //Serial.println(".");
    repeatCounter++;
    
  }

  String line;
  client.setNoDelay(false);
  while(client.connected() && client.available()) {
    line = client.readStringUntil('\n');
    line.toUpperCase();
    if (line.startsWith("DATE: ")) {
      date = "     "+line.substring(6, 17);
      h = line.substring(23, 25).toInt();
      m = line.substring(26, 28).toInt();
      s = line.substring(29, 31).toInt();
      localMillisAtUpdate = millis();
      localEpoc = (h * 60 * 60 + m * 60 + s);


      
    }
  }
  client.stop();
}

// =======================================================================

void updateTime()
{
  long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
  long epoch = fmod(round(curEpoch + 3600 * utcOffset + 86400L), 86400L);
  h = ((epoch  % 86400L) / 3600) % 24;
  m = (epoch % 3600) / 60;
  s = epoch % 60;
}

// =======================================================================



void processUDPCommand(const char* command) {
    if (strncmp(command, "$$", 2) == 0) {
        const char* message = command + 2; // Skip the "$$" prefix
        // printStringWithShift(message, 50);

        char spacedMessage[50]; // Adjust the size as needed
        snprintf(spacedMessage, sizeof(spacedMessage), "   %s", message);
        printStringWithShift(spacedMessage, 25);
        String broadcastString = "dACK";
        broadcastString += " displayMessage";
        udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
        udp.print(broadcastString);
        udp.endPacket();

        delay(holdInterval);
    }
    else if (strncmp(command, "$F", 2) == 0) {
        const char* message = command + 2; // Skip the "$F" prefix
        String messageString = String(message);

        switch (messageString.charAt(0)) {
        case 'L':
            if (messageString.length() >= 2) {
                int num = messageString.substring(1).toInt();
                if (num >= 0 && num <= 15) {
                    // Do something with the valid L value
                    myIntensity = num;
                    String intensityString = "Intensity:" + String(num);
                    saveText(0, intensityString.c_str());
                    Serial.println("saved " + intensityString);
                    String broadcastString = "dACK";
                    broadcastString += " Intensity";
                    udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
                    udp.print(broadcastString);
                    udp.endPacket();

                }
            }
            break;

        case 'T':
            if (messageString.length() >= 2) {
                long num = messageString.substring(1).toFloat();
                if (num >= 0 && num <= 260000) {
           
                    clockInterval = num;
                    // Do something with the valid T value
                    Serial.print("clock interval set to ");
                    Serial.println(num);


                    String intervalString = "clockInterval:" + String(num);
                    saveText(100, intervalString.c_str());
                    Serial.println("saved " + intervalString);
                    
                    String broadcastString = "dACK";
                    broadcastString += " clockInterval";
                    udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
                    udp.print(broadcastString);
                    udp.endPacket();


                }
            }
            break;


        case 'H':
            if (messageString.length() >= 2) {
                long num = messageString.substring(1).toFloat();
                if (num >= 0 && num <= 260000) {

                    holdInterval = num;
                    // Do something with the valid T value
                    Serial.print("hold interval set to ");
                    Serial.println(num);


                    String intervalString = "holdInterval:" + String(num);
                    saveText(200, intervalString.c_str());
                    Serial.println("saved " + intervalString);

                    String broadcastString = "dACK";
                    broadcastString += " holdInterval";
                    udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
                    udp.print(broadcastString);
                    udp.endPacket();


                }
            }
            break;

       

        case 'O':
            if (messageString.length() >= 2) {
                long num = messageString.substring(1).toFloat();
                if (num >= -12 && num <= 12) {

                    utcOffset = num;
                    // Do something with the valid T value
                    Serial.print("time offset set to ");
                    Serial.println(num);


                    String intervalString = "utcOffset:" + String(num);
                    saveText(300, intervalString.c_str());
                    Serial.println("saved " + intervalString);
                    
                    String broadcastString = "dACK";
                    broadcastString += " utcOffSet";
                    udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
                    udp.print(broadcastString);
                    udp.endPacket();

                }
            }
            break;


        case '?':
            if (messageString.length() >= 1) {

                               
                   Serial.println("help command received ");
                   Serial.println("broadcasting... ");
                   String ipAddressString = WiFi.localIP().toString();
                   String CommandList = "COMMANDS[send without brackets]\n$$[your_text_here],$FO[-12~12],$FH[0~260000],$FT[0~260000],$FL[0~15]";
                   String broadcastString = "INFO \nutcOffset (hours)=" + String(utcOffset) + "\n";
                   broadcastString += "holdInterval (millisecs)=" + String(holdInterval) + "\n";
                   broadcastString += "clockInterval (millisecs)=" + String(clockInterval) + "\n";
                   broadcastString += "myIntensity (0-15)=" + String(myIntensity) + "\n";
                   broadcastString += "myIP (dynamic)=" + ipAddressString; +"\n";
                   broadcastString += "\n\n" + CommandList;
                   // Send the broadcast message
                   udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
                   udp.print(broadcastString);
                   udp.endPacket();
                               
            }
            break;

        }
    }
}



void saveText(int address, const char* text) {
    for (int i = 0; i < TEXT_SIZE; i++) {
        char c = text[i];
        EEPROM.put(address + i, c);
    }
    EEPROM.commit();
}

void readText(int address, char* text) {
    for (int i = 0; i < TEXT_SIZE; i++) {
        char c;
        EEPROM.get(address + i, c);
        text[i] = c;
    }
}


