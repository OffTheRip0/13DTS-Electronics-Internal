// Load Wi-Fi library
#include <WiFi.h>
#include <ESP32Servo.h>

// Network credentials Here
const char* ssid     = "ESP32-Network";
const char* password = "wejefwijefiwebfhwebfhbwefhbwehifbwiehwejkgbwigbqejfblwjefbkjwe";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;


String lockState = "off";
//Output variable to GPIO pins
const int ledPin19 = 19;
Servo servo;
int servoPin = 14;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  servo.attach(servoPin);
  pinMode(ledPin19, OUTPUT);      // set the LED pin mode
  digitalWrite(ledPin19, 0);      // turn LED off by default
  servo.write(45);
  WiFi.softAP(ssid,password);
  
  // Print IP address and start web server
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.softAPIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client

    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            if (header.indexOf("GET /safe/on") >= 0) {
              lockState = "on";
              digitalWrite(ledPin19, HIGH);     
              servo.write(45);
            } else if (header.indexOf("GET /safe/off") >= 0) {
              lockState = "off";
              digitalWrite(ledPin19, LOW);                //turns the LED off
              servo.write(90);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            client.println("<style>html { font-family: monospace; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: yellowgreen; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 32px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: gray;}</style></head>");

            client.println("<body><h1>Safe Box Lock Control</h1>");
            client.println("<p>:)</p>");

            if (lockState == "off") {
              client.println("<p><a href=\"/safe/on\"><button class=\"button\">Lock</button></a></p>");
            } else {
              client.println("<p><a href=\"/safe/off\"><button class=\"button button2\">Unlock</button></a></p>");
            }
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}