/*
TCP-Server
Change SSID and PASSWORD.
*/

#define SSID "o2-WLAN76"
#define PASSWORD "SalinenStr48"

#define LED 5
#define LED_WLAN 13

#define DEBUG true

#include <SoftwareSerial.h>
//#include<LiquidCrystal.h>

// RS, E, D4, D5, D6, D7
//LiquidCrystal lcd(A0, A1, A2, A3, 10, 9);
SoftwareSerial esp8266(2,3); // RX, TX

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);
//  lcd.begin(16, 2);
  pinMode(LED, OUTPUT);
  
  if (!espConfig()) serialDebug();
  else digitalWrite(LED_WLAN, HIGH);

  debug("ESP config Complete !!");
  
//  lcd.print("WLAN CONNECTED");

  if (configTCPServer())  debug("Server Aktiv"); else debug("Server Error");
//  lcd.setCursor(0, 1);
 // lcd.print("Server Aktiv");
  
}


void loop() {
  if (esp8266.available()) // check if the esp is sending a message
  {
    if (esp8266.find("+IPD,"))
    {
      debug("Incomming Request");
      int connectionId = esp8266.parseInt(); //Abfrage der Connection ID, da ja mehrere Clients möglich sind
      
      if (esp8266.findUntil("LED","\n")) digitalWrite(LED, !digitalRead(LED));

      String webpage = "<h1>Hello World!</h1>Open [IP]/LED to Toggle LED on D5"; //alles innerhalb der h1 Klammer ist Überschrift. Text ist reine Benutzernweisung

      if (sendCom("AT+CIPSEND=" + String(connectionId) + "," + String(webpage.length()), ">"))
      {
        sendCom(webpage,"SEND OK");
        sendCom("AT+CIPCLOSE=" + String(connectionId),"OK");
        debug("Send and Close");
      }
    }
  }
}

//-----------------------------------------Config ESP8266------------------------------------

boolean espConfig()
{
  boolean success = true;
  esp8266.setTimeout(5000);
  success &= sendCom("AT+RST", "ready");
  esp8266.setTimeout(1000);
  if (configStation(SSID, PASSWORD)) {
    success &= true;
    debug("WLAN Connected");
    debug("My IP is:");

    if(sendCom("AT+CIPSTA=\"192.168.1.160\",\"192.168.1.1\",\"255.255.255.0\"","OK")) debug("Static IP 192.168.1.160"); else debug("Static ip error");
    
    debug(sendCom("AT+CIFSR"));
  }
  else
  {
    success &= false;
  }
  //shorter Timeout for faster wrong UPD-Comands handling
  success &= sendCom("AT+CIPMODE=0", "OK");  //So rum scheit wichtig!
  success &= sendCom("AT+CIPMUX=0", "OK");

  return success;
}

boolean configTCPServer()
{
  boolean success = true;

  success &= (sendCom("AT+CIPMUX=1", "OK"));
  success &= (sendCom("AT+CIPSERVER=1,80", "OK"));

  return success;

}


boolean configStation(String vSSID, String vPASSWORT)
{
  boolean success = true;
  success &= (sendCom("AT+CWMODE=1", "OK"));
  esp8266.setTimeout(20000);
  success &= (sendCom("AT+CWJAP=\"" + String(vSSID) + "\",\"" + String(vPASSWORT) + "\"", "OK"));
  esp8266.setTimeout(1000);
  return success;
}

boolean configAP()
{
  boolean success = true;

  success &= (sendCom("AT+CWMODE=2", "OK"));
  success &= (sendCom("AT+CWSAP=\"NanoESP\",\"\",5,0", "OK"));

  return success;
}

boolean configUDP()
{
  boolean success = true;

  success &= (sendCom("AT+CIPMODE=0", "OK"));
  success &= (sendCom("AT+CIPMUX=0", "OK"));
  success &= sendCom("AT+CIPSTART=\"UDP\",\"192.168.255.255\",90,91,2", "OK"); //Importand Boradcast...Reconnect IP
  return success;
}

//-----------------------------------------------Controll ESP-----------------------------------------------------

boolean sendUDP(String Msg)
{
  boolean success = true;

  success &= sendCom("AT+CIPSEND=" + String(Msg.length() + 2), ">");    //+",\"192.168.4.2\",90", ">");
  if (success)
  {
    success &= sendCom(Msg, "OK");
  }
  return success;
}


boolean sendCom(String command, char respond[])
{
  esp8266.println(command);
  if (esp8266.findUntil(respond, "ERROR"))
  {
    return true;
  }
  else
  {
    debug("ESP SEND ERROR: " + command);
    return false;
  }
}

String sendCom(String command)
{
  esp8266.println(command);
  return esp8266.readString();
}



//-------------------------------------------------Debug Functions------------------------------------------------------
void serialDebug() {
  while (true)
  {
    if (esp8266.available())
      Serial.write(esp8266.read());
    if (Serial.available())
      esp8266.write(Serial.read());
  }
}

void debug(String Msg)
{
  if (DEBUG)
  {
    Serial.println(Msg);
  }
}
