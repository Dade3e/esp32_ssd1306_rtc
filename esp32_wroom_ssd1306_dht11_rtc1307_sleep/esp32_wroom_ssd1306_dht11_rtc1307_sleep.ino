//Sveglia per Ele :3
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "RTClib.h"


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

RTC_DS1307 RTC;

RTC_DATA_ATTR unsigned long bootCount = 0;

RTC_DATA_ATTR unsigned long clock_H = 7;
RTC_DATA_ATTR unsigned long clock_M = 45;
RTC_DATA_ATTR bool svegliaAttiva = true;
RTC_DATA_ATTR bool showLeft = true;

DHT dht(DHTPIN, DHTTYPE);

//soglia touch
const int threshold = 55;

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

//DHT11
float h = 0;
float t = 0;


//BUZZER
int atn = 162;
int atn2 = 50;

int freq = 2000;
int channel = 0;
int resolution = 8;

int mySongNotes[] =  {740, 831, 932, 1109, 1245, 1480, 1661, 1865,  2217, 740, 831,   932, 1109, 1245,  1480, 1661, 1865, 2489, 2489, 2489};
int mySongDelay1[] = {atn, atn, atn*2, atn, atn, atn*2, atn,  atn, atn*2, atn, atn, atn*2,  atn,  atn, atn*2,  atn,  atn,  atn*2,  atn,  atn};
int mySongDelay2[] = {atn2, atn2, atn2,atn2,atn2,atn2, atn2, atn2,  atn2, atn2,atn2, atn2, atn2, atn2,  atn2, atn2, atn2, atn2, atn2, atn2 };

bool svegliaSuona = true;

String week_days[] =  {"Dimanche","Lundi","Mardi","Mercredi","Jeudi","Vendredi","Samedi"};

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

const uint8_t PROGMEM heart48_1[]{
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x3, 0xe0, 0xf, 0x80, 0x0, 
0x0, 0xf, 0xf8, 0x3f, 0xe0, 0x0, 
0x0, 0x1f, 0xfc, 0x7f, 0xf0, 0x0, 
0x0, 0x3f, 0xfe, 0xff, 0xf8, 0x0, 
0x0, 0x3f, 0xfe, 0xff, 0xf8, 0x0, 
0x0, 0x7f, 0xff, 0xff, 0xfc, 0x0, 
0x0, 0x7f, 0xff, 0xff, 0xfc, 0x0, 
0x0, 0x7f, 0xff, 0xff, 0xfc, 0x0, 
0x0, 0x7f, 0xff, 0xff, 0xfc, 0x0, 
0x0, 0x7f, 0xff, 0xff, 0xfc, 0x0, 
0x0, 0x3f, 0xff, 0xff, 0xf8, 0x0, 
0x0, 0x3f, 0xff, 0xff, 0xf8, 0x0, 
0x0, 0x1f, 0xff, 0xff, 0xf0, 0x0, 
0x0, 0xf, 0xff, 0xff, 0xe0, 0x0, 
0x0, 0x7, 0xff, 0xff, 0xc0, 0x0, 
0x0, 0x3, 0xff, 0xff, 0x80, 0x0, 
0x0, 0x1, 0xff, 0xff, 0x0, 0x0, 
0x0, 0x0, 0xff, 0xfe, 0x0, 0x0, 
0x0, 0x0, 0x7f, 0xfc, 0x0, 0x0, 
0x0, 0x0, 0x3f, 0xf8, 0x0, 0x0, 
0x0, 0x0, 0x1f, 0xf0, 0x0, 0x0, 
0x0, 0x0, 0xf, 0xe0, 0x0, 0x0, 
0x0, 0x0, 0x7, 0xc0, 0x0, 0x0, 
0x0, 0x0, 0x3, 0x80, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

const uint8_t PROGMEM heart48_2[]{
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x3, 0xe0, 0xf, 0x80, 0x0, 
0x0, 0x1f, 0xf8, 0x3f, 0xf0, 0x0, 
0x0, 0x3f, 0xfc, 0x7f, 0xf8, 0x0, 
0x0, 0x7f, 0xfe, 0xff, 0xfc, 0x0, 
0x0, 0xff, 0xfe, 0xff, 0xfe, 0x0, 
0x1, 0xff, 0xff, 0xff, 0xff, 0x0, 
0x1, 0xff, 0xff, 0xff, 0xff, 0x0, 
0x3, 0xff, 0xff, 0xff, 0xff, 0x80, 
0x3, 0xff, 0xff, 0xff, 0xff, 0x80, 
0x3, 0xff, 0xff, 0xff, 0xff, 0x80, 
0x3, 0xff, 0xff, 0xff, 0xff, 0x80, 
0x3, 0xff, 0xff, 0xff, 0xff, 0x80, 
0x3, 0xff, 0xff, 0xff, 0xff, 0x80, 
0x1, 0xff, 0xff, 0xff, 0xff, 0x0, 
0x1, 0xff, 0xff, 0xff, 0xff, 0x0, 
0x0, 0xff, 0xff, 0xff, 0xfe, 0x0, 
0x0, 0xff, 0xff, 0xff, 0xfe, 0x0, 
0x0, 0x7f, 0xff, 0xff, 0xfc, 0x0, 
0x0, 0x3f, 0xff, 0xff, 0xf8, 0x0, 
0x0, 0x1f, 0xff, 0xff, 0xf0, 0x0, 
0x0, 0xf, 0xff, 0xff, 0xe0, 0x0, 
0x0, 0x7, 0xff, 0xff, 0xc0, 0x0, 
0x0, 0x3, 0xff, 0xff, 0x80, 0x0, 
0x0, 0x1, 0xff, 0xff, 0x0, 0x0, 
0x0, 0x0, 0xff, 0xfe, 0x0, 0x0, 
0x0, 0x0, 0x7f, 0xfc, 0x0, 0x0, 
0x0, 0x0, 0x3f, 0xf8, 0x0, 0x0, 
0x0, 0x0, 0x1f, 0xf0, 0x0, 0x0, 
0x0, 0x0, 0xf, 0xe0, 0x0, 0x0, 
0x0, 0x0, 0x7, 0xc0, 0x0, 0x0, 
0x0, 0x0, 0x3, 0x80, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

const uint8_t PROGMEM e_acc1[]{
B00001100, 
B00011000, 
B00000000, 
B01110000, 
B10001000, 
B11111000, 
B10000000, 
B01110000
};

const uint8_t PROGMEM e_acc2[]{
B00000001, B10000000, 
B00000111, B10000000, 
B00000110, B00000000, 
B00000000, B00000000, 
B00111111, B00000000, 
B00111111, B00000000, 
B11000000, B11000000, 
B11000000, B11000000, 
B11111111, B11000000, 
B11111111, B11000000, 
B11000000, B00000000, 
B11000000, B00000000, 
B00111111, B00000000, 
B00111111, B00000000, 
B00000000, B00000000, 
B00000000, B00000000
};

void callback(){
  //placeholder callback function
}

void setup(){
  RTC.begin();
   
  if(bootCount == 0){
    RTC.adjust(DateTime(2023, 3, 2, 16, 38, 0));
  }
    
  ++bootCount;

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

  touchAttachInterrupt(TOUCH1, callback, threshold);
  touchAttachInterrupt(TOUCH2, callback, threshold);
  touchAttachInterrupt(TOUCH3, callback, threshold);

  stato = 1;

  orologio();

  if(showLeft && svegliaAttiva){
    printTempoRimanente();
    delay(3000);
  }
  unsigned long sec_wake_up = sec_rimanenti();
  
  display.ssd1306_command(SSD1306_DISPLAYOFF);

  if(svegliaAttiva)
    esp_sleep_enable_timer_wakeup( sec_wake_up * uS_TO_S_FACTOR);

  esp_sleep_enable_touchpad_wakeup();
  
  delay(100);

  esp_deep_sleep_start();

}

void loop(){

}

void orologio() {
  //esp_sleep_wakeup_cause_t wakeup_reason;
  //wakeup_reason = esp_sleep_get_wakeup_cause();
  //if(wakeup_reason == ESP_SLEEP_WAKEUP_TIMER && sec_rimanenti() > 120)
  //  stato = 0;
  int wait = 0;
  int sens = 0;
  int disabilita = 0;
  display.ssd1306_command(SSD1306_DISPLAYON);
  while(stato != 0 && disabilita < (30*5) ){
    disabilita++;
    //SVEGLIA
    DateTime now = RTC.now();
    if(now.hour() == clock_H && now.minute() == clock_M && svegliaAttiva)
      stato = 13;
    else
      svegliaSuona = true;
    
    if(stato == 1){
      if(wait == 0)
        printTime(sens);
      wait++;
      if(wait >= 4){
        sens = sens * -1 + 1;
        wait = 0;
      }

      if(touchRead(TOUCH1) < threshold){
        stato_tmp = 0;
        disabilita = 0;
      }

      if(touchRead(TOUCH2) < threshold){
        stato_tmp = 2;
        disabilita = 0;
      }

      if(touchRead(TOUCH3) < threshold){
        stato_tmp = 10;
        disabilita = 0;
      }

      if(touchRead(TOUCH1) < threshold && touchRead(TOUCH2) < threshold){
        stato_tmp = 20;
      }

      if(touchRead(TOUCH1) > threshold && touchRead(TOUCH2) > threshold && touchRead(TOUCH3) > threshold){
        stato = stato_tmp;
      }
      
    }

    if(stato == 2){
      wait = 0;
      printDate();

      if(touchRead(TOUCH1) < threshold){
        stato_tmp = 1;
        disabilita = 0;
      }

      if(touchRead(TOUCH2) < threshold){
        stato_tmp = 3;
        disabilita = 0;
      }

      if(touchRead(TOUCH3) < threshold){
        stato_tmp = 11;
        disabilita = 0;
      }

      if(touchRead(TOUCH1) > threshold && touchRead(TOUCH2) > threshold && touchRead(TOUCH3) > threshold){
        stato = stato_tmp;
      }
    }

    if(stato == 3){
      if(wait == 0)
        if(sens)
          printTempDHT11(); 
        else  
          printTempBMP280();
      wait++;
      if(wait >= 25)
        wait = 0;

      if(touchRead(TOUCH1) < threshold){
        stato_tmp = 2;
        disabilita = 0;
      }

      if(touchRead(TOUCH2) < threshold){
        stato_tmp = 4;
        disabilita = 0;
      }

      if(touchRead(TOUCH3) < threshold){
        sens = sens * -1 + 1;
        wait = 0;
        waitRelease();
        disabilita = 0;
      }

      if(touchRead(TOUCH1) > threshold && touchRead(TOUCH2) > threshold){
        stato = stato_tmp;
      }
    }

    if(stato == 4){
      wait = 0;
      printClock();

      if(touchRead(TOUCH1) < threshold){
        stato_tmp = 3;
        disabilita = 0;
      }

      if(touchRead(TOUCH2) < threshold){
        stato_tmp = 0;
        disabilita = 0;
      }

      if(touchRead(TOUCH3) < threshold){
        stato_tmp = 12;
        disabilita = 0;
      }

      if(touchRead(TOUCH1) < threshold && touchRead(TOUCH2) < threshold){
        stato_tmp = 20;
        disabilita = 0;
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
      disabilita = 0;
      printTime(1);
      bool heart = false;
      if(svegliaSuona){
        //for (int i=0;i< i < (sizeof(mySongNotes) / sizeof(mySongNotes[0])) -1; i++ ){
        for (int i=0;i <18; i++ ){
          if(touchRead(TOUCH1) < threshold || touchRead(TOUCH2) < threshold){
            svegliaSuona = false;
          }
          if(touchRead(TOUCH3) < threshold){
            svegliaSuona = false;
            heart = true;
          }
          if(svegliaSuona){
            ledcWriteTone(channel, mySongNotes[i]);
            delay(mySongDelay1[i]);  
            ledcWriteTone(channel, 0);
            delay(mySongDelay2[i]);
          }
        }
      }
      if(heart){
        for (int i =0; i < 10; i++){
          display.display();
          display.clearDisplay();
          display.setCursor(10, 10);
          display.setTextSize(1);
          display.print("Mi piaci un mondo");
          if(i%2==0)
            display.drawBitmap(40, 16, heart48_1, 48, 48, WHITE);
          else{
            display.drawBitmap(40, 16, heart48_2, 48, 48, WHITE);
          }

          display.display();
          delay(1000);
        }
        heart = false;
      }
      stato = 1;
      stato_tmp = 1;
    }

    if(stato == 20){
      int clock_EN = svegliaAttiva;
      clock_EN = clock_EN *-1 +1;
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(16, 6);
      display.print("R");
      display.drawBitmap(16+12, 6, e_acc2, 16, 16, WHITE);
      display.setCursor(16+12+12, 6);
      display.print("veille");

      display.setTextSize(2);
      
      if(clock_EN){
        display.setCursor(30, 36);
        display.print("Actif");
      }
      else{
        display.setTextSize(2);
        display.setCursor(10, 36);
        display.print("D");
        display.drawBitmap(10+12, 36, e_acc2, 16, 16, WHITE);
        display.setCursor(10+24, 36);
        display.print("sactiv");
        display.drawBitmap(10 + 12*8, 36, e_acc2, 16, 16, WHITE);
      }

      display.display();
      svegliaAttiva = clock_EN;
      delay(1000);
      stato = 1;
      stato_tmp = 1;
    }
    delay(200);
  }
}

void waitRelease(){
  while (touchRead(TOUCH1) < threshold || touchRead(TOUCH2) < threshold || touchRead(TOUCH3) < threshold){
    delay(50);
  }
}

int sec_rimanenti(){
  DateTime now = RTC.now();
  int tmp_h = now.hour();
  int tmp_min = now.minute();
  //int tmp_sec = rtc.getSecond();
  
  if(clock_M < tmp_min){
    tmp_min = clock_M + 59 - tmp_min;
  }else{
    tmp_min = clock_M - tmp_min ;
  }

  if(clock_H < tmp_h){
    tmp_h = clock_H + 23 - tmp_h;
  }else if(clock_H == tmp_h){
    if(clock_M < tmp_min)
      tmp_h = clock_H + 23 - tmp_h;
    else
      tmp_h = clock_H - tmp_h;      
  }else{
    tmp_h = clock_H - tmp_h;
  }
  return ((tmp_h * 60) + tmp_min ) * 60;
}

void setTime(){
  DateTime now = RTC.now();
  int tmp_h = now.hour();
  int tmp_min = now.minute();

  waitRelease();

  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(14, 12);
    display.print("R");
    display.drawBitmap(14+7, 11, e_acc1, 8, 8, WHITE);
    display.setCursor(14+14, 12);
    display.print("gler le temps");

    display.setCursor(24, 36);
    display.setTextSize(3);
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
    display.setTextSize(1);
    display.setCursor(14, 12);
    display.print("R");
    display.drawBitmap(14+7, 11, e_acc1, 8, 8, WHITE);
    display.setCursor(14+14, 12);
    display.print("gler le temps");

    display.setCursor(24, 36);
    display.setTextSize(3);
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
  now = RTC.now();
  RTC.adjust(DateTime(now.year(), now.month(), now.day(), tmp_h, tmp_min, 0));
}

void setClock(){
  int tmp_h = clock_H;
  int tmp_min = clock_M;

  waitRelease();

  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(14, 12);
    display.print("R");
    display.drawBitmap(14+7, 11, e_acc1, 8, 8, WHITE);
    display.setCursor(14+14, 12);
    display.print("gler l'alarme");


    display.setCursor(24, 36);
    display.setTextSize(3);
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
    display.setTextSize(1);
    display.setCursor(14, 12);
    display.print("R");
    display.drawBitmap(14+7, 11, e_acc1, 8, 8, WHITE);
    display.setCursor(14+14, 12);
    display.print("gler l'alarme");

    display.setCursor(24, 36);
    display.setTextSize(3);
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
  /*
  while(touchRead(TOUCH3) > threshold){
    int tmp_left = showLeft;
    display.clearDisplay();
    display.setCursor(40, 0);
    display.setTextSize(2);
    display.print("Show");
    display.setTextSize(1);
    display.setCursor(0,20);
    display.print("Time left to wake up");
    if(touchRead(TOUCH2) < threshold){
      tmp_left = tmp_left *-1 +1;
    }

    if(touchRead(TOUCH1) < threshold){
      tmp_left = tmp_left *-1 +1;
    }
    display.setCursor(40, 42);
    display.setTextSize(3);
    if(tmp_left){
      display.setCursor(40, 32);
      display.print("YES");
    }
    else{
      display.setCursor(50, 32);
      display.print("NO");        
    }

    display.display();
    showLeft = tmp_left;
    delay(200);
  }

  waitRelease();
  */

  clock_H = tmp_h;
  clock_M = tmp_min;
}

void setDate(){
  DateTime now = RTC.now();

  int tmp_y = now.year();
  int tmp_m = now.month();
  int tmp_d = now.day();

  waitRelease();

  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(14, 12);
    display.print("R");
    display.drawBitmap(14+7, 11, e_acc1, 8, 8, WHITE);
    display.setCursor(14+14, 12);
    display.print("gler la date");

    display.setCursor(24, 36);
    display.setTextSize(3);
    display.print("J:");
    if(touchRead(TOUCH2) < threshold){
      tmp_d++;
      if(tmp_d>31)
        tmp_d = 1;
    }

    if(touchRead(TOUCH1) < threshold){
      tmp_d--;
      if(tmp_d <= 0)
        tmp_d = 31;
    }

    display.print(tmp_d);
    display.display();

    delay(200);
  }

  waitRelease();

  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(14, 12);
    display.print("R");
    display.drawBitmap(14+7, 11, e_acc1, 8, 8, WHITE);
    display.setCursor(14+14, 12);
    display.print("gler la date");

    display.setCursor(24, 36);
    display.setTextSize(3);
    display.print("M:");
    if(touchRead(TOUCH2) < threshold){
      tmp_m++;
      if(tmp_m>12)
        tmp_m = 1;
    }

    if(touchRead(TOUCH1) < threshold){
      tmp_m--;
      if(tmp_m <= 0)
        tmp_m = 12;
    }
    display.print(tmp_m);
    display.display();

    delay(200);
  }

  waitRelease();

  while(touchRead(TOUCH3) > threshold){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(14, 12);
    display.print("R");
    display.drawBitmap(14+7, 11, e_acc1, 8, 8, WHITE);
    display.setCursor(14+14, 12);
    display.print("gler la date");

    display.setCursor(8, 36);
    display.setTextSize(3);
    display.print("A:");
    if(touchRead(TOUCH2) < threshold){
      tmp_y++;
      if(tmp_y>2099)
        tmp_y = 2023;
    }

    if(touchRead(TOUCH1) < threshold){
      tmp_y--;
      if(tmp_y < 2023)
        tmp_y = 2099;
    }
    display.print(tmp_y);
    display.display();

    delay(200);
  }

  waitRelease();

  now = RTC.now();
  RTC.adjust(DateTime( tmp_y, tmp_m, tmp_d, now.hour(), now.minute(), now.second()));
}

void printTime(bool sens){
  display.clearDisplay();
  if(svegliaAttiva && svegliaSuona)
    display.drawBitmap(52, -2, clock16, 16, 16, WHITE);
  
  display.setTextSize(4);
  display.setCursor(2,26);
  DateTime now = RTC.now();
  if(sens){
    if(now.hour() <10)
      display.print("0");
    display.print(now.hour());
    display.print(":");
    if(now.minute() <10)
      display.print("0");
    display.print(now.minute());
  }else{
    if(now.hour() <10)
      display.print("0");
    display.print(now.hour());
    display.print(" ");
    if(now.minute() <10)
      display.print("0");
    display.print(now.minute()); 
  }
  display.display();
}

void printClock(){
  display.clearDisplay();
  display.setCursor(16, 6);
  display.setTextSize(2);
  display.print("R");
  display.drawBitmap(16+12, 6, e_acc2, 16, 16, WHITE);
  display.setCursor(16+12+12, 6);
  display.print("veille");

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
  DateTime now = RTC.now();
  String day = week_days[now.dayOfTheWeek()];
  int centra = (128 - (day.length() * 12)) /2;
  display.setCursor(centra,12);
  //display.print(rtc.getTime("%A"));
  display.print(day);
  display.setTextSize(2);
  display.setCursor(14,40);
  if(now.day() <10)
    display.print("0");
  display.print(now.day());
  display.print("/");
  if(now.month() <10)
    display.print("0");
  display.print(now.month());
  display.print("/");
  display.print(now.year()%2000);
  display.display();
}

void printTempBMP280() {
  climateSensor.begin();
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(30,0);
  display.print("Temps");
  display.setTextSize(2);
  display.setCursor(22,26);
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

void printTempDHT11() {
  float h_tmp = dht.readHumidity();
  float t_tmp = dht.readTemperature();
  if (!isnan(h_tmp) && !isnan(t_tmp)) {
    h = h_tmp;
    t = t_tmp;
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(30,0);
  display.print("Temps");
  display.setTextSize(2);
  display.setCursor(22,26);
  display.print(t);
  display.print(" C");
  display.setTextSize(2);
  display.setCursor(22,46);
  display.print(h);
  display.print(" %");
  display.display();
  
}

void printTempoRimanente(){
  DateTime now = RTC.now();
  int tmp_h = now.hour();
  int tmp_min = now.minute();
  
  if(clock_M < tmp_min){
    tmp_min = clock_M + 59 - tmp_min;
  }else{
    tmp_min = clock_M - tmp_min ;
  }

  if(clock_H < tmp_h){
    tmp_h = clock_H + 23 - tmp_h;
  }else if(clock_H == tmp_h){
    if(clock_M < tmp_min)
      tmp_h = clock_H + 23 - tmp_h;
    else
      tmp_h = clock_H - tmp_h;      
  }else{
    tmp_h = clock_H - tmp_h;
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(20,6);
  //display.print("Time left to wake up");
  display.print("Temps restant");
  display.setCursor(18,16);
  display.print("pour le r");
  display.drawBitmap(7*10+2, 15, e_acc1, 8, 8, WHITE);
  display.setCursor(7*11+1, 16);
  display.print("veil");

  display.setTextSize(2);
  display.setCursor(0,40);
  //if(tmp_h <10)
  //  display.print("0");
    
  display.print(tmp_h);
  display.print(" H ");
  //if(tmp_min <10)
  //  display.print("0");
    
  display.print(tmp_min);
  display.print(" m");
  display.display();
}