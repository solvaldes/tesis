#include "Arduino.h"
#include "PS2Mouse.h"

class MouseDevice
{
  public:
    MouseDevice(int numero, int mouseData, int mouseClock);
	void setup();    
    int procesarEstado();
	bool estaVivo();
	int procesarMovimiento();
	bool hayMovimiento(int mx, int my);
  private:
    int _clockPin;
	int _dataPin;
	PS2Mouse* _mouse;
	int _data[2];
	int _numero;
	bool _estaVivo= false;
	int _vecesSinMov=0;
};