#include "scheduler.h"
#include "main.h"

sTask SCH_tasks_G[SCH_MAX_TASKS];
uint8_t n_task = 0;
uint32_t next_id = 1;
volatile uint32_t time_ms = 0;
volatile uint32_t lost = 0;      // so tick den luc task dau dang cho chay (Delay = 0)

// chen task vao dung vi tri (t.Delay dang la so tick tinh tu bay gio)
static void insert(sTask t) {
	uint8_t i = 0;
	while (i < n_task && t.Delay >= SCH_tasks_G[i].Delay) {
		t.Delay -= SCH_tasks_G[i].Delay;
		i++;
	}
	for (uint8_t j = n_task; j > i; j--) {
		SCH_tasks_G[j] = SCH_tasks_G[j - 1];
	}
	SCH_tasks_G[i] = t;
	if (i < n_task) {
		SCH_tasks_G[i + 1].Delay -= t.Delay;
	}
	n_task++;
}

// xoa task o vi tri i
static void remove_at(uint8_t i) {
	if (i + 1 < n_task) {
		SCH_tasks_G[i + 1].Delay += SCH_tasks_G[i].Delay;
	}
	for (uint8_t j = i; j < n_task - 1; j++) {
		SCH_tasks_G[j] = SCH_tasks_G[j + 1];
	}
	n_task--;
}

void SCH_Init(void) {
	n_task = 0;
	next_id = 1;
	time_ms = 0;
	lost = 0;
}

// goi trong ngat TIM2, chi giam task dau tien -> O(1)
void SCH_Update(void) {
	time_ms += TICK;
	if (n_task > 0) {
		if (SCH_tasks_G[0].Delay > 0) SCH_tasks_G[0].Delay--;
		else lost++;   // task dau dang cho Dispatch chay -> nho lai tick nay
	}
}

uint32_t SCH_Add_Task(void (*pFunction)(), uint32_t DELAY, uint32_t PERIOD) {
	if (n_task >= SCH_MAX_TASKS) return NO_TASK_ID;
	sTask t;
	t.pTask = pFunction;
	t.Delay = DELAY / TICK;
	t.Period = PERIOD / TICK;
	if (PERIOD > 0 && t.Period == 0) t.Period = 1;
	t.TaskID = next_id++;

	__disable_irq();
	insert(t);
	__enable_irq();
	return t.TaskID;
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
	uint8_t ok = 0;
	__disable_irq();
	for (uint8_t i = 0; i < n_task; i++) {
		if (SCH_tasks_G[i].TaskID == taskID) {
			remove_at(i);
			ok = 1;
			break;
		}
	}
	__enable_irq();
	return ok;
}

void SCH_Dispatch_Tasks(void) {
	while (1) {
		__disable_irq();
		if (n_task == 0) {
			lost = 0;
			__enable_irq();
			break;
		}
		// bu lai cac tick bi bo qua
		if (lost > 0 && SCH_tasks_G[0].Delay > 0) {
			uint32_t d = (lost < SCH_tasks_G[0].Delay) ? lost : SCH_tasks_G[0].Delay;
			SCH_tasks_G[0].Delay -= d;
			lost -= d;
		}
		if (SCH_tasks_G[0].Delay != 0) {
			__enable_irq();
			break;
		}
		sTask t = SCH_tasks_G[0];
		remove_at(0);
		if (t.Period > 0) {
			t.Delay = t.Period;
			insert(t);
		}
		__enable_irq();
		(*t.pTask)();
	}
}

uint32_t get_time(void) {
	return time_ms;
}
