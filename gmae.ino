// Paint example specifically for the TFTLCD breakout board.
// If using the Arduino shield, use the tftpaint_shield.pde sketch instead!
// DOES NOT CURRENTLY WORK ON ARDUINO LEONARDO
//Technical support:goodtft@163.com

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include "DHT.h"

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin

#define TS_MINX 130
#define TS_MAXX 905

#define TS_MINY 75
#define TS_MAXY 930

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define DHTPIN 22
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
int oldcolor, currentcolor;

int stars_i[100];
int stars_j[100];
int star_count = 0;
int loop_count = 0;

int ship_x;
int ship_y;
int ship_h;
int ship_last_x = ship_x;
int ship_last_y = ship_y; 

const int button_fire = 22;
const int button_left = 26;
const int button_right = 24;

int left_state = 0;
int right_state = 0;
int fire_state = 0;

int shots[] = {50,50,50,0,0,0,0,0,0};

int begin_st = true;

#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
void setup(void) {
  Serial.begin(9600);

  tft.reset();

  uint16_t identifier = tft.readID();

  pinMode(button_left, INPUT);
  pinMode(button_right, INPUT);
  pinMode(button_fire, INPUT);
  
  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  }else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  }else if(identifier == 0x7783) {
    Serial.println(F("Found ST7781 LCD driver"));
  }else if(identifier == 0x8230) {
    Serial.println(F("Found UC8230 LCD driver"));  
  }
  else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if(identifier==0x0101)
  {     
      identifier=0x9341;
       Serial.println(F("Found 0x9341 LCD driver"));
  }else if(identifier==0x9481)
  {     
       Serial.println(F("Found 0x9481 LCD driver"));
  }
  else if(identifier==0x9486)
  {     
       Serial.println(F("Found 0x9486 LCD driver"));
  }
  else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    identifier=0x9486;
    
  }
  
  tft.begin(identifier);
  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());
  tft.setRotation(0);

  tft.fillScreen(BLACK);

  /*void drawRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);*/
  tft.drawRect(0,0,tft.width(),tft.height(),WHITE);
  tft.drawRect(0,0,tft.width(),50,WHITE);

  //51 draw stars
  int rando_j = 0;
  int rando_i = 0;
  int v = 0;
  for (v; v < 100; v++) {
      rando_i = random(1, tft.width() - 1);
      rando_j = random(51, tft.height() - 1);
      stars_i[v] = rando_i;
      stars_j[v] = rando_j;
  }
  int i = 0;
  for (i; i < 100; i++) {
    tft.drawPixel(stars_i[i], stars_j[i], WHITE);
  }

  //draw ship
  tft.fillTriangle((tft.width()/2)-10,tft.height()-30,(tft.width()/2)+10,tft.height()-30, tft.width()/2, tft.height() - 50,WHITE);
  tft.drawTriangle((tft.width()/2)-10,tft.height()-30,(tft.width()/2)+10,tft.height()-30, tft.width()/2, tft.height() - 50,RED);
  ship_x = tft.width()/2;
  ship_y = tft.height() - 30;
  ship_h = 20;
  pinMode(13, OUTPUT);

  // draw title and score

  tft.setCursor(10,20);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
    tft.print("S");
  delay(30);
    tft.print("P");
  delay(30);
    tft.print("A");
  delay(30);
    tft.print("C");
  delay(30);
    tft.print("E");
  delay(30);
    tft.print(" ");
  delay(30);  
    tft.print("Q");
  delay(30);
    tft.print("U");
  delay(30);
    tft.print("E");
  delay(30);
    tft.print("S");
  delay(30);
    tft.print("T");
  delay(30);

  tft.setCursor(200,20);
      tft.print("S");
  delay(30);
    tft.print("C");
  delay(30);  
    tft.print("O");
  delay(30);
    tft.print("R");
  delay(30);
    tft.print("E");
  delay(30);
    tft.print(":");
  delay(30);
    tft.print(" ");
  delay(30);
    tft.print("0");
  delay(30);
}

#define MINPRESSURE 10
#define MAXPRESSURE 1000

void loop()
{
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  left_state = digitalRead(button_left);
  right_state = digitalRead(button_right);
  fire_state = digitalRead(button_fire);

  
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  tft.fillRect(1,50,tft.width() - 2, 10, BLACK);
  tft.drawLine(0,0,0,tft.height(), WHITE);

  if (begin_st == true) {
    tft.setCursor(tft.width()/2 - 10,tft.height()/2);
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.print("3");
    delay(1000);
    tft.setTextColor(BLACK);
    tft.setCursor(tft.width()/2 - 10,tft.height()/2);
    tft.print("3");
    tft.setTextColor(WHITE);
    tft.setCursor(tft.width()/2 - 10,tft.height()/2);
    tft.print("2");
    delay(1000);
    tft.setTextColor(BLACK);
    tft.setCursor(tft.width()/2 - 10,tft.height()/2);
    tft.print("2");
    tft.setTextColor(WHITE);
    tft.setCursor(tft.width()/2 - 10,tft.height()/2);
    tft.print("1");
    delay(1000);
    tft.setTextColor(BLACK);
    tft.setCursor(tft.width()/2 - 10,tft.height()/2);
    tft.print("1");
    begin_st = false;
  }

  
  if (left_state == HIGH && right_state == HIGH && fire_state == HIGH) {
    delay(150);
    int p_state = false;
    int l;
    int r;
    int f;
    tft.setCursor(tft.width()/2 - 50,tft.height()/2);
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.print("PAUSE");
    tft.setTextColor(BLACK);
    while(p_state != true) {
      l = digitalRead(button_left);
      r = digitalRead(button_right);
      f = digitalRead(button_fire);
      if(l == HIGH && r == HIGH && f == HIGH) {
        p_state = true;
        tft.setCursor(tft.width()/2 - 50,tft.height()/2);
        tft.print("PAUSE");
      }
      delay(50);
    }
  }
  
  if (loop_count == 0) {
    loop_count = 0;
    if (star_count == 100) {
      star_count = 0;
    }
    int rando_i = random(1, tft.width() - 1);
    int rando_j = random(51, tft.height() - 1);
    tft.drawPixel(stars_i[star_count], stars_j[star_count], BLACK);
    stars_i[star_count] = rando_i;
    stars_j[star_count] = rando_j;
    tft.drawPixel(stars_i[star_count], stars_j[star_count], WHITE);
    star_count++;
  }
  //loop_count++;

  ship_last_x = ship_x;
  ship_last_y = ship_y;  
  
  if (left_state == HIGH && right_state == LOW) {
    if (ship_x > 20) {
      ship_x -= 20;
    }
  } else if (right_state == HIGH &&  left_state == LOW) {
    if (ship_x < 300) {
      ship_x += 20;
    }
  } else {
    
  }

  if (fire_state == HIGH) {
    if (shots[0] == 50){
      shots[0] += 10;
      shots[3] = ship_x;
    } else if (shots[1] == 50) {
      shots[1] += 10;
      shots[4] = ship_x;
    } else if (shots[2] == 50) {
      shots[2] += 10;
      shots[5] = ship_x;
    } else {
      
    }
    delay(150);
  }
  if ((shots[0] != 50) || (shots[1] != 50) || (shots[2] != 50)){
    if ((shots[0] >= tft.height() - 50) || (shots[1] >= tft.height() - 50) || (shots[2] >= tft.height() - 50)) {
        if (shots[0] >= tft.height() - 50) {
          shots[0] = 50;
          shots[3] = 0;
        } else if (shots[1] >= tft.height() - 50) {
          shots[1] = 50;
          shots[4] = 0;
        } else {
          shots[2] = 50;
          shots[5] = 0;
        }
    } else {
      if (shots[0] != 50) {
        shots[0] += 10;
      }
      if (shots[1] != 50) {
        shots[1] += 10;
      }
      if (shots[2] != 50) {
        shots[2] += 10;
      }
    }
    if (shots[0] != 50) {
      tft.fillRect(shots[3],tft.height()-shots[0],10,10,RED);
      tft.fillRect(shots[6],tft.height()-shots[0] + 20,10,10,BLACK);
      tft.fillRect(shots[6],tft.height()-shots[0] + 10,10,10,BLACK);
      shots[6] = shots[3];
    }
    if (shots[1] != 50) {
       tft.fillRect(shots[4],tft.height()-shots[1],10,10,RED);
       tft.fillRect(shots[7],tft.height()-shots[1] + 20,10,10,BLACK);
       
       tft.fillRect(shots[7],tft.height()-shots[1] + 10,10,10,BLACK);
       shots[7] = shots[4];
    }
    if (shots[2] != 50) {
       tft.fillRect(shots[5],tft.height()-shots[2],10,10,RED);
       tft.fillRect(shots[8],tft.height()-shots[2] + 20,10,10,BLACK);
       
       tft.fillRect(shots[8],tft.height()-shots[2] + 10,10,10,BLACK);
        shots[8] = shots[5];
    }
  
  
 
  }
  

  
    Serial.print("Shots[0]: ");
  Serial.print(shots[0]);
    Serial.print(", Shots[1]: ");
  Serial.print(shots[1]);
    Serial.print(", Shots[2]: ");
  Serial.print(shots[2]);
    Serial.print(", Shots[3]: ");
  Serial.print(shots[3]);
    Serial.print(", Shots[4]: ");
  Serial.print(shots[4]);
    Serial.print(", Shots[5]: ");
  Serial.print(shots[5]);
    Serial.print("\n\n");
 
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    p.x = p.x + p.y;       
    p.y = p.x - p.y;            
    p.x = p.x - p.y;   
    
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = tft.height()-(map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));

    if (p.x > 230) {
      if (ship_x < 300) {
        ship_x += 10;
      }
    }

    if (p.x < 90) {
      if (ship_x > 20) {
        ship_x -= 10;
      }
    }

    if (p.x > 90 && p.x < 230) {

    Serial.print("HELLO");
    if (shots[0] == 50){
      shots[0] += 10;
      shots[3] = ship_x;
    } else if (shots[1] == 50) {
      shots[1] += 10;
      shots[4] = ship_x;
    } else if (shots[2] == 50) {
      shots[2] += 10;
      shots[5] = ship_x;
    } else {
      
    }
    delay(150);
  }

    

    Serial.print("X: ");
    Serial.print(p.x);
    Serial.print("\n");

    Serial.print("Y: ");
    Serial.print(p.y);
    Serial.print("\n\n");
}
  if (ship_last_x != ship_x || ship_last_y != ship_y) {
   tft.fillTriangle(ship_last_x-10,ship_last_y,ship_last_x+10,ship_last_y,ship_last_x,ship_last_y-ship_h,BLACK);
  }
  tft.fillTriangle(ship_x-10,ship_y,ship_x+10,ship_y,ship_x,ship_y-ship_h,WHITE);
  tft.drawTriangle(ship_x-10,ship_y,ship_x+10,ship_y,ship_x,ship_y-ship_h,GREEN);
}
