#include <stdint.h>
#include <LPC177x_8x.h>


#define LED1_BIT 0x100
#define LED2_BIT 0x1000

typedef struct {
uint16_t Interval;
uint32_t LastTick;
void (*Func)(void);
} TaskType;


void led1(void);
void led2(void);
uint32_t led1_wert = 0x100;					//global definiert, da sonst nicht sichtbar fuer die Funktionen
uint32_t led2_wert = 0x1000;				// ^
//uint32_t maske;										// ^ unötig in echter Umgebung; debug Fragment

void HardFault_Handler(void) {
  while(1);	
}

int main(void) {
	
	static TaskType Tasks[] = {
	{ 5, 0, led1 },
	{ 10, 0, led2 }
	};
	
	
	LPC_GPIO0->DIR = 0xFF00;					
	static uint32_t tick_counter = 0;
	volatile static uint32_t tick = 0; // System“tick“: aktuelle Zeit
	static TaskType *TaskPtr = Tasks; // Zeiger auf Prozesstabelle
	static uint8_t TaskIdx = 0; // Aktueller Task („Index“)
	const uint8_t NumTasks = sizeof(Tasks) / sizeof(*Tasks); // Anzahl Tasks
	SysTick->CTRL=1|(1<<2);
	SysTick->LOAD=800000;
	SysTick->VAL=0;

	tick = SysTick->VAL; // aktuelle Zeit ermitteln

	
	while (1) { // Bis der Strom ausfällt...
		
		for (TaskIdx = 0; TaskIdx < NumTasks; TaskIdx++) { // alle Tasks anschauen
			if (TaskPtr[TaskIdx].Interval == 0) {
			(*TaskPtr[TaskIdx].Func)(); // Idle-Task ausführen
			}
			else if ((tick-TaskPtr[TaskIdx].LastTick) >= TaskPtr[TaskIdx].Interval) {
			(*TaskPtr[TaskIdx].Func)(); // Periodische Tasks ausführen
			TaskPtr[TaskIdx].LastTick = tick; // Letzte Ausführungszeit merken
			}
	
		tick = SysTick->VAL;
		}
	}
}

	void led1(void) {


	if (led1_wert < 0xF00) {
		led1_wert = (led1_wert << 1) + LED1_BIT;
		/*
		maske = led1_wert + led2_wert;				//ist debug-Ersatz fuer 
		printf("%08x\n", maske);							//LPC_GPIO0->SET = led1_wert + led2_wert;
		*/
		LPC_GPIO0->SET = led1_wert;// + led2_wert;
	}
	else {
		while (led1_wert >= LED1_BIT) {
			led1_wert = (led1_wert - LED1_BIT) >> 1;
			/*
			maske = led1_wert + led2_wert;
			printf("%08x\n", maske);
			*/
			LPC_GPIO0->CLR = ~(led1_wert + led2_wert);
		}
	}

	return;
}

void led2(void) {

	if (led2_wert < 0xF000) {
		led2_wert = (led2_wert << 1) + LED2_BIT;
	/*
	maske = led1_wert + led2_wert;
	printf("%08x\n", maske);
	*/
		LPC_GPIO0->SET = led2_wert; //led1_wert + 
	}
	else {
		while (led2_wert >= LED2_BIT) {
			led2_wert = (led2_wert - LED2_BIT) >> 1;
			/*
			maske = led1_wert + led2_wert;
			printf("%08x\n", maske);
			*/
			LPC_GPIO0->CLR = ~(led1_wert + led2_wert);
		}
	}

	return;
}




// Nice Comments here
/*while (1) {

		if (!(tick_counter % 2)) {
			led1();
			tick_counter++;								// in die main gezogen, da tick_counter nicht 
		}																// sichtbar in led1() und led2()
		else {
			led2();
			tick_counter++;
		}
*/
