//OLED TIME


#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP32Time.h>

#include <forcedClimate.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

ESP32Time rtc(0);  // offset in seconds GMT+1

RTC_DATA_ATTR unsigned long bootCount = 0;

const int threshold = 40;

//stato: 0 = inattivo, solo rtc
//stato: 1 = mostra ora
//stato: 2 = mostra data
//stato: 3 = imposta ora
//stato: 4 = imposta data
int stato = 1;

int stato_tmp = stato;

int en_display = 26;

int conta_temp = 51;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

ForcedClimate climateSensor = ForcedClimate(Wire, 0x76);

void callback(){
  //placeholder callback function
}

void setup(){
  if(bootCount == 0)
    rtc.setTime(0, 11, 16, 3, 2, 2023);
  ++bootCount;

  //pinMode(en_display, OUTPUT);
  Wire.begin(5, 4);

  climateSensor.begin();
  //Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();

  delay(500);
  stato = 1;
//Setup interrupt on Touch Pad 3 (GPIO15)
  touchAttachInterrupt(2, callback, threshold);
  touchAttachInterrupt(15, callback, threshold);

  orologio();
  //spengo monitor con transistor

  //Configure Touchpad as wakeup source
  esp_sleep_enable_touchpad_wakeup();

  //pinMode(en_display, INPUT);

  esp_deep_sleep_start();


}

void loop(){

}

void orologio() {
  display.ssd1306_command(SSD1306_DISPLAYON);
  //digitalWrite(en_display, HIGH);
  while(stato != 0){

    if(stato == 1){

      printTime();

      if(touchRead(2) < threshold){
        stato_tmp = 2;
      }

      if(touchRead(15) < threshold){
        stato_tmp = 2;
      }

      if(touchRead(2) < threshold && touchRead(15) < threshold){
        stato_tmp = 3;
      }

      if(touchRead(2) > threshold && touchRead(15) > threshold){
        stato = stato_tmp;
      }
    }

    if(stato == 2){
      printDate();

      if(touchRead(2) < threshold){
        stato_tmp = 3;
      }

      if(touchRead(15) < threshold){
        stato_tmp = 3;
      }

      if(touchRead(2) < threshold && touchRead(15) < threshold){
        stato_tmp = 5;
      }
      if(touchRead(2) > threshold && touchRead(15) > threshold){
        stato = stato_tmp;
      }
    }

    if(stato == 3){
      if(conta_temp > 50){
        climateSensor.begin();
        conta_temp = 0;
        printTemp();
      }

      conta_temp++;
      

      if(touchRead(2) < threshold){
        stato_tmp = 0;
      }

      if(touchRead(15) < threshold){
        stato_tmp = 0;
      }
      if(touchRead(2) > threshold && touchRead(15) > threshold){
        stato = stato_tmp;
      }
    }

    if(stato == 4){
      setTime();
      stato = 1;
      stato_tmp = 1;
    }

    if(stato == 5){
      setDate();
      stato = 2;
      stato_tmp = 2;
    }
    delay(100);
  }
  //digitalWrite(en_display, LOW);
  display.ssd1306_command(SSD1306_DISPLAYOFF);

}

void waitRelease(){
  while (touchRead(2) < threshold || touchRead(15) < threshold){
    delay(50);
  }
}

void setTime(){
  int tmp_h = rtc.getHour(true);
  int tmp_min = rtc.getMinute();

  waitRelease();

  while(touchRead(2) > threshold || touchRead(15) > threshold ){
    display.clearDisplay();
    display.setCursor(16, 0);
    display.setTextSize(2);
    display.print("Set Time");

    display.setCursor(18, 30);
    display.setTextSize(4);
    display.setTextWrap(false);
    display.print("H:");

    if(touchRead(15) < threshold){
      tmp_h++;
      if(tmp_h>23)
        tmp_h = 0;
    }

    if(touchRead(2) < threshold){
      tmp_h--;
      if(tmp_h < 0)
        tmp_h = 23;
    }
    display.print(tmp_h);
    display.display();

    delay(200);
  }

  waitRelease();

  while(touchRead(2) > threshold || touchRead(15) > threshold ){
    display.clearDisplay();
    display.setCursor(16, 0);
    display.setTextSize(2);
    display.print("Set Time");

    display.setCursor(18, 30);
    display.setTextSize(4);
    display.print("M:");
    if(touchRead(15) < threshold){
      tmp_min++;
      if(tmp_min>59)
        tmp_min = 0;
    }

    if(touchRead(2) < threshold){
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

void setDate(){
  int tmp_y = rtc.getYear();
  int tmp_m = rtc.getMonth() + 1;
  int tmp_d = rtc.getDay();

  waitRelease();

  while(touchRead(2) > threshold || touchRead(15) > threshold ){
    display.clearDisplay();
    display.setCursor(16, 0);
    display.setTextSize(2);
    display.print("Set Date");

    display.setCursor(18, 30);
    display.setTextSize(4);
    display.print("D:");
    if(touchRead(15) < threshold){
      tmp_d++;
      if(tmp_d>31)
        tmp_d = 0;
    }

    if(touchRead(2) < threshold){
      tmp_d--;
      if(tmp_d < 0)
        tmp_d = 31;
    }

    display.print(tmp_d);
    display.display();

    delay(200);
  }

  waitRelease();

  while(touchRead(2) > threshold || touchRead(15) > threshold ){
    display.clearDisplay();
    display.setCursor(16, 0);
    display.setTextSize(2);
    display.print("Set Date");

    display.setCursor(18, 30);
    display.setTextSize(4);
    display.print("M:");
    if(touchRead(15) < threshold){
      tmp_m++;
      if(tmp_m>12)
        tmp_m = 0;
    }

    if(touchRead(2) < threshold){
      tmp_m--;
      if(tmp_m < 0)
        tmp_m = 12;
    }
    display.print(tmp_m);
    display.display();

    delay(200);
  }

  waitRelease();

  while(touchRead(2) > threshold || touchRead(15) > threshold ){
    display.clearDisplay();
    display.setCursor(16, 0);
    display.setTextSize(2);
    display.print("Set Date");

    display.setCursor(12, 30);
    display.setTextSize(3);
    display.print("Y:");
    if(touchRead(15) < threshold){
      tmp_y++;
      if(tmp_y>2100)
        tmp_y = rtc.getYear();
    }

    if(touchRead(2) < threshold){
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
  display.setTextSize(4);
  display.setCursor(6,20);
  display.print(rtc.getTime("%H:%M"));
  display.display();
}

void printDate(){
  display.clearDisplay();
  display.setTextSize(2);
  int centra = (128 - (rtc.getTime("%A").length() * 12)) /2;
  display.setCursor(centra,10);  
  display.print(rtc.getTime("%A"));

  display.setTextSize(2);
  display.setCursor(18,40);
  display.print(rtc.getTime("%d/%m/%y"));
  display.display();
}

void printTemp() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(20,0);
  display.print("Weather");
  display.setTextSize(2);
  display.setCursor(20,26);
  display.print(climateSensor.getTemperatureCelcius());
  display.print(" C");
  display.setTextSize(2);
  display.setCursor(20,46);
  display.print(climateSensor.getPressure());
  display.display();
}
