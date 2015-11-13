#include <stdint.h>

typedef void (* handler_t)(void);

typedef struct {
    handler_t handler;
    uint16_t period;
    uint16_t counter;
} task_t;

void task_setup(void);
void task_add(task_t);
void task_start(void);
void task_stop(void);
void task_manager(void);
