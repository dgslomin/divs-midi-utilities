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

  https://github.com/sparkfun/SparkFun_Qwiic_Scale_NAU7802_Arduino_Library

  SparkFun labored with love to create this code. Feel like supporting open
  source? Buy a board from SparkFun!
  https://www.sparkfun.com/products/15242
*/

#include <i2c/smbus.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "NAU7802.h"

struct NAU7802
{
  int fd;
  uint8_t buffer[3];
  uint8_t i2c_bus;  //I2C bus for NaU7802
  uint8_t i2c_addr; // Default unshifted 7-bit address of the NAU7802
  // y = mx+b
  int32_t _zeroOffset;      // This is b
  float _calibrationFactor; // This is m. User provides this number so that we can output y when requested
  struct timespec refTime;
};

//Constructor
NAU7802_t NAU7802_new(uint8_t i2c_bus, uint8_t i2c_addr)
{
    NAU7802_t nau7802 = (NAU7802_t)(malloc(sizeof(struct NAU7802)));
    nau7802->i2c_addr = i2c_addr;
    nau7802->i2c_bus = i2c_bus;
	clock_gettime(CLOCK_MONOTONIC_RAW, &(nau7802->refTime));
    return nau7802;
}

//Sets up the NAU7802 for basic function
//If initialize is true (or not specified), default init and calibration is performed
//If initialize is false, then it's up to the caller to initalize and calibrate
//Returns true upon completion
bool NAU7802_begin(NAU7802_t nau7802, bool initialize)
{
    char device[32];
    snprintf(device, sizeof(device), "/dev/i2c-%u", nau7802->i2c_bus); // creating device address buffer
    if ((nau7802->fd = open(device, O_RDWR)) < 0)
    {
        printf("File descriptor opening error %s\n", strerror(errno));
        return 0;
    }
    else {
        if (ioctl(nau7802->fd, I2C_SLAVE, nau7802->i2c_addr) < 0)
        {
            printf("Open fd error %d\n", errno);
            return 0;
        }
        printf("I2C connection established\n");
    }

    // Check if the device ack's over I2C
    if (!NAU7802_isConnected(nau7802))
    {
        // There are rare times when the sensor is occupied and doesn't ack. A 2nd try resolves this.
        if (!NAU7802_isConnected(nau7802))
            return (false);
    }

    bool result = true; // Accumulate a result as we do the setup

    if (initialize)
    {
        result &= NAU7802_reset(nau7802); // Reset all registers

        result &= NAU7802_powerUp(nau7802); // Power on analog and digital sections of the scale

        result &= NAU7802_setLDO(nau7802, NAU7802_LDO_3V3); // Set LDO to 3.3V

        result &= NAU7802_setGain(nau7802, NAU7802_GAIN_128); // Set gain to 128

        result &= NAU7802_setSampleRate(nau7802, NAU7802_SPS_80); // Set samples per second to 10

        result &= NAU7802_setRegister(nau7802, NAU7802_ADC, 0x30); // Turn off CLK_CHP. From 9.1 power on sequencing.

        result &= NAU7802_setBit(nau7802, NAU7802_PGA_PWR_PGA_CAP_EN,
                         NAU7802_PGA_PWR); // Enable 330pF decoupling cap on chan 2. From 9.14 application circuit note.

        result &= NAU7802_calibrateAFE(nau7802); // Re-cal analog front end when we change gain, sample rate, or channel
    }

    return (result);
}

//Returns true if device is present
//Tests for device ack to I2C address
bool NAU7802_isConnected(NAU7802_t nau7802)
{
   if (ioctl(nau7802->fd, I2C_SLAVE, nau7802->i2c_addr) < 0) {
        printf("Error While Opening I2C connection : 3, Error Number: %d\n", errno);
        return 0; // Sensor did not ACK
   }
   return 1;  // All good
}

//Returns true if Cycle Ready bit is set (conversion is complete)
bool NAU7802_available(NAU7802_t nau7802)
{
  return (NAU7802_getBit(nau7802, NAU7802_PU_CTRL_CR, NAU7802_PU_CTRL));
}

//Calibrate analog front end of system. Returns true if CAL_ERR bit is 0 (no error)
//Takes approximately 344ms to calibrate; wait up to 1000ms.
//It is recommended that the AFE be re-calibrated any time the gain, SPS, or channel number is changed.
bool NAU7802_calibrateAFE(NAU7802_t nau7802)
{
  NAU7802_beginCalibrateAFE(nau7802);
  return NAU7802_waitForCalibrateAFE(nau7802, 1000);
}

//Begin asynchronous calibration of the analog front end.
// Poll for completion with calAFEStatus() or wait with waitForCalibrateAFE()
void NAU7802_beginCalibrateAFE(NAU7802_t nau7802)
{
  NAU7802_setBit(nau7802, NAU7802_CTRL2_CALS, NAU7802_CTRL2);
}

//Check calibration status.
NAU7802_Cal_Status NAU7802_calAFEStatus(NAU7802_t nau7802)
{
  if (NAU7802_getBit(nau7802, NAU7802_CTRL2_CALS, NAU7802_CTRL2))
  {
    return NAU7802_CAL_IN_PROGRESS;
  }

  if (NAU7802_getBit(nau7802, NAU7802_CTRL2_CAL_ERROR, NAU7802_CTRL2))
  {
    return NAU7802_CAL_FAILURE;
  }

  // Calibration passed
  return NAU7802_CAL_SUCCESS;
}

//Wait for asynchronous AFE calibration to complete with optional timeout.
//If timeout is not specified (or set to 0), then wait indefinitely.
//Returns true if calibration completes succsfully, otherwise returns false.
bool NAU7802_waitForCalibrateAFE(NAU7802_t nau7802, uint32_t timeout_ms)
{
  uint32_t begin = NAU7802_millis(nau7802);
  NAU7802_Cal_Status cal_ready;

  while ((cal_ready = NAU7802_calAFEStatus(nau7802)) == NAU7802_CAL_IN_PROGRESS)
  {
    if ((timeout_ms > 0) && ((NAU7802_millis(nau7802) - begin) > timeout_ms))
    {
      break;
    }
    usleep(1E3);
  }

  if (cal_ready == NAU7802_CAL_SUCCESS)
  {
    return (true);
  }
  return (false);
}

//Set the readings per second
//10, 20, 40, 80, and 320 samples per second is available
bool NAU7802_setSampleRate(NAU7802_t nau7802, uint8_t rate)
{
  if (rate > 0b111)
    rate = 0b111; //Error check

  uint8_t value = NAU7802_getRegister(nau7802, NAU7802_CTRL2);
  value &= 0b10001111; //Clear CRS bits
  value |= rate << 4;  //Mask in new CRS bits

  return (NAU7802_setRegister(nau7802, NAU7802_CTRL2, value));
}

//Select between 1 and 2
bool NAU7802_setChannel(NAU7802_t nau7802, uint8_t channelNumber)
{
  if (channelNumber == NAU7802_CHANNEL_1)
    return (NAU7802_clearBit(nau7802, NAU7802_CTRL2_CHS, NAU7802_CTRL2)); //Channel 1 (default)
  else
    return (NAU7802_setBit(nau7802, NAU7802_CTRL2_CHS, NAU7802_CTRL2)); //Channel 2
}

//Power up digital and analog sections of scale
bool NAU7802_powerUp(NAU7802_t nau7802)
{
  NAU7802_setBit(nau7802, NAU7802_PU_CTRL_PUD, NAU7802_PU_CTRL);
  NAU7802_setBit(nau7802, NAU7802_PU_CTRL_PUA, NAU7802_PU_CTRL);

  //Wait for Power Up bit to be set - takes approximately 200us
  uint8_t counter = 0;
  while (1)
  {
    if (NAU7802_getBit(nau7802, NAU7802_PU_CTRL_PUR, NAU7802_PU_CTRL)) {
      break; //Good to go
    }
      usleep(1E3);
    if (counter++ > 100) {
      return (false); //Error
    }
  }
  return (true);
}

//Puts scale into low-power mode
bool NAU7802_powerDown(NAU7802_t nau7802)
{
  NAU7802_clearBit(nau7802, NAU7802_PU_CTRL_PUD, NAU7802_PU_CTRL);
  return (NAU7802_clearBit(nau7802, NAU7802_PU_CTRL_PUA, NAU7802_PU_CTRL));
}

//Resets all registers to Power Of Defaults
bool NAU7802_reset(NAU7802_t nau7802)
{
  NAU7802_setBit(nau7802, NAU7802_PU_CTRL_RR, NAU7802_PU_CTRL); //Set RR
  usleep(1E3);
  return (NAU7802_clearBit(nau7802, NAU7802_PU_CTRL_RR, NAU7802_PU_CTRL)); //Clear RR to leave reset state
}

//Set the onboard Low-Drop-Out voltage regulator to a given value
//2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.2, 4.5V are available
bool NAU7802_setLDO(NAU7802_t nau7802, uint8_t ldoValue)
{
  if (ldoValue > 0b111)
    ldoValue = 0b111; //Error check

  //Set the value of the LDO
  uint8_t value = NAU7802_getRegister(nau7802, NAU7802_CTRL1);
  value &= 0b11000111;    //Clear LDO bits
  value |= ldoValue << 3; //Mask in new LDO bits
  NAU7802_setRegister(nau7802, NAU7802_CTRL1, value);

  return (NAU7802_setBit(nau7802, NAU7802_PU_CTRL_AVDDS, NAU7802_PU_CTRL)); //Enable the internal LDO
}

//Set the gain
//x1, 2, 4, 8, 16, 32, 64, 128 are avaialable
bool NAU7802_setGain(NAU7802_t nau7802, uint8_t gainValue)
{
  if (gainValue > 0b111)
    gainValue = 0b111; //Error check

  uint8_t value = NAU7802_getRegister(nau7802, NAU7802_CTRL1);
  value &= 0b11111000; //Clear gain bits
  value |= gainValue;  //Mask in new bits

  return (NAU7802_setRegister(nau7802, NAU7802_CTRL1, value));
}

//Get the revision code of this IC
uint8_t NAU7802_getRevisionCode(NAU7802_t nau7802)
{
  uint8_t revisionCode = NAU7802_getRegister(nau7802, NAU7802_DEVICE_REV);
  return (revisionCode & 0x0F);
}

//Returns 24-bit reading
//Assumes CR Cycle Ready bit (ADC conversion complete) has been checked to be 1
int32_t NAU7802_getReading(NAU7802_t nau7802)
{
    uint8_t data[3];
    uint8_t ret;
    // read Current from register
    ret = i2c_smbus_read_i2c_block_data(nau7802->fd, NAU7802_ADCO_B2, sizeof(data), data);
    // data[0] contains the length of the data
    if (ret > 1) // number of bytes that were read
    {
        uint32_t valueRaw = (uint32_t)data[0] << 16; // MSB
        valueRaw |= (uint32_t)data[1] << 8;          // MidSB
        valueRaw |= (uint32_t)data[2];               // LSB
        // the raw value coming from the ADC is a 24-bit number, so the sign bit now
        // resides on bit 23 (0 is LSB) of the uint32_t container. By shifting the
        // value to the left, I move the sign bit to the MSB of the uint32_t container.
        // By casting to a signed int32_t container I now have properly recovered
        // the sign of the original value
        int32_t valueShifted = (int32_t)(valueRaw << 8);
        // shift the number back right to recover its intended magnitude
        int32_t value = (valueShifted >> 8);

        return (value);
    }

    return (0); // Error
}

//Return the average of a given number of readings
//Gives up after 1000ms so don't call this function to average 8 samples setup at 1Hz output (requires 8s)
int32_t NAU7802_getAverage(NAU7802_t nau7802, uint8_t averageAmount)
{
  long total = 0;
  uint8_t samplesAquired = 0;

  unsigned long startTime = NAU7802_millis(nau7802);
  while (1)
  {
    if (NAU7802_available(nau7802))
    {
      total += NAU7802_getReading(nau7802);
      if (++samplesAquired == averageAmount)
        break; //All done
    }
    if (NAU7802_millis(nau7802) - startTime > 1000) {
      return (0); //Timeout - Bail with error
    }
      usleep(1E3);
  }
  total /= averageAmount;

  return (total);
}

//Call when scale is setup, level, at running temperature, with nothing on it
void NAU7802_calculateZeroOffset(NAU7802_t nau7802, uint8_t averageAmount)
{
  NAU7802_setZeroOffset(nau7802, NAU7802_getAverage(nau7802, averageAmount));
}

//Sets the internal variable. Useful for users who are loading values from NVM.
void NAU7802_setZeroOffset(NAU7802_t nau7802, int32_t newZeroOffset)
{
  nau7802->_zeroOffset = newZeroOffset;
}

int32_t NAU7802_getZeroOffset(NAU7802_t nau7802)
{
  return (nau7802->_zeroOffset);
}

//Call after zeroing. Provide the float weight sitting on scale. Units do not matter.
void NAU7802_calculateCalibrationFactor(NAU7802_t nau7802, float weightOnScale, uint8_t averageAmount)
{
  int32_t onScale = NAU7802_getAverage(nau7802, averageAmount);
  float newCalFactor = (onScale - nau7802->_zeroOffset) / (float)weightOnScale;
  NAU7802_setCalibrationFactor(nau7802, newCalFactor);
}

//Pass a known calibration factor into library. Helpful if users is loading settings from NVM.
//If you don't know your cal factor, call setZeroOffset(), then calculateCalibrationFactor() with a known weight
void NAU7802_setCalibrationFactor(NAU7802_t nau7802, float newCalFactor)
{
  nau7802->_calibrationFactor = newCalFactor;
}

float NAU7802_getCalibrationFactor(NAU7802_t nau7802)
{
  return (nau7802->_calibrationFactor);
}

//Returns the y of y = mx + b using the current weight on scale, the cal factor, and the offset.
float NAU7802_getWeight(NAU7802_t nau7802, bool allowNegativeWeights, uint8_t samplesToTake)
{
  int32_t onScale = NAU7802_getAverage(nau7802, samplesToTake);

  //Prevent the current reading from being less than zero offset
  //This happens when the scale is zero'd, unloaded, and the load cell reports a value slightly less than zero value
  //causing the weight to be negative or jump to millions of pounds
  if (!allowNegativeWeights)
  {
    if (onScale < nau7802->_zeroOffset)
      onScale = nau7802->_zeroOffset; //Force reading to zero
  }

  float weight = (onScale - nau7802->_zeroOffset) / nau7802->_calibrationFactor;
  return (weight);
}

//Set Int pin to be high when data is ready (default)
bool NAU7802_setIntPolarityHigh(NAU7802_t nau7802)
{
  return (NAU7802_clearBit(nau7802, NAU7802_CTRL1_CRP, NAU7802_CTRL1)); //0 = CRDY pin is high active (ready when 1)
}

//Set Int pin to be low when data is ready
bool NAU7802_setIntPolarityLow(NAU7802_t nau7802)
{
  return (NAU7802_setBit(nau7802, NAU7802_CTRL1_CRP, NAU7802_CTRL1)); //1 = CRDY pin is low active (ready when 0)
}

//Mask & set a given bit within a register
bool NAU7802_setBit(NAU7802_t nau7802, uint8_t bitNumber, uint8_t registerAddress)
{
  uint8_t value = NAU7802_getRegister(nau7802, registerAddress);
  value |= (1 << bitNumber); //Set this bit
  return (NAU7802_setRegister(nau7802, registerAddress, value));
}

//Mask & clear a given bit within a register
bool NAU7802_clearBit(NAU7802_t nau7802, uint8_t bitNumber, uint8_t registerAddress)
{
  uint8_t value = NAU7802_getRegister(nau7802, registerAddress);
  value &= ~(1 << bitNumber); //Set this bit
  return (NAU7802_setRegister(nau7802, registerAddress, value));
}

//Return a given bit within a register
bool NAU7802_getBit(NAU7802_t nau7802, uint8_t bitNumber, uint8_t registerAddress)
{
  uint8_t value = NAU7802_getRegister(nau7802, registerAddress);
  value &= (1 << bitNumber); //Clear all but this bit
  return (value);
}

//Get contents of a register
uint8_t NAU7802_getRegister(NAU7802_t nau7802, uint8_t registerAddress)
{
    int32_t retVal;
    retVal = i2c_smbus_read_byte_data(nau7802->fd, registerAddress);
    if (retVal < 0) {
        printf("Error While reading Nau7802 I2C register, Error: %d\n", errno);
        return 0;
    }
    else {
        return (uint8_t)retVal;
    }
}

//Send a given value to be written to given address
//Return true if successful
bool NAU7802_setRegister(NAU7802_t nau7802, uint8_t registerAddress, uint8_t value)
{
    int32_t retVal;
    retVal = i2c_smbus_write_byte_data(nau7802->fd, registerAddress, value);
    if (retVal < 0) {
        printf("Error While setting Nau7802 I2C register, Error#: %d\n", errno);
        return 0;
    }

    return 1;
}

unsigned long NAU7802_millis(NAU7802_t nau7802) {
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC_RAW, &now);
	unsigned long value_ms = (now.tv_sec - nau7802->refTime.tv_sec) * 1000 + (now.tv_nsec - nau7802->refTime.tv_nsec) / 1000000;
    return value_ms;
}

unsigned long NAU7802_micros(NAU7802_t nau7802) {
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC_RAW, &now);
	unsigned long value_us = (now.tv_sec - nau7802->refTime.tv_sec) * 1000000 + (now.tv_nsec - nau7802->refTime.tv_nsec) / 1000;
    return value_us;
}

void NAU7802_free(NAU7802_t nau7802)
{
    close(nau7802->fd);
    free(nau7802);
}

