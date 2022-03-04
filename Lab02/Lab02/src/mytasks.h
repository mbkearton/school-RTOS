/******************************************************************************
Author:			Michael Kearton
File Name:		mytasks.h
Date Created:	01/18/2022
Modified:		01/25/2022 -- renamed myButtonTask, added taskSystemControl
******************************************************************************/

#ifndef MYTASKS_H_
#define MYTASKS_H_

#define FOREVER		1

// polls buttons, managing the creation, deletion, and suspension of heartbeat
// tasks responsible for controlling expansion board LEDs
void taskSystemControl	(void *);	// no args
void taskHeartbeat		(void *);	// toggles LED referenced by arg every 0.5s

#endif /* MYTASKS_H_ */
