#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "00000";
const char* password = "00000";

extern const String StartHeader;
extern const String EndHeader;
extern const String ButtonOn;
extern const String ButtonOff;

//------------------------------------------
//SETUP LED
long time_led;
#define LED_DELAY 1000
#define LED_PIN 2


//------------------------------------------
//SETUP RELAY
#define RELAY0_PIN D12
#define RELAY1_PIN D13

//------------------------------------------
//SETUP BUTTON
#define BUTTON_PIN D3
#define BUTTON_DELAY 300

long t_button;

//------------------------------------------
//WIFI i server HTTP
ESP8266WebServer server(80);
#define WIFI_SSID ssid
#define WIFI_PASSWORD password
#define HOST_NAME "onoff"


bool button_last;
long t_now;

void setup() {
  //Setup pin mode
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT); //Initialize onboard LED as output
  pinMode(RELAY0_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);  
  SW_all_off(); //Turn off all relays
  button_last = HIGH;

  //Setup UART
  Serial.begin(115200);
  Serial.println("");
  Serial.println("START");

  //Setup WIFI 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.hostname( HOST_NAME );
  Serial.print("Connecting to WIFI");

  //Wait for WIFI connection
  while( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println( WIFI_SSID );
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //Setup HTTP server
  server.on("/", handleRoot);
  server.on("/sw", handleSW);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  
}
 
void loop() {
  t_now = millis();
  
  server.handleClient();

  //Blinking an LED
  if( t_now - time_led > LED_DELAY ){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    time_led = t_now;
  }

  //Check button
  bool b = digitalRead( BUTTON_PIN );
  if( b != button_last && t_now - t_button > BUTTON_DELAY ){
    if( b==HIGH && button_last==LOW ){
      SW(0, digitalRead( RELAY0_PIN ));
      Serial.println("BUTTON");
    }

    t_button = t_now;
    button_last = b;
  }
}

void SW_all_on(){
  digitalWrite(RELAY0_PIN, LOW);
  digitalWrite(RELAY1_PIN, LOW);  
}

void SW_all_off(){
  digitalWrite(RELAY0_PIN, HIGH);
  digitalWrite(RELAY1_PIN, HIGH);  
}

void SW(byte num, bool sw){
  switch( num ){
    case 0: digitalWrite(RELAY0_PIN, !sw); break;
    case 1: digitalWrite(RELAY1_PIN, !sw); break;
  }
}

void handleRoot(){
  String html = StartHeader;  

  if( !digitalRead(RELAY0_PIN) ){
    html += ButtonOff;
  }else{
    html += ButtonOn;
  }

  html += EndHeader;  

  //server.send(200, "text/html", html);
  server.send(200, "text/html", html);
}

void handleSW(){
  if (server.arg("sw")== "on"){
    SW_all_on();
  }else{
    SW_all_off();
  }

  String ip = WiFi.localIP().toString();
  server.sendHeader("Location", String("http://") + ip, true);
  server.send ( 302, "text/plain", "");
  server.client().stop();
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

const String StartHeader = 
"<!doctype html>\
<html>\
<head>\
    <meta charset=\"utf-8\">\
    <meta http-equiv=\"Content-Type\" content=\"text/html;charset=UTF-8\">\
    <meta http-equiv=\"x-ua-compatible\" content=\"ie=edge\">\
    <title>Power supply switch</title>\
    <meta name=viewport content=\"width=device-width,initial-scale=1,maximum-scale=1,user-scalable=0\">\
    <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-EVSTQN3/azprG1Anm3QDgpJLIm9Nao0Yz1ztcQTwFspd3yD65VohhpuuCOmLASjC\" crossorigin=\"anonymous\">\
</head>\
<body>\
    <div class=\"container\">\
        <div class=\"row\">\
          <div class=\"col text-center\">\
            <div class=\"card mt-4\">\
                <div class=\"card-header\">\
                    Power supply\
                </div>\
                <div class=\"card-body\">";

const String ButtonOn = "<a href=\"sw?sw=on\" class=\"btn btn-success btn-lg\">Включить</a>";
const String ButtonOff = "<a href=\"sw?sw=off\" class=\"btn btn-danger  btn-lg\">Выключить</a>";

const String EndHeader = 
"                </div>\
      </div>\
</body>\
</html>";