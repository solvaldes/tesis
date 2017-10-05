#include <PS2Mouse.h>
#include <MouseDevice.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 2
const int CANTIDAD_LEDS= 73;
const int CANTIDAD_LEDS_ESTADO1= 15;
const int CANTIDAD_LEDS_ESTADO2= 33;
const int CANTIDAD_LEDS_ESTADO3= 53;
const int CANTIDAD_LEDS_ESTADO4= CANTIDAD_LEDS;
MouseDevice mouseDevice(1,5,6); // Mouse 1 en Pines 5 (data-Naranja) y 6 (clock-Blanco)
MouseDevice mouseDevice2(2,7,8); // Mouse 2 en Pines 7 (data-Naranja) y 8 (clock-Blanco)
MouseDevice mouseDevice3(3,9,10); // Mouse 3 en Pines 9 (data-Naranja) y 10 (clock-Blanco)
MouseDevice mouseDevice4(4,11,12); // Mouse 4 en Pines 11 (data-Naranja) y 12 (clock-Blanco)

struct LedsEstado{
  int leds[CANTIDAD_LEDS];
  int cantidadLeds;
  bool iniciado;  
};

LedsEstado ledsEstado1= {{},CANTIDAD_LEDS_ESTADO1, false};
LedsEstado ledsEstado2= {{},CANTIDAD_LEDS_ESTADO2, false};
LedsEstado ledsEstado3= {{},CANTIDAD_LEDS_ESTADO3, false};
LedsEstado ledsEstado4= {{},CANTIDAD_LEDS_ESTADO4, false};

int cantidadRatonesVivos= 0;
int cantidadRatonesVivosAnt= 0;
int vecesLoopEstado4= 0;
bool cantidadRatonesModificada= false;
uint16_t colorPivot=0;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(CANTIDAD_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  setupMice();

  SetRandomSeed();
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  configurarEstado();
}

void setupMice(){
  Serial.begin(9600);
  mouseDevice.setup();
  mouseDevice2.setup();
  mouseDevice3.setup();
  mouseDevice4.setup();
  
  
}

void randomizarLeds(int cantidadLeds, int* leds){
  //Inicializando arreglo de leds
  int aux=CANTIDAD_LEDS;
  int todosLosLeds[CANTIDAD_LEDS]= {};
  for (int i=0; i < CANTIDAD_LEDS; i++){
    todosLosLeds[i]= i;  
  }
  
  // Seteo aleatoriamente la cantidad de leds especificada en el arreglo de leds
  for (int k=0; k < cantidadLeds; k++){
    int posicion= random(aux);
    leds[k]= todosLosLeds[posicion];
    aux--;  
    for (int l=posicion; l < CANTIDAD_LEDS; l++){
      todosLosLeds[l]= todosLosLeds[l+1];
    }
  }  
}

void configurarEstado(){
    randomizarLeds(ledsEstado1.cantidadLeds, ledsEstado1.leds);
    randomizarLeds(ledsEstado2.cantidadLeds, ledsEstado2.leds);
    randomizarLeds(ledsEstado3.cantidadLeds, ledsEstado3.leds);
    randomizarLeds(ledsEstado4.cantidadLeds, ledsEstado4.leds);
}

void transicionarEstado(LedsEstado *ledsEstado){

    colorWipe(strip.Color(0, 0, 255), 40, ledsEstado->cantidadLeds, ledsEstado->leds); 
    ledsEstado->iniciado= true;
  
 
}

void loopEstado1(){
  rainbow(50, tomarColorFrio1(), CANTIDAD_LEDS_ESTADO1, ledsEstado1.leds);
  colorPivot++;
}

void loopEstado2(){
  theaterChaseRainbow(50, colorPivot%256, CANTIDAD_LEDS_ESTADO2, ledsEstado2.leds);
  colorPivot++;
}

void loopEstado3(){
  theaterChaseRainbow(40, colorPivot%256, CANTIDAD_LEDS_ESTADO3, ledsEstado3.leds);
  colorPivot++;
}

void loopEstado4(){
  if (vecesLoopEstado4 <= 30){
    theaterChaseRainbow(40, colorPivot%256, CANTIDAD_LEDS_ESTADO4, ledsEstado4.leds);
          vecesLoopEstado4++;
     Serial.println(vecesLoopEstado4);     
  }
  else{
    theaterChaseRainbowOriginal(20);  
    vecesLoopEstado4= 0;
  }
}


void loopEstado0(){
  
      colorWipe(strip.Color(0, 0, 255), 20, CANTIDAD_LEDS, ledsEstado4.leds); 

    
}

void loopMice(){

 procesarEstadoMouse(&mouseDevice);
 procesarEstadoMouse(&mouseDevice2);
 procesarEstadoMouse(&mouseDevice3);
 procesarEstadoMouse(&mouseDevice4);

 Serial.print("Cant. Ratones vivos: ");
 Serial.print(cantidadRatonesVivos);
 Serial.println();
 
  delay(300);  
}

void procesarEstadoMouse(MouseDevice *mouseDevice){

  int estado= mouseDevice->procesarEstado();  
  cantidadRatonesVivos+= estado;
  if (estado != 0){    
    cantidadRatonesModificada= true;
  }  
}

void loop() {
  cantidadRatonesVivosAnt= cantidadRatonesVivos;
  loopMice();
  if (cantidadRatonesModificada){
    cantidadRatonesModificada= false;
    switch (cantidadRatonesVivos) {
      case 0: loopEstado0();break;
      case 1: if(cantidadRatonesVivosAnt == 0){transicionarEstado(&ledsEstado1);};break;
      case 2: if(cantidadRatonesVivosAnt == 0){transicionarEstado(&ledsEstado2);};break;
      case 3: if(cantidadRatonesVivosAnt == 0){transicionarEstado(&ledsEstado3);};break;
      case 4: if(cantidadRatonesVivosAnt == 0){transicionarEstado(&ledsEstado4);};break;
    }   
  }
    switch (cantidadRatonesVivos) {
      case 1: loopEstado1();break;
      case 2: loopEstado2();break;
      case 3: loopEstado3();break;
      case 4: loopEstado4();break;
    }  
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait, int cantidad, int leds[]) {
  // Recorro cada uno de los leds
  for (uint16_t k=0; k < CANTIDAD_LEDS;k++){
    // Seteo el color a negro (por defecto)
    strip.setPixelColor(k, 0);    
    // Recorro los leds asignados al arreglo recibido para verificar si el led actual debe prenderse con el color indicado
    for (uint16_t l=0; l < cantidad;l++){
      if (k == leds[l]){
                        if (cantidadRatonesVivos == 0){
          strip.setPixelColor(leds[l], 0);    //turn every third pixel on
        }
                if (cantidadRatonesVivos == 1){
          strip.setPixelColor(leds[l], tomarColorFrio1());    //turn every third pixel on
        }
        if (cantidadRatonesVivos == 2){
          strip.setPixelColor(leds[l], tomarColorFrio2());    //turn every third pixel on
          }       
          if (cantidadRatonesVivos == 3){
          strip.setPixelColor(leds[l], tomarColorCalido1());    //turn every third pixel on
          }
          if (cantidadRatonesVivos == 4){
          strip.setPixelColor(leds[l], tomarColorCalido2());    //turn every third pixel on
          } 
        
      }
    }
    strip.show();      
    delay(wait);
  }
}

void theaterChaseRainbowOriginal(uint8_t wait) {
  for (int j=0; j < 256; j++) {
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, tomarColorCalido2());    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  } 
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void theaterChaseRainbow(uint8_t wait, uint16_t colorPivot, int cantidad, int leds[]) {
    
      for (uint16_t i=0; i < cantidad; i++) {
        if (cantidadRatonesVivos == 1){
          strip.setPixelColor(leds[i], tomarColorFrio1());    //turn every third pixel on
        }
        if (cantidadRatonesVivos == 2){
          strip.setPixelColor(leds[i], tomarColorFrio2());    //turn every third pixel on
          }       
          if (cantidadRatonesVivos == 3){
          strip.setPixelColor(leds[i], tomarColorCalido1());    //turn every third pixel on
          }
          if (cantidadRatonesVivos == 4){
          strip.setPixelColor(leds[i], tomarColorCalido2());    //turn every third pixel on
          }          
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < cantidad; i++) {
        strip.setPixelColor(leds[i], 0);        //turn every third pixel off
      }
    
}

void rainbow(uint8_t wait, uint16_t colorPivot, int cantidad, int leds[]) {
  uint16_t i, j;


    for(i=0; i<cantidad; i++) {        
        int color= tomarColorFrio1();
        strip.setPixelColor(leds[i], color);
    }
    strip.show();    
    delay(wait);
  
}

uint32_t tomarColorFrio1(){
  int valorRojo= random(132);
  int valorAzul= random(255-128) + 128;
  int valorVerde= random(100);
  return strip.Color(valorRojo,valorVerde,valorAzul);  
}

uint32_t tomarColorFrio2(){
  int morado= random(2);
  if (morado == 1){
    int valorRojo= random(206-128) + 128;
    int valorAzul= random(128-99) + 99;    
    return strip.Color(valorRojo,0,valorAzul);  

  }
  else{
     int valorVerde= random(255-160) + 160;
     int valorAzul= random(140);    
      return strip.Color(0,valorVerde,valorAzul);    
  }
}

uint32_t tomarColorCalido1(){
  int calido= random(10);
  if (calido > 4){
     int valorVerde= random(255-156) + 156;      
      return strip.Color(255,valorVerde,0);        
   }
   else{
      return tomarColorFrio2(); 
    }
}

uint32_t tomarColorCalido2(){
  int calido= random(20);
  if (calido > 10){
     int valorVerde= random(156);       
      return strip.Color(255,valorVerde,0);        
   }
   else{
      if (calido > 10 && calido <= 15){
        return tomarColorCalido1();
      }
      else{
        return tomarColorFrio2(); 
      }
    }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}



void SetRandomSeed()
{
    uint32_t seed;

    // random works best with a seed that can use 31 bits
    // analogRead on a unconnected pin tends toward less than four bits
    seed = analogRead(0);
    delay(1);

    for (int shifts = 3; shifts < 31; shifts += 3)
    {
        seed ^= analogRead(0) << shifts;
        delay(1);
    }

    randomSeed(seed);
}
