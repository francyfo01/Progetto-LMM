#include <Servo.h> //Inserire la libreria Servo
#include <math.h>
Servo Servo[5]; //Il nome del servo è Servo1
#define PINZA 0
#define POLSO 1
#define GOMITO 2
#define SPALLA 3
#define BASE 4

#define PI 3.14159265


int posGradi[5] = {80,150,10,80,60};

struct limite{
  int massimo;
  int minimo;
};


limite limiteServo[5];



String incomingString[3]; // for incoming serial data

void setup() {
  Serial.begin(115200); // opens serial port, sets data rate to 9600 bps
  Servo[PINZA].attach (3); //Il Servo1 è collegato al pin digitale
  Servo[POLSO].attach (4); //Il Servo1 è collegato al pin digitale
  Servo[GOMITO].attach (5); //Il Servo1 è collegato al pin digitale
  Servo[SPALLA].attach (6); //Il Servo1 è collegato al pin digitale
  Servo[BASE].attach (7); 


limiteServo[PINZA].massimo = 70;
limiteServo[POLSO].massimo = 180;
limiteServo[GOMITO].massimo = 140;
limiteServo[SPALLA].massimo = 160;
limiteServo[BASE].massimo = 170;

limiteServo[PINZA].minimo = 20;
limiteServo[POLSO].minimo = 0;
limiteServo[GOMITO].minimo = 10;
limiteServo[SPALLA].minimo = 30;
limiteServo[BASE].minimo = 10;


  posRiposo();

  


  
    
   
   

}

void loop() {
  // send data only when you receive data:
  if (Serial.available() > 0) {
    // read the incoming byte:

    for (int i = 0; i < 3; i++){
      incomingString[i] = Serial.readStringUntil('\n');
    }
    // say what you got:
    Serial.print("Servo: ");
    Serial.println(incomingString[0]);
    int numServo = incomingString[0].toInt();

    Serial.print("Gradi: ");
    Serial.println(incomingString[1]);
    int gradi = incomingString[1].toInt();
    
    Serial.print("Velocità: ");
    Serial.println(incomingString[2]);
    int velBase = incomingString[2].toInt();


    setServo(numServo, gradi, velBase);
}
}


void posRiposo()
{
  setServo(BASE, 90,1 );
  setServo(SPALLA, 160, 1);
  setServo(POLSO, 90, 1);
  setServo(PINZA, 70, 1);
  setServo(GOMITO, 10, 1);
}
  

void pos2()
{
  setServo(BASE, 90,2 );
  setServo(SPALLA, 160, 2);
  setServo(GOMITO, 70, 2);

  for(int i = 0;i < 10; i++){
      if(i%2 == 0)
      {
        setServo(POLSO, 50, 8);
        setServo(BASE, 50,8);
      }
      else
      {
        setServo(POLSO, 130, 8);
        setServo(BASE, 130,8);
      }
  }
  setServo(POLSO, 90, 8);
  setServo(BASE, 90,8);
}
  






void setServo(const int numServo, int gradi, int velocity)
{

  if(numServo == -2){ pos2(); return;}
  if(numServo == -3) { posBilancia(); return;}
  gradi = limite(numServo, gradi);
  int movimento = gradi;
  movimento -= posGradi[numServo];

  if(velocity <= 0) velocity = 1;

  if(movimento < 0)
  {
    
    for(; posGradi[numServo] >= gradi; posGradi[numServo]-=2){
    Servo[numServo].write(posGradi[numServo]);
    delay(40/velocity);
    }
    return;
  }

  for(; posGradi[numServo] <= gradi; posGradi[numServo]+=2){
    Servo[numServo].write(posGradi[numServo]);
    delay(40/velocity);
    }

  
  
}



int limite(const int numServo, const int gradi)
{
  if(gradi > limiteServo[numServo].massimo) return limiteServo[numServo].massimo;
  if(gradi < limiteServo[numServo].minimo) return limiteServo[numServo].minimo;
  return gradi;
  
}

int balance(int alpha){
  
  double seno, coseno, KB, risultato;
  seno = sin(alpha*PI/180);
  coseno = cos(alpha*PI/180);
  
  KB = pow(((20*coseno + (sqrt(400*(coseno*coseno) + 400 * seno + 125 )))/2),2) + 25;
  risultato = ((acos((256.25-KB)/250))*180)/PI;
  return (int)risultato;
  
}


void posBilancia()
{

  posRiposo();
  delay(500);
  int velocity = 1;

  for(int volte = 0; volte < 7; volte++){
    

    
    for(int gradi = 19; gradi < 130; gradi+=1)
    {

      //2 gradi ogni 40/velocita ms
      Servo[SPALLA].write(gradi + 40); 
      int gradiGomito = balance(gradi);
      gradiGomito -= 13;
      gradiGomito = limite(GOMITO, gradiGomito);
      Servo[GOMITO].write(gradiGomito);
      
      Serial.print("Gradi Spalla: ");
      Serial.println(gradi + 50);
      Serial.print("gradiGomito: ");
      Serial.println(gradiGomito+13);


      delay(10);
    }
 
    for(int gradi = 130; gradi > 18; gradi-=1)
    {

      //2 gradi ogni 40/velocita ms
      Servo[SPALLA].write(gradi+20); 
      int gradiGomito = balance(gradi);
      gradiGomito -= 13;
      gradiGomito = limite(GOMITO, gradiGomito);
      Servo[GOMITO].write(gradiGomito);
      Serial.print("Gradi Spalla: ");
      Serial.println(gradi + 50);
      Serial.print("gradiGomito: ");
      Serial.println(gradiGomito+13);

      delay(10);
    }
  }
  
}
