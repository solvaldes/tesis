#include "Arduino.h"
#include "MouseDevice.h"
#include <PS2Mouse.h>

MouseDevice::MouseDevice(int numero, int mouseData, int mouseClock)
{
	_numero = numero;
	_clockPin= mouseClock;
	_dataPin= mouseData;	
}

void MouseDevice::setup(){
	_mouse= new PS2Mouse(_clockPin, _dataPin, STREAM);
	_mouse->initialize();	
	_mouse->set_resolution(02);
	
}

int MouseDevice::procesarEstado()
{	_mouse->report(_data);
	return procesarMovimiento();
}

bool MouseDevice::estaVivo(){
	return _estaVivo;
}

int MouseDevice::procesarMovimiento(){
	int mx= _data[1];
	int my= _data[2];
	int result= 0;
    if (hayMovimiento(mx,my)){
        if (!_estaVivo){
            result= 1;
        }
        _estaVivo= true;
    }   
    else{
        if (_estaVivo){
            result= -1;
        }
        _estaVivo= false;           
    } 
	return result;
}

bool MouseDevice::hayMovimiento(int mx, int my){
  return my != 0 || mx != 0;
}


