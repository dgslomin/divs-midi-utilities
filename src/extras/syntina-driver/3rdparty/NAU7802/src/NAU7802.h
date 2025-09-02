/*
  This is an Arduino library written for the NAU7802 24-bit wheatstone
  bridge and load cell amplifier.
  By Nathan Seidle @ SparkFun Electronics, March 3nd, 2019

  The NAU7802 is an I2C device that converts analog signals to a 24-bit
  digital signal. This makes it possible to create your own digital scale
  either by hacking an off-the-shelf bathroom scale or by creating your
  own scale using a load cell.

  The NAU7802 is a better version of the popular HX711 load cell amplifier.
  It uses a true I2C interface so that it can share the bus with other
  I2C devices while still taking very accurate 24-bit load cell measurements
  up to 320Hz.

  https://github.com/sparkfun/SparkFun_NAU7802_Scale_Arduino_Library

  SparkFun labored with love to create this code. Feel like supporting open
  source? Buy a board from SparkFun!
  https://www.sparkfun.com/products/15242
*/

#ifndef _NAU7802_Library_h
#define _NAU7802_Library_h

#ifdef __cplusplus
extern "C" {
#endif

//Register Map
typedef enum
{
  NAU7802_PU_CTRL = 0x00,
  NAU7802_CTRL1,
  NAU7802_CTRL2,
  NAU7802_OCAL1_B2,
  NAU7802_OCAL1_B1,
  NAU7802_OCAL1_B0,
  NAU7802_GCAL1_B3,
  NAU7802_GCAL1_B2,
  NAU7802_GCAL1_B1,
  NAU7802_GCAL1_B0,
  NAU7802_OCAL2_B2,
  NAU7802_OCAL2_B1,
  NAU7802_OCAL2_B0,
  NAU7802_GCAL2_B3,
  NAU7802_GCAL2_B2,
  NAU7802_GCAL2_B1,
  NAU7802_GCAL2_B0,
  NAU7802_I2C_CONTROL,
  NAU7802_ADCO_B2,
  NAU7802_ADCO_B1,
  NAU7802_ADCO_B0,
  NAU7802_ADC = 0x15, //Shared ADC and OTP 32:24
  NAU7802_OTP_B1,     //OTP 23:16 or 7:0?
  NAU7802_OTP_B0,     //OTP 15:8
  NAU7802_PGA = 0x1B,
  NAU7802_PGA_PWR = 0x1C,
  NAU7802_DEVICE_REV = 0x1F,
} Scale_Registers;

//Bits within the PU_CTRL register
typedef enum
{
  NAU7802_PU_CTRL_RR = 0,
  NAU7802_PU_CTRL_PUD,
  NAU7802_PU_CTRL_PUA,
  NAU7802_PU_CTRL_PUR,
  NAU7802_PU_CTRL_CS,
  NAU7802_PU_CTRL_CR,
  NAU7802_PU_CTRL_OSCS,
  NAU7802_PU_CTRL_AVDDS,
} PU_CTRL_Bits;

//Bits within the CTRL1 register
typedef enum
{
  NAU7802_CTRL1_GAIN = 2,
  NAU7802_CTRL1_VLDO = 5,
  NAU7802_CTRL1_DRDY_SEL = 6,
  NAU7802_CTRL1_CRP = 7,
} CTRL1_Bits;

//Bits within the CTRL2 register
typedef enum
{
  NAU7802_CTRL2_CALMOD = 0,
  NAU7802_CTRL2_CALS = 2,
  NAU7802_CTRL2_CAL_ERROR = 3,
  NAU7802_CTRL2_CRS = 4,
  NAU7802_CTRL2_CHS = 7,
} CTRL2_Bits;

//Bits within the PGA register
typedef enum
{
  NAU7802_PGA_CHP_DIS = 0,
  NAU7802_PGA_INV = 3,
  NAU7802_PGA_BYPASS_EN,
  NAU7802_PGA_OUT_EN,
  NAU7802_PGA_LDOMODE,
  NAU7802_PGA_RD_OTP_SEL,
} PGA_Bits;

//Bits within the PGA PWR register
typedef enum
{
  NAU7802_PGA_PWR_PGA_CURR = 0,
  NAU7802_PGA_PWR_ADC_CURR = 2,
  NAU7802_PGA_PWR_MSTR_BIAS_CURR = 4,
  NAU7802_PGA_PWR_PGA_CAP_EN = 7,
} PGA_PWR_Bits;

//Allowed Low drop out regulator voltages
typedef enum
{
  NAU7802_LDO_2V4 = 0b111,
  NAU7802_LDO_2V7 = 0b110,
  NAU7802_LDO_3V0 = 0b101,
  NAU7802_LDO_3V3 = 0b100,
  NAU7802_LDO_3V6 = 0b011,
  NAU7802_LDO_3V9 = 0b010,
  NAU7802_LDO_4V2 = 0b001,
  NAU7802_LDO_4V5 = 0b000,
} NAU7802_LDO_Values;

//Allowed gains
typedef enum
{
  NAU7802_GAIN_128 = 0b111,
  NAU7802_GAIN_64 = 0b110,
  NAU7802_GAIN_32 = 0b101,
  NAU7802_GAIN_16 = 0b100,
  NAU7802_GAIN_8 = 0b011,
  NAU7802_GAIN_4 = 0b010,
  NAU7802_GAIN_2 = 0b001,
  NAU7802_GAIN_1 = 0b000,
} NAU7802_Gain_Values;

//Allowed samples per second
typedef enum
{
  NAU7802_SPS_320 = 0b111,
  NAU7802_SPS_80 = 0b011,
  NAU7802_SPS_40 = 0b010,
  NAU7802_SPS_20 = 0b001,
  NAU7802_SPS_10 = 0b000,
} NAU7802_SPS_Values;

//Select between channel values
typedef enum
{
  NAU7802_CHANNEL_1 = 0,
  NAU7802_CHANNEL_2 = 1,
} NAU7802_Channels;

//Calibration state
typedef enum
{
  NAU7802_CAL_SUCCESS = 0,
  NAU7802_CAL_IN_PROGRESS = 1,
  NAU7802_CAL_FAILURE = 2,
} NAU7802_Cal_Status;

struct NAU7802;
typedef struct NAU7802* NAU7802_t;

NAU7802_t NAU7802_new(uint8_t i2c_bus, uint8_t i2c_addr /* = 0x2A */);                                               //Default constructor
void NAU7802_free(NAU7802_t nau7802);                                              //Default destructor
bool NAU7802_begin(NAU7802_t nau7802, bool initialize /* = true */);      // Check communication and initialize sensor
bool NAU7802_isConnected(NAU7802_t nau7802);                                      //Returns true if device acks at the I2C address

bool NAU7802_available(NAU7802_t nau7802);                          //Returns true if Cycle Ready bit is set (conversion is complete)
int32_t NAU7802_getReading(NAU7802_t nau7802);                      //Returns 24-bit reading. Assumes CR Cycle Ready bit (ADC conversion complete) has been checked by .available()
int32_t NAU7802_getAverage(NAU7802_t nau7802, uint8_t samplesToTake); //Return the average of a given number of readings

void NAU7802_calculateZeroOffset(NAU7802_t nau7802, uint8_t averageAmount /* = 8 */); //Also called taring. Call this with nothing on the scale
void NAU7802_setZeroOffset(NAU7802_t nau7802, int32_t newZeroOffset);           //Sets the internal variable. Useful for users who are loading values from NVM.
int32_t NAU7802_getZeroOffset(NAU7802_t nau7802);                             //Ask library for this value. Useful for storing value into NVM.

void NAU7802_calculateCalibrationFactor(NAU7802_t nau7802, float weightOnScale, uint8_t averageAmount /* = 8 */); //Call this with the value of the thing on the scale. Sets the calibration factor based on the weight on scale and zero offset.
void NAU7802_setCalibrationFactor(NAU7802_t nau7802, float calFactor);                                      //Pass a known calibration factor into library. Helpful if users is loading settings from NVM.
float NAU7802_getCalibrationFactor(NAU7802_t nau7802);                                                    //Ask library for this value. Useful for storing value into NVM.

float NAU7802_getWeight(NAU7802_t nau7802, bool allowNegativeWeights /* = false */, uint8_t samplesToTake /* = 8 */); //Once you've set zero offset and cal factor, you can ask the library to do the calculations for you.

bool NAU7802_setGain(NAU7802_t nau7802, uint8_t gainValue);        //Set the gain. x1, 2, 4, 8, 16, 32, 64, 128 are available
bool NAU7802_setLDO(NAU7802_t nau7802, uint8_t ldoValue);          //Set the onboard Low-Drop-Out voltage regulator to a given value. 2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.2, 4.5V are avaialable
bool NAU7802_setSampleRate(NAU7802_t nau7802, uint8_t rate);       //Set the readings per second. 10, 20, 40, 80, and 320 samples per second is available
bool NAU7802_setChannel(NAU7802_t nau7802, uint8_t channelNumber); //Select between 1 and 2

bool NAU7802_calibrateAFE(NAU7802_t nau7802);                               //Synchronous calibration of the analog front end of the NAU7802. Returns true if CAL_ERR bit is 0 (no error)
void NAU7802_beginCalibrateAFE(NAU7802_t nau7802);                          //Begin asynchronous calibration of the analog front end of the NAU7802. Poll for completion with calAFEStatus() or wait with waitForCalibrateAFE().
bool NAU7802_waitForCalibrateAFE(NAU7802_t nau7802, uint32_t timeout_ms /* = 0 */); //Wait for asynchronous AFE calibration to complete with optional timeout.
NAU7802_Cal_Status NAU7802_calAFEStatus(NAU7802_t nau7802);                 //Check calibration status.

bool NAU7802_reset(NAU7802_t nau7802); //Resets all registers to Power Of Defaults

bool NAU7802_powerUp(NAU7802_t nau7802);   //Power up digital and analog sections of scale, ~2mA
bool NAU7802_powerDown(NAU7802_t nau7802); //Puts scale into low-power 200nA mode

bool NAU7802_setIntPolarityHigh(NAU7802_t nau7802); //Set Int pin to be high when data is ready (default)
bool NAU7802_setIntPolarityLow(NAU7802_t nau7802);  //Set Int pin to be low when data is ready

uint8_t NAU7802_getRevisionCode(NAU7802_t nau7802); //Get the revision code of this IC. Always 0x0F.

bool NAU7802_setBit(NAU7802_t nau7802, uint8_t bitNumber, uint8_t registerAddress);   //Mask & set a given bit within a register
bool NAU7802_clearBit(NAU7802_t nau7802, uint8_t bitNumber, uint8_t registerAddress); //Mask & clear a given bit within a register
bool NAU7802_getBit(NAU7802_t nau7802, uint8_t bitNumber, uint8_t registerAddress);   //Return a given bit within a register

uint8_t NAU7802_getRegister(NAU7802_t nau7802, uint8_t registerAddress);             //Get contents of a register
bool NAU7802_setRegister(NAU7802_t nau7802, uint8_t registerAddress, uint8_t value); // Send a given value to be written to given address. Return true if successful
unsigned long NAU7802_millis(NAU7802_t nau7802);
unsigned long NAU7802_micros(NAU7802_t nau7802);

#ifdef __cplusplus
}
#endif

#endif
