#include <SPI.h>                  
#include <nRF24L01.h>                 //biblioteki do obsługi nrf24L01                 
#include <RF24.h>    

#include <Servo.h>                    //biblioteka do obsługi serwomechanizmów

Servo servo1;                       
Servo servo2;                            
Servo servo3;                          //definiowanie nazw poszczególnych serw
Servo servo4;                        
Servo servo5;                        

#define serwo_min_1  0  //kciuk
#define serwo_min_2  0 //serdeczny
#define serwo_min_3  0   //mały palec            //wartości minimalne dla każdego serwa    
#define serwo_min_4  0 //srodkowy
#define serwo_min_5  180  // wskazujacy


#define serwo_max_1  180
#define serwo_max_2  180
#define serwo_max_3  180              //wartości maksymalne dla każdego serwa    
#define serwo_max_4  180
#define serwo_max_5  0

const uint64_t pipe = 0x1CF8361637LL;   //kod kanału odbiornika

RF24 radio(9, 10);                   //piny ce cs modułu nrf24L01

int odczyt[6];                       //zmienne do przechowywania otrzymanych danych
//int dane[2];
//long wybor, wybor_u, wybor_a;        //przechowywanie wyboru urzytkownika i losowania ręki podczas gry

void setup() {
  servo1.attach(2);
  servo2.attach(3);
  servo3.attach(4);                  //przypisywanie pinów serw do ich nazw
  servo4.attach(5);
  servo5.attach(6);
  delay(1000);                       //czekaj 1s
                              
  radio.begin();                     //włączenie komunikacji między arduino, a nrf24L01          
  radio.setDataRate(RF24_2MBPS);     //prędkość transmisji danych na 2mbps          
  radio.setPALevel(RF24_PA_HIGH);    //siła nadawania/odbierania na max       
  radio.openReadingPipe(1,pipe);     //odbieraj/nadawaj na kanale pipe      
  radio.startListening();            //zacznij nasłuchiwać
              
  Serial.begin(9600);               //zacznij komunikację na porcie szeregowym
  randomSeed(analogRead(A0));       //ustaw "ziarno" do losowania na nie używany port A0
}

void loop() {
  delay(5);
  radio.startListening();          //zacznij nasłuchiwać 
  
  if ( radio.available() ){                           //jeśli coś przyszło     
    radio.read( odczyt, sizeof(odczyt) );              //odczytaj i włóż do każdej szufladki zmiennej odczyt
  }

  switch(odczyt[0]){                            //wykonaj daną funkcję 
    case 1:                             
      Serial.print(odczyt[0]);                 // wypisz dane na Serial porcie i
      Serial.print(" || ");                    // wpisz wartości do serw
      Serial.print(odczyt[1]);
      Serial.print(" || ");
      Serial.print(odczyt[2]);
      Serial.print(" || ");
      Serial.print(odczyt[3]);
      Serial.print(" || ");
      Serial.print(odczyt[4]);
      Serial.print(" || ");
      Serial.println(odczyt[5]);
    
      servo1.write(odczyt[1]);
      servo2.write(odczyt[2]);         //ustaw serwo mechanizmy na danej pozycji
      servo3.write(odczyt[3]);
      servo4.write(odczyt[4]);
      servo5.write(odczyt[5]);
      break;

    case 2:
      Serial.println(odczyt[2]);

      switch(odczyt[2]){                     //jeżeli wybrano daną funkcję w menu Figury
        case 1:case 6: 
        kamien();                         //wykonaj funkcję kamień
          delay(2000);                       //czekaj 0,5s 
          break;
   
           case 3:  case 4: 
 nozyce();                         //wykonaj funkcję nożyce
          delay(2000);                       //czekaj 0,5s 
          break;
   
        case 2: case 5:
  papier();                         //wykonaj funkcję papier
          delay(2000);                       //czekaj 0,5s 
          break;
      }
    break;

    zeruj();                    //wyzeruj pozycję serw
  }      
}
  
void zeruj(){
  servo1.write(serwo_min_1);
  servo2.write(serwo_min_2);  
  servo3.write(serwo_min_3);
  servo4.write(serwo_min_4);
  servo5.write(serwo_min_5);                                                          
}




void papier(){
  servo1.write(serwo_min_1);
  servo2.write(serwo_min_2);  
  servo3.write(serwo_min_3);
  servo4.write(serwo_min_4);
  servo5.write(serwo_min_5); 
}

void kamien(){ 
  servo2.write(serwo_max_2);  
  servo3.write(serwo_max_3);
  servo4.write(serwo_max_4);
  servo5.write(serwo_max_5); 
  servo1.write(serwo_max_1);  
}

void nozyce(){
  servo1.write(serwo_max_1); 
  servo2.write(serwo_max_2);  
  servo3.write(serwo_max_3);
  servo4.write(serwo_min_4);
  servo5.write(serwo_min_5);
}
