#include <Metro.h>
#include <elapsedMillis.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

#define PIR_PIN D1
#define LED_PIN D5
#define LED_PIN2 D6
Metro senseTime = Metro(1000);


const char *ssid = "NodeMCU";
const char *pass = "00000000";
WiFiUDP Udp;
const IPAddress ipSalidaLED1(192, 168, 0, 101);        // ip nodemcu1
const unsigned int puertoSalidaLED1 = 7001;
const unsigned int puertoEntrada = 7004;
IPAddress iplocal(192, 168, 0, 104); //elegir ip para el nodemcu LED2
IPAddress gateway(192, 168, 0, 1); //direccion del router, igual para todos los nodemcu
elapsedMillis timeOff;
int pirState = 1;
int pirStateLast = 1;
int pirFlag;
int shutdownFlag;
int ledState;
int pirStateOSC;
unsigned long intervalo = 1800000; 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.config(iplocal, gateway, IPAddress(255, 255, 255, 0));
  Serial.print("Conectandose a ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  pinMode(2, OUTPUT);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(2, ledState);
    ledState = !ledState;
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi conectado");
  Serial.println("direccion IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("Inicializando UDP");
  Udp.begin(puertoEntrada);
  Serial.print("Puerto entrada: ");
  Serial.println(Udp.localPort());
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
}

void loop() {

  if (senseTime.check() == 1) {
    Serial.print("SEND OSC WITH  ");
    if (pirState) {
      Serial.println(1);
      pirStateOSC = 1;
    }
    else if (!pirState && shutdownFlag) {
      Serial.println(0);
      pirStateOSC = 0;
    }
    else if (!pirState && !shutdownFlag) {
      Serial.println(1);
      pirStateOSC = 1;
    }
    OSCMessage msg("/pir");
    msg.add(pirStateOSC);
    Udp.beginPacket(ipSalidaLED1, puertoSalidaLED1);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
  }
  // put your main code here, to run repeatedly:
  pirState = digitalRead(PIR_PIN);
  if (pirState != pirStateLast) {
    if (pirState) {
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(LED_PIN2, HIGH);
      timeOff = 0;
      pirFlag = true;
      shutdownFlag = false;
    }
    else {
      timeOff = 0;
      pirFlag = false;
      digitalWrite(LED_PIN, LOW);
      shutdownFlag = false;
    }
    Serial.println(pirState);
    pirStateLast = pirState;
  }
  Serial.print("time Off  ");
  Serial.println(timeOff);


  if (timeOff > intervalo && pirFlag == false) {
    Serial.println("SHUTDOWN");
    shutdownFlag = true;
    digitalWrite(LED_PIN2, LOW);
    timeOff = 0;
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(2, LOW);
  }
  else {
    digitalWrite(2, HIGH); //nos desconectamo
    Serial.println("CONNECTION LOST");
    //count = 0; //si perdemos conexion seteamos count en 0 (65535)para que cuando vuelva, no escriba el valor anterior, que quede apagado hasta que reciba el nuevo valor por osc
    //analogWrite(D1, 65535); //y apaga el led inmediatamente
    ESP.restart();
  }


  //delay(5);
}
