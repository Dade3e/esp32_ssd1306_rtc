//OLED TIME


#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP32Time.h>

#include "DHT.h"

#include <forcedClimate.h>

#define DHTPIN 4    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

#define TOUCH1 15
#define TOUCH2 13
#define TOUCH3 12

#define LED 14

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

ForcedClimate climateSensor = ForcedClimate();


ESP32Time rtc(0);  // offset in seconds GMT+1

RTC_DATA_ATTR unsigned long bootCount = 0;

RTC_DATA_ATTR unsigned long clock_H = 15;
RTC_DATA_ATTR unsigned long clock_M = 36;
RTC_DATA_ATTR bool svegliaAttiva = true;
RTC_DATA_ATTR unsigned long tempo_rimanente = 24*60*60;

DHT dht(DHTPIN, DHTTYPE);

const int threshold = 55;

//BUZZER
int atn = 162;
int atn2 = 50;

int freq = 2000;
int channel = 0;
int resolution = 8;

//stato: 0 = inattivo, solo rtc
//stato: 1 = mostra ora
//stato: 2 = mostra data
//stato: 3 = meteo
//stato: 4 = sveglia
//stato: 10 = imposta ora
//stato: 11 = imposta data
//stato: 11 = imposta sveglia
int stato = 1;

int stato_tmp = stato;

int en_display = 26;

float h = 0;
float t = 0;

int mySongNotes[] =  {740, 831, 932, 1109, 1245, 1480, 1661, 1865,  2217, 740, 831,   932, 1109, 1245,  1480, 1661, 1865, 2489, 2489, 2489};
int mySongDelay1[] = {atn, atn, atn*2, atn, atn, atn*2, atn,  atn, atn*2, atn, atn, atn*2,  atn,  atn, atn*2,  atn,  atn,  atn*2,  atn,  atn};
int mySongDelay2[] = {atn2, atn2, atn2,atn2,atn2,atn2, atn2, atn2,  atn2, atn2,atn2, atn2, atn2, atn2,  atn2, atn2, atn2, atn2, atn2, atn2 };

bool svegliaSuona = true;

//infinity 16x8
const uint8_t PROGMEM sveglia[]{
B00001100, B01100000,
B00011011, B10110000,
B00010100, B01010000,
B00001001, B00100000,
B00001001, B10100000,
B00001001, B00100000,
B00000100, B01000000,
B00001011, B10100000
};

const uint8_t PROGMEM clock16[]{
B00000000, B00000000,
B00000000, B00000000,
B00001100, B01100000,
B00011000, B00110000,
B00110111, B11011000,
B00101000, B00101000,
B00010001, B00010000,
B00100001, B00001000,
B00100001, B00001000,
B00100001, B00001000,
B00100000, B10001000,
B00100000, B01001000,
B00010000, B00010000,
B00001000, B00100000,
B00010111, B11010000,
B00000000, B00000000
};





void callback(){
  //placeholder callback function
}

void setup(){
  if(bootCount == 0)
    rtc.setTime(30, 35, 16, 10, 2, 2023);   //s:m:h g/m/y
  ++bootCount;

  //pinMode(en_display, OUTPUT);
  //Wire.begin(5, 4); //se cambio da sda 21 e scl 22
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(18, channel);
  ledcWrite(channel, 127);
  ledcWriteTone(channel, 0);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  dht.begin();

  climateSensor.begin();
  //Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setRotation(2);
  display.display();
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();

  //delay(500);
  stato = 1;
//Setup interrupt on Touch Pad 3 (GPIO15)
  touchAttachInterrupt(TOUCH1, callback, threshold);
  touchAttachInterrupt(TOUCH2, callback, threshold);
  touchAttachInterrupt(TOUCH3, callback, threshold);

  orologio();
  //spengo monitor con transistor

  //Configure Touchpad as wakeup source
  esp_sleep_enable_touchpad_wakeup();
  esp_sleep_enable_timer_wakeup(tempo_rimanente * uS_TO_S_FACTOR);

  //pinMode(en_display, INPUT);

  esp_deep_sleep_start();


}

void loop(){

}

void orologio() {
  //digitalWrite(en_display, HIGH);
  display.ssd1306_command(SSD1306_DISPLAYON);
  while(stato != 0){
    float h_tmp = dht.readHumidity();
    float t_tmp = dht.readTemperature();

    //SVEGLIA
    if(rtc.getHour(true) == clock_H && rtc.getMinute() == clock_M && svegliaAttiva)
      stato = 13;
    else
      svegliaSuona = true;

    if (!isnan(h_tmp) && !isnan(t_tmp)) {
      h = h_tmp;
      t = t_tmp;
    }
    if(stato == 1){

      printTime();

      if(touchRead(TOUCH1) < threshold){
        stato_tmp = 0;
      }

      if(touchRead(TOUCH2) < threshold){
        stato_tmp = 2;
      }

      if(touchRead(TOUCH3) < threshold){
        stato_tmp = 10;
      }

      if(touchRead(TOUCH1) < threshold && touchRead(TOUCH2) < threshold){
        stato_tmp = 20;
      }

      if(touchRead(TOUCH1) > threshold && touchRead(TOUCH2) > threshold && touchRead(TOUCH3) > threshold){
        stato = stato_tmp;
      }

      
    }

    if(stato == 2){
      printDate();

      if(touchRead(TOUCH1) < threshold){
        stato_tmp = 1;
      }

      if(touchRead(TOUCH2) < threshold){
        stato_tmp = 3;
      }

      if(touchRead(TOUCH3) < threshold){
        stato_tmp = 11;
      }

      if(touchRead(TOUCH1) > threshold && touchRead(TOUCH2) > threshold && touchRead(TOUCH3) > threshold){
        stato = stato_tmp;
      }
    }

    if(stato == 3){

      printTemp();
      

      if(touchRead(TOUCH1) < threshold){
        stato_tmp = 2;
      }

      if(touchRead(TOUCH2) < threshold){
        stato_tmp = 4;
      }
      if(touchRead(TOUCH1) > threshold && touchRead(TOUCH2) > threshold){
        stato = stato_tmp;
      }
    }

    if(stato == 4){

      printClock();

      if(touchRead(TOUCH1) < threshold){
        stato_tmp = 3;
      }

      if(touchRead(TOUCH2) < threshold){
        stato_tmp = 0;
      }

      if(touchRead(TOUCH3) < threshold){
        stato_tmp = 12;
      }

      if(touchRead(TOUCH1) < threshold && touchRead(TOUCH2) < threshold){
        stato_tmp = 20;
      }

      if(touchRead(TOUCH1) > threshold && touchRead(TOUCH2) > threshold && touchRead(TOUCH3) > threshold){
        stato = stato_tmp;
      }
    }

    if(stato == 10){
      setTime();
      stato = 1;
      stato_tmp = 1;
    }

    if(stato == 11){
      setDate();
      stato = 2;
      stato_tmp = 2;
    }

    if(stato == 12){
      setClock();
      stato = 4;
      stato_tmp = 4;
    }

    if(stato == 13){
      printTime();
      if(svegliaSuona){
        //for (int i=0;i< i < (sizeof(mySongNotes) / sizeof(mySongNotes[0])) -1; i++ ){
        for (int i=0;i <18; i++ ){
          if(touchRead(TOUCH1) < threshold || touchRead(TOUCH2) < threshold){
            svegliaSuona = false;
          }
          if(svegliaSuona){
            ledcWriteTone(channel, mySongNotes[i]);
            delay(mySongDelay1[i]);  
            ledcWriteTone(channel, 0);
            delay(mySongDelay2[i]);
          }
        }
      }
      stato = 1;
      stato_tmp = 1;
    }

    if(stato == 20){
      int clock_EN = svegliaAttiva;
      clock_EN = clock_EN *-1 +1;
      display.clearDisplay();
      display.setCursor(30, 10);
      display.setTextSize(2);
      display.print("Allarm");

      display.setCursor(40, 32);
      display.setTextSize(3);
      if(clock_EN)
        display.print("ON");
      else
        display.print("OFF");

      display.display();
      svegliaAttiva = clock_EN;
      delay(1000);
      stato = 1;
      stato_tmp = 1;
    }
    delay(100);
  }
  int tmp_h = rtc.getHour(true);
  int tmp_min = rtc.getMinute();
  
  if(clock_H < tmp_h){
    tmp_h = tmp_h + 24 - clock_H;
  }else{
    tmp_h = tmp_h - clock_H;
  }

  if(clock_M < tmp_min){
    tmp_min = tmp_min + 24 - clock_M;
  }else{
    tmp_min = tmp_min - clock_M;
  }

  tempo_rimanente = ((tmp_h * 60) + tmp_min ) * 60;
  //digitalWrite(en_display, LOW);
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  
}

void waitRelease(){
  while (touchRead(TOUCH1) < threshold || touchRead(TOUCH2) < threshold || touchRead(TOUCH3) < threshold){
    delay(50);
  }
}

void setTime(){
  int tmp_h = rtc.getHour(true);
  int tmp_min = rtc.getMinute();

  waitRelease();

  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setCursor(16, 10);
    display.setTextSize(2);
    display.print("Set Time");

    display.setCursor(18, 30);
    display.setTextSize(4);
    display.setTextWrap(false);
    display.print("H:");

    if(touchRead(TOUCH2) < threshold){
      tmp_h++;
      if(tmp_h>23)
        tmp_h = 0;
    }

    if(touchRead(TOUCH1) < threshold){
      tmp_h--;
      if(tmp_h < 0)
        tmp_h = 23;
    }
    display.print(tmp_h);
    display.display();

    delay(200);
  }

  waitRelease();

  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setCursor(16, 10);
    display.setTextSize(2);
    display.print("Set Time");

    display.setCursor(18, 30);
    display.setTextSize(4);
    display.print("M:");
    if(touchRead(TOUCH2) < threshold){
      tmp_min++;
      if(tmp_min>59)
        tmp_min = 0;
    }

    if(touchRead(TOUCH1) < threshold){
      tmp_min--;
      if(tmp_min < 0)
        tmp_min = 59;
    }

    display.print(tmp_min);
    display.display();

    delay(200);
  }

  waitRelease();

  rtc.setTime(0, tmp_min, tmp_h, rtc.getDay(), rtc.getMonth() + 1, rtc.getYear());
}

void setClock(){
  int tmp_h = clock_H;
  int tmp_min = clock_M;

  waitRelease();

  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setCursor(14, 10);
    display.setTextSize(2);
    display.print("Set Clock");

    display.setCursor(18, 30);
    display.setTextSize(4);
    display.setTextWrap(false);
    display.print("H:");

    if(touchRead(TOUCH2) < threshold){
      tmp_h++;
      if(tmp_h>23)
        tmp_h = 0;
    }

    if(touchRead(TOUCH1) < threshold){
      tmp_h--;
      if(tmp_h < 0)
        tmp_h = 23;
    }
    display.print(tmp_h);
    display.display();

    delay(200);
  }

  waitRelease();

  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setCursor(14, 10);
    display.setTextSize(2);
    display.print("Set Clock");

    display.setCursor(18, 30);
    display.setTextSize(4);
    display.print("M:");
    if(touchRead(TOUCH2) < threshold){
      tmp_min++;
      if(tmp_min>59)
        tmp_min = 0;
    }

    if(touchRead(TOUCH1) < threshold){
      tmp_min--;
      if(tmp_min < 0)
        tmp_min = 59;
    }

    display.print(tmp_min);
    display.display();

    delay(200);
  }

  waitRelease();

  int clock_EN = svegliaAttiva;
  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setCursor(14, 10);
    display.setTextSize(2);
    display.print("Set Clock");

    display.setCursor(24, 30);
    display.setTextSize(4);
    if(touchRead(TOUCH2) < threshold){
      clock_EN = clock_EN *-1 +1;
    }

    if(touchRead(TOUCH1) < threshold){
      clock_EN = clock_EN *-1 +1;
    }

    if(clock_EN)
      display.print("ON");
    else
      display.print("OFF");

    display.display();

    delay(200);
  }

  waitRelease();

  clock_H = tmp_h;
  clock_M = tmp_min;

  svegliaAttiva = clock_EN;
}

void setDate(){
  int tmp_y = rtc.getYear();
  int tmp_m = rtc.getMonth() + 1;
  int tmp_d = rtc.getDay();

  waitRelease();

  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setCursor(16, 10);
    display.setTextSize(2);
    display.print("Set Date");

    display.setCursor(18, 30);
    display.setTextSize(4);
    display.print("D:");
    if(touchRead(TOUCH2) < threshold){
      tmp_d++;
      if(tmp_d>31)
        tmp_d = 0;
    }

    if(touchRead(TOUCH1) < threshold){
      tmp_d--;
      if(tmp_d < 0)
        tmp_d = 31;
    }

    display.print(tmp_d);
    display.display();

    delay(200);
  }

  waitRelease();

  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setCursor(16, 10);
    display.setTextSize(2);
    display.print("Set Date");

    display.setCursor(18, 30);
    display.setTextSize(4);
    display.print("M:");
    if(touchRead(TOUCH2) < threshold){
      tmp_m++;
      if(tmp_m>12)
        tmp_m = 0;
    }

    if(touchRead(TOUCH1) < threshold){
      tmp_m--;
      if(tmp_m < 0)
        tmp_m = 12;
    }
    display.print(tmp_m);
    display.display();

    delay(200);
  }

  waitRelease();

  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setCursor(16, 10);
    display.setTextSize(2);
    display.print("Set Date");

    display.setCursor(12, 30);
    display.setTextSize(3);
    display.print("Y:");
    if(touchRead(TOUCH2) < threshold){
      tmp_y++;
      if(tmp_y>2100)
        tmp_y = rtc.getYear();
    }

    if(touchRead(TOUCH1) < threshold){
      tmp_y--;
      if(tmp_y < 2022)
        tmp_y = 2100;
    }
    display.print(tmp_y);
    display.display();

    delay(200);
  }

  waitRelease();

  rtc.setTime(rtc.getSecond(), rtc.getMinute(), rtc.getHour(true), tmp_d, tmp_m, tmp_y);

}

void printTime(){
  display.clearDisplay();
  if(svegliaAttiva && svegliaSuona)
    display.drawBitmap(50, 0, clock16, 16, 16, WHITE);
  
  display.setTextSize(4);
  display.setCursor(0,28);
  display.print(rtc.getTime("%H:%M"));
  display.display();
}

void printClock(){
  display.clearDisplay();
  display.setCursor(30, 10);
  display.setTextSize(2);
  display.print("Clock");

  display.setTextSize(3);
  display.setCursor(16,36);
  if(clock_H <10)
    display.print("0");
    
  display.print(clock_H);
  display.print(":");
  if(clock_M <10)
    display.print("0");
    
  display.print(clock_M);
  display.display();
}

void printDate(){
  display.clearDisplay();
  display.setTextSize(2);
  int centra = (128 - (rtc.getTime("%A").length() * 12)) /2;
  display.setCursor(centra,12);  
  display.print(rtc.getTime("%A"));

  display.setTextSize(2);
  display.setCursor(14,40);
  display.print(rtc.getTime("%d/%m/%y"));
  display.display();
}

void printTemp() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(22,0);
  display.print("Weather");
  display.setTextSize(2);
  display.setCursor(22,22);
  //display.print(t);
  display.print(climateSensor.getTemperatureCelcius());
  display.print(" C");
  display.setTextSize(2);
  display.setCursor(6,46);
  //display.print(h);
  float p = climateSensor.getPressure();
  if(p < 1000)
    display.setCursor(10,46);
  display.print(p);
  display.print("hPa");
  display.display();
  
}

void printPres() {

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10,10);
  display.print("Pressure:");

  display.setTextSize(3);
  display.setCursor(0,40);
  //display.print(dht.readHumidity());
  display.print(climateSensor.getPressure());
  display.display();
}
