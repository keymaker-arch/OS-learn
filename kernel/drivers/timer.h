#ifndef __LIB_TIMER_H
#define __LIB_TIMER_H

#define CONTROL_PORT 0x43
#define COUNTER0_PORT 0x40
#define COUNTER0_ID 0
#define WORK_MODE 2
#define READ_WRITE_LATCH 3

// TARGET_REQ control frequency of actual clock interrupt
#define TARGET_FREQUENCY 100
#define CLOCK_FREQUENCY 1193180
#define COUNTER0_VALUE CLOCK_FREQUENCY/TARGET_FREQUENCY

void timer_init();
#endif