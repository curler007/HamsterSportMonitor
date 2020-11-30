
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

//Variables de debug
long lastLoopTime = 0;
long loopRate = 0;

//Pin donde está conectada la señal del sensor Hall
const int pinHall = A0;
//Definimos el tiempo mínimo de refresco de la pantalla en ms. Si reducimos mucho esto podemos provocar que perdamos alguna lectura.
//Podemos dejar un valor tan grande como queramos ya que cuando cambie la vuelta se invocará un refresco 
const int frameRefreshTime = 1000;
long lastFrameMillis = 0;
//Media de la medida. Se tomará el valor de esta variable como el normal 
float med = 0;

float lastMed = 0;
//Error de la magnitud del sensor Hall aceptada 
const float err = 5;

long numVueltas = 0;
//Almacena las últimas vueltas mostradas por pantalla
long numVueltasPrinted = 0;

void calibrarHall();
float getMedidaSensorHall();
void conteoDeVuelta();
void refreshOLED();
void calculaLoopRate();

void setup() {
  //Iniciamos el pin del sensor Hall
  pinMode(pinHall, INPUT);

  Serial.begin(115200);
  //Iniciamos la conexión con la pantalla OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Dirección 0x3C para mi OLED 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  //Mostramos información y calibramos el sensor
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

void loop() {
  calculaLoopRate();
  conteoDeVuelta();
  refreshOLED();
}


//Función de debug que calcula el número de iteraciones de loop por segundo 
void calculaLoopRate(){
  long now = millis();
  if(lastLoopTime>0){
    loopRate = 1000/(now-lastLoopTime);
  }
  lastLoopTime=now;
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

//Funcion que toma la medida del sensor de efecto Hall y determina si debe incrementar las vueltas o no.
//Mientras el imán está pasando cerca del sensor esta función queda bloqueada hasta obtener un valor normal 
void conteoDeVuelta(){
  long medida = getMedidaSensorHall();
  //Dependiendo de cómo coloquemos el imán (polo N o polo S), la fuerza magnética medida por el sensor será positiva o negativa con respecto a la media
  //Acercamos norte
  if(medida < med-err){
    while(medida < med-err){
      medida = getMedidaSensorHall();
      delay(1);
    }
    numVueltas++;
  }
  //Acercamos sur
  if(medida > med+err){
    while(medida > med+err){
      medida = getMedidaSensorHall();
      delay(1);
    }
    numVueltas++;
  }
}

//Obtiene la media del sensor de efecto Hall filtrando el ruido 
float getMedidaSensorHall(){
   //media de 10 medidas para filtrar ruido
  float measure = 0;
  for(int i = 0; i < 10; i++){
      measure += analogRead(pinHall);
  }
  measure /= 10.0f;
  lastMed=measure;
  //si la medida obtenida está dentro de lo aceptable, incluimos la medida ponderada al 1% para evitar 
  //que desviaciones progresivas del valor obtenido nos saquen constantemente de la media
  if(abs(measure - med)<err){
    med = (99.0f*med + measure)/100.0f;
  }
  Serial.print(med);
  Serial.print(" ");
  Serial.println(measure);
  return measure;
}

//Pinta en la pantalla los datos de las vueltas (y de debug)
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
