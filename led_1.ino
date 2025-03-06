#include <Metro.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <elapsedMillis.h>


const char *ssid = "NodeMCU";
const char *pass = "00000000";
/////NODEMCU MIO, GATEWAY 192.168.4.1
WiFiUDP UdpLED2;                                // protocolo udp
//WiFiUDP UdpLED3;                                // protocolo udp
const IPAddress ipSalidaLED2(192, 168, 0, 102);        // ip nodemcu2
const unsigned int puertoSalidaLED2 = 7002;
const IPAddress ipSalidaLED3(192, 168, 0, 103);        // ip nodemcu2
const unsigned int puertoSalidaLED3 = 7003;
const unsigned int puertoEntrada = 7001;

//IPAddress ipLocal(192, 168, 1, 100);
//IPAddress gateway(192, 168, 4, 1);
//IPAddress subnet(255, 255, 255, 0);

elapsedMillis dropoutTime;

#define DROPOUT_TIME 300000
Metro fadeIn = Metro(10);
IPAddress iplocal(192, 168, 0, 101); //elegir ip para el nodemcu LED2
IPAddress gateway(192, 168, 0, 1); //direccion del router, igual para todos los nodemcu
int ledState;
void setup() {
  // put your setup code here, to run once:
  //setupPWM16();

  analogWriteRange(65535);
  analogWriteFreq(100);
  analogWrite(D1, 65535); //apagamos LED
  Serial.begin(115200);
  WiFi.config(iplocal, gateway, IPAddress(255, 255, 255, 0));
  Serial.print("Conectandose a ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  WiFi.mode(WIFI_STA);
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
  UdpLED2.begin(puertoEntrada);
  Serial.print("Puerto entrada: ");
  Serial.println(UdpLED2.localPort());
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
int pirState = 1;
int pirStateLast = 1;

void pirReading(OSCMessage &msg) {
  pirState = msg.getInt(0);
   Serial.print("/pir: ");
  Serial.println(pirState);
  /

}

OSCErrorCode error;
void resetLED() {

  Serial.println("SHUTDOWN");
  count1Flag = true;
  count2Flag = false;
  count3Flag = false;
  count = 0;
  count2 = 0;
  count3 = 0;
  dir = 0;
  dir2 = 0;
  dir3 = 0;
  analogWrite(D1, 65535);
  OSCMessage msg("/led2");
  msg.add(0);
  UdpLED2.beginPacket(ipSalidaLED2, puertoSalidaLED2);
  msg.send(UdpLED2);
  UdpLED2.endPacket();
  msg.empty();
  OSCMessage msg2("/led3");
  //Serial.println(count3);
  msg2.add(0);
  UdpLED2.beginPacket(ipSalidaLED3, puertoSalidaLED3);
  msg2.send(UdpLED2);
  UdpLED2.endPacket();
  msg2.empty();
}
void loop() {
  // put your main code here, to run repeatedly:

  OSCMessage msg;
  int size = UdpLED2.parsePacket();
  if (size > 0) {
    while (size--) {
      msg.fill(UdpLED2.read());
    }
    if (!msg.hasError()) {
      msg.dispatch("/pir", pirReading);
      dropoutTime = 0;

    } else {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
  //Serial.println(dropoutTime);
  if(dropoutTime > DROPOUT_TIME){ //si no recibimos nada por DROPOUT_TIME (5min), ignorar y mantener el sistema activo
    //Serial.print("ignorar PIR  ");
    pirState = 1;
    //Serial.println(pirState);
    dropoutTime = 0;
    
  }
  if (pirState != pirStateLast) {
    if (pirState == false) {
      resetLED(); 
    }
    else if(pirState == true){
      //restart
      Serial.println("RESTART");
    }
    pirStateLast = pirState;
  }
  if (fadeIn.check() == 1 && pirState == true) {

    /////////////////////
    if (count1Flag) {
      //Serial.print("count1:  ");
      //Serial.println(count);

      if (dir == UP) {
        count += 10;
      }
      else if (dir == DOWN) {
        count -= 10;
      }

      if (count > TIME || count < 0) {

        dir = !dir;
        if (count < 0) { //si volvio una vez, que se frene esta luz
          count1Flag = false; //que se frene...
        }

      }
      if (count <= THRESHOLD  && count2Flag == false && dir == DOWN) { //transicion de luz 1 a 2
        count2Flag = true;
        // Serial.print("   ");
        //Serial.print("Go 2");
      }
    }
    //////////////////
    if (count2Flag) {
      OSCMessage msg("/led2");
      msg.add(count2);
      UdpLED2.beginPacket(ipSalidaLED2, puertoSalidaLED2);
      msg.send(UdpLED2);
      UdpLED2.endPacket();
      msg.empty();
      if (dir2 == UP) {
        count2 += 10;
      }
      else if (dir2 == DOWN) {
        count2 -= 10;
      }

      if (count2 > TIME || count2 < 0) {

        dir2 = !dir2;
        if (count2 < 0) { //si volvio una vez, que se frene esta luz
          count2Flag = false; //que se frene...
        }

      }
      if (count2 <= THRESHOLD && count3Flag == false && dir2 == DOWN) { //transicion de luz 1 a 2
        count3Flag = true;
        //Serial.print("   ");
        //Serial.println("Go 3");
      }
    }

    if (count3Flag) {
      //Serial.print("count3:  ");
      //Serial.println(count3);
      OSCMessage msg2("/led3");
      //Serial.println(count3);
      msg2.add(count3);
      UdpLED2.beginPacket(ipSalidaLED3, puertoSalidaLED3);
      msg2.send(UdpLED2);
      UdpLED2.endPacket();
      msg2.empty();
      if (dir3 == UP) {
        count3 += 10;
      }
      else if (dir3 == DOWN) {
        count3 -= 10;
      }

      if (count3 > TIME || count3 < 0) {

        dir3 = !dir3;
        if (count3 < 0) { //si volvio una vez, que se frene esta luz
          count3Flag = false; //que se frene...
        }

      }
      if (count3 <= THRESHOLD && count1Flag == false && dir3 == DOWN) { //transicion de luz 1 a 2
        count1Flag = true;
        // Serial.print("   ");
        //Serial.println("Go 1");
      }
    }
    //////////////////


  }

  if (count1Flag && pirState) {
    pwmCount = fscale(count, 0, 9000, 65535, -3000, -3);

    analogWrite(D1, pwmCount);
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(2, LOW);
  }
  else {
    digitalWrite(2, HIGH); //nos desconectamo
    Serial.println("CONNECTION LOST");
    count = 0; //si perdemos conexion seteamos count en 0 (65535)para que cuando vuelva, no escriba el valor anterior, que quede apagado hasta que reciba el nuevo valor por osc
    analogWrite(D1, 65535); //y apaga el led inmediatamente
    ESP.restart();
  }
  //Serial.print("count2:  ");
  //Serial.println(count2);
  if (count3Flag) {
    // OSCMessage msg2("/led3");
    //Serial.println(count3);
    // msg2.add(count3);
    // UdpLED2.beginPacket(ipSalidaLED3, puertoSalidaLED3);
    //msg2.send(UdpLED2);
    // UdpLED2.endPacket();
    // msg2.empty();
  }
  //delay(1);
  if (count2Flag) {
    //OSCMessage msg("/led2");
    // msg.add(count2);
    // UdpLED2.beginPacket(ipSalidaLED2, puertoSalidaLED2);
    // msg.send(UdpLED2);
    // UdpLED2.endPacket();
    // msg.empty();
  }
  //delay(1);


  //analogWrite(9, 127);
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
