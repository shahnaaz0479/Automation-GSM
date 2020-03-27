#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include<ArduinoJson.h> 
/* Set these to your desired credentials. */
const char *ssid = "...................";  //ENTER YOUR WIFI SETTINGS
const char *password = ".................";

const char *host = "..............com";
const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80
 
//SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "...........................................";
//=======================================================================
//                    Power on setup
//=======================================================================
 
void setup() {
  delay(10000);
  Serial.begin(9600);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //Only Station No AP, This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
 
  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}
 
//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  WiFiClientSecure httpsClient;    //Declare object of class WiFiClient
 
  Serial.println(host);
 
  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(1000);
  
  Serial.print("HTTPS Connecting...");
  int r=0; //retry counter
  while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
      delay(100);
      Serial.print(".");
      r++;
  }
  if(r==30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected to web");
  }
  
  String ADCData, getData, Link;
  int adcvalue=analogRead(A0);  //Read Analog value of LDR
  ADCData = String(adcvalue);   //String to interger conversion
 
  //GET Data
  Link = "........................";
 
  Serial.print("requesting URL: ");
  Serial.println(host+Link);
 
  httpsClient.print(String("GET ") + Link + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +               
               "Connection: close\r\n\r\n");
 
  Serial.println("request sent");
                  
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
 
  Serial.println("JSON Response: ");
  Serial.println("==========");
  String line;
  while(httpsClient.available()){        
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response

    DynamicJsonDocument jsonDoc(1024);
    deserializeJson(jsonDoc,line);
    JsonObject root = jsonDoc.as<JsonObject>();
    // Parameters
    const int code = root["code"]; // code
    
    if(code == 200){
      const String message = root["message"]; // message
      const String number = root["number"]; // number
      // Output to serial monitor
      Serial.print("Code: "); Serial.println(code);
      Serial.print("Number: "); Serial.println(number);
      Serial.print("Message: "); Serial.println(message);
      processSms(message,number);  
    } else if(code == 300) {
      const String number = root["number"]; // number
      // Output to serial monitor
      Serial.print("Code: "); Serial.println(code);
      Serial.print("Number: "); Serial.println(number);
      //processCall(number);
      Serial.println();
    } else if(code == 400) {
      Serial.println("No process to be done");
    }
  }
  Serial.println("==========");
  Serial.println("closing connection");
  delay(30000);  //GET Data at every 30 seconds
}

void processSms(String message, String number) {

    Serial.print("AT");  //Start Configuring GSM Module
    delay(1000);         //One second delay
    Serial.println();
    Serial.println("AT+CMGF=1");  // Set GSM in text mode
    Response();
    delay(1000);                  // One second delay
    Serial.println();
    Serial.print("AT+CMGS=");     // Enter the receiver number
    Serial.print("\"+" + number + "\"");
    Response();
    Serial.println();
    delay(1000);
    Serial.print(message); // SMS body - Sms Text
    Response();
    delay(1000);
    Serial.println();
    Serial.write(26);                //CTRL+Z Command to send text and end session*/
    
    Serial.print("AT");  //Start Configuring GSM Module
    delay(1000);         //One second delay
    Serial.println();
    Serial.println("AT+CMGF=1");  // Set GSM in text mode
    delay(1000);                  // One second delay
    Serial.println();
    Serial.print("AT+CMGS=");     // Enter the receiver number
    Serial.print("\"+number\"");
    Serial.println();
    delay(1000);
    Serial.println();
    Serial.write(26);                //CTRL+Z Command to send text and end session
    while(1);                        //Just send the text ones and halt 

}


void processCall(String number) {
  
  Serial.println("AT"); //Once the handshake test is successful, it will back to OK
  Response();
  delay(1000);
  Serial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  Response();
  delay(1000);
  Serial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  Response();
  delay(1000);
  Serial.println("AT+CREG?"); //Check whether it has registered in the network
  Response();
  delay(1000);
  Serial.println("ATD +"+number+";"); //  change ZZ with country code and xxxxxxxxxxx with phone number to dial
  delay(2000);
   Response();
  delay(20000); // wait for 20 seconds...
  Serial.println("ATH"); //hang up
  Serial.println("AT+CLCC");
   Response();
}

void Response()
{
int count = 0;
Serial.println();
while(1)
{
if(Serial.available())
{
char data =Serial.read();
if(data == 'K'){Serial.println("OK");break;}
if(data == 'R'){Serial.println("GSM Not Working");break;}
//Serial.print((char)Serial.read());
}
count++;
delay(10);
if(count == 1000){Serial.println("GSM not Found");break;}

}
 delay(500);
  while (Serial.available())
  {
    Serial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
}
