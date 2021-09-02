#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>                  //biblioteki do obsługi wyświetlacz 
#include <Adafruit_SSD1306.h>
                         
#include <nRF24L01.h>                      //biblioteki do obsługi nrf24L01 
#include <RF24.h>                           

#define outputA 4
#define outputB 3                          //definicja pinów od enkodera
#define selectButton 2

#define SCREEN_WIDTH 128                   //wysokość i szerokość wyświetlacza
#define SCREEN_HEIGHT 32 

#define OLED_RESET     -1                 //linijka potrzebna do działania wyświetlacza
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //definicja wartości wyświetlacza
                      
const uint64_t pipe = 0x1CF8361637LL;      //kod kanału
RF24 radio(9, 10);                         //tworzymy instancje komunikacji

int menu = 1;                              //strona menu

int aState;
int aLastState;  

int dane[6];                              //przechowuje dane do wysłania
int odczyt[5]; 

int low[5];                             //buffory do przechowywania danych 
int high[5];

int wybor_u;
int wybor_a;

boolean tylkoNasladowanie = false;
boolean plus = false;
 
void setup() {
  pinMode(outputA,INPUT_PULLUP);
  pinMode(outputB,INPUT_PULLUP);        //ustawienie pinów enkodera jako wejście 
  pinMode(selectButton, INPUT_PULLUP);  //podciągnięte do zasilania
  
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);                   //ustawienie pinów czujników zgięcia jako
  pinMode(A3, INPUT);                   //wejście
  pinMode(A6, INPUT);
  
  Serial.begin (9600);                 //rozpoczęcie komunikacji szeregowej
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){ //sprawdzenie działania wyświetlacz
  Serial.println(F("SSD1306 problem z inicjacja wyswietlacza")); //błąd z inicjalizacją wyświetlacza
    //for(;;);                                      //czekaj ciągle
  tylkoNasladowanie = true;
  }
  
  randomSeed(analogRead(A7));          //ustaw "ziarno" do losowania na nie używany port A0

  radio.begin();                       //uruchom moduł
  radio.setDataRate(RF24_2MBPS);       //ustaw prędkość transmisji na 2Mb/s
  radio.setPALevel(RF24_PA_HIGH);      //ustaw wzmocnienie modułu radiowego na wysokie 
  radio.openWritingPipe(pipe);         //rozpocznij transmisję
  
  aLastState = digitalRead(outputA);  //odczytaj stan pinu enkodera
  
  if(!tylkoNasladowanie){
    display.display();                   //zainicjuj wyświetlacz
    display.clearDisplay();              //wyczyść wyświetlacz
  //calibration();                       //kalibracja czujników ugięcia
    drawMenu();                      //pokaż menu
  }
}

void loop() {
  if(!tylkoNasladowanie){
    aState = digitalRead(outputA);     //odczytaj stan pinu enkodera
    if (aState != aLastState){        //jeżeli odczytany stan jest różny od poprzedniego stanu
      if (digitalRead(outputB) != aState) { //????
        plus = true;                 //plus ustawiamy na 1
      }
    } 
    aLastState = aState;           //poprzedni stan = stan
     
    if(plus == true){             //jeżeli plus = 1
      menu++;                     //kolejna strona menu
      drawMenu();                 //pokaż menu na ekranie
      delay(100);                 //czekaj 0,1s
      while(!plus);               //czekaj do póki !plus
      plus=false;                 //ustaw plus na 0
    }
    
    if (!digitalRead(selectButton)){
      delay(200);
      display.clearDisplay();
      do{
        executeAction();
      }while(digitalRead(selectButton) == HIGH);
      drawMenu();
      while (!digitalRead(selectButton)); //????
    }
    display.display();
  } else {
  menu = 1;  
  executeAction();
  }
}

void calibration(){
  display.clearDisplay();
  delay(500);
  for(int i=5;i>-1;i--){
    display.setTextSize(1);
  display.setCursor(4,0);
    display.setTextColor(1,0);
    display.print("Kalibracja");
    display.setCursor(2,15);
    display.setTextColor(1,0);
    display.print("Wyprostowane: ");
    display.setCursor(55,15);
    display.setTextColor(1,0);
    display.print(i);
    display.display();
    delay(500);
  }
   
  low[0] = analogRead(A6);
  low[1] = analogRead(A3);
  low[2] = analogRead(A0);
  low[3] = analogRead(A1);
  low[4] = analogRead(A2);
  
  printDataFromSensors();
  
  display.clearDisplay();
  delay(500);
  for(int i=5;i>-1;i--){
    display.setTextSize(1);
  display.setCursor(4,0);
    display.setTextColor(1,0);
    display.print("Kalibracja");
    display.setCursor(2,15);
    display.setTextColor(1,0);
    display.print("Zgięte: ");
    display.setCursor(55,15);
    display.setTextColor(1,0);
    display.print(i);
    display.display();
    delay(500);
  }
   
  high[0] = analogRead(A6);
  high[1] = analogRead(A3);
  high[2] = analogRead(A0);
  high[3] = analogRead(A1);
  high[4] = analogRead(A2);
  
  printDataFromSensors();
}

void drawMenu() {
  display.clearDisplay();
  
  dane[0] = 0; //Pierwszy parametr oznacza wybór opcji z menu 
  dane[1] = 0; 
  dane[2] = 0; //mapowanie odczytów z pinów
  dane[3] = 0; //analogowych na ruch serw.
  dane[4] = 0;
  dane[5] = 0;
  
  radio.write(dane, sizeof(dane));
  
  switch (menu) {
    case 0:
      menu = 1;
      break;
    case 1:
      showMenu(1);
      break;
    case 2:
    showMenu(2);
      break;
    case 3:
      showMenu(3);
      break;
    case 4:
      menu = 0;
      break;
  }
}

void showMenu(int choice){
  int a1 = 1;
  int b1 = 0;
  int a2 = 1;
  int b2 = 0;
  int a3 = 1;
  int b3 = 0;
  
  switch (choice) {
    case 1:
      a1 = 0;
    b1 = 1;
      break;
    case 2:
      a2 = 0;
    b2 = 1;
      break;
  case 3:
      a3 = 0;
    b3 = 1;
      break;
  }
  
    display.display();
    display.setTextSize(1);
    
    display.setTextColor(a1,a2);
    display.setCursor(5,0);
    display.print("Nasladowanie");
    
    display.setTextColor(a2,b2);
    display.setCursor(5,10);
    display.print("Kamien,Papier,Nozyce");
    
    display.setTextColor(a3,b3);
    display.setCursor(5,20);
    display.print("Informacje");
    display.display();
}
  
void executeAction() {
  switch (menu) {
    case 1:
      action1();
      break;
    case 2:
      action2();
      break;
    case 3:
      action3();
      break;
  }
}

void printDataFromSensors(){
  Serial.print("kciuk: ");
  Serial.print(analogRead(A6));
  Serial.print(" || ");
  Serial.print("serdeczny: ");
  Serial.print(analogRead(A3));
  Serial.print(" || ");
  Serial.print("maly: ");
  Serial.print(analogRead(A0));
  Serial.print(" || ");
  Serial.print("srodkowy: ");
  Serial.print(analogRead(A1));
  Serial.print(" || ");
  Serial.print("wskazujacy: ");
  Serial.println(analogRead(A2));
}

void action1() {
  //TODO: zamienic wartości na te z tabel low i high
  //Odczytanie wartości z pinów analogowych i wpisanie ich do zmiennych dane
  dane[0] = 1; //Pierwszy parametr oznacza wybór opcji z menu
  dane[1] = map(analogRead(A6), 390, 330, 180, 0); //kciuk
  dane[2] = map(analogRead(A3), 400, 260, 180, 0); //serdeczny - mapowanie odczytów z pinów
  dane[3] = map(analogRead(A3), 400, 260, 180, 0); //maly - analogowych na ruch serw.
  dane[4] = map(analogRead(A1), 310, 250, 180, 0); //srodkowy
  dane[5] = map(analogRead(A2), 340, 230, 0, 180); //wskazujacy
  
 
  //Wyświetlanie menu oraz wysyłanych danych 
  printDataFromSensors();
  if(!tylkoNasladowanie){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(8,0);
    display.setTextColor(1,0);
    display.print("Nasladowanie");
    display.drawLine(0,9,128,9,1);
    display.display();
    display.setCursor(0,20);
    display.print(dane[1]);
    display.setCursor(30,20);
    display.print(dane[2]);
    display.setCursor(55,20);
    display.print(dane[3]);
    display.setCursor(80,20);
    display.print(dane[4]);
    display.setCursor(105,20);
    display.print(dane[5]);
    display.display();
  }

  // Wysyłanie danych do odbiornika
  radio.openWritingPipe(pipe);
  radio.write(dane, sizeof(dane));
}
void action2() {
  display.clearDisplay();
  delay(500);
  for(int i=3;i>-1;i--){
    display.setTextSize(1);
    display.setCursor(8,0);
    display.setTextColor(1,0);
    display.print("Gra za: ");
    display.setCursor(55,0);
    display.setTextColor(1,0);
    display.print(i);
    display.display();
    delay(500);
  }
  
  odczyt[0] = 2; //Pierwszy parametr oznacza wybór opcji z menu 
  odczyt[1] = analogRead(A6);
  odczyt[2] = analogRead(A3);
  odczyt[3] = analogRead(A3);
  odczyt[4] = analogRead(A1);
  odczyt[5] = analogRead(A2);
  
  printDataFromSensors();
  
  //TODO: skasować jak będzie odczyt z środkowego palca
  //odczyt[4] = 200;

  //TODO: zamienic wartości na te z tabel low i high ((high-low)/2+low)
  if(odczyt[1] > 320 && odczyt[2] > 260 && odczyt[3] > 220 && odczyt[4] > 250 && odczyt[5] > 250){//odczyt palców, wybrano KAMIEŃ
    Serial.println("kamien_reka");
    wybor_u = 1;
  }
  else if(odczyt[1] < 320 && odczyt[2] < 260 && odczyt[3] < 220 && odczyt[4] < 250 && odczyt[5] < 250){//odczyt palców, wybrano PAPIER
    Serial.println("papier_reka");
    wybor_u = 2;
  }
  else if(odczyt[1] > 320 && odczyt[2] > 260 && odczyt[3] > 220 && odczyt[4] < 250 && odczyt[5] < 250){//odczyt palców, wybrano NOŻYCE
    Serial.println("nozycze_reka");
    wybor_u = 3;
  }
        
  //Wybór użytkownika
  display.clearDisplay();
  showChoice(0,10,"Ty:");
  switch(wybor_u){
    case 1:
    showChoice(70,10," Kamien");
      break;
   
    case 2:
    showChoice(70,10," Papier");
      break;
   
    case 3:
    showChoice(70,10," Nozyce");
      break;
   
    default:
    showChoice(70,10," Zly Gest");
      break;
  }

  //Wybór ręki
  showChoice(0,20,"Reka:");
  int wybor_a = random(1,6);
  switch(wybor_a){ 
    case 1: 
    case 6: 
    showChoice(70,20," Kamien");
      break;
    case 3: 
    case 4:
    showChoice(70,20," Nozyce");
    break;
    case 2:
    case 5:
      showChoice(70,20," Papier");    
      break;
    default:
    showChoice(70,20," Zly Gest");
      break;
  }
  delay(5);
  
  dane[0] = 2;
  dane[1] = wybor_u; //wybór użytkownika
  dane[2] = wybor_a; //wybór ręki
  dane[3] = 0;
  dane[4] = 0;
  dane[5] = 0;
   
  radio.openWritingPipe(pipe);
  radio.write(dane, sizeof(dane));
  
  delay(2500);
  display.display();
}

void showChoice(int cursorH, int cursorW, String text){
  display.setCursor(cursorH,cursorW);
  display.setTextColor(1,0);  
  display.setTextSize(1); 
  display.print(text);
  display.display();
}

void action3() {
  display.setTextSize(1);
  display.setCursor(8,0);
  display.setTextColor(1,0);
  display.print("Informacje");
  display.drawLine(0,9,128,9,1);
  display.display();
  
  display.setCursor(15,15);
  display.setTextColor(1,0);  
  display.setTextSize(1);
  display.print("Bioniczna Reka");
  display.display();
}
