/*

 TFT Pong

 This example for the Arduino screen reads the values
 of 2 potentiometers to move a rectangular platform
 on the x and y axes. The platform can intersect
 with a ball causing it to bounce.

 This example code is in the public domain.

 Created by Tom Igoe December 2012
 Modified 15 April 2013 by Scott Fitzgerald

 http://www.arduino.cc/en/Tutorial/TFTPong

 */

#include <TFT.h>  // Arduino LCD library
#include <SPI.h>

// pin definition for the Uno
#define cs   10
#define dc   9
#define rst  8

// Rotary encoder for contrast
#define APIN 6
#define BPIN 7

#define RE_SWITCH 2
#define RE_PADDLE 1
#define RE_COLOR 0

TFT TFTscreen = TFT(cs, dc, rst);

// variables for the position of the ball and paddle
int paddleX = 0;
int paddleY = 0;
int oldPaddleX, oldPaddleY;
int ballDirectionX = 1;
int ballDirectionY = 1;

int ballSpeed = 10; // lower numbers are faster
int paddle_speed = 4; // higher numbers are faster

int ballX, ballY, oldBallX, oldBallY;

int paddle_width = 20;
int paddle_height = 5;
  
// save the width and height of the screen
int myWidth = 0; // TFTscreen.width();
int myHeight = 0; //TFTscreen.height();

int display_state = RE_PADDLE;  // starting state, push button toggles state

int red = 255;
int green = 255;
int blue = 255;
int hue = 0;

void setup() {
  // Setup rotary encoder
  pinMode(APIN, INPUT);
  pinMode(BPIN, INPUT);
  pinMode(RE_SWITCH, INPUT);

  while (!Serial) ;
  Serial.begin(115200);
  
  // initialize the display
  TFTscreen.begin();
  // black background
  TFTscreen.background(0, 0, 0);
    // save the width and height of the screen
  myWidth = TFTscreen.width();
  myHeight = TFTscreen.height();

  paddleY = 128 - 32;

  attachInterrupt(0, push_button, LOW); //use interrupt 0 (pin 2) and run function wake_up when pin 2 gets LOW
}

const int debounce_delay = 50;
unsigned long pb_start; // time when the button is pushed
bool pb_pushed = false;

// Callback for interrupt
void push_button() {
  detachInterrupt(0);

  pb_pushed = true;
  pb_start = millis();
  
  // now have the main loop call the completion routine that debounces the push button and toggles the display state
}

void complete_push_button() {
  pb_pushed = false;
  unsigned long end;
  Serial.print("S time: "); Serial.println(pb_start);
  while (digitalRead(RE_SWITCH) == LOW) {
    end = millis();
  }

  Serial.print("Start Display state: "); Serial.println(display_state);
  Serial.print("E time: "); Serial.println(end);
  
  // Only switch the state it the push button is held for more than 50 ms
  Serial.print("time: "); Serial.println(end - pb_start);
  // can be a state machine to cycle through a number of settings
  if (end - pb_start > debounce_delay) {
    if (display_state == RE_PADDLE) {
      display_state = RE_COLOR;
    }
    else if (display_state == RE_COLOR) {
      display_state = RE_PADDLE;
    }
  }

  Serial.print("End Display state: "); Serial.println(display_state);

  attachInterrupt(0, push_button, LOW); //use interrupt 0 (pin 2) and run function wake_up when pin 2 gets LOW
}

void loop() 
{
  if (pb_pushed) {
    complete_push_button();
  }

  if (display_state == RE_PADDLE) {
    // Read encoder and update paddle
    int change = getEncoderTurn();
    paddleX += change * paddle_speed;
    if (paddleX > myWidth) {
      paddleX = myWidth;
    }
    else if (paddleX < 0) {
      paddleX = 0;
    }
  }
  else if (display_state == RE_COLOR) {
    int change = getEncoderTurn();
    hue += change;
    if (hue > 360)
      hue = 360;
    else if (hue < 10)
      hue = 10;

    update_color(hue);
  }
  
  // set the fill color to black and erase the previous
  // position of the paddle if different from present
  TFTscreen.fill(0, 0, 0);

  if (oldPaddleX != paddleX || oldPaddleY != paddleY) {
    TFTscreen.rect(oldPaddleX, oldPaddleY, paddle_width, paddle_height);
  }

  // draw the paddle on screen, save the current position
  // as the previous.
  TFTscreen.fill(red, green, blue);

  TFTscreen.rect(paddleX, paddleY, 20, 5);
  oldPaddleX = paddleX;
  oldPaddleY = paddleY;

  // update the ball's position and draw it on screen
  if (millis() % ballSpeed < 2) {
    moveBall();
  }
}

// HSV to RGB
// This will enable cycling colors that have full saturation and value.
// See https://en.wikipedia.org/wiki/HSL_and_HSV

// Given the value of Hue and assuming Saturation and Value are 1,
// compute the value for each of the colors (RGB) over the interval 
// [0,1]. Then scale the colors so they range from [0,255]. The 
// value for hue should range between [0,360].
int update_color(int hue) {
  // since Sat and Value are both 1, there's no need to compute this. If we use
  // a different chroma, use: c = value *  saturation;
  float c = 1.0;
  float h_prime = hue/60;
  float x = c * (1 - abs(((int)h_prime % 2) - 1));

  switch ((int)h_prime) {
    case 0:
      red = 255;
      green = x * 255;
      blue = 0;
      break;
    case 1:
      red = x * 255;
      green = 255;
      blue = 0;
      break;
    case 2:
      red = 0; 
      green = 255;
      blue = x * 255;
      break;
    case 3:
      red = 0; 
      green = x * 255;
      blue = 255;
      break;
    case 4:
      red = x * 255; 
      green = 0;
      blue = 255;
      break;
    case 5:
      red = 255; 
      green = 0;
      blue = x * 255;
      break;
    default:
      red = 0; 
      green = 0;
      blue = 0;
  }
}

// this function determines the ball's position on screen
void moveBall() {
  // if the ball goes offscreen, reverse the direction:
  if (ballX > TFTscreen.width() || ballX < 0) {
    ballDirectionX = -ballDirectionX;
  }

  if (ballY > TFTscreen.height() || ballY < 0) {
    ballDirectionY = -ballDirectionY;
  }

  // check if the ball and the paddle occupy the same space on screen
  if (inPaddle(ballX, ballY, paddleX, paddleY, 20, 5)) {
    ballDirectionX = -ballDirectionX;
    ballDirectionY = -ballDirectionY;
  }

  // update the ball's position
  ballX += ballDirectionX;
  ballY += ballDirectionY;

  // erase the ball's previous position
  TFTscreen.fill(0, 0, 0);

  if (oldBallX != ballX || oldBallY != ballY) {
    TFTscreen.rect(oldBallX, oldBallY, 5, 5);
  }


  // draw the ball's current position
  TFTscreen.fill(red, green, blue);
  TFTscreen.rect(ballX, ballY, 5, 5);

  oldBallX = ballX;
  oldBallY = ballY;

}

// this function checks the position of the ball
// to see if it intersects with the paddle
boolean inPaddle(int x, int y, int rectX, int rectY, int rectWidth, int rectHeight) {
  boolean result = false;

  if ((x >= rectX && x <= (rectX + rectWidth)) &&
      (y >= rectY && y <= (rectY + rectHeight))) {
    result = true;
  }

  return result;
}

#if 0
int get_switch_state() 
{
  static int last_switch_state = RE_PADDLE;
  static int current_switch_state = RE_PADDLE;
  static long last_debounce_time = 0;
  
  // Set RE Switch
  int current_switch_state = digitalRead(RE_SWITCH);
  Serial.print("Switch state: "); Serial.print(re_switch_state); Serial.print(", "); Serial.println(re_switch);
  
  if (current_switch_state != last_switch_state) {
    last_debounce_time = millis();
  }

  if ((millis() - last_debounce_time) > debounce_delay) {
     return 1;
  }

  last_switch_state = current_switch_state;

  ret
}
#endif

int getEncoderTurn()
{
  
  //return -1, 0 or +1
  static int oldA = LOW;
  static int oldB = LOW;
  int result = 0;
  int newA = digitalRead(APIN);
  int newB = digitalRead(BPIN);
  if (newA != oldA || newB != oldB) {
    if (oldA == LOW && newA == HIGH) {
      result = -(oldB * 2 - 1);
    }
  }
  oldA = newA;
  oldB = newB;
  return result;
}
