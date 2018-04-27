#include <RotaryEncoder.h>
#include <WiFi.h>
#include <SparkFun_TB6612.h>


void lock(int state,int sol,Motor motor);
void unlock(int state,int sol,Motor motor); 


const int INIT= 0;
const int UNLOCK=1;
const int LOCKING=2;
const int LOCK=3;
const int UNLOCKING=4;
const int UNLOCK_TIME=2000;//ms
const int LOCK_TIME=2000; //ms
const int INSERT_KEY=2000;//ms
const int REMOVE_KEY=2000;//ms
const int PAUSE=500;



// these constants are used to allow you to make your motor configuration 
// line up with function names like forward.  Value can be 1 or -1
const int offsetA = 1;
const int offsetB = 1;

// Initializing motors.  The library will allow you to initialize as many
// motors as you have memory for.  If you are using functions like forward
// that take 2 motors as arguements you can either write new functions or
// call the function more than once.
// Replace with your network credentials
const char* ssid     = "Free-TAU";
const char* password = "free-tau";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String locked = "Unlocked";

// Assign output variables to GPIO pins
const int ledPin = 22;

const int enc_out1 = 32;
const int enc_out2 = 33;

const int ctl_stby = 14;
const int ctl_ain1 = 25;
const int ctl_ain2 = 26;
const int ctl_pwma = 27;

const int v5_en = 12;
const int v5_pg = 23;

const int v6_en = 18;
const int v6_pg = 19;

const int sol=12;

const int state=INIT;








Motor motor1 = Motor(ctl_ain1, ctl_ain2, ctl_pwma, offsetA, ctl_stby);
// Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);





void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(ledPin, OUTPUT);
  pinMode(sol,OUTPUT);
  
  // Set outputs to HIGH
  digitalWrite(ledPin, HIGH);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  digitalWrite(sol,LOW);

 
}

void loop(){
  
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
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
            
            // Locks and Unlocks
            if (header.indexOf("GET /lock") >= 0) {
              Serial.println("Locked");
              locked = "Locked";
              digitalWrite(ledPin, LOW);
              lock(state,sol,motor1);
            } else if (header.indexOf("GET /unlock") >= 0) {
              Serial.println("Unlocked");
              locked = "Unlocked";
              unlock(state,sol,motor1);
              digitalWrite(ledPin, HIGH);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>TauRD's SmartLock</h1>");
            

            // Display current state, and ON/OFF buttons for GPIO 22  
            client.println("<p>State: " + locked + "</p>");
            // If the output22State is off, it displays the ON button       
            if (locked=="Unlocked") {
              client.println("<p><a href=\"/lock\"><button class=\"button\">LOCK</button></a></p>");
            } else {
              client.println("<p><a href=\"/unlock\"><button class=\"button button2\">UNLOCK</button></a></p>");
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
void lock(int state,int sol,Motor motor){
  state=LOCKING;
  digitalWrite(sol,HIGH);
  delay(PAUSE);
  motor.drive(255,INSERT_KEY);
  motor.brake();
  delay(PAUSE);
  digitalWrite(sol,LOW);
  delay(PAUSE);
  motor.drive(255, LOCK_TIME);
  motor.brake();
  delay(PAUSE);
  digitalWrite(sol,HIGH);
  delay(PAUSE);
  motor.drive(-255, REMOVE_KEY);
  motor.brake();
  delay(PAUSE);
  digitalWrite(sol,LOW);
  delay(PAUSE);
  state=LOCK;
}

void unlock(int state,int sol,Motor motor){
  state=UNLOCKING;
  digitalWrite(sol,HIGH);
  delay(PAUSE);
  motor.drive(255,INSERT_KEY);
  motor.brake();
  delay(PAUSE);
  digitalWrite(sol,LOW);
  delay(PAUSE);
  motor.drive(-255, UNLOCK_TIME);
  motor.brake();
  delay(PAUSE);
  digitalWrite(sol,HIGH);
  delay(PAUSE);
  motor.drive(-255, REMOVE_KEY);
  motor.brake();
  delay(PAUSE);
  digitalWrite(sol,LOW);
  delay(PAUSE);
  state=UNLOCK;
}



