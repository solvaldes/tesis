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
MouseDevice mouseDevice3(3,10,11); // Mouse 3 en Pines 10 (data-Naranja) y 11 (clock-Blanco)

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
  //mouseDevice4.setup();
  
  
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
  ledsEstado1.iniciado=false;
  ledsEstado2.iniciado=false;
  ledsEstado3.iniciado=false;
  ledsEstado4.iniciado=false;
  if (!ledsEstado->iniciado){
    colorWipe(strip.Color(0, 0, 255), 50, ledsEstado->cantidadLeds, ledsEstado->leds); // Blue
    ledsEstado->iniciado= true;
  }
 
}

void loopEstado1(){
  rainbow(20, colorPivot%256, CANTIDAD_LEDS_ESTADO1, ledsEstado1.leds);
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
 theaterChaseRainbowOriginal(50);
}


void loopEstado0(){
    for (uint16_t k=0; k < CANTIDAD_LEDS;k++){
      // Seteo el color a negro (por defecto)
      strip.setPixelColor(k, 0);    
      strip.show();        
      delay(20);
    }
    
}

void loopMice(){

 procesarEstadoMouse(&mouseDevice);
 procesarEstadoMouse(&mouseDevice2);
 procesarEstadoMouse(&mouseDevice3);
//procesarEstadoMouse(&mouseDevice4);

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
  /*
  loopEstado1();
  delay(2000);
  loopEstado2();
  delay(2000);
  loopEstado3();  
  delay(2000);
  loopEstado4();    
  */
  loopMice();
  if (cantidadRatonesModificada){
    cantidadRatonesModificada= false;
    switch (cantidadRatonesVivos) {
      case 0: loopEstado0();break;
      case 1: transicionarEstado(&ledsEstado1);break;
      case 2: transicionarEstado(&ledsEstado2);break;
      case 3: transicionarEstado(&ledsEstado3);break;
      case 4: transicionarEstado(&ledsEstado4);break;
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
        strip.setPixelColor(leds[l], c);
      }
    }
    strip.show();      
    delay(wait);
  }
}

void theaterChaseRainbowOriginal(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
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
        strip.setPixelColor(leds[i], Wheel( (i+colorPivot) % 255));    //turn every third pixel on
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
      strip.setPixelColor(leds[i], Wheel((i+colorPivot) & 255));
    }
    strip.show();
    delay(wait);
  
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
