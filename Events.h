/*
  
	Arduino Event Library
	Started 2010-01-07 by MH
	http://github.com/master-hamster/Events
	Main library
	General Objects
*/	

/*	
	Задачи:
	4. Реализовать аналоговое устройство ввода с выдачей событий по уровням. 
	5. реализовать в таймере паузу и обработку переполнения millis()
	6. Сделать возможность вешать на устройство ввода определенное событие взамен штатного
	12. Сделать протоколирование событий включения
	13. Сделать отправление событий при переключении исполнительных устройств
	14. Сделать запрос-ответ состояния различных устройств
 */

#ifndef EVENT_H
#define EVENT_H

#if ARDUINO < 100
#include "WProgram.h"
#else
#include "Arduino.h"
#endif

#include "EDebug.h"


//EVent NoEvent
// 0 - 49 - system events, 50-99 - reserved for future, 101+ - application user-defined
#define evNone						0
//Object manipulation events
#define evEnable                    1   //enable event handling and generating
#define evDisable                   2   //disable event handling and generating
#define evTurnOn                    3   //turn on  EOutputDevice and successors
#define evTurnOff                   4   //turn off EOutputDevice and successors
#define evTellMe                    5   //Ask EObject to send answer Event

//ETimer create evTimerExpired when it has expired 
#define evTimerExpired               6
//ETimer stop working whet it got evTimerStop
#define evTimerStop                  7
//ETimer resume counting whet it has got evTimerStart
#define evTimerStart                 8

//Abstract Input Events
#define evInputUp                    9
#define evInputDown                 10
#define evInputToggle               11
#define evInputHold                 12
// button's events - created by EButton
#define evKeyPressed                13
#define evKeyDoublePressed          14
#define evKeyHold                   15

//событие от устройства аналогового ввода
#define evAIData                    16
//событие изменения уровня аналогового ввода/кнопки
#define evLevelChanged              17

//обнаружено движение, в данных - условный ID датчика движения
#define evMotionDetected            18
#define evFlicker                   19
//Key released - reserve event name for future
#define evKeyReleased   20

//============= SYSTEM PARAMETERS ===============
//максимальное количество событий в стеке событий
#define EVENTSTACKSIZE        8
//Maximum objects that can be registered, change it according to free memory
#define MAXAPPOBJECTS        20

//============= OBJECT SETTINGS =================
//Debounce delay in ms, recommended 10..50
#define DEBOUNCEDELAY       25
//Duoble press time in ms, recommended 500-1000
#define DOUBLEPRESSDELAY   600
//Key hold delay in ms, recommended 800-1500
#define KEYHOLDDELAY      1000
//#define HOLDREPEATDELAY 3000 // may be in future?

//длительность писка бипера по умолчанию
#define EBEEPER_DEFAULT_BEEP_TIME   25

//максимальное количество событий в мультиуровневой аналоговой кнопке
#define ABUTTONMAXLEVELS       5

// Candle parameters
#define CANDLEMINIMALLIGHTLEVEL		5
#define CANDLEMAXIMALLIGHTLEVEL		255
#define CANDLELIGHTDELTA			2 

//=============== only substitutions, no need to change  =======
#define PORT_REVERSE_MODE true
#define PORT_NO_REVERSE_MODE false
#define BROADCAST_OID 0
#define EAPPLICATION_SOURCE_OID 65535
#define TIMER_AUTOSTART true
#define TIMER_NO_AUTOSTART false
#define TIMER_AUTORESTART true
#define TIMER_NO_AUTORESTART false

//============== TYPES DEFINITION =============
typedef uint16_t oid_t;
typedef uint16_t event_t;
typedef uint16_t port_t;

//====== EInputDevice modes ==================
enum InputMode {
	imUpOnly,    //выдается событие только при повышении уровня до 1 evInputUp
	imDownOnly,	 //выдается событие только при падении уровня до 0 evInputDown
	imUpDown,    //выдается событие и при падении и при повышении
	imToggle     //аналогично, но событие evToggle
};
//====== ECandle modes =================
enum CandleState {
	csOff,        //полностью выключено
	csFadeIn,     //включается
	csFadeOut,    //выключается
	csOn,         //полностью включено
	csFlickering  //мелькает
};


//Main class definition 
class Event {
public:
	event_t eventType;    // Event Type :-)
	oid_t sourceID;       // Source Object ID
	oid_t destinationID;  // Destination Object ID or Broadcast ID
	int16_t eventData;    // Optional data
	int16_t sourceGroupID;// optional data: source sensor group/node
	const void print();   // Print event data to Serial
	Event& operator =( const Event& from );  // copy one event to another
};


//ниже идет определение стека событий - места, куда все объекты кидают событи
//и откуда их берет обработчик событий
//при передаче все данные дублируются, работы с памятью не производится, размер стека ограничен
class EventStack {
public:
	int push( Event& newEvent );
	int pushEvent( event_t evntType, //Push event to stack, first - Event Type
		oid_t sourceID = 0,          //Source Object ID
		oid_t destinationID = 0,     //Destination ID, 0 if no desination
		int16_t eventData = 0 );     //16-bit optional data
	int pop( Event& newEvent );      //вытащить событие из стека, 0 - нет, 1 - вытащено
	void clear(){size = 0;};         //Clear stack content
	void print();                    //print all events from stack to Serial
private:
	int size; //количество объектов в стеке
	Event items[EVENTSTACKSIZE];  //буффер с событиями
};

extern EventStack eventStack;

//простой таймер без генерации событий и т.п., все времена в миллисекундах
//если установлен autorestart, то после истечения таймера первая положительная 
// выдача expired()==TRUE приведет к перезапуску таймера, если нет - по истечении
// таймера все время будет возвращаться TRUE
class Timer {
public:
	Timer();
	Timer( const unsigned long interval );
	void init( const unsigned long interval, 
				const bool autorestart = false, 
				const bool autostart = true );
	void setInterval( const unsigned long interval );
	void setStartTime( const unsigned long newTime ){ this->startTime = newTime; };
	unsigned long int elapsedTime() const;
	bool expired();
	void start();
	unsigned long getStartTime() const {return this->startTime;}; //когда таймер был запущен
	unsigned long getInterval() const {return this->interval;};	//какой интервал
private:
	bool autorestart;          //флажок авторестарта
	bool stoped;               // if stoped then expired() always return false
	unsigned long  interval;   //интервал таймера
	unsigned long  startTime;  //дата начала интервала
};

//==================================  Objects =====================
//другой основной класс - объект, работающий с событиями
class  EObject {
public:
//	static uint8_t nextID; //счетчик идентификаторов для класса
	EObject();
	oid_t init(); // get own ID, return it
	virtual int handleEvent( Event& tmpEvent );
	virtual void idle(){};
	bool eventForMe( const Event& tmpEvent ) const;
	virtual void getName( char* result ) const;
	oid_t getID() const { return this->ID; };
private:
	oid_t ID;          //This Object ID
protected:
	Event event;       //Event buffer
	bool isEnabled;    //Can handle and rise events
};



#endif

