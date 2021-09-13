// Include application, user and local libraries
//#include <SPI.h>
#include <TFT_22_ILI9225.h>

// This code works with the 2.0in Arduino Color LCD Modual from 
// Include font definition files
// NOTE: These files may not have all characters defined! Check the GFXfont def
// params 3 + 4, e.g. 0x20 = 32 = space to 0x7E = 126 = ~

/*
 * To create these fonts go to http://oleddisplay.squix.ch/#/home
 * Choose the TFT display under Preview Display.
 * Choose "Adafruit GTX Font" under the "Library Version".
 * The rest should be obvious.
 * Copy-n-past to a file as named in the #include below.
 */
#include "Roboto_Mono_Bold_16.h"
#include "Roboto_Mono_Bold_15.h"
#include "bc.h"

#define DEBUG true

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS,
                                    TFT_LED, TFT_BRIGHTNESS);
// Use software SPI (slower)
//TFT_22_ILI9225 tft = TFT_22_ILI9225(
//  TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED,
//  TFT_BRIGHTNESS);

// Variables and constants
MeterHeaderType headers[HEADER_STRUCT_SIZE];
MeterDataType meter_data[METER_DATA_ARRAY_SIZE];
const double resConvert = 4095.0; // 12 bit ADC counts

void setup()
  {
#if DEBUG == true
  SerialUSB.begin(115200);
  //while(!SerialUSB);
#endif
  // Set analog input resolution to max, 12-bits
  analogReadResolution(12);
  // Config 74HC4051 address select pins.
  pinMode(PIN_S0, OUTPUT);
  pinMode(PIN_S1, OUTPUT);
  pinMode(PIN_S2, OUTPUT);
  // Start the TFT LCD display
  tft.begin();
  tft.clear();
  //tft.setBackgroundColor(DEFAULT_BACKGROUND_COLOR);
  // 0=portrait, 1=right rotated landscape, 2=reverse portrait,
  // 3=left rotated landscape
  tft.setOrientation(1);
  populateHeaders();
  initialPopulateData();
  // Draw the header voltage, current, and power strings.
  drawHeaders();
  }

void populateHeaders()
  {
  headers[0].text = "Constant";
  headers[0].f = &Roboto_Mono_Bold_16;
  headers[0].color = COLOR_CYAN;
  headers[0].x1 = 65;
  headers[0].y1 = 0;
  headers[0].x2 = 0;
  headers[0].y2 = 0;
  headers[1].text = "Current & Voltage";
  headers[1].f = &Roboto_Mono_Bold_16;
  headers[1].color = COLOR_CYAN;
  headers[1].x1 = 10;
  headers[1].y1 = 15;
  headers[1].x2 = 0;
  headers[1].y2 = 0;
  headers[2].text = ""; // Horizontal Title Line
  headers[2].f = NULL;
  headers[2].color = COLOR_CYAN;
  headers[2].x1 = 0;
  headers[2].y1 = 33;
  headers[2].x2 = 220;
  headers[2].y2 = 33;
  headers[3].text = "Set Values  Actual";
  headers[3].f = &Roboto_Mono_Bold_15;
  headers[3].color = COLOR_YELLOW;
  headers[3].x1 = 5;
  headers[3].y1 = 38;
  headers[3].x2 = 0;
  headers[3].y2 = 0;
  }

void drawHeaders()
  {
  int16_t y, w, h;

  for (uint16_t i = 0; i < NUM_ITEMS(headers); i++)
    {
    if (headers[i].text == "")
      {
      tft.drawLine(headers[i].x1, headers[i].y1, headers[i].x2,
                   headers[i].y2, headers[i].color);
      }
    else
      {
      // Draw the header text.
      tft.setGFXFont(headers[i].f); // Set current font
      // Get string extents
      tft.getGFXTextExtent(headers[i].text, headers[i].x1,
                           headers[i].y1, &w, &h);
      y = headers[i].y1 + h;
      // Print the string to the LCD TFT.
      tft.drawGFXText(headers[i].x1, y, headers[i].text,
                      headers[i].color);
      }
    }
  }

void initialPopulateData(void)
  {
  meter_data[0].text = ""; // Set Voltage
  meter_data[0].changed = true;
  meter_data[0].f = &Roboto_Mono_Bold_16;
  meter_data[0].color = COLOR_YELLOW;
  meter_data[0].x = 10;
  meter_data[0].y = 80;
  meter_data[1].text = ""; // Set Current
  meter_data[1].changed = true;
  meter_data[1].f = &Roboto_Mono_Bold_16;
  meter_data[1].color = COLOR_YELLOW;
  meter_data[1].x = 10;
  meter_data[1].y = 120;
  meter_data[2].text = ""; // Actual Voltage
  meter_data[2].changed = true;
  meter_data[2].f = &Roboto_Mono_Bold_16;
  meter_data[2].color = COLOR_YELLOW;
  meter_data[2].x = 139;
  meter_data[2].y = 80;
  meter_data[3].text = ""; // Actual Current
  meter_data[3].changed = true;
  meter_data[3].f = &Roboto_Mono_Bold_16;
  meter_data[3].color = COLOR_YELLOW;
  meter_data[3].x = 139;
  meter_data[3].y = 120;
  }

void loop()
  {
  double sensorData[MUX_74HC4051_SIZE] = {0.0, 0.0, 0,  0};
  //                                       V    C  vSW cSW
  int average;
  double total;
  static uint16_t cSelect = 0, vSelect = 0;

  for (byte sensorCount = 0; sensorCount < MUX_74HC4051_SIZE; sensorCount++)
    {
    selectChannel(sensorCount);
    total = 0.0;

    for (int i = 0; i < NUM_READS; i++)
      {
      average = analogRead(ADC_PIN);
      total += average;
      delay(2);
      }

    average = total / NUM_READS;
#if DEBUG == true
      SerialUSB.print("ADC Average (");
      SerialUSB.print(sensorCount);
      SerialUSB.print("): ");
      SerialUSB.println(average);
#endif

    if (sensorCount == 0) // Actual Voltage
      {
      // For zero reference use 125 and (resConvert / 26 [max voltage swing -1 to 25])
      sensorData[sensorCount] = ((average - 125) / (resConvert / 26));
      setChanged(meter_data, String("") + sensorData[sensorCount] + 'V', 2);
      }
    else if (sensorCount == 1) // Actual Current
      {
      // For zero reference use 1933 and (35.67567 = 3.3V/0.185A*2).
      sensorData[sensorCount] = ((average - 1933) * (35.67567 / resConvert));
      setChanged(meter_data, String("") + sensorData[sensorCount] + 'A', 3);
#if DEBUG == true
      //SerialUSB.print("  Current: ");
      //SerialUSB.println(sensorData[sensorCount]);
#endif
      }
    else if (sensorCount == 2) // Voltage Switch
      {
      // Read three states then loop.
      vSelect = incSelectCount(average, vSelect, 3);
      setVoltage(vSelect);
      }
    else if (sensorCount == 3) // Current Switch
      {
      // Read two states then loop.
      cSelect = incSelectCount(average, cSelect, 2);
      setCurrent(cSelect);
      }
#if DEBUG == true
    else
      {
      SerialUSB.print("Invalid sensorCount: ");
      SerialUSB.println(sensorCount);
      }
#endif
    }

  drawData(DEFAULT_METER_COLOR);
  delay(400);
  }

void selectChannel(byte counter)
  {
  bool S0 = bool(counter >> 0 & 0x01);
  bool S1 = bool(counter >> 1 & 0x01);
  bool S2 = bool(counter >> 2 & 0x01);

  digitalWrite(PIN_S0, S0);
  digitalWrite(PIN_S1, S1);
  digitalWrite(PIN_S2, S2);
  }

void setChanged(MeterDataType *data, String sensor, uint16_t cnt)
  {
  if (data[cnt].text == sensor)
    {
    data[cnt].changed = false;
    }
  else
    {
    meter_data[cnt].changed = true;
    meter_data[cnt].old_text = data[cnt].text;
    }

   meter_data[cnt].text = sensor;
  }

void drawData(uint16_t color)
  {
  int16_t y, w, h;

  for (uint16_t i = 0; i < NUM_ITEMS(meter_data); i++)
    {
    if (meter_data[i].changed)
      {
      // Set current font
      tft.setGFXFont(meter_data[i].f);
      // Get string extents
      tft.getGFXTextExtent(meter_data[i].old_text, meter_data[i].x,
                           meter_data[i].y, &w, &h);
      y = meter_data[i].y + h;
      tft.drawGFXText(meter_data[i].x, y, meter_data[i].old_text,
                      DEFAULT_BACKGROUND_COLOR);
      // Print string
      tft.getGFXTextExtent(meter_data[i].text, meter_data[i].x,
                           meter_data[i].y, &w, &h);
      y = meter_data[i].y + h;
      tft.drawGFXText(meter_data[i].x, y, meter_data[i].text, color);
      }
    //else
    //  {
    //  SerialUSB.println(meter_data[i].text);
    //  }
    }
  }

uint16_t incSelectCount(int average, uint16_t select, uint16_t max)
  {
  if (average >= 3000)
    {
    uint16_t count = select + 1;
    select = (count < max) ? count : 0;
#if DEBUG == true
    SerialUSB.print("Average: (");
    SerialUSB.print(max == 2 ? "I): " : "V): ");
    SerialUSB.print(average);
    SerialUSB.print(", Select: ");
    SerialUSB.println(select);
#endif
    }

  return select;
  }

void setVoltage(uint16_t vSelect)
  {
  static String voltages[] = {"12.6V", "16.8V", "21.0V"};
  setChanged(meter_data, voltages[vSelect], 0);

  }

void setCurrent(uint16_t cSelect)
  {
  static String status[] = {"1250mA", "ADJUSTABLE"};
  setChanged(meter_data, status[cSelect], 1);
  //SerialUSB.print("Old test: ");
  //SerialUSB.print(meter_data[1].old_text);
  //SerialUSB.print(", Current text: ");
  //SerialUSB.print(meter_data[1].text);
  //SerialUSB.print(", changed: ");
  //SerialUSB.println(meter_data[1].changed);

  }
