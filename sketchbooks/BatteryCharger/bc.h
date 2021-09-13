/*
 * Battery Charger Header
 */

#ifndef  _BC_H
#define  _BC_H

#ifndef TFT_22_ILI9225_h
  #include <TFT_22_ILI9225.h>
#endif

typedef struct
  {
  String text;
  const GFXfont *f;
  uint16_t color;
  int16_t x1;
  int16_t y1;
  int16_t x2;
  int16_t y2;
  } MeterHeaderType;

typedef struct
  {
  String text;
  String old_text;
  bool changed;
  const GFXfont *f;
  uint16_t color;
  int16_t x;
  int16_t y;
  } MeterDataType;

/*
 * Configured for the Seeeduino XIAO
 */
#define TFT_RST 1  // D1
#define TFT_RS  2  // D3
#define TFT_CS  3  // D2
#define TFT_SDI 10 // MOSI
#define TFT_CLK 8  // SCK
#define TFT_LED 0  // D0 -- NC if wired to +3.3V internally.

#define PIN_S0  4  // D4
#define PIN_S1  5  // D5
#define PIN_S2  6  // D6
#define ADC_PIN 7  // A7 (ADC pin)

#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)
#define DEFAULT_METER_COLOR COLOR_GOLD
#define DEFAULT_BACKGROUND_COLOR COLOR_BLACK
#define ERROR_COLOR COLOR_RED

#define NUM_ITEMS(a) (sizeof(a) / sizeof(*a))

#define HEADER_STRUCT_SIZE 4
#define METER_DATA_ARRAY_SIZE 4
// MUX_74HC4051_SIZE -- Voltage, Current, and a switch to
// set voltage and current values.
#define MUX_74HC4051_SIZE 4
#define NUM_READS 200

/*
 * Prototypes
 */
void populateDisplays(void);
void drawHeaders(void);
void initialPopulateData(void);
void drawData(uint16_t color);
void selectChannel(byte counter);
void setChanged(MeterDataType *data, String sensor, uint16_t cn);
uint16_t incSelectCount(uint16_t average, uint16_t select, uint16_t max);
void setVoltage(uint16_t vSelect);
void setCurrent(uint16_t cSelect);

#endif /* _BC_H */
