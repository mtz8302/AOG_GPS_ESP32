
#include "Arduino.h"
#include <Wire.h>

#include "zAOG_BNO_RVC.h"

//constructor destructor
BNO_rvc::BNO_rvc(void) {}
BNO_rvc::~BNO_rvc(void) {}

//set the object variables
bool BNO_rvc::begin(Stream *theSerial) {
  serial_dev = theSerial;
  return true;
}

//read the 16 byte sentence AA AA Index Yaw Pitch Roll LSB MSB
bool BNO_rvc::read(BNO_rvcData* bnoData) {
    if (!bnoData) return false;

    if (!serial_dev->available()) return false;

    if (serial_dev->peek() != 0xAA)
    {
        serial_dev->read();
        return false;
    }

    // Now read all 19 bytes
    if (serial_dev->available() < 19) return false;

    // at this point we know there's at least 19 bytes available and the first is AA
    if (serial_dev->read() != 0xAA) return false;

    // make sure the next byte is the second 0xAA
    if (serial_dev->read() != 0xAA) return false;

    uint8_t buffer[19];
    if (!serial_dev->readBytes(buffer, 17)) return false;

    // get checksum ready
    uint8_t sum = 0;
    for (uint8_t i = 0; i < 16; i++) sum += buffer[i];

    if (sum != buffer[16]) return false;

    //clean out any remaining bytes in case teensy was busy
    //while (serial_dev->available() > 0) serial_dev->read();

    int16_t temp;

    if (angCounter < 20)
    {
        temp = buffer[1] + (buffer[2] << 8);
        bnoData->yawX100 = temp; //For angular velocity calc
        bnoData->angVel += (temp - prevYAw);
        angCounter++;
        prevYAw = temp;
    }
    else
    {
        angCounter = 0;
        prevYAw = bnoData->angVel = 0;
        temp = 0;
    }

    bnoData->yawX10 = (int16_t)((float)temp * DEGREE_SCALE);
    if (bnoData->yawX10 < 0) bnoData->yawX10 +=3600;

    temp = buffer[3] + (buffer[4] << 8);
    bnoData->pitchX10 = (int16_t)((float)temp * DEGREE_SCALE);

    temp = buffer[5] + (buffer[6] << 8);
    bnoData->rollX10 = (int16_t)((float)temp * DEGREE_SCALE);

    return true;
}
