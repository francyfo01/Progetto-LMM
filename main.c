#include "stm32f30x.h" //libreria
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "startup_stm32f30x.s"//libreria per le interrupt
#include "system_stm32f30x.h"

#define PI 3.14159265

#define SYS_CLK 8000000  
#define BAUD_RATE 115200


#define PINZA 0
#define POLSO 1
#define GOMITO 2
#define SPALLA 3
#define BASE 4


char stringBuffer[50] = "Non inzializzata!\n";
  

int velBase = 2;


struct limite{
  int massimo;
  int minimo;
};

struct limite limiteServo[5];

void serialPrint(const char carattere) {
    
    while ((USART1->ISR & 1<<7) == 0); //aspetta finché il bit TXE è 0
    USART1->TDR = carattere;
}


void serialPrintString(const char *stringa) {
	for (int i=0; stringa[i]!='\0'; i++) {
		serialPrint(stringa[i]);
	}
        serialPrint('\n');
}



int limite(const int numServo, const int gradi)
{
  if(gradi > limiteServo[numServo].massimo) return limiteServo[numServo].massimo;
  if(gradi < limiteServo[numServo].minimo) return limiteServo[numServo].minimo;
  return gradi;
  
}

void setServo(const int servo, int gradi, int velocity)
{
  char tempStr[10];
  gradi = limite(servo, gradi);
  sprintf(tempStr,"%d", servo); //converte int to string
  strcpy(stringBuffer, tempStr);
  serialPrintString(stringBuffer);
  
  sprintf(tempStr,"%d", gradi); //converte int to string
  strcpy(stringBuffer, tempStr);
  serialPrintString(stringBuffer); 
  
  sprintf(tempStr,"%d", velBase); //converte int to string
  strcpy(stringBuffer, tempStr);
  serialPrintString(stringBuffer); 
 
  
}



int balance(int alpha){
  
  double seno, coseno, KB, risultato;
  seno = sin(alpha*PI/180);
  coseno = cos(alpha*PI/180);
  
  KB = pow(((20*coseno + (sqrt(400*(coseno*coseno) + 400 * seno + 125 )))/2),2) + 25;
  risultato = ((acos((256.25-KB)/250))*180)/PI;
  return (int)risultato;
  
}

char reciveBuffer;


void posRiposo()
{
  setServo(BASE, limite(BASE,90),velBase );
  setServo(SPALLA, limite(SPALLA,160), velBase);
  setServo(POLSO, limite(POLSO,90), velBase);
  setServo(PINZA, limite(PINZA,70), velBase);
  setServo(GOMITO, limite(GOMITO,10), velBase);
}


void pos1()
{  
  setServo(BASE, limite(BASE,45),velBase );
  setServo(SPALLA, limite(SPALLA,130), velBase);
  setServo(POLSO, limite(POLSO,140), velBase);
  setServo(PINZA, limite(PINZA,90), velBase);
  setServo(GOMITO, limite(GOMITO,70), velBase);
}



void pos0()
{
  setServo(BASE, limite(BASE,10),velBase );
  setServo(SPALLA, limite(SPALLA,160), velBase);
  setServo(POLSO, limite(POLSO,35), velBase);
  setServo(PINZA, limite(PINZA,0), velBase);
  setServo(GOMITO, limite(GOMITO,20), velBase);
}


void pos2()
{
  setServo(-2, 0,0);  
}
 


void posBilancia()
{
  setServo(-3, 0,0);  
}

int main() {
   
    //setup limiti
  
    limiteServo[PINZA].massimo = 70;
    limiteServo[POLSO].massimo = 180;
    limiteServo[GOMITO].massimo = 170;
    limiteServo[SPALLA].massimo = 160;
    limiteServo[BASE].massimo = 170;

    limiteServo[PINZA].minimo = 20;
    limiteServo[POLSO].minimo = 0;
    limiteServo[GOMITO].minimo = 10;
    limiteServo[SPALLA].minimo = 30;
    limiteServo[BASE].minimo = 10;

    RCC->AHBENR |= (1 << 17) | (1 << 19) | (1 << 21); //abilitazione clock A/C/E
    RCC->APB1ENR |= 1 << 4;      
    RCC->APB2ENR |= 1 << 14;  // USART1

    for(int bit = 16; bit <= 30; bit+=2)
    { //impostazione dei bit dei led in modalità output
      GPIOC->MODER |= (1<<bit);
    }
    
    
    // GPIO
    GPIOA->MODER |= (0b00 << 0);
    GPIOC->MODER |= (0b10 << 8) | (0b10 << 10); //settaggio pin nella modalità alternativa
    GPIOC->AFR[0] |= (0b0111 << 16) | (0b0111 << 20); //settaggio funzione alternativa dei pin PC4 e PC5 con il valore AF7
    GPIOE->MODER |= 0x5555 << 16;
    
    
    
    // UART
    USART1->CR1 = (1 << 3) | (1 << 2);  // TE | RE  |(1<<?) RXNEIE
    USART1->CR2 = (0b00 << 12);         // 1 Stop Bit
    USART1->BRR = SYS_CLK / BAUD_RATE;
    USART1->CR1 |= 1;  // Enable
    
    
    //NVIC (interrupt)
    //NVIC->ISER[1] |= (1 << 5); //ISR posizione 37 (sta nel registro [1] perchè superiore a 32 -> 37-32 = 5)
    
    
   uint8_t hasBeenPressed = 0; //flag per capire se il bottone è stato premuto
   int num = 0; //variabile per selettore posizione
   
   
   for(int bit = 8; bit <=15; bit++) //accensione di tutti i led per segnalare posizione di riposo 
   {
     GPIOE->ODR |= (1<<bit);
   }
   
   
   posRiposo(); //invia ad arduino le posizioni dei singoli servo per mettere il braccio nella posizione di riposo
   while (1){
      
     /* while((USART1->ISR & (1<<5)) == (1<<5)) //BIT RXNE del registro ISR che si alza se riceve su rx
      {
        serialPrint('a');
        reciveBuffer = USART1->RDR;
        serialPrint(reciveBuffer);
        USART1->ICR |= (1 << 3);
      }
      reciveBuffer = USART1->RDR;*/
      
        
      uint8_t buttonIDRValue = (GPIOA->IDR & 0x1); //uso "uint8_t" perch? questo il tipo usato da "GPIOA->IDR" nella libreria
      if(buttonIDRValue == 0x1){ //faccio la verifica del valore del registro per vedere se è stato premuto il pulsante
        hasBeenPressed = 1;
      }
      else
      {
        if(hasBeenPressed)
        {
          
          
          for(int bit = 8; bit <=15; bit++) //for che spegne tutti i led
         {
           GPIOE->ODR &= ~(1<<bit);
           
         }
          
          
          
          switch (num)
          {
            case 0:
              pos0();
              GPIOE->ODR &= ~(1<<11); //spegne il led 4
              GPIOE->ODR |= (1<<8); //accendi il led 1
              break;
            case 1:
              pos1();
              GPIOE->ODR &= ~(1<<8); //spegne led 1
              GPIOE->ODR |= (1<<9); //accende led 2
              break;
            case 2:
              pos2();
              GPIOE->ODR &= ~(1<<9); //spegne 2
              GPIOE->ODR |= (1<<10); //accende led 3
              break;
            case 3:
              
              posBilancia();
              GPIOE->ODR &= ~(1<<10); //spegne led 3
              GPIOE->ODR |= (1<<11);  //accende led 4
              break;
          default:
              break;
          }
          num = (num + 1)%4; //num deve rimanere tra 0 e 3
        }
        
        hasBeenPressed = 0; //flag bottone a 0
      }
      
    }
}



/*ISR per interrupt su RX della USART1.
  Politica PUBWEAK (pubblica debole): dentro il file "startup_stm32f30x.s" ci sono le ISR 
  di default per ogni interrupt interna, ma non fanno niente e danno errore
  e quindi dichiaro una funzione con lo stesso nome di quella del file relativa alla isr che mi serve
  e dato che la st legge prima la mia nel main.c e poi quella, usa la mia e lascia stare quella (grazie alla Politica PUBWEAK (pubblica debole))
*/
/*
void USART1_IRQHandler()
{
    if((USART1->ISR & (1<<5)) == (1<<5))
    {
    USART1->ICR |= (1 << 3);  
    reciveBuffer = USART1->RDR;
    USART1->ICR |= (1 << 3);
    serialPrint(reciveBuffer);
    }
}

*/




