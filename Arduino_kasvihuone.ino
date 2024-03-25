#include <Adafruit_ST7789.h>   //.kbv for ENES
#include <Fonts/FreeSans9pt7b.h>   //.kbv use FreeFonts etc
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#define Arial_24 &FreeSans9pt7b    //.kbv FreeFonts use address
#define Arial_48 &FreeSans12pt7b
#define Arial_60 &FreeSans18pt7b
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
//kosteus ja lämpötila
#include <DHT22.h>
// servo
#include <Servo.h>
Servo myServo;
#define CS_PIN  4  //Waveshare Touch
#define TFT_DC  7  //Waveshare 
#define TFT_CS 10  //Waveshare
#define TFT_BL  9  //Waveshare backlight
// MOSI=11, MISO=12, SCK=13
//XPT2046_Touchscreen ts(CS_PIN);
XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, 255);  // Param 2 - 255 - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled pollg
//#define TIRQ_PIN  2
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_BL); //.kbv for ENES

//DHT22
#define pinDATA 2 // SDA, or almost any other I/O pin

DHT22 dht22(pinDATA); 

enum property{
  temperature = 0,
  humidity = 1
};

int door = 0;
int check = 0;
int tempSetting = 20;
int humdSetting = 50;
float temp;
int humd;
int currentPage = 0;
int MAXX = 3790;
int MINX = 390;
int MAXY = 3750;
int MINY = 430;
bool openDoor = false;
bool pumpOn = false;
unsigned long startTime = millis();
int angle;
int sense = 0;
int value;
int handMode = true;
int oviSensori = 0;

property selection;

void setup() {
  Serial.begin(9600);
  //tft.begin();  //.kbv diff method used for ST7789
  tft.init(240, 320, SPI_MODE0); //.kbv for ENES
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  ts.begin();
  ts.setRotation(1);
  while (!Serial && (millis() <= 1000));
  homescreen();
  pinMode(2,INPUT);
  myServo.attach(8);
  pinMode(6,OUTPUT);

}

void loop() {
 if (millis() - startTime > 2000 && currentPage == 0){ // kahden sekunnin välein lukee arvot ja päivittää pääsivun arvot kunhan on pääsivulla
  value = analogRead(sense)/10;                       // Hakee mullankosteus arvon ja jakaa sen 10 jotta saataisiin vähän pienempiä lukemia
  drawTemperature();
  drawHumidity();
  startTime = millis();

} 
if(handMode == false){
if(value > 20 && pumpOn == true){ //Pumppu pois päältä kun multa on liian kuivaa
  pumpOn = false;            
  checkPump();
  }
if(value < 20 && pumpOn == false){ //Pumppu päälle kun multa on tarpeeksi kosteaa
  pumpOn = true;              
  checkPump();
  }

if(humd > humdSetting && temp >= tempSetting && angle == map(0,0,1023,0,179)){ // Ovi auki lämpötilan ja kosteusprosentin mukaan
  angle = map(1023, 0, 1023, 0, 179);
  myServo.write(angle);
  openDoor = true;
  checkDoor();

}
if(humd < humdSetting && temp < tempSetting && angle == map(1023,0,1023,0,179)){ // Ovi kiinni lämpötila ja kosteusprosentin mukaan
  angle = map(0,0,1023,0,179);
  myServo.write(angle);
  openDoor = false;
  checkDoor();

}
}

TS_Point p;                           
while(ts.touched())
p = ts.getPoint();
  p.x = map(p.x, MAXX, MINX, 320, 0);
  p.y = map(p.y,MAXY,MINY, 0, 240);
  if(currentPage == 1){                                     //Jos sivu on asetussivu  niin tästä alaspäin koskee sitä.
    
    if((p.x >= 0)&& (p.x <= 100) && (p.y >=0) && (p.y <= 60)){  // BACK BUTTON
      currentPage = 0;
      homescreen();
    }
    if((p.x >= 10) && (p.x <= 140) && (p.y >= 80) && (p.y <= 120)){ // LÄMPÖTILAVALINTA
        setSelection(0);
        tft.setTextColor(RED);
        tft.setFont(Arial_48);
        tft.setCursor(10,120);
        tft.print("Temperature");
        tft.setTextColor(WHITE);
        tft.setCursor(10,180);
        tft.print("Humidity");

    }
        else if((p.x >= 10) && (p.x <= 140) && (p.y >= 140) && (p.y <= 180)){ // HUMIDITY
        setSelection(1);
        tft.setTextColor(RED);
        tft.setCursor(10,180);
        tft.print("Humidity");
        tft.setTextColor(WHITE);
        tft.setFont(Arial_48);
        tft.setCursor(10,120);
        tft.print("Temperature");

    }

    if((p.x > 107) && (p.x < 214) && (p.y >= 0 ) && (p.y <= 60)){ // ylös nappula 
      setSelectedValue(getSelectedValue()+1);
    }
    if((p.x > 214) && (p.x < 323) && (p.y >= 0) && ( p.y <= 60)){ // alas nappula
      setSelectedValue(getSelectedValue()-1);
    }
  }
  else if(currentPage == 0){                            //Jos sivu on kotisivu eli 0 niin tästä eteenpäin siihen liittyvät

    if((p.x >= 120) && (p.x <= 200) && (p.y >= 130) && (p.y <= 210) && handMode == true){  // Oven nappula 
      openDoor = !openDoor;
      checkDoor(); 
    }
    if((p.x >= 220) && (p.x <= 300) && (p.y >= 130) && (p.y <= 210) && handMode == true){ // Pumpun nappula
      pumpOn = !pumpOn;
      checkPump();
    }
    if((p.x >= 20) && (p.x <= 100) && (p.y >= 130) && (p.y <= 210)){  // Asetukset nappula
      currentPage = 1;
      settings();
      
      

    }
    if((p.x >= 10) && (p.x <= 50) && (p.y >= 10 ) && ( p.y <= 50)){
      handMode = !handMode;
      drawHandmode();
    }

}
delay(50);

}

void setSelection(property prop){
  selection = prop;
  drawSettingNumber(getSelectedValue());
}

int getSelectedValue(){
  switch(selection){
    case 0:
     return tempSetting;
    case 1:
      return humdSetting;
  }
  return 0;
}
void setSelectedValue(int number){
  switch(selection){
    case 0:
      tempSetting = number;
      break;
    case 1:
      humdSetting = number;
      break;
  }
drawSettingNumber(number);
}

//katsoo onko ovi auki vai kiinni
void checkDoor(){
      tft.setFont(Arial_24);
  if(openDoor == false && currentPage == 0){
      Serial.print("paska");
      tft.fillCircle(160,170,40,GREEN);
      tft.setCursor(143,155);
      tft.print("OVI");
      tft.setCursor(138,185);
      tft.print("KIINNI");
      angle = map(0,0,1023,0,179);
      myServo.write(angle); 
    } else if (openDoor == true && currentPage == 0){ 
      Serial.print("kusi");
      Serial.print(openDoor);
      tft.fillCircle(160,170,40,RED);
      tft.setCursor(143,155);
      tft.print("OVI");
      tft.setCursor(134,185);
      tft.print("AUKI");
      angle = map(1023,0,1023,0,179);
      myServo.write(angle);
    }
}
//katsoo onko pumppu päällä vai pois
void checkPump(){
    tft.setFont(Arial_24);
    if(!pumpOn){
    tft.fillCircle(260,170,40, GREEN);
    tft.setCursor(223,155);
    tft.print("PUMPPU");
    tft.setCursor(244,185);
    tft.print("OFF");
    digitalWrite(6,LOW);
    } else {
    tft.fillCircle(260,170,40, RED);
    tft.setCursor(223,155);
    tft.print("PUMPPU");
    tft.setCursor(244,185);
    tft.print("ON");
    digitalWrite(6,HIGH);
    }
}
//piirtää näyttöön asetusruudussa valitun asetusvaihtoehdon
void drawSettingNumber(int number){
  tft.fillRect(180,120,100,100,BLACK);
  tft.setFont(Arial_48);
  tft.setCursor(200,150);
  tft.print(number);
  if(selection == 0){
    tft.print(" C");
  } else {
    tft.print(" %");
  }
}
 // Piirtää näyttöön mitatun lämpötilan
void drawTemperature(){
tft.fillRect(60,10,200,40, BLUE);
tft.setFont(Arial_48);
tft.setCursor(70,40);
tft.print("Temp: ");
temp = dht22.getTemperature();
tft.print(temp);
tft.print(" C");
}

//piirtää näyttöön mitatun kosteuden
void drawHumidity(){
tft.fillRect(60,60,200,40, BLUE);
tft.setCursor(70,90);
humd = dht22.getHumidity();
tft.print("Hum: ");
tft.print(humd);
tft.print(" %");
}

//Käsi/automaatti
void drawHandmode(){
  tft.fillRect(10,10,40,90,WHITE);
  tft.setTextColor(BLACK);
  tft.setCursor(20,60);
  tft.print("A");
  tft.setTextColor(WHITE);

  if(handMode == true){
    tft.fillRect(12,12,36,44,GREEN);
    tft.setCursor(20,35);
    tft.setFont(Arial_48);
    tft.setTextColor(BLACK);
    tft.print("K");
    tft.fillRect(12,52,36,44,RED);
    tft.setCursor(20,80);
    tft.print("A");
    tft.setTextColor(WHITE);

  } else {
    tft.fillRect(12,12,36,44,RED);
    tft.setCursor(20,35);
    tft.setFont(Arial_48);
    tft.setTextColor(BLACK);
    tft.print("K");
    tft.fillRect(12,52,36,44,GREEN);
    tft.setCursor(20,80);
    tft.print("A");
      tft.setTextColor(WHITE);
  }

  
}



void settings(){
  tft.setFont(Arial_48);
  tft.fillScreen(BLACK);
  tft.fillRect(107,0,107,60,BLUE); // vasen nappi ylös
  tft.drawRect(107,0,107,60,BLACK);
  tft.fillTriangle(133,50,187,50,160,10,WHITE);
  tft.fillRect(214,0,107,60,BLUE); // oikee nappi alas
  tft.drawRect(214,0,107,60,BLACK);
  tft.fillTriangle(240,10,294,10,267,50,WHITE);
  tft.fillRect(0,0,106,60,RED); // takaisin paluu nappi
  tft.drawRect(0,0,106,60,BLACK);
  tft.setCursor(10,40);
  tft.print("BACK");
  tft.setCursor(10,120);
  tft.print("Temperature");
  tft.setCursor(10,180);
  tft.print("Humidity");
  setSelection(0);
  tft.setTextColor(RED);
  tft.setCursor(10,120);
  tft.print("Temperature");
  tft.setTextColor(WHITE);
  tft.setCursor(10,180);
  tft.print("Humidity");
}

void homescreen(){
tft.fillScreen(BLACK);
tft.fillRect(60,10,200,40, BLUE); //ylempi sininen neliö
tft.fillRect(60,60,200,40, BLUE); // alempi sininen neliö
tft.fillCircle(60,170,40,BLUE); // SETTINGS
checkDoor();
drawHandmode();
checkPump();
drawTemperature();
drawHumidity();
}
