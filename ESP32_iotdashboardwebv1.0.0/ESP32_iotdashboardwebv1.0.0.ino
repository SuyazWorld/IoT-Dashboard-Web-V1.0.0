#include <WiFi.h>
#include <MQTT.h>
WiFiClient clientwifi; //karena kita pakai wifi connect ke mqtt, jadi kita buat objek wifi untuk mqtt
MQTTClient clientmqtt; //kita juga buat objek mqtt
const char clientID[] = "ESP32-01"; //this device client id, harus char, bisa sih string tapi nanti diubah lagi dari string ke char
// const char ssid[] = "Wifi kost lantai 1";
// const char pass[] = "enamenamtigaenam";
const char ssid[] = "wadudu";
const char pass[] = "bgstt123";
const char* brokerserver =  "prickleshift857.cloud.shiftr.io";
const char* userbrokermqtt = "prickleshift857";      // Tipe data const char*
const char* passbrokermqtt = "kelasiotcoba12";      // Tipe data const char*

String topic1 = "kelasiot/#";
String topic_masuk_baru;
String Data_payload_baru;

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int lebar = 128;
const int tinggi = 64;
const int reset = 4; 
Adafruit_SSD1306 oledz(lebar,tinggi,&Wire, reset);

#include <ESP32Servo.h>
int servopins1 = 33, servopins2 = 5;  
Servo servo1ku;
Servo servo2ku;
int led[] = {12,14,27};
int ledyellow = 25, ledblue = 13;
unsigned long wktuskrg = 0;
unsigned long wktuskrg2 = 0;

#define poten1 32
#define poten2 35
int potenread1, potenread2;
int old_pot1 = 0, old_pot2 = 0;

#include <NewPing.h>
#define TRIGGER_PIN 26
#define ECHO_PIN 26
NewPing sonar(TRIGGER_PIN, ECHO_PIN, 400);;
int jarak, old_jarak = 0;

void setup() {
  Serial.begin(115200);
  for(int i = 0; i < 3; i++){
    pinMode(led[i], OUTPUT);
  }
  pinMode(ledblue, OUTPUT);
  pinMode(ledyellow, OUTPUT);
  servo1ku.attach(servopins1); 
  servo2ku.attach(servopins2);
  oledz.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oledz.clearDisplay();
  rgbled(0,0,0);

  WiFi.begin(ssid, pass);
  clientmqtt.begin(brokerserver, clientwifi); //start mqtt with object mqtt //parameter("broker", wifiobject)
  // Serial.println("Menghubungkan ke Wifi");
  clientmqtt.onMessage(pesansubscribemasuk);
  reconnect();
  
}

void loop() {
  clientmqtt.loop(); //untuk mengecek apakah ada data yang diterima
  if(!clientmqtt.connected()){
    reconnect();
  }
  pesanclient_doit();
  publish_poten();
  publish_ultras();
}
void reconnect(){
  rgbled(0,1,0);
  //checking wifi
  // while (!WiFi.isConnected())  // Untuk ESP32 (tergantung library)
  while (WiFi.status() != WL_CONNECTED){
    // Serial.println("");
    // Serial.println("menghubungkan");
    lcdkecil(1,1,"WifiSTAT = ---");
    lcdkecil(1,10,"MQTTSTAT = ---");
    oledz.clearDisplay();
    delay(500);
    rgbled(1,1,1);
  }
  rgbled(1,0,0);
  // Serial.println("Berhasil Terhubung");
  lcdkecil(1,2,"WifiConnected");
  oledz.clearDisplay();
  //put setwill, this is used for device connection status, if device still not connected to mqtt
  clientmqtt.setWill("kelasiot/ESP32-01/Status", "offline", true, 1); //topic, payload, retain,  //topicnya di fungsi setwill ini tidak bisa String melainkan harus char
  //checking mqtt
  // while(!clientmqtt.connect(clientID,username,password))
  while(!clientmqtt.connect(clientID, userbrokermqtt, passbrokermqtt)){
    // Serial.println("");
    // Serial.println("MQTT menghubungkan");
    lcdkecil(1,1,"WifiSTAT = Connected");
    lcdkecil(1,10,"MQTTSTAT = ---");
    oledz.clearDisplay();
    delay(500);
    rgbled(1,1,1);
  }
  rgbled(0,0,1);
  // Serial.println("MQTT Berhasil Terhubung");
  lcdkecil(1,1,"WifiSTAT = Connected");
  lcdkecil(1,10,"MQTTSTAT = Connected");
  //lalu kita publish status online, untuk memberitahu client lain kalau device ini online
  clientmqtt.publish("kelasiot/ESP32-01/Status", "online", true, 1);
  //subscribe
  clientmqtt.subscribe(topic1, 1); //topic, qos
}
void pesansubscribemasuk(String &topicmasuk, String &payloadmasuk){
  topic_masuk_baru = topicmasuk;
  Data_payload_baru = payloadmasuk;
  // Serial.println("");
  // Serial.println("Ada data masuk, Topic: " + topicmasuk + ", data: " + payloadmasuk);
}
void pesanclient_doit(){
  int dataled = Data_payload_baru.toInt();
  if(topic_masuk_baru == "kelasiot/ESP32-01/led1"){
    if(dataled == 1){
      digitalWrite(ledblue, HIGH);
    }
    else if(dataled == 0){
      digitalWrite(ledblue, LOW);
    }
  }
  if(topic_masuk_baru == "kelasiot/ESP32-01/led2"){
    if(dataled == 1){
      digitalWrite(ledyellow, HIGH);
    }
    else if(dataled == 0){
      digitalWrite(ledyellow, LOW);
    }
  } 
  if(topic_masuk_baru == "kelasiot/ESP32-01/servoxxx"){
    //kita buat dulu variable untuk mengatur derajat servo, variable ini berfungsi mengubah String dari "datapayload" yang kita terima dari publisher menjadi int
    int servoposition =  Data_payload_baru.toInt(); //ini kita tambah .toInt() untuk mengubah String ke int
    servo1ku.write(servoposition); //ini juga void yang aku bikin, bisa dilihat dari file jalaninservo.h yang aku bikin
    Serial.println("Servo 1 Position: " + String(servoposition));
    // lcdkecil(1, 40, "Servo: " + Data_payload_baru);
  }
  if(topic_masuk_baru == "kelasiot/ESP32-01/srvvzz"){
    int servoposition2 =  Data_payload_baru.toInt();
    servo2ku.attach(servopins2); 
    servo2ku.write(servoposition2);
    Serial.println("Servo 2 Position: " + String(servoposition2));
  }
  lcdkecil(1, 20, "Topic: " + topic_masuk_baru);
  lcdkecil(1, 30, "Data: " + String(Data_payload_baru));
}
void lcdkecil(int koor1, int koor2, String katakata){
  // Clear a specific rectangular area where the new text will be written
  // Arguments: (x position, y position, width, height, color)
  // Width = 128 (full width of the OLED display)
  // Height = 10 (enough to clear one line of text; adjust if needed)
  oledz.fillRect(koor1, koor2, 128, 10, BLACK); 

  // Set the text size to 1 (smallest size available)
  oledz.setTextSize(1); 

  // Set the text color to WHITE (OLED displays text in white on a black background)
  oledz.setTextColor(WHITE);

  // Set the cursor position where the text will start
  // koor1 = x coordinate, koor2 = y coordinate
  oledz.setCursor(koor1, koor2);

  // Print the text on the display buffer
  oledz.println(katakata);

  // Push the updated display buffer to the actual screen
  oledz.display();
}
void rgbled(int pin1, int pin2, int pin3){
    digitalWrite(led[0], pin1);
    digitalWrite(led[1], pin2);
    digitalWrite(led[2], pin3);
}
void publish_poten(){
  potenread1 = analogRead(poten1);
  potenread2 = analogRead(poten2);
  if(millis()-wktuskrg > 100){
    wktuskrg = millis();
    if(potenread1 != old_pot1){
      // String pesan4 = "poten1: " + String(potenread1);
      // String(potenread1).c_str() mengubah string ke char
      clientmqtt.publish("kelasiot/ESP32-01/pot1", String(potenread1).c_str(), true, 1);
      old_pot1 = potenread1;
    }
    if(potenread2 != old_pot2){
      // String pesan5 = "poten2: " + String(potenread2);
      clientmqtt.publish("kelasiot/ESP32-01/pot2", String(potenread2).c_str(), true, 1);
      old_pot2 = potenread2;
    }
    lcdkecil(1, 40, "POT1: " + String(potenread1));
    lcdkecil(1, 50, "POT2: " + String(potenread2));
    // Serial.println("POT1: " + String(potenread1));
    // Serial.println("POT2: " + String(potenread2));
  }
}
void publish_ultras(){
  jarak = sonar.ping_cm();
  if(millis() - wktuskrg2 > 500){
    wktuskrg2 = millis();
    if(jarak != old_jarak){
      String pesan6 = "jarak: " + String(jarak);
      clientmqtt.publish("kelasiot/ESP32-01/ultras",  String(jarak).c_str(), true, 1);
      // Serial.println("ULTRAS: " + String(jarak));
      old_jarak = jarak;
    }
    // lcdkecil(1, 60, "ULTRAS: " + jarak);
  }
}