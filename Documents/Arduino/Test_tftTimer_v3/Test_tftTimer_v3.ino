 #if 1

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define CLK 22
#define DT 24
#define SW 26


const int XP=7,XM=A1,YP=A2,YM=6; //240x320 ID=0x1526
const int TS_LEFT=908,TS_RT=208,TS_TOP=193,TS_BOT=929; //re-calibrated for the screen at hand 
const int buzzer = 31;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Adafruit_GFX_Button add_btn, subtr_btn, stop_btn, start_btn, hrs_btn;

int pixel_x, pixel_y;     //Touch_getXY() updates global vars
bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
         pixel_x = map(p.y, TS_LEFT, TS_RT, 0, 320); // Specifying screan layout and creating a map
         pixel_y = map(p.x, TS_TOP, TS_BOT, 0, 240);
    }
    return pressed;
}


int hrs = 0;
int Min = 0;
int sec = 0;
bool countdown = false;

int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;



#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

unsigned long startMillis;  //Start value on millis (internal clock that sarts upon power on)
unsigned long currentMillis; // current value since start
unsigned long benchMillis;

void setup() {
  // Set up section for program to run:
 // uint16_t ID = tft.readID();
  Serial.begin(9600);
  tft.begin(0x9341);
  tft.fillScreen(BLACK);
  tft.setRotation(3);
  add_btn.initButton(&tft,  210, 190 , 80, 70, WHITE, CYAN, BLACK, "+", 2);
  subtr_btn.initButton(&tft, 110 , 190 , 80, 70, WHITE, CYAN, BLACK, "-", 2);
  add_btn.drawButton(false);
  subtr_btn.drawButton(false);

  start_btn.initButton(&tft, 270, 40, 80, 70, WHITE, BLACK, GREEN, ">", 2);
  stop_btn.initButton(&tft,  160, 40, 80, 70, WHITE, BLACK, RED, "||", 2);
  stop_btn.drawButton(false);
  start_btn.drawButton(false);

  hrs_btn.initButton(&tft, 50, 40, 80, 70, WHITE, BLACK, GREEN, "+30min", 2);
  hrs_btn.drawButton(false);  


  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);
  pinMode(buzzer, OUTPUT);
}

void loop(void)
{
    bool down = Touch_getXY();
    add_btn.press(down && add_btn.contains(pixel_x, pixel_y));
    subtr_btn.press(down && subtr_btn.contains(pixel_x, pixel_y));
    start_btn.press(down && start_btn.contains(pixel_x, pixel_y));
    stop_btn.press(down && stop_btn.contains(pixel_x, pixel_y));
    hrs_btn.press(down && hrs_btn.contains(pixel_x, pixel_y));
        
    if (add_btn.justPressed()) {
        add_btn.drawButton(true);
        tft.fillRect(70, 90, 210, 50, BLACK);
            if (sec+10 < 60) {
                sec = sec + 10;
            } else if ( sec + 10 >= 60) {
                Min = Min + 1;
                if (Min >= 60) {
                  hrs = hrs + 1;
                  Min = Min - 60;
                }
                sec = sec + 10 - 60; 
             } 
        add_btn.drawButton(false);
    }

     if (hrs_btn.justPressed()) {
        hrs_btn.drawButton(true);
        tft.fillRect(70, 90, 210, 50, BLACK);
            if (Min + 30 >= 60) {
              hrs = hrs + 1;
              Min = Min - 60;
            }
            hrs_btn.drawButton(false);  
         } 
       
    
    if (subtr_btn.justPressed()) {
        subtr_btn.drawButton(true);
        tft.fillRect(70, 90, 210, 50, BLACK);
              if(sec-10 < 0 && Min == 0 && hrs == 0) {
                sec = 0;
             }else if (sec-10 >= 0) {
                sec = sec - 10;
            } else if ( sec - 10 < 0) {
                Min = Min - 1;
                sec = sec  - 10 + 60;
                if (Min < 0) {
                  hrs = hrs - 1;
                  Min = Min + 60;
                }
             }  
        subtr_btn.drawButton(false);
        delay(100); 
    }

     if (start_btn.justPressed()) { // start button, turns countdown bool to true
        start_btn.drawButton(true);
        startMillis = millis();
        countdown = true;
        start_btn.drawButton(false);   
     }
     
     if (stop_btn.justPressed()) { //stop button, turns bool to false = pause
        stop_btn.drawButton(true);
        startMillis = millis();
        countdown = false;
        stop_btn.drawButton(false);   
     } 
     if (hrs_btn.justPressed()) { // button for setting 1h automaticaly
        hrs_btn.drawButton(true);
        tft.fillRect(70, 90, 210, 50, BLACK);
        //hrs = 0;
        Min = Min + 30;
       // sec = 0; 
        hrs_btn.drawButton(false);   
     }
      

     lastStateCLK = currentStateCLK;

     // Read the button state
      int btnState = digitalRead(SW);
    
      //If we detect LOW signal, button is pressed
      if (btnState == LOW) {
        
        //if 100ms have passed since last LOW pulse, it means that the
        //button has been pressed, released and pressed again
        //if value is lower (50ms) the button and loop registers 2 clicks...
        if (millis() - lastButtonPress > 100) {
          countdown = !countdown;
        }
      
        // Remember last button press event
        lastButtonPress = millis();
      }
      if (Min == 0 && hrs == 0 && sec == 0) { //stop statement for timer
                  countdown = false;
      }

      // Put in a slight delay to help debounce the reading
      delay(1);

    currentStateCLK = digitalRead(CLK);

      // If last and current state of CLK are different, then pulse occurred
      // React to only 1 state change to avoid double count
      if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){
    
        // If the DT state is different than the CLK state then
        // the encoder is rotating CCW so decrement
        if (digitalRead(DT) != currentStateCLK) {
          counter ++;
          currentDir ="CW";
        } else {
          // Encoder is rotating CW so increment
          counter --;
          currentDir ="CCW";
        }
    
        Serial.print("Direction: ");
        Serial.print(currentDir);
        Serial.print(" | Counter: ");
        Serial.println(counter);

        if (currentDir == "CW") {
          tft.fillRect(70, 90, 210, 50, BLACK);
          if (sec+10 < 60) {
                sec = sec + 10;
            } else if ( sec + 10 >= 60) {
                Min = Min + 1;
                if (Min >= 60) {
                  hrs = hrs + 1;
                  Min = Min - 60;
                }
                sec = sec + 10 - 60; 
             } 
        }

         if (currentDir == "CCW") {
          tft.fillRect(70, 90, 210, 50, BLACK);
          if(sec-10 < 0 && Min == 0 && hrs == 0) {
                sec = 0;
             }else if (sec-10 >= 0) {
                sec = sec - 10;
            } else if ( sec - 10 < 0) {
                Min = Min - 1;
                sec = sec  - 10 + 60;
                if (Min < 0) {
                  hrs = hrs - 1;
                  Min = Min + 60;
                }
             }  
          }
        
      }
 


    if (sec >= 0 && countdown == true) {
          currentMillis = millis();
          if (currentMillis-startMillis > 1000) { //sampel time for update rate
            sec = sec -1;

            if (sec < 0) {
                Min = Min - 1;
                sec = sec + 60;
                if (Min < 0) {
                    hrs = hrs - 1;
                    Min = Min + 60;
                }
            }
            /*
              if (Min > 0 && sec < 0) {
                Min = Min - 1;
                sec = sec + 60;
              }
              if (hrs > 0 && Min < 0) {
                hrs = hrs - 1;
                Min = Min + 60;
               // sec = sec + 59;
              }
              */
              if (Min == 0 && hrs == 0 && sec == 0) { //stop statement for timer
                  countdown = false;
                  tft.fillRect(70, 90, 210, 50, BLACK); //print animation for timer being done
                  tft.setCursor(70, 90);
                  tft.setTextSize(6);
                  tft.setTextColor(GREEN); 
                  tft.print("DONE");
                  tone(buzzer, 1000); // Send 1KHz sound signal...
                  delay(1000);        // ...for 1 sec
                  noTone(buzzer);     // Stop sound...
                  delay(1000);        
                  start_btn.drawButton(false); //blinking done button
                  start_btn.drawButton(true);
                  delay(500);
                  start_btn.drawButton(false);
                  delay(500);
                  start_btn.drawButton(true);
                  delay(500);
                  start_btn.drawButton(false);
                  delay(500);
                  start_btn.drawButton(true);
                  delay(500);
                  start_btn.drawButton(false);
                                     
              }
                tft.fillRect(70, 90, 210, 50, BLACK);
                tft.setCursor(70, 90);
                tft.setTextSize(5);
                tft.setTextColor(WHITE);
                tft.print(String(hrs) + ":" + String(Min) + ":" + String(sec));
                //benchMillis=currentMillis;
                Serial.println(sec);
                Serial.println(Min);
                Serial.println(hrs);
                startMillis = currentMillis;
              }        
          }
    
    tft.setCursor(70, 90);
    tft.setTextSize(5);
    tft.setTextColor(WHITE); 
    tft.print(String(hrs) + ":" + (Min) + ":" + String(sec));

}
#endif
