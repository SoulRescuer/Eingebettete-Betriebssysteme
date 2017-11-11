#include <stdint.h>
#include <LPC177x_8x.h>


#define LED1_BIT 0x100
#define LED2_BIT 0x1000


#define NPROCS 3							// Anzahl von Prozessen die verwaltet werden können


typedef uint32_t result_t;					// Ergebnistyp von Systemfunktionen
typedef uint32_t pid_t;						// Typ für Prozess-ID



/*************************** Prozesskontext *********************************
 * id   => eindeutige Prozess ID	
 * pZstd => Aktueller Zustand des Prozesses
 * stackPointer => Eigener SP für jeden Prozess
 * Func => Funktion die der Task ausführt
*****************************************************************************/
enum ProzessZustand{						
	LAUFEND,
	BEREIT,
	WARTEND,
	UNBENUTZT;
};

typedef struct{								
	pid_t id;
	uint32_t interval;
	enum ProzessZustand pZstd;
	uintptr_t stackPointer;
	void (*Func)(void);	
}pcb_entry;									
	
pcb_entry processTable[NPROCS];		// Anlegen der Prozesstabelle				
uint32_t stack[NPROCS][32];			// Jeder Prozess eigenen Stack je 32 Worte	


/************************** Function Prototypes ***************************
 * Wait wird noch nicht gebraucht. Erst inplementieren,
 * wenn Betriebsystem Funktionen für Ein/Ausgabe bekommt.                 
***************************************************************************/
pid_t create(void(*entry)(void), uint32_t zyklus);
result_t destroy(pid_t pid);
//result_t wait(pid_t pid);
void idle(void);					// Task 1
void led1(void);					// Task 2
void led2(void);					// Task 3
void HardFault_Handler(void);
void yield(pcb_entry*);
extern void ctxswitch(uintptr_t sp_old, uintptr_t sp_new);							


/*********************** Neuen Prozess erzeugen ****************************
 * Max. Anzahl der Prozesse = 3
 * Prozess bekommt als pid seinen platz in der Prozesstabelle zugewisen(i)
 * Erfolgreiches anlegen: pid wird zurückgegeben (hier als temp)
 * Im Fehlerfall wird -1 returned
****************************************************************************/
pid_t create(void(*entry)(void), uint32_t zyklus) {
	static int i = 1;
	int temp;
	if (i <= NPROCS) {
		processTable[i].pZstd = BEREIT;
		processTable[i].id = i;
		processTable[i].Func = entry;
		processTable[i].interval = zyklus;
		processTable[i].stackPointer = (uintptr_t) &(stack[i][31] - 9 * 4);
		stack[i][31] = entry;
		i = temp;
		i++;
		return temp;
	}
	else {
		return -1;
	}
}


/************************* Prozess löschen *******************************
 * Prozesszustand wird auf Unbenutzt gesetzt
 * Erfolg return 1
 * Fehler return -1
**************************************************************************/
result_t destroy(pid_t pid) {
	for (int i = 0; (processTable[i].pid) == pid; i++) {
		processTable[i].pZstd = UNBENUTZT;
		return 1;
	}
	return -1;
}


/*** Waiting ***/
/*
result_t wait(pid_t pid) {
int i = 0;
while((processTable[i].pZstd==LAUFEND) && i<=NPROCS){
return 1;
}
processTable[i].pZstd=WARTEND;
return 0;
}*/


/********************* Yield - Freiwillige CPU Abgabe **************************
 * Zustand Laufend: es kann nur in Bereit oder Wartend gewechselt werden.
 * Zustand Wartend: kann nur in Zustand Bereit gewechselt werden.
 * Zustand Wartend: Aufruf bei z.B. Betriebssystemfunktion zur Ein-/Ausgabe.
 * Bei Aufruf von Yield:
 - Auswahl des nächsten Prozesses
 – Update von Prozessstatus in der Prozesstabelle
 – Aufruf von switchContext mit richtigen Parametern
*******************************************************************************/
void yield(pcb_entry* calling_task) {
	for (int i = 0; i < NPROCS; i++) {
		if ((processTable[i].pZstd == BEREIT) && (processTable[i].interval >= processTable[i + 1].interval)) {
			calling_task->pZstd = BEREIT;
			processTable[i].pZstd = LAUFEND;
			ctxsw(calling_task.stackPointer, (uintptr_t)&processTable[i]->stackPointer);
		}
	}
}


/********************* HardFault_Handler **************************
 * Bei Fehler wie falscher Speicherzugriff
 * Kein Rücksprung aus Handler möglich
*******************************************************************/
void HardFault_Handler(void) {
	while (1);
}

uint32_t led1_wert = 0x100;					
uint32_t led2_wert = 0x1000;				

int main(void) {
	
	LPC_GPIO0->DIR = 0xFF00;

	create(&idle, 1);
	create(&led, 100000);
	create(&led2, 500000);


	processTable[0].pZstd = LAUFEND;
	firstProcess(idle.sp);

	while (1);


	/*	Systick erst bei prämtiv */
	/* 
	static uint32_t tick_counter = 0;
	volatile static uint32_t tick = 0;													
	SysTick->CTRL=1|(1<<2);
	SysTick->LOAD=800000;
	SysTick->VAL=0;
	tick = SysTick->VAL;															
	*/
	
	
}

void idle(void) {
	while (1) {
		yield(&processTable[0]);
	}
}


void led1(void) {
	while (1) {
		if (led1_wert < 0xF00) {
			led1_wert = (led1_wert << 1) + LED1_BIT;

			LPC_GPIO0->SET = led1_wert;
			yield(&processTable[1]);
		}
		else {
			while (led1_wert >= LED1_BIT) {
				led1_wert = (led1_wert - LED1_BIT) >> 1;

				LPC_GPIO0->CLR = ~(led1_wert + led2_wert);
			}
		}
	}
}


void led2(void) {
	while (1) {
		if (led2_wert < 0xF000) {
			led2_wert = (led2_wert << 1) + LED2_BIT;

			LPC_GPIO0->SET = led2_wert;
			yield(&processTable[2]);
		}
		else {
			while (led2_wert >= LED2_BIT) {
				led2_wert = (led2_wert - LED2_BIT) >> 1;

				LPC_GPIO0->CLR = ~(led1_wert + led2_wert);
			}
		}
	}
}
