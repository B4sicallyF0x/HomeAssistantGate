#include "WiFiEsp.h"
//#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial softserial(A5, A4); // A5 to ESP_TX, A4 to ESP_RX by default
//#endif
#define RELAY_PIN_1 6
#define RELAY_PIN_2 7

char ssid[] = "SSID"; // replace ****** with your network SSID (name)
char pass[] = "PASSWORD"; // replace ****** with your network password
int status = WL_IDLE_STATUS;

int ledStatus1 = LOW;
int ledStatus2 = LOW;

WiFiEspServer server(80);

// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(8);

void setup()
{
  pinMode(RELAY_PIN_1, OUTPUT);	// initialize digital pin RELAY_PIN_1 as an output.
  pinMode(RELAY_PIN_2, OUTPUT);	// initialize digital pin RELAY_PIN_2 as an output.
  Serial.begin(9600);   // initialize serial for debugging
  softserial.begin(115200);
  softserial.write("AT+CIOBAUD=9600\r\n");
  softserial.write("AT+RST\r\n");
  softserial.begin(9600);    // initialize serial for ESP module
  WiFi.init(&softserial);    // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  printWifiStatus();
  
  // start the web server on port 80
  server.begin();
}


void loop()
{
  WiFiEspClient client = server.available();  // listen for incoming clients

  if (client) {                               // if you get a client,
    Serial.println("New client");             // print a message out the serial port
    buf.init();                               // initialize the circular buffer
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
        char c = client.read();               // read a byte, then
        buf.push(c);                          // push it to the ring buffer
        
        // you got two newline characters in a row
        // that's the end of the HTTP request, so send a response
        if (buf.endsWith("\r\n\r\n")) {
          sendHttpResponse(client);
          break;
        }

        // Check to see if the client request was "GET /H" or "GET /L" or "GET /A":
        if (buf.endsWith("GET /H")) {
          Serial.println("Turn RELAY_PIN_1 ON");
          ledStatus1 = HIGH;
          digitalWrite(RELAY_PIN_1, HIGH);   // turn the RELAY_PIN_1 on (HIGH is the voltage level)
          // Wait for 2 seconds (2000 milliseconds)
          delay(2000);
          Serial.println("Turn RELAY_PIN_1 OFF");
          ledStatus1 = LOW;
          digitalWrite(RELAY_PIN_1, LOW);    // turn the RELAY_PIN_1 off by making the voltage LOW
        }
        else if (buf.endsWith("GET /L")) {
          Serial.println("Turn RELAY_PIN_2 ON");
          ledStatus2 = HIGH;
          digitalWrite(RELAY_PIN_2, HIGH);    // turn the RELAY_PIN_2 on by making the voltage HIGH
          // Wait for 2 seconds (2000 milliseconds)
          delay(2000);
          Serial.println("Turn RELAY_PIN_2 OFF");
          ledStatus2 = LOW;
          digitalWrite(RELAY_PIN_2, LOW);    // turn the RELAY_PIN_2 off by making the voltage LOW
        }
        else if (buf.endsWith("GET /A")) {
          Serial.println("Turn RELAY_PIN_1 and RELAY_PIN_2 ON");
          ledStatus1 = HIGH;
          digitalWrite(RELAY_PIN_1, HIGH);   // turn the RELAY_PIN_1 on (HIGH is the voltage level)
          ledStatus2 = HIGH;
          digitalWrite(RELAY_PIN_2, HIGH);   // turn the RELAY_PIN_2 on (HIGH is the voltage level)
          // Wait for 2 seconds (2000 milliseconds)
          delay(2000);
          Serial.println("Turn RELAY_PIN_1 and RELAY_PIN_2 OFF");
          ledStatus1 = LOW;
          digitalWrite(RELAY_PIN_1, LOW);    // turn the RELAY_PIN_1 off by making the voltage LOW
          ledStatus2 = LOW;
          digitalWrite(RELAY_PIN_2, LOW);    // turn the RELAY_PIN_2 off by making the voltage LOW
        }
      }
    }
    
    // close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
}


void sendHttpResponse(WiFiEspClient client)
{
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  
  // the content of the HTTP response follows the header:
  client.print("RELAY_PIN_1 ");
  if (ledStatus1 == HIGH) 
    client.print("ON");
  else 
    client.print("OFF");
  client.println("<p>");
  client.print("RELAY_PIN_2 ");
  if (ledStatus2 == HIGH) 
    client.print("ON");
  else 
    client.print("OFF");
  client.println("<p>");
  client.println("Click <a href=\"/H\">here</a> to open portal 1<br>");
  client.println("Click <a href=\"/L\">here</a> to open portal 2<br>");
  client.println("Click <a href=\"/A\">here</a> to open all portals<br>");
  
  // The HTTP response ends with another blank line:
  client.println();
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go
}
