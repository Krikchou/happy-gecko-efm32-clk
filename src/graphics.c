/***************************************************************************//**
 * @file
 * @brief Draws the graphics on the display
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

#include "graphics_c.h"
#include "em_types.h"
#include "glib.h"
#include "dmd.h"
#include "display.h"
#include "textdisplay.h"
#include "retargettextdisplay.h"
#include "clock_control.h"
#include "extra_fonts.h"
#include <string.h>
#include <stdio.h>

static GLIB_Context_t glibContext;          /* Global glib context */

static const uint8_t bitmap_drop[] = {0x00,
		0x00,
		0x1F,
		0xF8,
		0x1F,
		0xF8,
		0x07,
		0x80,
		0x07,
		0x80,
		0x07,
		0x80,
		0x07,
		0x80,
		0x07,
		0x80,
		0x07,
		0x80,
		0x07,
		0x80,
		0x1F,
		0xE0,
		0x7F,
		0xF8,
		0x7F,
		0xF8,
		0x7F,
		0xF8,
		0x1F,
		0xE0,
		0x07,
		0x80};

static void GRAPHICS_DrawThermometer(int32_t xoffset, int32_t yoffset, uint32_t max, int32_t level, char scale);
static void GRAPHICS_DrawTemperatureC(int32_t xoffset, int32_t yoffset, int32_t tempData);
static void GRAPHICS_DrawTemperatureF(int32_t xoffset, int32_t yoffset, int32_t tempData);
static void GRAPHICS_DrawHumidity(int32_t xoffset, int32_t yoffset, uint32_t rhData);
static void GRAPHICS_CreateString(char *string, int32_t value);
static void GRAPHICS_DrawThermometer_Weather_Station(int32_t xoffset, int32_t yoffset, int32_t min, int32_t max, int32_t level, char scale);
static void GRAPHICS_DrawTemperatureC_Weather_Station(int32_t xoffset, int32_t yoffset, int32_t tempData, int32_t temp_min_mC, int32_t temp_max_mC);
static void GRAPHICS_DrawTemperatureF_Weather_Station(int32_t xoffset, int32_t yoffset, int32_t tempData, int32_t temp_min_mC, int32_t temp_max_mC);
static void GRAPHICS_DrawHumidity_Weather_Station(int32_t xoffset, int32_t yoffset, int32_t rhData, int32_t humidity_min, int32_t humidity_max);
static void GRAPHICS_DrawThermometerFrame(int32_t xoffset, int32_t yoffset);
void GLIB_drawStringCentered(GLIB_Context_t *pContext, const char *s, unsigned int len, int xCenter, int y, bool opaque);


/***************************************************************************//**
 * @brief Initializes the graphics stack.
 * @note This function will /hang/ if errors occur (usually
 *       caused by faulty displays.
 ******************************************************************************/
void GRAPHICS_Init(void)
{
  EMSTATUS status;

  /* Initialize the display module. */
  status = DISPLAY_Init();
  if (DISPLAY_EMSTATUS_OK != status) {
    while (1)
      ;
  }

  /* Initialize the DMD module for the DISPLAY device driver. */
  status = DMD_init(0);
  if (DMD_OK != status) {
    while (1)
      ;
  }

  status = GLIB_contextInit(&glibContext);
  if (GLIB_OK != status) {
    while (1)
      ;
  }

  glibContext.backgroundColor = Black;
  glibContext.foregroundColor = White;

  /* Use Narrow font */
  GLIB_setFont(&glibContext, (GLIB_Font_t *)&GLIB_FontNarrow6x8);
}

/***************************************************************************//**
 * @brief This function draws the background image
 * @param xoffset
 *        The background image is wider than the display. This parameter
 *        selects which part to show. The offset is given in multiples of 8.
 *        If you increase xoffset by 1, the bacground will be shifted by 8
 *        pixels
 ******************************************************************************/
void GRAPHICS_ShowStatus(bool si7013_status, bool lowBat)
{
  GLIB_clear(&glibContext);

  if (lowBat) {
    GLIB_drawString(&glibContext, "Low battery!", 16, 5, 115, 0);
  } else if (!si7013_status) {
    GLIB_drawString(&glibContext, "Failed to detect\nsi7021 sensor.", 32, 5, 5, 0);
  } else {
    GLIB_drawString(&glibContext, "si7021 sensor ready.\n"DEMO_VERSION, 32, 5, 5, 0);
  }
  DMD_updateDisplay();
}

/***************************************************************************//**
 * @brief This function draws the UI
 * @param tempData
 *        Temperature data (given in Celsius) multiplied by 1000
 * @param rhData
 *        Relative humidity (in percent), multiplied by 1000.
 * @param degF
 *        Set to 0 to display temperature in Celsius, otherwise Fahrenheit.
 ******************************************************************************/
void GRAPHICS_Draw(int32_t tempData, uint32_t rhData, uint32_t sec, bool lowBat)
{
  GLIB_clear(&glibContext);

  if (lowBat) {
    GLIB_drawString(&glibContext, "LOW BATTERY!", 12, 5, 120, 0);
  } else {
    /* Draw temperature and RH */
    //GRAPHICS_DrawTemperatureC(6, 3, tempData);
    //GRAPHICS_DrawTemperatureF(64 - 17, 3, tempData);
    //GRAPHICS_DrawHumidity(127 - 40, 3, rhData);
	  char str[50];


	  Time t;
	  t = GetCurrTime(sec);
	  GLIB_setFont(&glibContext, (GLIB_Font_t *)&GLIB_font7Segment);
	  snprintf(str, 50, "%d%d:%d%d:%d%d",
			  t.tm_hour / 10,
			  t.tm_hour % 10,
			  t.tm_min  / 10,
			  t.tm_min  % 10,
			  t.tm_sec  / 10,
			  t.tm_sec  % 10);


	  GLIB_drawString(&glibContext, str, 25, 5, 50, 0);

	  snprintf(str, 50, "%d%d/%d%d/%d",
			  t.tm_mday / 10,
			  t.tm_mday % 10,
			  t.tm_mon  / 10,
			  t.tm_mon  % 10,
			  t.tm_year);

	  GLIB_drawString(&glibContext, str, 25, 5, 75, 0);

	  GLIB_drawBitmap(&glibContext, 5, 95, 16, 16, bitmap_drop);

	  snprintf(str, 10, "%d%d.%d%d'C",
			  (tempData / 1000 % 100) / 10,
			  (tempData / 1000 % 1000) / 100,
			  (tempData / 1000 % 10000) / 1000,
			  (tempData / 1000 % 100000) / 10000);
	  GLIB_drawString(&glibContext, str, 25, 30, 95, 0);
  }
  DMD_updateDisplay();
}

/***************************************************************************//**
 * @brief Helper function for drawing the temperature in Fahrenheit
 * @param xoffset
 *        This parameter selects which part of the UI to show.
 * @param yoffset
 *        This parameter selects which part of the UI to show.
 * @param tempData
 *        Temperature data (given in Celsius) multiplied by 1000
 ******************************************************************************/
static void GRAPHICS_DrawTemperatureF(int32_t xoffset, int32_t yoffset, int32_t tempData)
{
  char string[10];

  tempData = ((tempData * 9) / 5) + 32000;

  GRAPHICS_CreateString(string, tempData);
  GLIB_drawString(&glibContext, string, strlen(string), xoffset, yoffset, 0);

  GRAPHICS_DrawThermometer(xoffset + 15, yoffset + 17, 95, tempData / 1000, 'F');
}

/***************************************************************************//**
 * @brief Helper function for drawing the temperature in Celsius.
 * @param xoffset
 *        This parameter selects which part of the UI to show.
 * @param yoffset
 *        This parameter selects which part of the UI to show.
 * @param tempData
 *        Temperature data (given in Celsius) multiplied by 1000
 ******************************************************************************/
static void GRAPHICS_DrawTemperatureC(int32_t xoffset, int32_t yoffset, int32_t tempData)
{
  char string[10];

  GRAPHICS_CreateString(string, tempData);
  GLIB_drawString(&glibContext, string, strlen(string), xoffset, yoffset, 0);

  GRAPHICS_DrawThermometer(xoffset + 15, yoffset + 17, 35, tempData / 1000, 'C');
}

/***************************************************************************//**
 * @brief Helper function for drawing a thermometer.
 * @param xoffset
 *        Top left pixel X offset
 * @param yoffset
 *        Top left pixel Y offset
 ******************************************************************************/
static void GRAPHICS_DrawThermometer(int32_t xoffset,
                                     int32_t yoffset,
                                     uint32_t max,
                                     int32_t level,
                                     char scale)
{
  GLIB_Rectangle_t thermoScale;
  GLIB_Rectangle_t mercuryLevel;
  const uint32_t minLevelY = yoffset + 76;
  const uint32_t maxLevelY = yoffset + 3;
  uint32_t curLevelY;
  bool levelNeg;
  char string[10];

  /* Draw outer frame */
  thermoScale.xMin = xoffset - 4;
  thermoScale.xMax = xoffset + 4;
  thermoScale.yMin = yoffset;
  thermoScale.yMax = yoffset + 90;
  GLIB_drawCircleFilled(&glibContext, xoffset, yoffset + 90, 12);
  GLIB_drawRectFilled(&glibContext, &thermoScale);

  glibContext.backgroundColor = White;
  glibContext.foregroundColor = Black;

  /* Draw the "mercury" */
  levelNeg = (level < 0);
  /* Abs value and saturate at max */
  level = levelNeg ? level * -1 : level;
  level = level > (int32_t)max ? (int32_t)max : level;
  curLevelY = yoffset + (((minLevelY - maxLevelY) * (max - level)) / max);

  /* Moving part */
  mercuryLevel.xMin = xoffset - 2;
  mercuryLevel.xMax = xoffset + 2;
  mercuryLevel.yMin = curLevelY;
  mercuryLevel.yMax = minLevelY;
  GLIB_drawRectFilled(&glibContext, &mercuryLevel);

  /* Non-moving part */
  mercuryLevel.xMin = xoffset - 2;
  mercuryLevel.xMax = xoffset + 2;
  mercuryLevel.yMin = minLevelY - 1;
  mercuryLevel.yMax = minLevelY + 5;
  GLIB_drawRectFilled(&glibContext, &mercuryLevel);

  /* Glass bulp */
  GLIB_drawCircleFilled(&glibContext, xoffset, yoffset + 90, 9);

  glibContext.backgroundColor = Black;
  glibContext.foregroundColor = White;

  /* Draw min/max lines and numbers */
  GLIB_drawLineH(&glibContext, xoffset - 6, minLevelY, xoffset + 6);
  GLIB_drawLineH(&glibContext, xoffset - 6, maxLevelY, xoffset + 6);

  GLIB_drawString(&glibContext, "0", 1, xoffset + 8, minLevelY - 4, 0);
  snprintf(string, 4, "%ld", max);
  GLIB_drawString(&glibContext, string, 4, xoffset + 8, maxLevelY - 4, 0);

  if (levelNeg) {
    GLIB_drawString(&glibContext, "-", 1, xoffset - 2, yoffset + 87, 0);
  } else {
    GLIB_drawString(&glibContext, (char *)&scale, 1, xoffset - 2, yoffset + 87, 0);
  }
}

/***************************************************************************//**
 * @brief Helper function for drawing the humidity part of the UI.
 * @param xoffset
 *        This parameter selects which part of the UI to show.
 * @param rhData
 *        Relative humidity (in percent), multiplied by 1000.
 ******************************************************************************/
static void GRAPHICS_DrawHumidity(int32_t xoffset, int32_t yoffset, uint32_t rhData)
{
  char string[10];

  GRAPHICS_CreateString(string, rhData);
  GLIB_drawString(&glibContext, string, strlen(string), xoffset, yoffset, 0);

  GRAPHICS_DrawThermometer(xoffset + 15, yoffset + 17, 100, rhData / 1000, '%');
}

/***************************************************************************//**
 * @brief Helper function for printing numbers. Consumes less space than
 *        snprintf. Limited to two digits and one decimal.
 * @param string
 *        The string to print32_t to
 * @param value
 *        The value to print
 ******************************************************************************/
static void GRAPHICS_CreateString(char *string, int32_t value)
{
  if (value < 0) {
    value = -value;
    string[0] = '-';
  } else {
    string[0] = ' ';
  }
  string[5] = 0;
  string[4] = '0' + (value % 1000) / 100;
  string[3] = '.';
  string[2] = '0' + (value % 10000) / 1000;
  string[1] = '0' + (value % 100000) / 10000;

  if (string[1] == '0') {
    string[1] = ' ';
  }
}

static void GRAPHICS_DrawThermometerFrame(int32_t xoffset, int32_t yoffset)
{
  GLIB_Rectangle_t thermoScale;

  glibContext.backgroundColor = Black;
  glibContext.foregroundColor = White;

  /* Draw outer frame */
  thermoScale.xMin = xoffset - 4;
  thermoScale.xMax = xoffset + 4;
  thermoScale.yMin = yoffset;
  thermoScale.yMax = yoffset + 90;
  GLIB_drawCircleFilled(&glibContext, xoffset, yoffset + 90, 12);
  GLIB_drawRectFilled(&glibContext, &thermoScale);

  /* Draw the empty glass part inside */
  glibContext.backgroundColor = White;
  glibContext.foregroundColor = Black;
  GLIB_drawCircleFilled(&glibContext, xoffset, yoffset + 90, 9); // Inner bulb
  thermoScale.xMin = xoffset - 2;
  thermoScale.xMax = xoffset + 2;
  thermoScale.yMin = yoffset + 3;
  thermoScale.yMax = yoffset + 90;
  GLIB_drawRectFilled(&glibContext, &thermoScale); // Inner tube

  /* Reset colors for drawing on top */
  glibContext.backgroundColor = Black;
  glibContext.foregroundColor = White;
}


/**
 * @brief Draws a string horizontally centered around a given x-coordinate.
 *
 * @param pContext
 *   Pointer to the graphics context.
 * @param s
 *   The string to draw.
 * @param len
 *   The length of the string.
 * @param xCenter
 *   The horizontal CENTER coordinate for the text.
 * @param y
 *   The vertical (top) coordinate for the text.
 * @param opaque
 *   Set to true to draw the background color, false for transparent text.
 */
void GLIB_drawStringCentered(GLIB_Context_t *pContext,
                             const char *s,
                             unsigned int len,
                             int xCenter,
                             int y,
                             bool opaque)
{
  // The default font used by the GLIB library is a fixed-width 6x8 pixel font.
  #define FONT_WIDTH 6

  // 1. Calculate the total pixel width of the string.
  int32_t textWidth = len * FONT_WIDTH;

  // 2. Calculate the starting x-coordinate to make the text centered.
  //    The formula is: centerPoint - (totalWidth / 2)
  int32_t xStart = xCenter - (textWidth / 2);

  // 3. Call the standard library function with the calculated starting coordinate.
  GLIB_drawString(pContext, s, len, xStart, y, opaque);
}


void GRAPHICS_Draw_Weather_Station(int32_t tempData,
                                   int32_t rhData,
                                   bool lowBat,
                                   int32_t temp_min_mC,
                                   int32_t temp_max_mC,
                                   int32_t humidity_min,
                                   int32_t humidity_max)
{
  GLIB_clear(&glibContext);

  if (lowBat) {
    GLIB_drawString(&glibContext, "LOW BATTERY!", 12, 5, 120, 0);
  } else {
    /* Draw temperature and RH */
    GRAPHICS_DrawTemperatureC_Weather_Station(6, 3, tempData, temp_min_mC, temp_max_mC);
    GRAPHICS_DrawTemperatureF_Weather_Station(64 - 17, 3, tempData, temp_min_mC, temp_max_mC);
    GRAPHICS_DrawHumidity_Weather_Station(127 - 40, 3, rhData, humidity_min, humidity_max);
  }
  DMD_updateDisplay();
}


/***************************************************************************//**
 * @brief Helper function for drawing the temperature in Celsius.
 * @param xoffset
 *        This parameter selects which part of the UI to show.
 * @param yoffset
 *        This parameter selects which part of the UI to show.
 * @param tempData
 *        Temperature data (given in Celsius) multiplied by 1000
 * @param temp_min_mC
 * 	      Min measured temperature from the sensor.
 * @param temp_max_mC
 * 		  Max measured temperature from the sensor.
 ******************************************************************************/
static void GRAPHICS_DrawTemperatureC_Weather_Station(int32_t xoffset,
									  int32_t yoffset,
									  int32_t tempData,
									  int32_t temp_min_mC,
									  int32_t temp_max_mC)
{
  char string[10];

  GRAPHICS_CreateString(string, tempData);
  GLIB_drawString(&glibContext, string, strlen(string), xoffset, yoffset, 0);

  GRAPHICS_DrawThermometer_Weather_Station(xoffset + 15, yoffset + 17, temp_min_mC / 1000, temp_max_mC / 1000, tempData / 1000, 'C');
}

/***************************************************************************//**
 * @brief Helper function for drawing the temperature in Fahrenheit
 * @param xoffset
 *        This parameter selects which part of the UI to show.
 * @param yoffset
 *        This parameter selects which part of the UI to show.
 * @param tempData
 *        Temperature data (given in Celsius) multiplied by 1000
 * @param temp_min_mC
 * 	      Min measured temperature from the sensor.
 * @param temp_max_mC
 * 		  Max measured temperature from the sensor.
 ******************************************************************************/
static void GRAPHICS_DrawTemperatureF_Weather_Station(int32_t xoffset,
									  int32_t yoffset,
									  int32_t tempData,
									  int32_t temp_min_mC,
									  int32_t temp_max_mC)
{
  char string[10];

  tempData = ((tempData * 9) / 5) + 32000;
  temp_min_mC = ((temp_min_mC * 9) / 5) + 32000;
  temp_max_mC = ((temp_max_mC * 9) / 5) + 32000;

  GRAPHICS_CreateString(string, tempData);
  GLIB_drawString(&glibContext, string, strlen(string), xoffset, yoffset, 0);

  GRAPHICS_DrawThermometer_Weather_Station(xoffset + 15, yoffset + 17, temp_min_mC / 1000, temp_max_mC / 1000, tempData / 1000, 'F');
}

/***************************************************************************//**
 * @brief Helper function for drawing the humidity part of the UI.
 * @param xoffset
 *        This parameter selects which part of the UI to show.
 * @param rhData
 *        Relative humidity (in percent), multiplied by 1000.
 * @param humidity_min
 * 		  Minimum humidity
 * @param humidity_max
 * 		  Maximum humidity
 ******************************************************************************/
static void GRAPHICS_DrawHumidity_Weather_Station(int32_t xoffset,
								  int32_t yoffset,
								  int32_t rhData,
								  int32_t humidity_min,
								  int32_t humidity_max)
{
  char string[10];

  GRAPHICS_CreateString(string, rhData);
  GLIB_drawString(&glibContext, string, strlen(string), xoffset, yoffset, 0);

  GRAPHICS_DrawThermometer_Weather_Station(xoffset + 15, yoffset + 17, humidity_min / 1000, humidity_max / 1000, rhData / 1000, '%');
}


/**
 * @brief Draws the dynamic level and scale character for a thermometer graphic.
 * @note  This version ONLY draws the mercury level and the symbol in the bulb.
 *        It assumes the frame has already been drawn.
 */
static void GRAPHICS_DrawThermometer_Weather_Station(int32_t xoffset,
                                     int32_t yoffset,
                                     int32_t min,
                                     int32_t max,
                                     int32_t level,
                                     char scale)
{
  GLIB_Rectangle_t thermoScale;
  GLIB_Rectangle_t mercuryLevel;
  const uint32_t minLevelY = yoffset + 76;
  const uint32_t maxLevelY = yoffset + 3;
  uint32_t curLevelY;
  char string[10];

  /* Draw outer frame of the thermometer */
  glibContext.backgroundColor = Black;
  glibContext.foregroundColor = White;
  thermoScale.xMin = xoffset - 4;
  thermoScale.xMax = xoffset + 4;
  thermoScale.yMin = yoffset;
  thermoScale.yMax = yoffset + 90;
  GLIB_drawCircleFilled(&glibContext, xoffset, yoffset + 90, 12);
  GLIB_drawRectFilled(&glibContext, &thermoScale);

  /* Draw the inner "glass" part of the thermometer */
  glibContext.backgroundColor = White;
  glibContext.foregroundColor = Black;

  /* Glass bulb */
  GLIB_drawCircleFilled(&glibContext, xoffset, yoffset + 90, 9);
  /* Glass tube */
  mercuryLevel.xMin = xoffset - 2;
  mercuryLevel.xMax = xoffset + 2;
  mercuryLevel.yMin = yoffset + 3;
  mercuryLevel.yMax = yoffset + 90;
  GLIB_drawRectFilled(&glibContext, &mercuryLevel);

  /* Calculate and draw the "mercury" level */
  if (level < min) level = min;
  if (level > max) level = max;
  float valueRatio = 0.0f;
  if ((max - min) != 0) { // Prevent division by zero
    valueRatio = (float)(level - min) / (float)(max - min);
  }
  curLevelY = minLevelY - (uint32_t)(valueRatio * (float)(minLevelY - maxLevelY));

  mercuryLevel.xMin = xoffset - 2;
  mercuryLevel.xMax = xoffset + 2;
  mercuryLevel.yMin = curLevelY;
  mercuryLevel.yMax = minLevelY + 5; // Overlap slightly into the bulb
  GLIB_drawRectFilled(&glibContext, &mercuryLevel);

  /* --- FONT SIZE CHANGE FOR THERMOMETER TEXT --- */
  glibContext.backgroundColor = Black;
  glibContext.foregroundColor = White;

  /* Draw min/max lines */
  GLIB_drawLineH(&glibContext, xoffset - 6, minLevelY, xoffset + 6);
  GLIB_drawLineH(&glibContext, xoffset - 6, maxLevelY, xoffset + 6);

  // 1. SET THE FONT to the smaller one.
  GLIB_setFont(&glibContext, &GLIB_FontNarrow6x8);

  // 2. DRAW ALL THE TEXT using the new, smaller font.
  snprintf(string, sizeof(string), "%ld", min);
  GLIB_drawString(&glibContext, string, strlen(string), xoffset + 8, minLevelY - 4, 0);

  snprintf(string, sizeof(string), "%ld", max);
  GLIB_drawString(&glibContext, string, strlen(string), xoffset + 8, maxLevelY - 4, 0);

  // Center the scale character in the bulb (assuming 6-pixel wide narrow font)
  GLIB_drawString(&glibContext, (char *)&scale, 1, xoffset - 3, yoffset + 87, 0);

  // 3. IMPORTANT: SET THE FONT BACK to the default for the rest of the application.
  GLIB_setFont(&glibContext, &GLIB_FontNormal8x8);
}
