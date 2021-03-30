#include <Wire.h>
#include <U8g2lib.h> //Oled Library
#include "logo.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,/*reset=*/U8X8_PIN_NONE);
//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,/*reset=*/U8G2_PIN_NONE);

const int max_muscles = 4;
float threshold[max_muscles];
int num_muscles = 1;

// constants won't change. They're used here to set pin numbers:
const int buttonPin = 19;     // the number of the pushbutton pin
const int buttonHigh = 18;
//Add as much pin_capteur and pin_transistor as you connect muscles on the case (max 8)
const byte pin_Capteur[] = {26,32};
const byte pin_Transistor[] = {25,33};
int array_values[128]; // tableau contenant les valeurs analogRead mappées pour l'affichage medecin


void setup()
{
  u8g2.setFont(u8g2_font_6x13B_tf);
  pinMode(buttonPin, INPUT);
  pinMode(buttonHigh, OUTPUT);
  digitalWrite(buttonHigh, HIGH);
  u8g2.begin();
  drawLogo();
  delay(3000);
  Serial.begin(115200); //default baud rate for ESP32
  delay(100); // give me time to bring up serial monitor
  u8g2.setFont(u8g2_font_6x13B_tf);
  
  DisplayOLED2("Let's Play To Heal!" ,0);
  DisplayOLED2("Distributed by",1);
  DisplayOLED2("Microsoft x ESILV",2);
  delay(3000);

  //set number of muscles to connect to the device.
  num_muscles = CountMuscles2();
  
  //set the loop that iterates as many times as there are muscles to connect
  for (int m=0; m<num_muscles; m++){
    pinMode(pin_Transistor[m], OUTPUT);
    digitalWrite(pin_Transistor[m], LOW);
    
    int clic = 0;
    DisplayOLED2("Muscle "+String(m+1),0);
    DisplayOLED2("Relax then",1);
    DisplayOLED2("press button",2);
    
    for (int i = 0; i<128;i++){
      int a = analogRead(pin_Capteur[m]);
      if(a<2000){
        a = 2000;
      }
      a = map(a,2000,4095,63,0);
      array_values[i] = a;
      delay(10);
    }
    
    while(clic != 1){
      if(digitalRead(buttonPin)==HIGH) clic = clic +1; 
    }

    // We have the minimum mean value
    float mean_decontract = Mean(m, 0);
    DisplayOLED2("Value Decontracted:",0);
    DisplayOLED2(String(mean_decontract),1);
    delay(4000);
    
    DisplayOLED2("Contract muscle "+String(m+1),0);
    DisplayOLED2("Keep contracting",1);
    DisplayOLED2("when pressing Button",2);
    
    while(clic != 2){
      if(digitalRead(buttonPin)==HIGH) clic = clic +1; 
    }

    // We have the contracted mean value
    float mean_contract = Mean(m, mean_decontract);
    DisplayOLED2("Value Contracted:",0);
    DisplayOLED2(String(mean_contract),1);
    
    threshold[m] = mean_decontract + 0.3 * (mean_contract - mean_decontract);
    delay(3000);
    DisplayOLED2("Threshold "+String(m+1)+":",0);
    DisplayOLED2(String(threshold[m]),1);
    delay(3000);
  }
}
  

void loop()
{
  for(int m=0;m<num_muscles;m++){
    SendSignal(m);
  }
  delay(100);
}


float Mean(int m, float mini)
{
  float value = analogRead(pin_Capteur[m]);
  float mean = value;
  
  for (int i = 1; i < 200 ; i++){
    value = analogRead(pin_Capteur[m]);

    // Si la valeur est dans le même ton que la moyenne
    if(value - mean < 1000 && value > 2*mini)
    {
      DisplayCurve(m, i);
      mean = mean*i/(i+1) + value/(i+1);
    }
    else
    {
      DisplayCurve(m, i);
      i--; //On ne compte pas cette itération
    }
    delay(1);
  }
  return mean;
}

//Function that update the array of values registered from muscle m.
//This is mostly useful for the DisplayCurve function
void UpdateArray(int m){
  // Fonction update tab
  for (int i = 0; i<127;i++){
    array_values[i]= array_values[i+1];
  }
  int c = analogRead(pin_Capteur[m]);
  if(c<2000){
    c = 2000;
  }
  //We map those values between 0 and 63 because this is the height of our OLED in pixels
  array_values[127] = map(c,2000,4095,63,0);
}

//Function that displays on the OLED the curve of the contraction of muscle m.
void DisplayCurve(int m, int ite){
  UpdateArray(m);
  u8g2.clearBuffer();
  for (int i = 0; i<128;i++){
    int b = array_values[i];
    u8g2.drawPixel(i,b);
    u8g2.setCursor(6, 30);
    u8g2.print(ite);
  }
  u8g2.sendBuffer();
}

//Function that returns the number of muscle to connect and play with.
int CountMuscles(){
  //we need to know how many muscles the user is going to connect.
  //To do so, we can ask the user to click the button as many times as he wants muscles
  int muscles = 1;
  bool changed = true;
  
  while(changed){
    DisplayOLED2("Click to add muscle",0);
    DisplayOLED2(String(muscles)+" so far",1);
    double start = millis();
    while(millis() - start <3000){
      if(digitalRead(buttonPin)==HIGH)
      {
        muscles = (muscles%max_muscles +1);
        DisplayOLED2("Click to add muscle",0);
        DisplayOLED2(String(muscles)+" so far",1);
        delay(300);
        start = millis();
      }
    }
    start=millis();

    DisplayOLED2("Connect "+String(muscles)+" muscles?",0);
    DisplayOLED2("Yes: press button.",1);
    DisplayOLED2("No: just wait.",2);
    while(millis()-start<3000){
      if(digitalRead(buttonPin)==HIGH)
      {
        changed = false;
        break;
      }
    }
  }
  DisplayOLED2(String(muscles)+" muscles to connect",0);
  delay(2000);
  return muscles;
}

int CountMuscles2(){
  //we need to know how many muscles the user is going to connect.
  //To do so, we can ask the user to click the button as many times as he wants muscles
  int muscles = 1;
  bool changed = true;
  bool memo_appuiLong = false;
  double memo_temps = 0;
  int appuiLimite = 2000;
  
  DisplayOLED2("Click to add muscle",0);
  DisplayOLED2(String(muscles)+" so far",1);
  DisplayOLED2("Press 2s to validate",2);
  
  while(changed){
    double temps = millis();
    if (digitalRead(buttonPin)==HIGH) { // si le bouton est à HIGH
        if (temps - memo_temps > appuiLimite ){
          changed = false;
        }
        memo_appuiLong = true;       
    }
    else {
      if(memo_appuiLong) {
        muscles = (muscles%max_muscles +1);
        DisplayOLED2("Click to add muscle",0);
        DisplayOLED2(String(muscles)+" so far",1);
        DisplayOLED2("Press 2s to validate",2);
        memo_appuiLong = false;
      }
      memo_temps=temps; // mémorise le temps quand on a appuyé sur le bouton
    }
  }
  DisplayOLED2(String(muscles)+" muscles to connect",0);
  delay(3000);
  return muscles;
}

//Function that sets the voltage of the pin transistor of muscle m to HIGH (=Contact) if the value read
//on pin capteur of muscle m is higher than the previously set threshold for muscle m
void SendSignal(int m){
  int muscle = analogRead(pin_Capteur[m]);
  if(muscle>threshold[m])
  {
    digitalWrite(pin_Transistor[m], HIGH);
    DisplayOLED2("CONTACT "+String(m+1),m);
  }
  else
  {
    digitalWrite(pin_Transistor[m], LOW);
    DisplayOLED2("RAS "+String(m+1),m);
  }
}

//Function to display elements on the OLED
//This function takes into parameters one array of strings.
//Then we iterate through this array and display each one element below the other.
//This function tends to work badly because arrays of strings are not well supported on C with allocation, pointers and stuff.
//Consequences were that it printed garbage values on the OLED and the serial monitor.
//An alternative would be to use char pointer arrays but we prefered to create another simpler function (below)
void DisplayOLED(char *s[]){ 
  u8g2.clearBuffer();
  for(int n=0; n<sizeof(s);n++){
    u8g2.setCursor(8, 10+20*n);
    u8g2.print(s[n]);
    Serial.println(s[n]);
    delay(100);
  }
  u8g2.sendBuffer();
}

//Function to display elements on the OLED
//This function takes into parameters one string and one integer corresponding to the position of the String.
//If the position of the string is 0, then we have to clear the buffer first then display it.
//Otherwise we simply display the string without clearing the buffer.
//Thanks to this function, we don't deal with arrays anymore and the display works perfectly.
void DisplayOLED2(String s, int pos){
  if(pos==0){u8g2.clearBuffer();}
  u8g2.setCursor(6, 10+20*pos);
  u8g2.print(s);
  u8g2.sendBuffer();
}

void drawLogo() {
 u8g2.firstPage();
 do {
   u8g2.drawXBMP(0, 0, logo_width, logo_height, logo);
 } while ( u8g2.nextPage() );
}
