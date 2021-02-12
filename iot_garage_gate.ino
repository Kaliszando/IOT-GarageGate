/*

  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

*/

// include the library code:
#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>

// setting parameters
#define NEO_PIN 6
#define NEO_PIX_NUM 16
#define PIX_GATE_NUM 10

#define DELAY_MILLIS 500

#define DELAY_RELAY 6000

// variables
unsigned long previousMillis = 0;
unsigned long currentMillis;
unsigned long switchTime;

int pixel_gate[PIX_GATE_NUM];

int moveGateState = LOW;
int moveGatePrevState = LOW;
  
int gateState = 0;
int gatePrevState = -1;

int pirState = LOW;

float distance = 0;

int pResistVal = 0;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Adafruit_NeoPixel pixels(NEO_PIX_NUM, NEO_PIN, NEO_GRB + NEO_KHZ800);

// update values in array that represents gate as pixels
void moveGate(int direction)
{
  int coeff = 0;
  if (direction > 0) {
  	coeff = 1;
    if(pixel_gate[PIX_GATE_NUM - 1] == NEO_PIX_NUM - 1)
      return;
  }
  if (direction < 0) {
    coeff = -1;
    if(pixel_gate[0] == 0)
      return;
  }

  for (int i = 0; i < PIX_GATE_NUM; i++)
    pixel_gate[i] += coeff;
}

long readUltrasonicDistance(int triggerPin, int echoPin)
{
  // Clear the trigger
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  
  // Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  
  // Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}


void setup()
{
  //Serial.begin(9600);

  // button
  pinMode(A0, INPUT);
  // piezo
  pinMode(A1, OUTPUT);
  // PIR
  pinMode(8, INPUT);
  // relay
  pinMode(9, OUTPUT);
  // photoresistor
  pinMode(A2, INPUT);
  
  // set up neopiexl led strip
  pixels.begin();
  for (int i = 0; i < PIX_GATE_NUM; i++)
    pixel_gate[i] = i;

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
}

void loop()
{
  currentMillis = millis();
  
  // buttons
  //
  moveGateState = digitalRead(A0);
  
  if(moveGatePrevState != moveGateState){
    if(moveGatePrevState == LOW && moveGateState == HIGH) {
      if(gateState == 0 && gatePrevState == -1) {
      	gateState = 1;
        gatePrevState = 1;
        Serial.println("up");
      }
      else if(gateState == 0 && gatePrevState == 1)
      {
        gateState = -1;
        gatePrevState = -1;
        Serial.println("down");
      }
      else {
      	gateState = 0;
        Serial.println("stop");
      }
      //lcd.clear();
    }
    moveGatePrevState = moveGateState;
  }
  delay(5);

  // loop that executes instrunctions every 0,5s
  if (currentMillis - previousMillis >= DELAY_MILLIS)
  {
    previousMillis = currentMillis;
    
      
    // Ultrasonic rangefinder
    //
    distance = 0.01723 * readUltrasonicDistance(7, 7);
    
    
    // Gate logic
    //
    if(distance < 333 && distance > 288) {
      if(pixel_gate[PIX_GATE_NUM - 1] == 10) moveGate(0);
      else moveGate(gateState);
    }
    else if(distance <= 288 && distance > 211){
      if(pixel_gate[PIX_GATE_NUM - 1] == 11) moveGate(0);
      else moveGate(gateState);
    }
    else if(distance <= 211 && distance > 134){
      if(pixel_gate[PIX_GATE_NUM - 1] == 12) moveGate(0);
      else moveGate(gateState);
    }
    else if(distance <= 134 && distance > 58){
      if(pixel_gate[PIX_GATE_NUM - 1] == 13) moveGate(0);
      else moveGate(gateState);
    }
    else if(distance <= 58 && distance > 13){
      if(pixel_gate[PIX_GATE_NUM - 1] == 14) moveGate(0);
      else moveGate(gateState);
    }
    else if(distance <= 13){
      if(pixel_gate[PIX_GATE_NUM - 1] == 15) moveGate(0);
      else moveGate(gateState);
    }
    else moveGate(gateState);
    
    if(pixel_gate[PIX_GATE_NUM - 1] == 15 || pixel_gate[0] == 0) {
    	gateState = 0;
    }
  
   
    // PIR and photoresistor
    //
    int pirVal = digitalRead(8);
    pResistVal = analogRead(A2);
    //Serial.println(pResistVal);
    
    if(pirVal == HIGH && pirState == LOW && pResistVal < 140){
      pirState = HIGH;
      switchTime = millis();
      // turn relay on
      digitalWrite(9, HIGH);
 
    }
    else if(pirState == HIGH && millis() - switchTime > DELAY_RELAY) {
      pirState = LOW;
      // turn relay off
      digitalWrite(9, LOW);
    }
    

    // Pixel gate
    // red - still
    // green - moving
    pixels.clear();   
    for (int i = 0; i < PIX_GATE_NUM; i++) {
      if(gateState == 0)
        pixels.setPixelColor(pixel_gate[i], pixels.Color(255, 0, 0));
      else
        pixels.setPixelColor(pixel_gate[i], pixels.Color(0, 255, 0));
    }
    pixels.show();

    // LCD
    //
    lcd.clear();
    lcd.setCursor(0, 0);
    if(gateState == 0){
    	if(pixel_gate[0] == 0) lcd.print("gate is closed");
      	else if(pixel_gate[0] == 6) lcd.print("gate is open");
        else lcd.print("gate stopped");
    }
    else lcd.print("gate is moving");

    lcd.setCursor(0, 1);
    if(pixel_gate[0] == 6) lcd.print("dist: 0cm");
    else lcd.print("dist: " + String(int(distance))+ "cm");
    
    
    // Alarm
    // triggers when gate should be closed
    // but is manually opened
    if(pixel_gate[0] == 0 && distance < 320) {
      tone(A1, 900, DELAY_MILLIS);
      delay(DELAY_MILLIS);
      tone(A1, 600, DELAY_MILLIS);
      delay(DELAY_MILLIS - 200);
    }
  }

  delay(100);
}
