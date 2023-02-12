#include <Arduino.h>



/*
  A simple Pong game:
 */

// #include <SPI.h>
#include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
#include "SSD1306Wire.h"  

#define UP_BUTTON D3
#define DOWN_BUTTON D4

const unsigned long PADDLE_RATE = 66;
const unsigned long BALL_RATE = 40;
const uint8_t PADDLE_HEIGHT = 12;
const uint8_t SCREEN_HEIGHT = 48;
const uint8_t SCREEN_WIDTH = 64;

// #define SCREEN_WIDTH 64 // OLED display width, in pixels
// #define SCREEN_HEIGHT 48 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     0 // Reset pin # (or -1 if sharing Arduino reset pin)
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_64_48 ); // WEMOS OLED shield

void drawCourt();

uint8_t ball_x = (SCREEN_WIDTH / 2), ball_y = 16;
uint8_t ball_dir_x = 1, ball_dir_y = 1;
unsigned long ball_update;

unsigned long paddle_update;
const uint8_t CPU_X = 3;
uint8_t cpu_y = 16;

const uint8_t PLAYER_X = 60;
uint8_t player_y = 10;

uint8_t score_player = 0;
uint8_t score_cpu = 0;

uint8_t new_x = 0;
uint8_t new_y = 0;

void waitButton() {
    int buttonState = 0;
  while(1){
    buttonState = digitalRead(UP_BUTTON);
    if (buttonState == LOW) {
      return;
    }
  }
}

void score() {
if(new_x == 0){
score_cpu = score_cpu + 1;
Serial.println(score_cpu);
} else {
  score_player = score_player + 1;
  Serial.println(score_player);
}

if(score_player > 9) {

display.clear();
display.drawString(32, 16, "CPU Won!");
display.display();
while(1);
} 
if(score_cpu > 9){
display.clear();
display.drawString(32, 16, "You Won!");
display.display();
while(1);
  }
}



void setup() {
  Serial.begin(9600);
    // display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
      // Initialising the UI will init the display too.
  display.init();

    
    display.display();
    display.flipScreenVertically();
    unsigned long start = millis();

    pinMode(UP_BUTTON, INPUT);
    pinMode(DOWN_BUTTON, INPUT);
    digitalWrite(UP_BUTTON,1);
    digitalWrite(DOWN_BUTTON,1);
    display.clear();
    drawCourt();

    while(millis() - start < 2000);

    // display.display();

    ball_update = millis();
    paddle_update = ball_update;
}

void loop() {
    bool update = false;
    unsigned long time = millis();

    static bool up_state = false;
    static bool down_state = false;
    
    up_state |= (digitalRead(UP_BUTTON) == LOW);
    down_state |= (digitalRead(DOWN_BUTTON) == LOW);

    if(time > ball_update) {
        new_x = ball_x + ball_dir_x;
        new_y = ball_y + ball_dir_y;

        // Check if we hit the vertical walls
        if(new_x == 0 || new_x == (SCREEN_WIDTH - 1)) {
          score();
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the horizontal walls.
        if(new_y == 0 || new_y == (SCREEN_HEIGHT - 1)) {
            ball_dir_y = -ball_dir_y;
            new_y += ball_dir_y + ball_dir_y;
        }

        // Check if we hit the CPU paddle
        if(new_x == CPU_X && new_y >= cpu_y && new_y <= cpu_y + PADDLE_HEIGHT) {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the player paddle
        if(new_x == PLAYER_X
           && new_y >= player_y
           && new_y <= player_y + PADDLE_HEIGHT)
        {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }
      
        display.setPixelColor(ball_x, ball_y, BLACK);
        display.setPixelColor(new_x, new_y, WHITE);
        ball_x = new_x;
        ball_y = new_y;

        ball_update += BALL_RATE;

        update = true;
    }

    if(time > paddle_update) {
        paddle_update += PADDLE_RATE;

        // CPU paddle
        // display.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT, BLACK);
        display.setColor(BLACK);
        display.drawVerticalLine(CPU_X, cpu_y, PADDLE_HEIGHT);
        const uint8_t half_paddle = PADDLE_HEIGHT >> 1;
        if(cpu_y + half_paddle > ball_y) {
            cpu_y -= 1;
        }
        if(cpu_y + half_paddle < ball_y) {
            cpu_y += 1;
        }
        if(cpu_y < 1) cpu_y = 1;
        if(cpu_y + PADDLE_HEIGHT > (SCREEN_HEIGHT - 1)) cpu_y = (SCREEN_HEIGHT - 1) - PADDLE_HEIGHT;
        // display.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT, WHITE);
        display.setColor(WHITE);
        display.drawVerticalLine(CPU_X, cpu_y, PADDLE_HEIGHT);

        // Player paddle
        // display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, BLACK);
        display.setColor(BLACK);
        display.drawVerticalLine(PLAYER_X, player_y, PADDLE_HEIGHT);
        if(up_state) {
            player_y -= 1;
        }
        if(down_state) {
            player_y += 1;
        }
        up_state = down_state = false;
        if(player_y < 1) player_y = 1;
        if(player_y + PADDLE_HEIGHT > (SCREEN_HEIGHT - 1)) player_y = (SCREEN_HEIGHT - 1) - PADDLE_HEIGHT;
        // display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, WHITE);
        display.setColor(WHITE);
        display.drawVerticalLine(PLAYER_X, player_y, PADDLE_HEIGHT);

        update = true;
    }
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.setColor(BLACK);
  display.fillRect(15,3,33,10);
  display.setColor(WHITE);
  display.drawString(32, 3, String(score_player) + ":" + String(score_cpu));


    if(update)
        display.display();
}


void drawCourt() {
    display.setColor(WHITE);
    display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}