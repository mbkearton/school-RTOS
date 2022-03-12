/*
 * commands.h
 *
 * Created: 3/11/2022 4:07:01 AM
 *  Author: mbkea
 */ 


#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <FreeRTOS.h>
#include <task.h>
#include "mytasks.h"
#include <FreeRTOS_CLI.h>

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

struct DataBlinkLED data_LEDs[3];

TaskHandle_t thLEDs[3];

void register_commands(void);

#endif /* COMMANDS_H_ */