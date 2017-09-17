#include <PS2Mouse.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include <MouseDevice.h>

#define MOUSE_DATA_3 9 //naranja
#define MOUSE_CLOCK_3 10 //blanco
#define MOUSE_DATA_4 11 //naranja
#define MOUSE_CLOCK_4 12 //blanco


MouseDevice mouseDevice(1,5,6); // Mouse 1 en Pines 5 (data-Naranja) y 6 (clock-Blanco)
MouseDevice mouseDevice2(2,7,8); // Mouse 2 en Pines 7 (data-Naranja) y 8 (clock-Blanco)
MouseDevice mouseDevice3(3,10,11); // Mouse 2 en Pines 7 (data-Naranja) y 8 (clock-Blanco)


PS2Mouse mouse3(MOUSE_CLOCK_3, MOUSE_DATA_3, STREAM);
PS2Mouse mouse4(MOUSE_CLOCK_4, MOUSE_DATA_4, STREAM);

#define colorSaturation 128
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor black(0);

const int CANTIDAD_LEDS= 8;
const int CANTIDAD_MOUSE= 3;
const int CANTIDAD_LEDS_COLOR = 4;

struct LedsColor{
    RgbColor color;
    int cantidad;
    int leds[CANTIDAD_LEDS];    
};
struct LedsEstado{
    int id;
    int cantidad;
    LedsColor configuracionLed[3];  
};


int cantidadRatonesVivos= 0;
bool cantidadRatonesModificada= false;


LedsEstado estadoLeds0= {0, 1, {{black, 8, {0,1,2,3,4,5,6,7}}}};
LedsEstado estadoLeds1= {1, 2, {{blue, 3, {1,3,5}},{black, 5, {0,2,4,6,7}}}};
LedsEstado estadoLeds2= {2, 3, {{blue, 3, {1,3,5}},{green, 3, {0,2,7}}, {black, 2, {4,6}}}};
LedsEstado estadoLeds3= {3, 3, {{blue, 3, {1,3,5}},{green, 3, {0,2,7}}, {red, 2, {4,6}}}};
//LedsEstado estadoLeds3= {3, 6, red, {0,1,2,3,4,5}};
//LedsEstado estadoLeds4= {4, cantidadLeds, red, {0,1,2,3,4,5,6,7}};

LedsEstado estadoLedsActual= estadoLeds0;

 

const uint16_t PixelCount = 8; // make sure to set this to the number of pixels in your strip
const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266
const uint8_t AnimationChannels = PixelCount; // we only need one as all the pixels are animated at once


NeoPixelAnimator animations(PixelCount, NEO_CENTISECONDS);
struct MyAnimationState
{
    RgbColor StartingColor;  // the color the animation starts at
    RgbColor EndingColor; // the color the animation will end at
    AnimEaseFunction Easeing; // the acceleration curve it will use 
};


NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
uint16_t effectState = 0;  // general purpose variable used to store effect state


// one entry per pixel to match the animation timing manager
MyAnimationState animationState[AnimationChannels];  

void setup()
{
  setupMice();
  setupLeds();
}

void setupLeds()
{
    strip.Begin();
    strip.Show();

    SetRandomSeed();
}

void setupMice(){
  Serial.begin(9600);
  mouseDevice.setup();
  mouseDevice2.setup();
  mouseDevice3.setup();
  
  
}

void loop()
{
  loopMice();
  loopLeds();
}

void loopMice(){

 procesarEstadoMouse(&mouseDevice);
 procesarEstadoMouse(&mouseDevice2);
 procesarEstadoMouse(&mouseDevice3);


 Serial.print("Cant. Ratones vivos: ");
 Serial.print(cantidadRatonesVivos);
 Serial.println();
 
  delay(300);  
}

void AnimUpdate(const AnimationParam& param)
{

    // first apply an easing (curve) to the animation
    // this simulates acceleration to the effect
    float progress = animationState[param.index].Easeing(param.progress);

    // this gets called for each animation on every time step
    // progress will start at 0.0 and end at 1.0
    // we use the blend function on the RgbColor to mix
    // color based on the progress given to us in the animation
    RgbColor updatedColor = RgbColor::LinearBlend(
        animationState[param.index].StartingColor,
        animationState[param.index].EndingColor,
        progress);
    // apply the color to the strip
    strip.SetPixelColor(param.index, updatedColor);
}


void transicionarAEstado(LedsEstado estado){
    
    uint16_t time = 100;
    for (int j=0; j < estado.cantidad; j++){
     RgbColor color = estado.configuracionLed[j].color;
    for (int i=0; i < estado.configuracionLed[j].cantidad; i++){
        int pixel= estado.configuracionLed[j].leds[i];
      RgbColor targetColor = color;
        // each animation starts with the color that was present
        RgbColor originalColor = strip.GetPixelColor(pixel);
        // and ends with a random color
        
        // with the random ease function
        AnimEaseFunction easing;

        switch (random(3))
        {
        case 0:
            easing = NeoEase::CubicIn;
            break;
        case 1:
            easing = NeoEase::CubicOut;
            break;
        case 2:
            easing = NeoEase::QuadraticInOut;
            break;
        }

#ifdef ARDUINO_ARCH_AVR        
        animationState[pixel].StartingColor = originalColor;
        // and ends with a random color
        animationState[pixel].EndingColor = targetColor;
        // using the specific curve
        animationState[pixel].Easeing = easing;

        // now use the animation state we just calculated and start the animation
        // which will continue to run and call the update function until it completes
        animations.StartAnimation(pixel, time, AnimUpdate);
#else
        AnimUpdateCallback animUpdate = [=](const AnimationParam& param)
        {
            // progress will start at 0.0 and end at 1.0
            // we convert to the curve we want
            float progress = easing(param.progress);

            // use the curve value to apply to the animation            
            RgbColor updatedColor = RgbColor::LinearBlend(originalColor, targetColor, progress);
            strip.SetPixelColor(pixel, updatedColor);
        };

        // now use the animation properties we just calculated and start the animation
        // which will continue to run and call the update function until it completes
        animations.StartAnimation(pixel, time, animUpdate);        
#endif        
    }}

            animations.UpdateAnimations();
        strip.Show();
    
}

void loopLeds()
{
  if (cantidadRatonesModificada){    
    
    if (cantidadRatonesVivos == 0){
      transicionarAEstado(estadoLeds0);
    }    
  
    if (cantidadRatonesVivos == 1){    
      transicionarAEstado(estadoLeds1);
    }
  
    if (cantidadRatonesVivos == 2){
      transicionarAEstado(estadoLeds2);
   }  
    if (cantidadRatonesVivos == 3){
      transicionarAEstado(estadoLeds3);
    } /*      
    if (cantidadRatonesVivos == 4){
      transicionarAEstado(estadoLeds4, red);
    }*/           
    cantidadRatonesModificada= false;
  }
  else{  
    if (animations.IsAnimating()){
       animations.UpdateAnimations();
        strip.Show();
    }
  }

  

  

  
}


void procesarEstadoMouse(MouseDevice *mouseDevice){

  int estado= mouseDevice->procesarEstado();
  cantidadRatonesVivos+= estado;
  if (estado != 0){
    cantidadRatonesModificada= true;
  }
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

// simple blend function
void BlendAnimUpdate(const AnimationParam& param)
{
    // this gets called for each animation on every time step
    // progress will start at 0.0 and end at 1.0
    // we use the blend function on the RgbColor to mix
    // color based on the progress given to us in the animation
    RgbColor updatedColor = RgbColor::LinearBlend(
        animationState[param.index].StartingColor,
        animationState[param.index].EndingColor,
        param.progress);

    // apply the color to the strip
    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
    {
        strip.SetPixelColor(pixel, updatedColor);
    }
}

void FadeInFadeOutRinseRepeat(float luminance)
{
    if (effectState == 0)
    {
        // Fade upto a random color
        // we use HslColor object as it allows us to easily pick a hue
        // with the same saturation and luminance so the colors picked
        // will have similiar overall brightness
        RgbColor target = HslColor(180 / 360.0f, 1.0f, luminance);
        uint16_t time = random(800, 2000);

        animationState[0].StartingColor = strip.GetPixelColor(0);
        animationState[0].EndingColor = target;

        animations.StartAnimation(0, time, BlendAnimUpdate);
    }
    else if (effectState == 1)
    {
        // fade to black
        uint16_t time = random(600, 700);

        animationState[0].StartingColor = strip.GetPixelColor(0);
        animationState[0].EndingColor = RgbColor(0);

        animations.StartAnimation(0, time, BlendAnimUpdate);
    }

    // toggle to the next effect state
    effectState = (effectState + 1) % 2;
}




