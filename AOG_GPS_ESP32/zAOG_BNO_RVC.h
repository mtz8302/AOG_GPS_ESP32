/*!
 * 	This is a library for use with thethe Adafruit BNO08x breakout:
 * 	https://www.adafruit.com/products/4754
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 */

#ifndef _BNO_rvc_H
#define _BNO_rvc_H

constexpr auto MILLI_G_TO_MS2 = 0.0098067; ///< Scalar to convert milli-gs to m/s^2;
constexpr auto DEGREE_SCALE = 0.1;        ///< To convert the degree values

#include "Arduino.h"

typedef struct BNO_rvcData {
    int16_t yawX10,     ///< Yaw in Degrees x 10
        pitchX10,     ///< Pitch in Degrees x 10
        rollX10,     ///< Roll in Degrees x 10
        yawX100,      // yaw in original x100
        angVel;         //running total of angular velocity
} BNO_rvcData;


class BNO_rvc {
public:
  BNO_rvc();
  ~BNO_rvc();

  bool begin(Stream *theStream);
  bool read(BNO_rvcData *heading);

  uint32_t angCounter;

private:
  Stream *serial_dev;
  int16_t prevYAw;
};

#endif