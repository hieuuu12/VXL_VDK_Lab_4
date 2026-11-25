#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>

#define SCH_MAX_TASKS 20
#define TICK 10          // chu ky ngat TIM2 (ms)
#define NO_TASK_ID 0

typedef struct {
	void (*pTask)(void);
	uint32_t Delay;      // so tick con lai, tinh tu task dung truoc no
	uint32_t Period;     // tick, = 0 la one-shot
	uint32_t TaskID;
} sTask;

void SCH_Init(void);
void SCH_Update(void);
void SCH_Dispatch_Tasks(void);
uint32_t SCH_Add_Task(void (*pFunction)(), uint32_t DELAY, uint32_t PERIOD);
uint8_t SCH_Delete_Task(uint32_t taskID);
uint32_t get_time(void);

#endif /* INC_SCHEDULER_H_ */
