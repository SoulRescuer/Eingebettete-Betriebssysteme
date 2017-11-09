#include <stdint.h>
#include <LPC177x_8x.h>


#define LED1_BIT 0x100
#define LED2_BIT 0x1000
#define NPROCS 2							// Anzahl von Prozessen die verwaltet werden können


typedef uint32_t result_t;					// Ergebnistyp von Systemfunktionen
typedef uint32_t pid_t;						// Typ für Prozess-ID

void HardFault_Handler(void) {
	while (1);
}

enum ProzessZustand{						
	UNBENUTZT,
	LAUFEND,
	BEREIT,
	WARTEND
};

typedef struct{								// Prozesskontext 
	uint8_t pid;							// eindeutige Prozess ID
	enum ProzessZustand pZstd = WARTEND;
	uint32_t stackPointer;					// noch keine Ahnung wie man da ran kommt =/
	uint32_t programCounter;				// ^
	//--uint32_t verbrauchteRechenzeit;
}pcb_entry;									

pcb_entry processTable[NPROCS];				// Anlegen der Prozesstabelle
uint32_t stack[NPROCS][32];					// stacks für alle Prozesse á 32 Worte


void led1(void);
void led2(void);
void swctx() {};																	// soll die vorher gesicherten register wiederholen MUSS in assembler realisiert werden!

// API Bereich Anfang

pid_t create(void(*entry)(void)) {
	for (int i = 0; !processTable[i] && processTable[i]<=processTable[NPROCS] ; i++){ // wenn der Slot im ProcessTable noch frei ist und das ende des tables noch nicht erreicht
		processTable[i].pZstd = BEREIT;
		processTable[i].pid = i;													// setzten der ID auf den PLatz im Table sollte eindeutig genug sein
		processTable[i].stackPointer ;												// noch nicht sicher wie der ermittelt wird vermutloch via Assembler
		
		return processTable[i].pid;													// falls erfolgreich -> Rückgabe der ID
	}
	return 0;																		// falls erfolglos -> Rückgabe 0
};

result_t destroy(pid_t pid) {


};
result_t wait(pid_t pid) {


};

// API Bereich ENDE
uint32_t led1_wert = 0x100;					
uint32_t led2_wert = 0x1000;				


//entscheiden welcher Prozess als nächster dran kommt
/*
void yield() {
	int old_index = index;
	int new_index = ++index;

	if (new_index >= NPROCS) {
		new_index = 0;
	}

	index = new_index;
	
	swctx(processTable[old_index], processTable[new_index]);
}
*/    //können wir uns vermutlich sparen, da eh  create, destroy etc. anstatt "yield" verwendet werden soll



int main(void) {
	
	static TaskType Tasks[] = {
	{ 5, 0, led1 },
	{ 10, 0, led2 }
	};
	
	
	LPC_GPIO0->DIR = 0xFF00;					
	/*static uint32_t tick_counter = 0;
	volatile static uint32_t tick = 0;													// System“tick“: aktuelle Zeit
	static TaskType *TaskPtr = Tasks;													// Zeiger auf Prozesstabelle
	static uint8_t TaskIdx = 0;															// Aktueller Task („Index“)
	const uint8_t NumTasks = sizeof(Tasks) / sizeof(*Tasks);							// Anzahl Tasks
	SysTick->CTRL=1|(1<<2);
	SysTick->LOAD=800000;
	SysTick->VAL=0;

	tick = SysTick->VAL;																// aktuelle Zeit ermitteln
	*/
	
	while (1) {																			// Bis der Strom ausfällt...
		
		for (TaskIdx = 0; TaskIdx < NumTasks-1; TaskIdx++) {							// alle Tasks anschauen
			if (TaskPtr[TaskIdx].Interval == 0) {
			(*TaskPtr[TaskIdx].Func)();													// Idle-Task ausführen
			}
			else if ((tick-TaskPtr[TaskIdx].LastTick) >= TaskPtr[TaskIdx].Interval) {
			(*TaskPtr[TaskIdx].Func)();													// Periodische Tasks ausführen
			TaskPtr[TaskIdx].LastTick = tick;											// Letzte Ausführungszeit merken
			}
	
		tick = SysTick->VAL;
		}
	}
}

/*die "Prozesse" bzw. funktionen sollen zunächst freiwillig ihre Kontrolle abgeben*/

void led1(void) {

	if (led1_wert < 0xF00) {
		led1_wert = (led1_wert << 1) + LED1_BIT;

		LPC_GPIO0->SET = led1_wert;
		yield();
	}
	else {
		while (led1_wert >= LED1_BIT) {
			led1_wert = (led1_wert - LED1_BIT) >> 1;
			
			LPC_GPIO0->CLR = ~(led1_wert + led2_wert);
		}
	}

	return;
}

void led2(void) {

	if (led2_wert < 0xF000) {
		led2_wert = (led2_wert << 1) + LED2_BIT;
	
		LPC_GPIO0->SET = led2_wert;	
		yield();
	}
	else {
		while (led2_wert >= LED2_BIT) {
			led2_wert = (led2_wert - LED2_BIT) >> 1;
			
			LPC_GPIO0->CLR = ~(led1_wert + led2_wert);
		}
	}

	return;
}

