/***************************************************************************//**
 * @file
 * @brief Relative humidity and temperature sensor demo for SLSTK3400A_EFM32HG
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <graphics_c.h>
#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_rtc.h"
#include "em_adc.h"
#include "i2cspm.h"
#include "capsense.h"
#include "si7013.h"
#include "sl_sleeptimer.h"
#include "bspconfig.h"
#include "clock_control.h"
#include "graphics.h"
#include "dmd.h"
#include "glib.h"


/***************************************************************************//**
 * Local defines
 ******************************************************************************/

/** Time (in ms) between periodic updates of the measurements. */
#define MEASUREMENT_INTERVAL_MS      2000
#define INTSEC 1000
/** Voltage defined to indicate dead battery. */
#define LOW_BATTERY_THRESHOLD   2800
#define STANDBY_MODE 0
#define CALIBRATE_MODE 1

typedef enum Page {
	CLOCK, //0
	WEATHER, //1
	TIME_ADJUST, //2
	WEATHER_ADJUST, //3
	MENU, //4
	EXIT //-1
} Page;

/***************************************************************************//**
 * Local variables
 ******************************************************************************/
/* Variables used by the display callback. */
static void (*mem_lcd_callback_func)(void*) = 0;
static void *mem_lcd_callback_arg = 0;

/** Flag used to indicate ADC is finished */
static volatile bool adcConversionComplete = false;

/** This flag indicates that a new measurement shall be done. */
static volatile bool measurement_flag = true;


/** Timer used for periodic update of the measurements. */
sl_sleeptimer_timer_handle_t measurement_timer;

sl_sleeptimer_timer_handle_t sense_timer;

/** Timer used for periodic update of the measurements. */
sl_sleeptimer_timer_handle_t clk_timer;

/** Timer used for periodic maintenance of the display **/
sl_sleeptimer_timer_handle_t display_timer;

// Baseline epoch
static int blink_freq = 2;
static volatile uint32_t cnt;
static volatile int32_t offsetInSeconds;
static volatile int32_t offsetInSecondsPrev;
static volatile TimeType selectedType;
static volatile int32_t temp;
static volatile int32_t rh;
static volatile int32_t page_state = 0;
static volatile int32_t prev_page_state = 0;
static volatile int32_t menu_selected = 0;
static volatile int btn0_state = 0;
static volatile int btn1_state = 0;
// 0 - hour
// 1 - minute
// 2 - second
// 3 - day
// 4 - month
// 5 - year
// 6 - exit%apply
// 7 - cancel
static volatile int32_t date_adjust_state = 0;
static volatile uint32_t stopped_at_time;

/***************************************************************************//**
 * Local prototypes
 ******************************************************************************/
static void gpioSetup(void);
static uint32_t checkBattery(void);
static void adcInit(void);
static void measure_humidity_and_temperature(I2C_TypeDef *i2c, uint32_t *rhData, int32_t *tData, uint32_t *vBat);
static void measurement_callback(sl_sleeptimer_timer_handle_t *handle, void *data);
static void time_callback(sl_sleeptimer_timer_handle_t *handle, void *data);
//static void touch_callback(sl_sleeptimer_timer_handle_t *handle, void *data);
void clear_display(void);
void GRAPHICS_Draw(int32_t temp, uint32_t rh, uint32_t time, bool lowBat);
void GRAPHICS_Draw_Weather_Station(int32_t tempData, uint32_t rhData, bool lowBat, int32_t temp_min_mC, int32_t temp_max_mC, uint32_t humidity_min, uint32_t humidity_max);
void resetTempHumidity(void);
int32_t temp_min_mC = INT32_MAX; // min will always be the biggest 32bit value, so any real measured temp will be lower and replace it
int32_t temp_max_mC = INT32_MIN; // max will always be the smallest 32bit value, so any real measured temp will be bigger and replace it
int32_t humidity_min = INT32_MAX;
int32_t	humidity_max = 0;
GLIB_Context_t   glibContext;

static volatile bool is_w_station = false; // Promenliva za rejim na rabota meterologichna stancia
static volatile bool redraw = false;

void clear_display(void)
{
  EMSTATUS status;

  // Clear the entire framebuffer in memory.
  // This function uses the background color set in the glibContext.
  status = GLIB_clear(&glibContext);
  if (status != GLIB_OK) {
    // Handle error if needed
    return;
  }

  // Update the physical display with the content of the cleared framebuffer.
  DMD_updateDisplay();
}

/***************************************************************************//**
 * @brief  Main function
 ******************************************************************************/
int main(void)
{
  I2CSPM_Init_TypeDef i2cInit = I2CSPM_INIT_DEFAULT;
  uint32_t         rhData;
  bool             si7013_status;
  int32_t          tempData;
  uint32_t         vBat = 3300;
  bool             lowBatPrevious = true;
  bool             lowBat = false;
  /* Chip errata */
  CHIP_Init();

  /* Use LFXO for rtc used by the sleeptimer */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  CMU_ClockEnable(cmuClock_HFLE, true);

  RTC_Setup();

  /* Initalize peripherals and drivers */
  gpioSetup();
  adcInit();
  sl_sleeptimer_init();
  GRAPHICS_Init();
  I2CSPM_Init(&i2cInit);
  CAPSENSE_Init();

  selectedType = HOUR;

  /* Get initial sensor status */
  si7013_status = Si7013_Detect(i2cInit.port, SI7021_ADDR, NULL);
  GRAPHICS_ShowStatus(si7013_status, false);
  sl_sleeptimer_delay_millisecond(2000);

  /* Set up periodic measurement timer */
  sl_sleeptimer_start_periodic_timer_ms(&measurement_timer, MEASUREMENT_INTERVAL_MS, measurement_callback, NULL, 0, 0);

  //sl_sleeptimer_start_periodic_timer_ms(&sense_timer, 100, touch_callback, NULL, 0, 0);


  sl_sleeptimer_start_periodic_timer_ms(&clk_timer, INTSEC, time_callback, NULL, 0, 0);

  EMU_EnterEM2(false);
  // Buttons PB0 and PB1
  GPIO_PinModeSet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, gpioModeInputPull, 1);
  GPIO_PinModeSet(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN, gpioModeInputPull, 1);

  while (true) {
    // Status of PB0 and PB1
    btn0_state = GPIO_PinInGet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN);
    btn1_state = GPIO_PinInGet(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN);

    // Determine mode of operation
    if ((btn0_state == 0) && (btn1_state == 1)){
    	if (page_state == 4) {
    		if (menu_selected == 3) {
    			menu_selected = -1;
    		} else {
    			menu_selected = menu_selected + 1;
    		}
    	} else {
    		if (page_state == 2) {
    			if (date_adjust_state == 5) {
    				offsetInSeconds += adjustOffset(stopped_at_time + offsetInSeconds, YEAR, INCR);
    			} else {
    				date_adjust_state = (date_adjust_state + 1)%8;
    			}
    		}
    	}
    }

    if ((btn0_state == 1) && (btn1_state == 0)) {
    	   if (page_state == 4) {
    		  if (menu_selected == -1) {
    			page_state = prev_page_state;
    			menu_selected = prev_page_state;
    			prev_page_state = 4;
    		  } else {
    			prev_page_state = 4;
    		    page_state = menu_selected;
    		    if (page_state == 2) {
    		    	prev_page_state = 0;
    		    	stopped_at_time = cnt;
    		    	offsetInSecondsPrev = offsetInSeconds;
    		    }
    		  }
    	   } else {
    		   if (page_state == 0 || page_state == 1) {
    	          prev_page_state = page_state;
    	          menu_selected = page_state;
    	          page_state = 4;
    		   } else {
    			   if (page_state == 2) {
    				   if (date_adjust_state != 5 && date_adjust_state != 6 && date_adjust_state != 7) {
                              offsetInSeconds += adjustOffset(stopped_at_time + offsetInSeconds, date_adjust_state, INCR);
    				   } else {
    					   if (date_adjust_state == 5) {
    						   offsetInSeconds += adjustOffset(stopped_at_time + offsetInSeconds, YEAR, DECR);
    					   } else {
    						   if (date_adjust_state == 6) {
    							   cnt = stopped_at_time;
    							   page_state = 4;
    						   } else {
    							   if (date_adjust_state == 7) {
    								   cnt = stopped_at_time;
    								   offsetInSeconds = offsetInSecondsPrev;
    								   page_state = 4;
    							   }
    						   }
    					   }
    				   }
    			   }
    		   }
    	  }
       }

       if ((btn0_state == 0) && (btn1_state == 0)) {
    	   if (page_state == 2 && date_adjust_state == 5) {
    		   date_adjust_state ++;
    	   } else {
    	       redraw = true;
    	   }
       }

       if (((btn0_state == 1) && (btn1_state == 1))) {
    	   redraw = true;
       }

    if (measurement_flag) {
		measure_humidity_and_temperature(i2cInit.port, &rhData, &tempData, &vBat);
		if (tempData < temp_min_mC) temp_min_mC = tempData;
		if (tempData > temp_max_mC) temp_max_mC = tempData;
		if (rhData < humidity_min) humidity_min = rhData;
		if (rhData > humidity_max) humidity_max = rhData;
//		measurement_flag = false;
		if (lowBatPrevious) {
		  lowBat = (vBat <= LOW_BATTERY_THRESHOLD);
		} else {
		  lowBat = false;
		}
		lowBatPrevious = (vBat <= LOW_BATTERY_THRESHOLD);
		//GRAPHICS_Draw_Weather_Station(tempData, rhData, lowBat, temp_min_mC, temp_max_mC, humidity_min, humidity_max);
	}
       if (page_state == 0){
    		  clear_display();
    		  GRAPHICS_Draw(temp, rh, cnt + offsetInSeconds, lowBat);
    		  redraw = false;
    	} else {
    		if (page_state == 1) {
        	//if ((btn0_state == 0) && (btn1_state == 0)) {

    	    //} else {
        		clear_display();
        		GRAPHICS_Draw_Weather_Station(tempData, rhData, lowBat, temp_min_mC, temp_max_mC, humidity_min, humidity_max);
        	//}
        	    redraw = false;
           } else {
        	 if (page_state == 2) {
        		 clear_display();
        		 GRPAHICS_DrawTimeAdj(date_adjust_state, stopped_at_time, offsetInSeconds, cnt % blink_freq == 0, lowBat);
        		 redraw=false;
        	 } else {
        	   if (page_state == 3) {

        	   } else {
        	     if (page_state == 4) {
    	           clear_display();
    	           GRAPHICS_DrawMenu(menu_selected, lowBat);
    	           redraw = false;
                 }
        	   }
        	 }
           }
    	}

//    if (measurement_flag) {
//      measure_humidity_and_temperature(i2cInit.port, &rh, &temp, &vBat);
//      measurement_flag = false;
//      if (lowBatPrevious) {
//        lowBat = (vBat <= LOW_BATTERY_THRESHOLD);
//      } else {
//        lowBat = false;
//      }
//      lowBatPrevious = (vBat <= LOW_BATTERY_THRESHOLD);
//    }

    }
    EMU_EnterEM2(false);
}


void resetMinMaxTemp(void)
{
	temp_min_mC = INT32_MAX;
	temp_max_mC = INT32_MIN;
}

void resetMinMacHumidity(void)
{
	humidity_min = INT32_MAX;
	humidity_max = 0;
}

void resetTempHumidity(void)
{
	resetMinMaxTemp();
	resetMinMacHumidity();
}
/***************************************************************************//**
 * @brief Setup GPIO interrupt for pushbuttons.
 *****************************************************************************/
static void gpioSetup(void)
{
  /* Enable GPIO clock */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Enable si7021 sensor isolation switch */
  GPIO_PinModeSet(gpioPortC, 8, gpioModePushPull, 1);

  GPIO_PinModeSet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, gpioModeInputPull, 1);
  GPIO_IntConfig(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, false, true, true);


  /* Configure PB1 as input and enable interrupt */
  GPIO_PinModeSet(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN, gpioModeInputPull, 1);
  GPIO_IntConfig(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN, false, true, true);

#if defined(_EFM32_GIANT_FAMILY)
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
#else
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
#endif

}

/***************************************************************************//**
 * @brief This function is called whenever we want to measure the supply v.
 *        It is reponsible for starting the ADC and reading the result.
 ******************************************************************************/
static uint32_t checkBattery(void)
{
  uint32_t vData;
  /* Sample ADC */
  adcConversionComplete = false;
  ADC_Start(ADC0, adcStartSingle);
  while (!adcConversionComplete) EMU_EnterEM1();
  vData = ADC_DataSingleGet(ADC0);
  return vData;
}

/***************************************************************************//**
 * @brief ADC Interrupt handler (ADC0)
 ******************************************************************************/
void ADC0_IRQHandler(void)
{
  uint32_t flags;

  /* Clear interrupt flags */
  flags = ADC_IntGet(ADC0);
  ADC_IntClear(ADC0, flags);

  adcConversionComplete = true;
}

/***************************************************************************//**
 * @brief ADC Initialization
 ******************************************************************************/
static void adcInit(void)
{
  ADC_Init_TypeDef       init       = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef initSingle = ADC_INITSINGLE_DEFAULT;

  /* Enable ADC clock */
  CMU_ClockEnable(cmuClock_ADC0, true);

  /* Initiate ADC peripheral */
  ADC_Init(ADC0, &init);

  /* Setup single conversions for internal VDD/3 */
  initSingle.acqTime = adcAcqTime16;
  initSingle.input   = adcSingleInpVDDDiv3;
  ADC_InitSingle(ADC0, &initSingle);

  /* Manually set some calibration values */
  ADC0->CAL = (0x7C << _ADC_CAL_SINGLEOFFSET_SHIFT) | (0x1F << _ADC_CAL_SINGLEGAIN_SHIFT);

  /* Enable interrupt on completed conversion */
  ADC_IntEnable(ADC0, ADC_IEN_SINGLE);
  NVIC_ClearPendingIRQ(ADC0_IRQn);
  NVIC_EnableIRQ(ADC0_IRQn);
}

void GPIO_ODD_IRQHandler(void) {
	  /* Get and clear all pending GPIO interrupts */
	  uint32_t interruptMask = GPIO_IntGet();


	  /* Act on interrupts */
	  if (interruptMask & (1 << BSP_GPIO_PB0_PIN)) {
		  GPIO_IntClear(interruptMask);
//	     if (mode == 0) {
//	    	 display_cnt = cnt;
//	    	 selectedType = HOUR;
//	    	 mode = 1;
//	     } else {
//	    	 cnt = display_cnt;
//	    	 mode = 0;
//	     }
	  }

	  if (interruptMask & (1 << BSP_GPIO_PB1_PIN)) {
		  GPIO_IntClear(interruptMask);

//	     if (mode == 1) {
//	    	 if (selectedType == YEAR) {
//	    		 selectedType = HOUR;
//	    	 } else {
//	    		 selectedType ++;
//	    	 }
//	     }
	  }
}

void GPIO_EVEN_IRQHandler(void)
{
  /* Get and clear all pending GPIO interrupts */
  uint32_t interruptMask = GPIO_IntGet();


  /* Act on interrupts */
  if (interruptMask & (1 << BSP_GPIO_PB0_PIN)) {
	  GPIO_IntClear(interruptMask);
//     if (mode == 0) {
//    	 display_cnt = cnt;
//    	 selectedType = HOUR;
//    	 mode = 1;
//     } else {
//    	 cnt = display_cnt;
//    	 mode = 0;
//     }
  }

  if (interruptMask & (1 << BSP_GPIO_PB1_PIN)) {
	  GPIO_IntClear(interruptMask);
//     if (mode == 1) {
//    	 if (selectedType == YEAR) {
//    		 selectedType = HOUR;
//    	 } else {
//    		 selectedType ++;
//    	 }
//     }
  }
}

/***************************************************************************//**
 * @brief  Helper function to perform data measurements.
 ******************************************************************************/
static void measure_humidity_and_temperature(I2C_TypeDef *i2c, uint32_t *rhData, int32_t *tData, uint32_t *vBat)
{
  *vBat = checkBattery();
  Si7013_MeasureRHAndTemp(i2c, SI7021_ADDR, rhData, tData);
}

/***************************************************************************//**
 * @brief Callback from timer used to initiate new measurement
 ******************************************************************************/
static void measurement_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void) handle;
  (void) data;
  measurement_flag = true;
}

static void time_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void) handle;
  (void) data;
  cnt++;
  measurement_flag = true;
  redraw = true;
}

/***************************************************************************//**
 * @brief   The actual callback for Memory LCD toggling
 ******************************************************************************/
static void display_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;
  mem_lcd_callback_func(mem_lcd_callback_arg);
}

//static void touch_callback(sl_sleeptimer_timer_handle_t *handle, void *data) {
//	if (mode != 0) {
//	CAPSENSE_Sense();
//	    	      if ( CAPSENSE_getPressed(BUTTON1_CHANNEL)
//	    	           && !CAPSENSE_getPressed(BUTTON0_CHANNEL)) {
//	    	    	  offsetInSeconds += adjustOffset(display_cnt + offsetInSeconds, selectedType, INCR);
//	    	          //+
//	    	      } else if ( CAPSENSE_getPressed(BUTTON0_CHANNEL)
//	    	                  && !CAPSENSE_getPressed(BUTTON1_CHANNEL)) {
//	    	    	  offsetInSeconds += adjustOffset(display_cnt + offsetInSeconds, selectedType, DECR);
//	    	    	  //-
//	 }
//	}
//}

/***************************************************************************//**
 * @brief   Register a callback function at the given frequency.
 *
 * @param[in] pFunction  Pointer to function that should be called at the
 *                       given frequency.
 * @param[in] argument   Argument to be given to the function.
 * @param[in] frequency  Frequency at which to call function at.
 *
 * @return  Always return 0
 ******************************************************************************/
int rtcIntCallbackRegister(void (*pFunction)(void*),
                           void* argument,
                           unsigned int frequency)
{
  mem_lcd_callback_func = pFunction;
  mem_lcd_callback_arg  = argument;
  uint32_t ticks = sl_sleeptimer_get_timer_frequency() / frequency;
  sl_sleeptimer_start_periodic_timer(&display_timer, ticks, display_callback, NULL, 0, 0);

  return 0;
}
