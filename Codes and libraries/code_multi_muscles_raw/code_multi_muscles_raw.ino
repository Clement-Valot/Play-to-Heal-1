#include <Wire.h>
#include <U8g2lib.h> //Oled Library
#include <Filters.h>
#include <Filters/Butterworth.hpp>
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
int pin_Capteur[] = {26,32};
int pin_Transistor[] = {25,33};
int array_values[128]; // tableau contenant les valeurs analogRead mappées pour l'affichage medecin


const int nEchantillons = 20;
float echantillon[nEchantillons]; // un tableau pour stocker les échantillons lus

const double f_s = 100; // Hz
// Cut-off frequency 
const double f_c = 2; // Hz
// Normalized cut-off frequency
const double f_n = 2 * f_c / f_s;
// Fourth-order Butterworth filter
auto filter1 = butter<4>(f_n);  // ici c'est un passe haut a priori a verifier sinon regarder comment utiliser passe bande (passe haut - passe bas le tout en butterworth ?)
   
// Cut-off frequency 
const double f_c2 = 40; // Hz
// Normalized cut-off frequency
const double f_n2 = 2 * f_c2 / f_s;
// Fourth-order Butterworth filter
auto filter2 = butter<4>(f_n2); 

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
  delay(100); // give time to bring up serial monitor
  u8g2.setFont(u8g2_font_6x13B_tf);
  
  DisplayOLED2("Let's Play To Heal!" ,0);
  DisplayOLED2("Distributed by",1);
  DisplayOLED2("Microsoft x ESILV",2);
  delay(3000);

  //set number of muscles to connect to the device.
  num_muscles = CountMuscles();
  
  //set the loop that iterates as many times as there are muscles to connect
  for (int m=0; m<num_muscles; m++){
    pinMode(pin_Transistor[m], OUTPUT);
    digitalWrite(pin_Transistor[m], LOW);

    DisplayOLED2("Muscle "+String(m+1),0);
    DisplayOLED2("Relax then",1);
    DisplayOLED2("press button",2);

    int clic = 0;
    float moyenne_setup = 0.0;
    float total_setup = InitSample(m);

    // To initialize array_values
    for (int i = 0; i<128; i++){
      total_setup = UpdateSample(m,total_setup);
      moyenne_setup = sqrt(total_setup / nEchantillons);
      if(moyenne_setup<20) moyenne_setup = 20; 
      array_values[i] = map(moyenne_setup,20,210,63,0);
      delay(1);
    }

    while(clic != 1){
      if(digitalRead(buttonPin)==HIGH) clic = clic +1;
    }

    DisplayOLED2("Stay decontracted",0);
    delay(3000);
    // We have the minimum mean value
    int moyenne_min = Mean(m);

    DisplayOLED2("Value Decontracted:",0);
    DisplayOLED2(String(moyenne_min),1);
    delay(3000);
    
    DisplayOLED2("Contract muscle "+String(m+1),0);
    DisplayOLED2("Keep contracting",1);
    DisplayOLED2("when pressing Button",2);
    
    while(clic != 2){
      if(digitalRead(buttonPin)==HIGH) clic = clic +1;
    }

    DisplayOLED2("Stay Contracted",0);
    delay(3000);
    // We have the contracted mean value
    int moyenne_max = Mean(m);
    DisplayOLED2("Value Contracted:",0);
    DisplayOLED2(String(moyenne_max),1);
    delay(3000);
    
    threshold[m] = moyenne_min + 0.3 * (moyenne_max - moyenne_min);
    DisplayOLED2("Threshold "+String(m+1)+":",0);
    DisplayOLED2(String(threshold[m]),1);
    delay(3000);
  }
}
  

void loop()
{
  for(int m=0; m<num_muscles; m++){
    SendSignal(m);
  }
  delay(10);
}

float Mean(int m)
{
  float mean_RMS=0.0;
  float total=0.0;
  float a=0.0;

  for(int ind=0; ind<nEchantillons; ind++){
    float valeur_capteur = analogRead(pin_Capteur[m]);
    a = filter1(valeur_capteur)-filter2(valeur_capteur);
    echantillon[ind] = a*a;
    total = total + echantillon[ind];
  }
  float mean = sqrt(total / nEchantillons);
  
  for (int i = 1; i < 250 ; i++){
    mean_RMS = sqrt(total / nEchantillons);
    if(mean_RMS - mean <50){     // a checker
        mean = mean*i/(i+1) + mean_RMS/(i+1);
    }
    else{
        i--; //On ne compte pas cette itération
    }
    DisplayCurve(m, i, total); 
    total = UpdateSample(m,total);
    delay(10);
  }
  return mean;
}

float InitSample(int m){
    float total = 0.0;
    float a = 0.0;
    for(int ind=0; ind<sizeof(echantillon); ind++){
        float valeur_capteur = analogRead(pin_Capteur[m]);
        a = filter1(valeur_capteur)-filter2(valeur_capteur);
        echantillon[ind] = a*a;
        total = total + echantillon[ind];
    }
    return total;
}

float UpdateSample(int m, float total){
  total = total - echantillon[0];
  int max_index = sizeof(echantillon)-1;
  for (int i = 0; i<max_index; i++){
    echantillon[i]= echantillon[i+1];
  }
  float valeur_capteur = analogRead(pin_Capteur[m]);
  float a = filter1(valeur_capteur)-filter2(valeur_capteur);
  echantillon[max_index] = a*a;
  
  total = total + echantillon[max_index];
  
  return total;
}

//Function that update the array of values registered from muscle m.
//This is mostly useful for the DisplayCurve function
void UpdateArray(int m, float total){
  // Fonction update tab
  for (int i = 0; i<127;i++){
    array_values[i]= array_values[i+1];
  }
  total = UpdateSample(m,total);
  float mean = sqrt(total / nEchantillons);
  if(mean<20)  mean = 20;
  //We map those values between 0 and 63 because this is the height of our OLED in pixels
  array_values[127] = map(mean,20,250,63,0);
}

//Function that displays on the OLED the curve of the contraction of muscle m.
void DisplayCurve(int m, int index, float total){
  UpdateArray(m, total);
  u8g2.clearBuffer();
  for (int i = 0; i<128;i++){
    int b = array_values[i];
    u8g2.drawPixel(i,b);
  }
  u8g2.setCursor(6, 30);
  u8g2.print(index);
  u8g2.sendBuffer();
}

//same function than previous one with the exception that to validate the number of muscles,
//user has to keep pressing the button for 2s instead of having to wait for the oled to display
//the confirmation message and click the button to confirm.
int CountMuscles(){
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
  float total = InitSample(m);
  float moyenne = sqrt(total / nEchantillons);
  
  if(moyenne>threshold[m]){
    digitalWrite(pin_Transistor[m], HIGH);
    DisplayOLED2("CONTACT "+String(m+1),m);
  }
  else{
    digitalWrite(pin_Transistor[m], LOW);
    DisplayOLED2("RAS "+String(m+1),m);
  }
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
