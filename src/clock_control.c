/*
 * clock_control.c
 *
 *  Created on: 22.06.2025 ã.
 *      Author: krama
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "em_rtc.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "clock_control.h"

#define DELAY_SECONDS 1.0
#define LFXOFREQ      32768
#define COMPARE_TOP   (DELAY_SECONDS * LFXOFREQ - 1)


static void ClockIncrement(uint32_t cnt) {
	cnt ++;
}

int32_t adjustOffset(uint32_t time, TimeType timeType, OperationType operation) {
	int32_t offsetBy = 0;

	switch (timeType) {
	case SECOND :
		offsetBy += 1;
		break;
	case MINUTE :
		offsetBy += 60;
		break;
	case HOUR :
		offsetBy += 3600;
		break;
	case DAY :
		offsetBy += 86400;
		break;
	case MONTH :
		offsetBy += 2592000;
		break;
	case YEAR :
		offsetBy += 946080000;
		break;
	}

	switch (operation) {
	case INCR :
		return + offsetBy;
		break;
	case DECR :
		if (time - offsetBy > 0) {
		   return - offsetBy;
		}

		return 0;
		break;
	}


}


Time GetCurrTime(uint32_t sec) {
	uint32_t z = (sec / 86400) + 719468;

	uint32_t era = z / 146097;
	uint32_t doe = z - era * 146097;
	uint32_t yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;
	int32_t year = (yoe) + era * 400;
	uint32_t doy = doe - (365*yoe + yoe/4 - yoe/100);                // [0, 365]
	uint32_t mp = (5*doy + 2)/153;                                   // [0, 11]
	uint32_t d = doy - (153*mp+2)/5 + 1;                             // [1, 31]
	uint32_t m = mp + (mp < 10 ? 3 : -9);
	int32_t wdays = (z >= -4 ? (z+4) % 7 : (z+5) % 7 + 6);

	struct Time t = {
		sec % 60,
		(sec / 60) % 60,
		(sec / 3600) % 24,
		d,
		m,
		year + (m <= 2),
		wdays
	};
	return t;
}


void RTC_Setup(void) {
	  // Enable the oscillator for the RTC
	  CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

	  // Turn on the clock for Low Energy clocks
	  CMU_ClockEnable(cmuClock_HFLE, true);
	  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	  // Turn on the RTC clock
	  CMU_ClockEnable(cmuClock_RTC, true);

	  // Set RTC compare value for RTC 0
	  RTC_CompareSet(0, COMPARE_TOP);

	  // Allow channel 0 to cause an interrupt
	  RTC_IntEnable(RTC_IEN_COMP0);
	  NVIC_ClearPendingIRQ(RTC_IRQn);
	  NVIC_EnableIRQ(RTC_IRQn);

	  // Configure the RTC settings
	  RTC_Init_TypeDef rtc = RTC_INIT_DEFAULT;

	  // Initialise RTC with pre-defined settings
	  RTC_Init(&rtc);
}
