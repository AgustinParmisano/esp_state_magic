#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

ESP8266WebServer server(80);

char* ssid = "test";
char* password = "12345678";
String st;
String content;
int statusCode;
int auxwebtype;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.println("Startup");
  // read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");
  String esid;
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
  if ( esid.length() > 1 ) {
    WiFi.begin(esid.c_str(), epass.c_str());
    if (testWifi()) {
      launchWeb(0);
      return;
    }
  }
  setupAP();
}

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
  IPAddress ip = WiFi.softAPIP();
  String apip = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  if(apip == "0.0.0.0"){
    Serial.println("La IP del AP es 0.0.0.0 entonces tengo que mostrar la web principal");
    createWebServer(0);
  }else{
     Serial.println("La IP del AP NO es 0.0.0.0 entonces tengo que mostrar la web de configuración");
     createWebServer(1);
  }
  //createWebServer(webtype);
  // Start the server
  server.begin();
  Serial.println("Server started");
}

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

void createWebServer(int webtype)
{
  Serial.println("If Webtype is 1 Configure SSID & PASS if is 0 show local IP: Webtype is:");
  Serial.println(webtype);
  auxwebtype = webtype;
  if ( webtype == 1 ) {
    Serial.println("CONFIGURING SSID CONNECTION");
    server.on("/", []() {
      if ( auxwebtype == 1 ) {
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
      }else if( auxwebtype == 0 ){
            IPAddress ip = WiFi.localIP();
            String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
            server.send(200, "text/html", "Second Stage {\"IP\":\"" + ipStr + "\"}");
      }
    });
    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String qurl = server.arg("url");

      if (qsid.length() > 0 && qpass.length() > 0 && qurl.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
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
        setup(); //hay que hacer que no se tenga que resetear con el botón.
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.send(statusCode, "application/json", content);
    });
  } else if (webtype == 0) {
    Serial.println("CONNECTED TO NETWORK");

    /*NEW*/
    server.on("/send", []() {
      Serial.print("Sending data to server ...");
      const uint16_t port = 5000;
      const char * host = "192.168.0.4";
      int tries = 0;

      WiFiClient client; // Use WiFI Client to create TCP connections

      while (!client.connect(host, port)) {
          Serial.println("connection failed");
          Serial.println("wait 5 sec... Tries: " + tries);
          delay(5000);
          tries = tries + 1;
          if (tries > 2){
              Serial.println("Sending state FAILED, exiting sending state");
              return;
          }
      }

      String data = "hola como estas";

      client.print(String("POST ") + host + " HTTP/1.1\r\n" +
                 "Host: " + WiFi.localIP() + "\r\n" +
                 //"Connection: close\r\n" +
                 "Content-Type: text/html; charset=utf-8\r\n" +
                 "Content-Length: " + data.length() + "\r\n" +
                 "\r\n" + // This is the extra CR+LF pair to signify the start of a body
                 data + "\n");

      //read back one line from server
      String line = client.readStringUntil('\r');
      client.println(line);

      Serial.println("closing connection");
      client.stop();

      delay(5000);
      return;
      /*END NEW*/
    });

    server.on("/", []() {
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      server.send(200, "text/html", "{\"IP\":\"" + ipStr + "\"}");
    });
    server.on("/cleareeprom", []() {
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>Clearing the EEPROM</p></html>";
      server.send(200, "text/html", content);
      Serial.println("clearing eeprom");
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      Serial.println("eeprom cleared...");
    });
    server.on("/disconnect", []() {
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>Disconecting</p></html>";
      server.send(200, "text/html", content);
      Serial.println("disconecting . . .");
      WiFi.disconnect();
      Serial.println("Disconected");
      setupAP();
    });
  }
}

void loop() {
  server.handleClient();
}
