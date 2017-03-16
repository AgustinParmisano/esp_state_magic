#include <DHT.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#define DHTPIN 5

#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);


ESP8266WebServer server(80);

char* ssid = "test";
char* password = "12345678";
String st;
String content;
int statusCode;
int auxwebtype;

String qurl;
String qsid;

String equrl;
String esid;

String apiurl;

String localip = "elektronip";
String elektronname = "Elektron";
String currtime = "elektrontime";
String data = "elektrondata";

// Temporary variables
static char celsiusTemp[7];
static char humidityTemp[7];


void setup() {
  Serial.begin(115200);
  
  EEPROM.begin(512);
  delay(10);

  dht.begin();
  
  Serial.println();
  Serial.println();
  Serial.println("Startup");
  
  // read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");
  esid = "";
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  } 
  Serial.print("SSID: ");
  Serial.println(esid);
  
  Serial.println("Reading EEPROM pass");
  String epass = "";
  for (int i = 32; i < 64; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);

  Serial.println("Reading EEPROM Server URL");
  equrl = "";
  for (int i = 64; i < 128; ++i)
  {
    equrl += char(EEPROM.read(i));
  }
  Serial.print("URL: ");
  Serial.println(equrl);  
  
  // If SSID is set then start web mode 0 (web server mode)
  if ( esid.length() > 1 ) {
    WiFi.begin(esid.c_str(), epass.c_str());
    if (testWifi()) {
      launchWeb(0);
      return;
    }
  }
  //If not connects to AP set up its own, start web mode 1 
  setupAP();
}

//test wifi to connect configured SSID AP
bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    Serial.print(WiFi.status());
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb(int webtype) {
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  IPAddress ip = WiFi.localIP();
  localip = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  if (localip != "0.0.0.0") {
    Serial.println("Local IP is NOT 0.0.0.0 so show main page: web mode 0 (connecting to configured SSID AP and start web server)");
    createWebServer(0);
  } else {
    Serial.println("Local IP is 0.0.0.0 so show SSID, PASS & URL configuration web: web mode 1");
    func_cleareeprom();
    createWebServer(1);
  }

  // Start the server
  server.begin();
  Serial.println("Server started");
}

//function to start AP mode
void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP(ssid, password, 6);
  Serial.println("softap");

  IPAddress ip = WiFi.softAPIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  launchWeb(1);
  Serial.println("over");
}

//function to create web server
void createWebServer(int webtype)
{
  Serial.println("If Webtype is 1 Configure SSID & PASS if is 0 show local IP: Webtype is:");
  Serial.println(webtype);
  auxwebtype = webtype;
  delay(5000);
  if ( webtype == 1 ) {
    Serial.println("CONFIGURING SSID CONNECTION");
    server.on("/", []() {
      if ( auxwebtype == 1 ) {
        Serial.println("CONFIGURING SSID CONNECTION STARTING");
        IPAddress ip = WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
        content += ipStr;
        content += "<p>";
        content += st;
        content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><label>PASS: </label><input name='pass' length=64><label>SERVER URL: </label><input name='url' length=64><input type='submit'></form>";
        content += "</html>";
        Serial.println("Sending configuration data");
        server.send(200, "text/html", content);
        content = "";
      } else if ( auxwebtype == 0 ) {
        IPAddress ip = WiFi.localIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        server.send(200, "text/html", "Second Stage {\"IP\":\"" + ipStr + "\"}");
      }
    });
    server.on("/setting", []() {
      qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      qurl = server.arg("url");

      if (qsid.length() > 0 && qpass.length() > 0 && qurl.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 128; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
        Serial.println(qurl);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        Serial.println("writing eeprom url:");
        for (int i = 0; i < qurl.length(); ++i)
        {
          EEPROM.write(64 + i, qurl[i]);
          Serial.print("Wrote: ");
          Serial.println(qurl[i]);
        }
        EEPROM.commit();
        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        Serial.println("stoping AP mode . . .");
        WiFi.mode(WIFI_STA);
        Serial.println("AP mode Stopped");
        setup();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.send(statusCode, "application/json", content);
    });
  } else if (webtype == 0) {

    Serial.println("stoping AP mode . . .");
    WiFi.mode(WIFI_STA);
    Serial.println("AP mode Stopped");

    char connected_ssid[80];
    esid.toCharArray(connected_ssid, 80);
    
    Serial.println("CONNECTED TO SSID: ");
    Serial.println(connected_ssid);

    server.on("/", []() {
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      server.send(200, "text/html", "{\"IP\":\"" + ipStr + "\"}");
    });

    server.on("/send", []() {

      func_post_data();
  
    });
    
    server.on("/cleareeprom", []() {
      
       func_cleareeprom();
       
    });
    server.on("/disconnect", []() {

      func_disconnect();

    });
  }
}

void func_disconnect() {
      //Disconnect from configured SSID AP and setup owns AP again web mode 1
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>Disconecting</p></html>";
      server.send(200, "text/html", content);
      Serial.println("disconecting . . .");
      WiFi.disconnect();
      Serial.println("Disconected");
      //setup owns AP again web mode 1
      setupAP();
}

void func_cleareeprom() {
      //Clear EEPROM to restart ESP default values
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>Clearing the EEPROM</p></html>";
      server.send(200, "text/html", content);
      Serial.println("clearing eeprom");
      for (int i = 0; i < 128; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      Serial.println("eeprom cleared...");
}

void func_post_data() {
      //Start sending data to configured server
      Serial.println("Sending data to server ...");
      const uint16_t port = 9292; //sinatra app port
      char host[90];
      equrl.toCharArray(host, port);
      //apiurl = "/uraqi/web/app_dev.php/api/uraqis.json?post_id="; //simfony 2 rest app url
      apiurl = "/elektronsends"; //sinatra app url
      
      WiFiClient client; // Use WiFI Client to create TCP connections

      Serial.print("Connecting to server host: ");
      Serial.println(host);
      

      if (!client.connect(host, port)) {
        Serial.println("connection failed");
      }

      

      while (client.connect(host, port)) {

        delay(5000);

        //retrieving data from GPIOs to send

        // read the input on analog pin 0:
        int sensorValue = analogRead(A0);
        // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
        int voltage = 1; //sensorValue * (5.0 / 1023.0);
        //voltage = (int) voltage * 1000;
        //voltage = voltage / 100;
        // print out the value you read:
        Serial.print("LDR: ");
        Serial.println(voltage);

        //Creates the PostData Json with device current data
        String PostData = "{\"ip\":\"" + localip + "\",\"time\":\"" + currtime + "\",\"name\":\"" + elektronname + "\",\"data\":\"" + data + "\"}"; 

        Serial.println("Sending data: " + PostData);
        Serial.println("API URL: " + apiurl);


        client.println("POST " + apiurl + " HTTP/1.1");
        client.println("Host: Elektron");
        client.println("Cache-Control: no-cache");
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.print("Content-Length: ");
        client.println(PostData.length() + 2);
        client.println();
        client.println(PostData);

        long interval = 2000;
        unsigned long currentMillis = millis(), previousMillis = millis();

        while (!client.available()) {

          if ( (currentMillis - previousMillis) > interval ) {

            Serial.println("Timeout");
            Serial.println("closing connection");
            client.stop();
            return;
          }
          currentMillis = millis();
        }

        Serial.println("Server response: ");
        while (client.connected())
        {
          if ( client.available() )
          {
            char str = client.read();
            Serial.print(str);
          }
        }
        Serial.println(" ");

      }
         
}

void func_configuration_mode() {
      Serial.println("CONFIGURING SSID CONNECTION");
      Serial.println("CONFIGURING SSID CONNECTION STARTING");
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><label>PASS: </label><input name='pass' length=64><label>SERVER URL: </label><input name='url' length=64><input type='submit'></form>";
      content += "</html>";
      Serial.println("Sending configuration data");
      server.send(200, "text/html", content);
      content = "";
      return;
}

void loop() {
  server.handleClient();
}
