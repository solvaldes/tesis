#include <PS2Mouse.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

#define MOUSE_DATA_2 7 //naranja
#define MOUSE_CLOCK_2 8 //blanco
#define MOUSE_DATA 5 //naranja
#define MOUSE_CLOCK 6 //blanco

PS2Mouse mouse(MOUSE_CLOCK, MOUSE_DATA, STREAM);
PS2Mouse mouse2(MOUSE_CLOCK_2, MOUSE_DATA_2, STREAM);

int cantidadRatonesVivos= 0;

struct Raton { 
    int numero;
    int mx;
    int my;
    bool estaVivo= false;
};

  Raton raton1;
  Raton raton2;

#define colorSaturation 128
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor black(0);

const uint16_t PixelCount = 6; // make sure to set this to the number of pixels in your strip
const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266
const uint8_t AnimationChannels = 1; // we only need one as all the pixels are animated at once

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
// For Esp8266, the Pin is omitted and it uses GPIO3 due to DMA hardware use.  
// There are other Esp8266 alternative methods that provide more pin options, but also have
// other side effects.
//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);
//
// NeoEsp8266Uart800KbpsMethod uses GPI02 instead

NeoPixelAnimator animations(AnimationChannels); // NeoPixel animation management object

uint16_t effectState = 0;  // general purpose variable used to store effect state


// what is stored for state is specific to the need, in this case, the colors.
// basically what ever you need inside the animation update function
struct MyAnimationState
{
    RgbColor StartingColor;
    RgbColor EndingColor;
};

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

void loopLeds()
{
  if (cantidadRatonesVivos == 0){
    strip.SetPixelColor(0, black);
    strip.SetPixelColor(1, black);
    strip.SetPixelColor(2, black);
    strip.SetPixelColor(3, black);
    strip.SetPixelColor(4, black);
    strip.SetPixelColor(5, black);
  }  
  

  if (cantidadRatonesVivos == 1){
    strip.SetPixelColor(0, red);
    strip.SetPixelColor(1, red);
    strip.SetPixelColor(2, red);
    strip.SetPixelColor(3, black);
    strip.SetPixelColor(4, black);
    strip.SetPixelColor(5, black);    
  }

  if (cantidadRatonesVivos == 2){
    strip.SetPixelColor(0, red);
    strip.SetPixelColor(1, red);
    strip.SetPixelColor(2, red);    
    strip.SetPixelColor(3, green);
    strip.SetPixelColor(4, green);
    strip.SetPixelColor(5, green);
  }  

  

  strip.Show();
}

void procesarEstadoMouse(Raton *raton){
    if (hayMovimiento(raton->mx,raton->my)){
        if (!raton->estaVivo){
            cantidadRatonesVivos++;
        }
        raton->estaVivo= true;
    }   
    else{
        if (raton->estaVivo){
            cantidadRatonesVivos--;
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




