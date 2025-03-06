#include <Metro.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>


const char *ssid = "NodeMCU";
const char *pass = "00000000";
//const char *ssid = "LucasLeal";
//const char *pass = "00000000";
//char ssid[] = "Telecentro-27bc"; //SSID
//char pass[] = "HJNFZMMVZYED";    // PASS
/////NODEMCU MIO, GATEWAY 192.168.4.1
OSCErrorCode error;
IPAddress iplocal(192, 168, 0, 103); //elegir ip para el nodemcu LED2
//IPAddress gateway(192, 168, 4, 1); //direccion del nodemcu 1, igual para todos los nodemcu
IPAddress gateway(192, 168, 0, 1); //direccion del router, igual para todos los nodemcu

const unsigned int puertoEntrada = 7003; //LED2
WiFiUDP Udp;


unsigned long previous_time = 0;
unsigned long intervalo = 5000;  // 20 seconds delay
int ledState;
void setup() {
  // put your setup code here, to run once:
  //setupPWM16();

  analogWriteRange(65535);
  analogWriteFreq(100);
  analogWrite(D1, 65535);
  delay(2000);
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  //  WiFi.config(iplocal, gateway, IPAddress(255, 255, 255, 0));
  WiFi.config(iplocal, gateway, IPAddress(255, 255, 255, 0));
  Serial.print("Conectandose a ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(2, ledState);
    ledState = !ledState;
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
}
int count;
int count2;
int count3;
int pwmCount;
int pwmCount2;
int pwmCoun32;
byte dir = 0;
byte dir2 = 0;
byte dir3;
#define UP 0
#define DOWN 1
#define THRESHOLD 2000
#define TIME 9000
uint16_t icr = 0xffff;
int vueltas;
int count1Flag = true;
int count2Flag;
int count3Flag;
void loop() {
  // put your main code here, to run repeatedly:




  OSCMessage msg;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      msg.dispatch("/led3", led3Count);

    } else {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    pwmCount = fscale(count, 0, 9000, 65535, -3000, -3);
    //analogWrite16(9, pwmCount);

    analogWrite(D1, pwmCount);
    //Serial.println(count);
    digitalWrite(2, LOW); //tamo bien
  }
  else {
    digitalWrite(2, HIGH); //nos desconectamo
    Serial.println("CONNECTION LOST");
    count = 0; //si perdemos conexion seteamos count en 0 (65535)para que cuando vuelva, no escriba el valor anterior, que quede apagado hasta que reciba el nuevo valor por osc
    analogWrite(D1, 65535); //y apaga el led inmediatamente
    ESP.restart();
  }
  //Serial.println(count);
  //analogWrite(9, 127);



  unsigned long current_time = millis(); // number of milliseconds since the upload

  // checking for WIFI connection
  /*if ((WiFi.status() != WL_CONNECTED) && (current_time - previous_time >= intervalo)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WIFI network");
    WiFi.disconnect();
    WiFi.begin(ssid,pass);
    previous_time = current_time;

    }*/
}
void led3Count(OSCMessage &msg) {
  count = msg.getInt(0);
  // Serial.print("/relay: ");
  //Serial.println(int(relay));
  //digitalWrite(RELAY_PIN, int(relay));

}




float fscale( float inputValue, float originalMin, float originalMax, float newBegin, float
              newEnd,  float curve) {

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;


  // condition curve parameter
  // limit range

  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;

  curve = (curve * -.1) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

  /*
    Serial.println(curve * 100, DEC);   // multply by 100 to preserve resolution
    Serial.println();
  */

  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero Refference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin) {
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd;
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float

  /*
    Serial.print(OriginalRange, DEC);
    Serial.print("   ");
    Serial.print(NewRange, DEC);
    Serial.print("   ");
    Serial.println(zeroRefCurVal, DEC);
    Serial.println();
  */

  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
  if (originalMin > originalMax ) {
    return 0;
  }

  if (invFlag == 0) {
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;

  }
  else     // invert the ranges
  {
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return rangedValue;
}
