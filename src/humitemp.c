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
static volatile uint32_t cnt;
static volatile int32_t offsetInSeconds;
static volatile uint32_t mode;
static volatile uint32_t display_cnt;
static volatile TimeType selectedType;

/***************************************************************************//**
 * Local prototypes
 ******************************************************************************/
static void gpioSetup(void);
static uint32_t checkBattery(void);
static void adcInit(void);
static void measure_humidity_and_temperature(I2C_TypeDef *i2c, uint32_t *rhData, int32_t *tData, uint32_t *vBat);
static void measurement_callback(sl_sleeptimer_timer_handle_t *handle, void *data);
static void time_callback(sl_sleeptimer_timer_handle_t *handle, void *data);
static void touch_callback(sl_sleeptimer_timer_handle_t *handle, void *data);
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

  sl_sleeptimer_start_periodic_timer_ms(&sense_timer, 100, touch_callback, NULL, 0, 0);


  sl_sleeptimer_start_periodic_timer_ms(&clk_timer, INTSEC, time_callback, NULL, 0, 0);

  EMU_EnterEM2(false);

  while (true) {
    if (measurement_flag) {
      measure_humidity_and_temperature(i2cInit.port, &rhData, &tempData, &vBat);
      measurement_flag = false;
      if (lowBatPrevious) {
        lowBat = (vBat <= LOW_BATTERY_THRESHOLD);
      } else {
        lowBat = false;
      }
      lowBatPrevious = (vBat <= LOW_BATTERY_THRESHOLD);
      if (mode == 0) {
          GRAPHICS_Draw(tempData, rhData, cnt + offsetInSeconds, lowBat);
      } else {

    	  GRAPHICS_Draw(tempData, rhData, display_cnt + offsetInSeconds, lowBat);
      }
    }
    EMU_EnterEM2(false);
  }
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
	     if (mode == 0) {
	    	 display_cnt = cnt;
	    	 selectedType = HOUR;
	    	 mode = 1;
	     } else {
	    	 cnt = display_cnt;
	    	 mode = 0;
	     }
	  }

	  if (interruptMask & (1 << BSP_GPIO_PB1_PIN)) {
		  GPIO_IntClear(interruptMask);
	     if (mode == 1) {
	    	 if (selectedType == YEAR) {
	    		 selectedType = HOUR;
	    	 } else {
	    		 selectedType ++;
	    	 }
	     }
	  }
}

void GPIO_EVEN_IRQHandler(void)
{
  /* Get and clear all pending GPIO interrupts */
  uint32_t interruptMask = GPIO_IntGet();


  /* Act on interrupts */
  if (interruptMask & (1 << BSP_GPIO_PB0_PIN)) {
	  GPIO_IntClear(interruptMask);
     if (mode == 0) {
    	 display_cnt = cnt;
    	 selectedType = HOUR;
    	 mode = 1;
     } else {
    	 cnt = display_cnt;
    	 mode = 0;
     }
  }

  if (interruptMask & (1 << BSP_GPIO_PB1_PIN)) {
	  GPIO_IntClear(interruptMask);
     if (mode == 1) {
    	 if (selectedType == YEAR) {
    		 selectedType = HOUR;
    	 } else {
    		 selectedType ++;
    	 }
     }
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

static void touch_callback(sl_sleeptimer_timer_handle_t *handle, void *data) {
	if (mode != 0) {
	CAPSENSE_Sense();
	    	      if ( CAPSENSE_getPressed(BUTTON1_CHANNEL)
	    	           && !CAPSENSE_getPressed(BUTTON0_CHANNEL)) {
	    	    	  offsetInSeconds += adjustOffset(display_cnt + offsetInSeconds, selectedType, INCR);
	    	          //+
	    	      } else if ( CAPSENSE_getPressed(BUTTON0_CHANNEL)
	    	                  && !CAPSENSE_getPressed(BUTTON1_CHANNEL)) {
	    	    	  offsetInSeconds += adjustOffset(display_cnt + offsetInSeconds, selectedType, DECR);
	    	    	  //-
	 }
	}
}

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
