#include <WiFi.h>
#include <TFT_eSPI.h> 
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <vector> 
#include <HTTPClient.h>
#include <ArduinoJson.h> 
#include "orb.h"
#include "frame.h"

TFT_eSPI tft = TFT_eSPI(); 

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;  

const char* ssid     = "xxxxxx";       ///EDIIIT
const char* password = "xxxxxx"; //edit
int timeZone=1; //edit
#define gray 0x39C7
#define dblue 0x01A9
#define purple 0xF14F
#define green 0x2D51


String payload="";
const String endpoint ="https://api.cryptonator.com/api/ticker/btc-usd";

double current=0;
double last=0;

double readings[12]={0};
int n=0;
int fromtop=60;
int f=0; //frame in animation

double minimal;
double maximal;

int p[12]={0};

unsigned long currTime=0;
unsigned long refresh=120000;

int deb=0;
int brightnes[5]={40,80,120,160,200};
int b=1;

int spe=0; //speed of animation/

StaticJsonDocument<6000> doc;

void setup() {
  pinMode(35,INPUT_PULLUP);
  Serial.begin(9600);
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, brightnes[b]);
  
  WiFi.begin(ssid, password);
  tft.print("connecting");
  

  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    tft.print(".");
   }
tft.print("CONECTED!!");
delay(1000);
tft.fillScreen(TFT_BLACK);
getData();
}

void loop() {

  if(millis()>currTime+refresh)
  {
  getData();
  currTime=millis(); 
  }

  if(spe>8000){
  tft.pushImage(4,22,38,38,frame[f]);
  spe=0;
   f++;
  if(f==59)
  f=0;
  }
 

  if(digitalRead(35)==0)
  {
    if(deb==0)
    {
      deb=1;
    b++;
    if(b==6)
    b=0;
    ledcWrite(pwmLedChannelTFT, brightnes[b]); 
    }
    }else deb=0;
  
  spe++;
}

void getData()
   {
    tft.fillScreen(TFT_BLACK);
    //tft.fillRect(200,126,4,4,TFT_GREEN);
    tft.fillRect(46,32,56,28,dblue);

    //tft.fillRect(118,22,120,100,dblue);
    
    for(int i=0;i<13;i++)
    tft.drawLine(118+(i*10),22,118+(i*10),122,gray);
    for(int i=0;i<10;i++)
    tft.drawLine(118,22+(i*10),238,22+(i*10),gray);
    tft.drawLine(118,22,118,122,TFT_WHITE);
    tft.drawLine(118,122,238,122,TFT_WHITE);
    if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
   HTTPClient http;
   http.begin(endpoint); //Specify the URL
   int httpCode = http.GET();  //Make the request
   if (httpCode > 0) { //Check for the returning code
   payload = http.getString();
   char inp[payload.length()];
   payload.toCharArray(inp,payload.length());
   deserializeJson(doc,inp);

   String v=doc["ticker"]["price"];
   String c=doc["ticker"]["change"];
   String t=doc["timestamp"];
   Serial.print(t);
   unsigned long t1=t.toInt()+(timeZone*3600);
   //day(t), month(t), year(t), hour(t), minute(t), second(t)

   tft.setTextColor(TFT_WHITE,dblue);
   tft.drawString("updated:",50,37);
   tft.drawString(String(hour(t1))+":"+String(minute(t1)),50,47);
   tft.setTextColor(TFT_WHITE,TFT_BLACK);
   
   current=v.toDouble();
   tft.drawString("PRICE (usd):",4,fromtop+4,2);
   tft.drawString("CHANGE:",4,fromtop+32+8,2);
   tft.setFreeFont(&Orbitron_Medium_16);
    tft.setTextColor(green,TFT_BLACK);
   tft.drawString(String(current),4,fromtop+20);
   tft.setTextColor(TFT_ORANGE,TFT_BLACK);
   tft.drawString("bitCoin",4,0);
    tft.setTextColor(green,TFT_BLACK);
   
   tft.drawString(String(current-last),4,fromtop+46+10,2);
   tft.setTextColor(0x0B52,TFT_BLACK);
   tft.setTextFont(1);
   tft.drawString("LAST 12 READINGS",118,6);
tft.setTextColor(TFT_ORANGE,TFT_BLACK);
   tft.setTextFont(1);
   tft.drawString("MAX",94,16);
   tft.drawString("MIN",94,122,1);
   last=current;

   if(n<12)
   {readings[n]=current;
   n++;}
   else
   {
    for(int i=1;i<12;i++)
    readings[i-1]=readings[i];
    readings[11]=current;
    }

     minimal=readings[0];
     maximal=readings[0];
     
   for(int i=0;i<n;i++){
   
   if(readings[i]<minimal)
   minimal=readings[i]; 
   if(readings[i]>maximal)
   maximal=readings[i]; 

   int mx=maximal/2;
   int mi=minimal/2;
   int re=readings[i]/2;
   //tft.drawString(String(i)+"."+String(readings[i]),120,i*10);
   
   }
   int mx=maximal/2;
   int mi=minimal/2;
  
   for(int i=0;i<n;i++)
   {
   int re=readings[i]/2;
   p[i]=map(re,mi,mx,0,100);
  // tft.drawString(String(p[i]),190,i*10);
    }
    if(n>=1)
    for(int i=1;i<n;i++){
    tft.drawLine(118+((i-1)*10),122-p[i-1],118+((i)*10),122-p[i],TFT_RED);
    tft.fillCircle(118+((i-1)*10),122-p[i-1],2,TFT_RED);
    tft.fillCircle(118+((i)*10),122-p[i],2,TFT_RED);
    }
   //tft.drawString(String(minimal),190,10);
   //tft.drawString(String(maximal),190,20);
   }
    }}
