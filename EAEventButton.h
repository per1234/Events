#ifndef EAEventButton_h
#define EAEventButton_h
#include "EAButton.h"

//объект аналогичен EAButton, за исключением того, что генерируются 
//заранее заданные события
// only stub, doesn't work!!!
//
class EAEventButton : public EAButton { //!!!???не работает пока
public:
	oid_t init( const port_t port );
	uint8_t addLevel( const int newLevel, const uint16_t eventType, const uint16_t eventDest = 0 );
	virtual void idle();  //!!!!Doesn't work
	virtual void getName( char* result );
//	virtual int getData();
private:
	int events[ABUTTONMAXLEVELS];
	int destinations[ABUTTONMAXLEVELS];
};


#endif