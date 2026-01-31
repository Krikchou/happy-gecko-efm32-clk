/*
 * clock_control.h
 *
 *  Created on: 22.06.2025 ã.
 *      Author: krama
 */

#ifndef CLOCK_CONTROL_H
#define CLOCK_CONTROL_H

typedef enum Day {
	MON,
	TUE,
	WED,
	THU,
	FRI,
	SAT,
	SUN,
	WEEKDAY,
	WEEKEND
} Day;

typedef enum TimeType {
	HOUR,
	MINUTE,
	SECOND,
	DAY,
	MONTH,
	YEAR
} TimeType;

typedef enum AlarmType {
	SIMPLE,
	REPEATABLE,
} AlarmType;

typedef enum OperationType {
	INCR,
	DECR
} OperationType;


typedef struct Time {
	uint32_t tm_sec;
	uint32_t tm_min;
	uint32_t tm_hour;
	uint32_t tm_mday;
	uint32_t tm_mon;
	int32_t tm_year;
	int32_t tm_wday;
} Time;

typedef struct Alarm {
	AlarmType type;
	uint32_t time_of;
} Alarm;

void RTC_Setup(void);
Time GetCurrTime(uint32_t sec);
// with looping
int32_t adjustOffset(uint32_t time, TimeType timeType, OperationType operation);

#endif /* SRC_CLOCK_CONTROL_H_ */
