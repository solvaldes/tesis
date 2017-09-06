#include <PS2Mouse.h>
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

void setup()
{
  setupMice();
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


void imprimirMovimiento(int mouse, int mx, int my){
   Serial.print("Mouse "); // Status Byte
   Serial.print(mouse);
   Serial.print(" - ");
   Serial.print("x: ");
   Serial.print(mx);
   Serial.print(" | y: ");
   Serial.print(my);
   Serial.println();
}




