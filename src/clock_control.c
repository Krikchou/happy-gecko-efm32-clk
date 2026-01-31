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
	cnt++;
}

int32_t adjustOffset(uint32_t time, TimeType timeType, OperationType operation) {
	volatile int32_t offsetBy = 0;

	Time t = GetCurrTime(time);
	uint32_t hr = t.tm_hour;

	bool isLeap = (t.tm_year % 100 != 0 && t.tm_year % 4 == 0)
			|| t.tm_year % 400;

	if (timeType == SECOND) {
		if (operation == INCR) {
			if (t.tm_sec == 59) {
				offsetBy -= 59;
			} else {
				offsetBy += 1;
			}
		} else {
			if ((t.tm_sec == 0)) {
				offsetBy += 59;
			} else {
				offsetBy -= 1;
			}
		}
	}
	if (timeType == MINUTE) {
		if (t.tm_min == 59 && operation == INCR) {
			offsetBy -= 59 * 60;
		}

		if (t.tm_min == 0 && operation == DECR) {
			offsetBy += 59 * 60;
		}

		if (!(t.tm_min == 59 && operation == INCR)
				&& !(t.tm_min == 0 && operation == DECR)) {
			if (operation == INCR) {
				offsetBy += 60;
			} else {
				offsetBy -= 60;
			}
		}
	}

	if (timeType == HOUR) {
		if (operation == INCR) {
			if (hr == 23) {
				offsetBy = offsetBy - 23 * 3600;
			} else {
				offsetBy += 3600;
			}
		} else {
			if (t.tm_hour == 0) {
				offsetBy += 23 * 3600;
			} else {
				offsetBy -= 3600;
			}
		}
	}
	if (timeType == DAY) {
		if (operation == INCR) {
			offsetBy += 86400;
		} else {
			offsetBy -= 86400;
		}

		if (t.tm_mon == 2 && isLeap && t.tm_mday == 29 && operation == INCR) {
			offsetBy -= 29 * 86400;
		}

		if (t.tm_mon == 2 && !isLeap && t.tm_mday == 28 && operation == INCR) {
			offsetBy -= 28 * 86400;
		}

		if (t.tm_mon == 2 && isLeap && t.tm_mday == 1 && operation == DECR) {
			offsetBy -= 29 * 86400;
		}

		if (t.tm_mon == 2 && !isLeap && t.tm_mday == 1 && operation == DECR) {
			offsetBy -= 28 * 86400;
		}

		if ((t.tm_mon == 1 || t.tm_mon == 3 || t.tm_mon == 5 || t.tm_mon == 7
				|| t.tm_mon == 8 || t.tm_mon == 10 || t.tm_mon == 12)
				&& t.tm_mday == 31 && operation == INCR) {
			offsetBy -= 31 * 86400;
		}

		if ((t.tm_mon == 1 || t.tm_mon == 3 || t.tm_mon == 5 || t.tm_mon == 7
				|| t.tm_mon == 8 || t.tm_mon == 10 || t.tm_mon == 12)
				&& t.tm_mday == 1 && operation == DECR) {
			offsetBy += 31 * 86400;
		}

		if (!(t.tm_mon == 1 || t.tm_mon == 3 || t.tm_mon == 5 || t.tm_mon == 7
				|| t.tm_mon == 8 || t.tm_mon == 10 || t.tm_mon == 12)
				&& t.tm_mday == 30 && operation == INCR) {
			offsetBy -= 30 * 86400;
		}

		if (!(t.tm_mon == 1 || t.tm_mon == 3 || t.tm_mon == 5 || t.tm_mon == 7
				|| t.tm_mon == 8 || t.tm_mon == 10 || t.tm_mon == 12)
				&& t.tm_mday == 1 && operation == DECR) {
			offsetBy += 30 * 86400;
		}
	}

	if (timeType == MONTH) {
		if (operation == INCR) {
			if (t.tm_mon == 12) {
				if (isLeap) {
					offsetBy -= 86400 * 366;
				} else {
					offsetBy -= 86400 * 365;
				}
			} else {
				if (t.tm_mon == 1 || t.tm_mon == 3 || t.tm_mon == 5
						|| t.tm_mon == 7 || t.tm_mon == 8 || t.tm_mon == 10) {
					offsetBy += 86400 * 31;
				} else {
					if (t.tm_mon == 2) {
						if (isLeap) {
							offsetBy += 86400 * 29;
						} else {
							offsetBy += 86400 * 28;
						}
					} else {
						offsetBy += 86400 * 30;
					}
				}
			}
		} else {
			if (t.tm_mon == 1) {
				if (isLeap) {
					offsetBy += 86400 * 366;
				} else {
					offsetBy += 86400 * 365;
				}
			} else {
				if (t.tm_mon == 12 || t.tm_mon == 3 || t.tm_mon == 5
						|| t.tm_mon == 7 || t.tm_mon == 8 || t.tm_mon == 10) {
					offsetBy -= 86400 * 31;
				} else {
					if (t.tm_mon == 2) {
						if (isLeap) {
							offsetBy -= 86400 * 29;
						} else {
							offsetBy -= 86400 * 28;
						}
					} else {
						offsetBy -= 86400 * 30;
					}
				}
			}
		}
	}

	if (timeType == YEAR) {
		if (operation == INCR) {
			if (isLeap) {
				offsetBy += 365*86400;
			} else {
				offsetBy += 364*86400;
			}
		} else {
			if (t.tm_year == 1970) {
				return 0;
			}

			if (isLeap) {
				offsetBy -= 365*86400;
			} else {
				offsetBy -= 364*86400;
			}
		}
	}

	if (offsetBy + time >= 0) {
		return offsetBy;
	} else {
		return 0;
	}

}

Time GetCurrTime(uint32_t sec) {
	uint32_t z = (sec / 86400) + 719468;

	uint32_t era = z / 146097;
	uint32_t doe = z - era * 146097;
	uint32_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
	int32_t year = (yoe) + era * 400;
	uint32_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);          // [0, 365]
	uint32_t mp = (5 * doy + 2) / 153;                                // [0, 11]
	uint32_t d = doy - (153 * mp + 2) / 5 + 1;                        // [1, 31]
	uint32_t m = mp + (mp < 10 ? 3 : -9);
	int32_t wdays = (z >= -4 ? (z + 4) % 7 : (z + 5) % 7 + 6);

	struct Time t = { sec % 60, (sec / 60) % 60, (sec / 3600) % 24, d, m, year
			+ (m <= 2), wdays };
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
