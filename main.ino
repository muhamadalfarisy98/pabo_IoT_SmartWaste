// Muhamad Alfarisy (23223056)
// PABO - pemrograman aplikasi berorientasi objek

#include <TinyGPS++.h> //Libary TinyGPS
#include <SoftwareSerial.h> //Library communication rx tx
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>

/*Put your SSID & Password*/
const char* ssid = "yourssid";  // Enter SSID here
const char* password = "yourpassword";  //Enter Password here

// Choose two Arduino pins to use for software serial
int RXPin = 16; //Connect ke TX GPS
int TXPin = 17; //Connect ke RX GPS

// ultrasonic pin
int trigPin = 18;
int echoPin = 19;

// indikator pin
int redPin = 27;
int bluePin = 26;
int greenPin = 25;

int LED_BUILTIN = 2;

// set baudrate
int GPSBaud = 9600; 

long duration;
int distance;

// Membuat objek TinyGPS++
TinyGPSPlus gps;

// Membuat koneksi serial dengan nama "gpsSerial"
SoftwareSerial gpsSerial(RXPin, TXPin);

WebServer server(80);

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

unsigned long task1Interval = 500;  // 1 second interval for task 1
unsigned long task2Interval = 500;   // 0.5 second interval for task 2

unsigned long previousTask1Millis = 0;
unsigned long previousTask2Millis = 0;

void init_communication() {
  // init serial communication
  // Memulai koneksi serial pada baudrate 9600
  Serial.begin(9600);

  //Memulai koneksi serial dengan sensor
  gpsSerial.begin(GPSBaud);
}

void init_pin(){
  pinMode (LED_BUILTIN, OUTPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  
  pinMode (greenPin, OUTPUT);
  pinMode (redPin, OUTPUT);
  pinMode (bluePin, OUTPUT);
}

void init_lcd(){
  lcd.init();
  lcd.backlight();
}

void setup()
{
  init_communication();
  init_pin();
  init_lcd();  

  Serial.println("Connecting to ..");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  // endpoint
  server.on("/", handle_home);
  server.on("/maps", handle_map);
  server.on("/volume", handle_volume);
  server.onNotFound(handle_notfound);

  server.begin();
  Serial.println("HTTP server started");
}

void indikatorOn(int val){
  if (val < 10) {
      trigLed(redPin);
  } else if (val > 10 and val < 20) {
      trigLed(bluePin);  
  } else if (val > 20) {
      trigLed(greenPin);  
   }
}

void read_ultrasonic(){
  lcd.setCursor(0, 0);
  lcd.print("Distance");
  
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;

  lcd.setCursor(0, 1);
  lcd.print(distance);
  delay(500);

  // clears the display to print new message
  lcd.clear();

  // mapping indikator
  indikatorOn(distance);
}


void trigLed(int Pin) {
  digitalWrite(Pin, HIGH);
  delay(250);
  digitalWrite(Pin, LOW);
  delay(250);
}

// run program
void loop()
{
  server.handleClient();
  server.enableCORS();

  if (millis() - previousTask1Millis >= task1Interval) {
    read_ultrasonic();;
    previousTask1Millis = millis();
  }
  
  while (gpsSerial.available() > 0)
    if (gps.encode(gpsSerial.read()))
      displayInfo();
  //Membuat tampilan data ketika terdapat koneksi
  
  // Jika dalam 5 detik tidak ada koneksi, maka akan muncul error "No GPS detected"
  // Periksa sambungan dan reset arduino
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println("No GPS detected");
    while(true);
  }
}

void displayInfo()
{
  // print_gps_date();
  print_gps_time();
  
  // mock data
  // print_mockdata();

  // print real_data 
  print_current_position();
}

void print_gps_date() {
  Serial.print("Date: ");
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print("/");
    Serial.print(gps.date.day());
    Serial.print("/");
    Serial.println(gps.date.year());
  }
  else
  {
    Serial.println("Not Available");
  }
}

void print_gps_time(){
  Serial.print("Time: ");
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(":");
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(":");
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(".");
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.println(gps.time.centisecond());
  }
  else
  {
    Serial.println("Not Available");
  }
}

void print_current_position() {
  Serial.print("Latitude: ");
  Serial.println(gps.location.lat(), 6);
  Serial.print("Longitude: ");
  Serial.println(gps.location.lng(), 6);
  Serial.println();
  delay(100);
}

void print_mockdata() {
  delay(1000);
  Serial.print("Latitude: ");
  Serial.println(-6.890431992479764);
  Serial.print("Longitude: ");
  Serial.println(107.61100193643512);
  Serial.println("\n");
}

void handle_home() {
  server.send(200, "text/json", "{\"message\": \"Welcome\", \"dev\": \"Muhamad Alfarisy (23223056)\",\"project\": \"Smart Waste Management\"}"); 
}

float roundFloat(float value, int numDecimalPlaces) {
  float multiplier = pow(10, numDecimalPlaces);
  return round(value * multiplier) / multiplier;
}

// handle_map - gets data gps
void handle_map() {
  char buffer_lon[15];  // Panjang buffer sesuaikan dengan jumlah digit dan tanda desimal yang diperlukan
  dtostrf(gps.location.lng(), 12, 9, buffer_lon);
  Serial.println(buffer_lon);

  char buffer_lat[15];  // Panjang buffer sesuaikan dengan jumlah digit dan tanda desimal yang diperlukan
  dtostrf(gps.location.lat(), 12, 8, buffer_lat);
  Serial.println(buffer_lat);

  char buffer[256];
  sprintf(buffer, "{\"latitude\": %s, \"longitude\": %s}", String(buffer_lat), String(buffer_lon));

  delay(50);
  // server.send(200, "text/json", "{\"latitude\": \"-1.645981300921818\", \"longitude\": \"103.584137051162\"}"); 
  server.send(200, "text/json", buffer); 
}

// handle_volume - handles volume tempat sampah
void handle_volume() {
  // volume , distance, status
  // status : kosong , sedang, penuh
  // if sensor > 100 - default 100
  int volume = 100 ;

  volume = volume - distance;   
  if (distance > 100) {
    volume = 0;
  } 
  String statusVal = mappingStatus(volume);

  char buffervolume[128];
  sprintf(buffervolume, "{\"volume\": %s, \"distance\": %s, \"status\":\"%s\"}", String(volume), String(distance), statusVal);
  // sprintf(buffer, "{\"latitude\": %s, \"longitude\": %s}", String(buffer_lat), String(buffer_lon));
  delay(100);
  server.send(200, "application/json", buffervolume); 
}

String mappingStatus(int volume ){
  if (volume < 10) {
    return "Kosong";
  } 
  if (volume > 10 and volume < 80) {
    return "Sedang";
  } 
  if (volume > 80) {
    return "Penuh";
  }
}

// handle_notfound - handles non registered endpoints
void handle_notfound(){
  server.send(404, "text/json", "{\"message\": \"not found\"}");
}
