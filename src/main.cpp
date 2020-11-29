
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/Picopixel.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int pinHall = A0;
//Definimos el tiempo mínimo de refresco de la pantalla en ms. Si reducimos mucho esto podemos provocar que perdamos alguna lectura.
//Podemos dejar un valor tan grande como queramos ya que cuando cambie la vuelta se invocará un refresco 
const int frameRefreshTime = 1000;
long lastFrameMillis = 0;
//Esta variable la debe inicializar el método calibrarHall
float med = 0;
float lastMed = 0;
//Error de la magnitud aceptada sin contar vuelta
const float err = 5;
long lastLoopTime = 0;
long loopRate = 0;
long numVueltas = 0;
//Almacena las últimas vueltas mostradas por pantalla
long numVueltasPrinted = 0;
void calibrarHall();
float getMedidaSensorHall();
void conteoDeVuelta();
void refreshOLED();


void setup() {
  pinMode(pinHall, INPUT);
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setTextColor(WHITE, BLACK);
  display.fillScreen(BLACK);
  display.setCursor(0, 10);
  display.setFont(&FreeSans12pt7b);
  display.println("Calibrando sensor...");
  display.display();
  calibrarHall();
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Listo para");
  display.println("correr?");
  display.display();
}

void calculaLoopRate(){
  long now = millis();
  if(lastLoopTime>0){
    loopRate = 1000/(now-lastLoopTime);
  }
  lastLoopTime=now;
}

void loop() {
  calculaLoopRate();
  conteoDeVuelta();
  refreshOLED();
}

//Establece la media con medidas sucesivas
void calibrarHall(){
  float media = 0;
  int iter = 10;
  for(int i=0;i<iter;i++){
    media += getMedidaSensorHall();
    delay(50);
  }
  med = media/iter;
}


void conteoDeVuelta(){
  long medida = getMedidaSensorHall();
  //Dependiendo de cómo coloquemos el imán (polo N o polo S), la fuerza magnética medida por el sensor será positiva o negativa con respecto a la media
  if(medida < med-err){
    while(medida < med-err){
      medida = getMedidaSensorHall();
      delay(1);
    }
    numVueltas++;
  }
  if(medida > med+err){
    while(medida > med+err){
      medida = getMedidaSensorHall();
      delay(1);
    }
    numVueltas++;
  }
}

float getMedidaSensorHall(){
   //media de 10 medidas para filtrar ruido
  float measure = 0;
  for(int i = 0; i < 10; i++){
      measure += analogRead(pinHall);
  }
  measure /= 10.0f;
  lastMed=measure;
  if(abs(measure - med)<err){
    med = (99.0f*med + measure)/100.0f;
  }
  Serial.print(med);
  Serial.print(" ");
  Serial.println(measure);
  return measure;
}

void refreshOLED(){
  long now = millis();
  if((lastFrameMillis+frameRefreshTime<now)
    || numVueltas != numVueltasPrinted){
    lastFrameMillis=now;
    numVueltasPrinted=numVueltas;
    display.fillScreen(BLACK);
    display.clearDisplay();
    display.setCursor(0, 34);
    display.setFont(&FreeSans24pt7b);
    display.print(numVueltas);
    // display.setFont(&FreeSans12pt7b);
    display.setFont(&Picopixel);
    display.setCursor(0,63);
    display.print(med);
    display.print("  ");
    display.print(lastMed);
    display.print(" ");
    display.print(loopRate);
    display.display();
  }
}
