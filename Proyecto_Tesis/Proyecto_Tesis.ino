#include <PS2Mouse.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

#define MOUSE_DATA_2 7 //naranja
#define MOUSE_CLOCK_2 8 //blanco
#define MOUSE_DATA 5 //naranja
#define MOUSE_CLOCK 6 //blanco

PS2Mouse mouse(MOUSE_CLOCK, MOUSE_DATA, STREAM);
PS2Mouse mouse2(MOUSE_CLOCK_2, MOUSE_DATA_2, STREAM);

#define colorSaturation 128
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor black(0);


const int cantidadLeds= 4;
struct LedsEstado{
    int id;
    int cantidad;
    RgbColor color;
    int leds[cantidadLeds];  
};

int cantidadRatonesVivos= 0;
bool cantidadRatonesModificada= false;


LedsEstado estadoLeds0= {0, cantidadLeds, black,{0,1,2,3}};
LedsEstado estadoLeds1= {1, 2, blue, {0,1}};
LedsEstado estadoLeds2= {2, cantidadLeds, green, {0,1,2,3}};

LedsEstado estadoLedsActual= estadoLeds0;

struct Raton { 
    int numero;
    int mx;
    int my;
    bool estaVivo= false;
};



  Raton raton1;
  Raton raton2;


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
  mouse.initialize();
  mouse.set_resolution(02);
  mouse2.initialize();
  mouse2.set_resolution(02);
}

void loop()
{
  loopMice();
  loopLeds();
}

void loopMice(){
  int data[2];
  int data2[2];
  mouse.report(data);
  mouse2.report(data2);


  raton1.numero=1;
  raton1.mx=data[1];
  raton1.my=data[2];
  raton2.numero=2;
  raton2.mx=data2[1];
  raton2.my=data2[2];

 procesarEstadoMouse(&raton1);
 procesarEstadoMouse(&raton2);

 Serial.print("Cant. Ratones vivos: ");
 Serial.print(cantidadRatonesVivos);
 Serial.println();
 
  delay(300);  
}

void AnimUpdate(const AnimationParam& param)
{

  Serial.print("update");
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

void transicionarAEstado(LedsEstado estado, RgbColor color){
    Serial.print(estado.id );
    uint16_t time = 200;
    for (int i=0; i < cantidadLeds; i++){

        int pixel= estadoLeds0.leds[i];
        RgbColor targetColor = black;
        if (i < estado.cantidad){
          for (int j=0; j < estado.cantidad; j++){
            if (estado.leds[j] == pixel){
              targetColor = color;  
            }            
          }
          //pixel= estado.leds[i];
          //targetColor = color;
        }

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
    }

            animations.UpdateAnimations();
        strip.Show();
}

void loopLeds()
{
  if (cantidadRatonesModificada){    
    
    if (cantidadRatonesVivos == 0){
      transicionarAEstado(estadoLeds0, black);
    }    
  
    if (cantidadRatonesVivos == 1){    
      transicionarAEstado(estadoLeds1, blue);
    }
  
    if (cantidadRatonesVivos == 2){
      transicionarAEstado(estadoLeds2, green);
    }   
  }
  else{  
    if (animations.IsAnimating()){
       animations.UpdateAnimations();
        strip.Show();
    }
  }

  cantidadRatonesModificada= false;

  

  
}


void procesarEstadoMouse(Raton *raton){
    if (hayMovimiento(raton->mx,raton->my)){
        if (!raton->estaVivo){
            cantidadRatonesVivos++;
            cantidadRatonesModificada= true;
        }
        raton->estaVivo= true;
    }   
    else{
        if (raton->estaVivo){
            cantidadRatonesVivos--;
            cantidadRatonesModificada= true;
        }
        raton->estaVivo= false;           
    } 
}

bool hayMovimiento(int mx, int my){
  return my != 0 || mx != 0;
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




