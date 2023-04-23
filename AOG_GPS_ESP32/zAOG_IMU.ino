void init_BNO_serial() {
    Serial.println();
    Serial2.begin(115200, SERIAL_8N1, Set.RX2, Set.TX2);// This is the baud rate specified by the datasheet
    while (!Serial2)
        delay(10);
    if (!rvc.begin(&Serial2)) { // connect to the sensor over hardware serial
        Serial.println("Could not find BNO08x in RVC mode at serial pins"); Serial.println();
        Set.IMUType = 0;
        Set.sendIMUPGN = 0;
    }
    else { Serial.println("BNO08x found! Running in serial RVC mode."); Serial.println(); }
}

//-------------------------------------------------------------------------------------------------

void readBNO() {
    //Data is stored to ring buffer
    BNO_rvcData bnoData;
    if (rvc.read(&bnoData))
    {
        byte nextIMUDataRingCount = (IMUDataRingCount + 1) % 10;
        //the heading x10
        if (!bnoData.yawX10 == 0) {
            IMUHeading[nextIMUDataRingCount] = bnoData.yawX10;

            //roll
            if (Set.IMUswapRollPitch) {
                IMURoll[nextIMUDataRingCount] = bnoData.pitchX10;
                IMUPitch[nextIMUDataRingCount] = bnoData.rollX10;
            }
            else {
                IMURoll[nextIMUDataRingCount] = bnoData.rollX10;
                IMUPitch[nextIMUDataRingCount] = bnoData.pitchX10;
            }
            if (Set.IMUinvertRoll) { IMURoll[nextIMUDataRingCount] *= -1; }

            //yaw rate
            IMUYawRate[nextIMUDataRingCount] = bnoData.angVel;

            IMUDataRingCount = nextIMUDataRingCount;
        }
        else {
            IMUnoData++;
        }
    }
    else {
        IMUnoData++;
    }
}

//-------------------------------------------------------------------------------------------------

void init_CMPS14() {
    //init wire for CMPS
    delay(200);
    if (Set.debugmode) { Serial.println("I2C / wire init for CMPS14"); }
    if (!Wire.begin(Set.SDA, Set.SCL, 400000)) {
    //if (!Wire.begin()) {
        Serial.println("I2C init failed, CMPS14 not availble");
        Set.IMUType = 0;
    }
    else {
        delay(200);

        if (Set.debugmode) { Serial.println("Starting wire connection to CMPS14"); }
        Wire.beginTransmission(Set.CMPS14_ADDRESS);
        delay(100);
        byte error = Wire.endTransmission();
        if (Set.debugmode) { Serial.println("wire transmission completed"); }
        if (error == 0)
        {
            if (Set.debugmode) {
                Serial.println("Error = 0");
                Serial.print("CMPS14 ADDRESs: 0x");
                Serial.println(Set.CMPS14_ADDRESS, HEX);
            }
            Serial.print("CMPS14 Ok. Started at I2C adress (hex): ");
            Serial.println(Set.CMPS14_ADDRESS, HEX);
        }
        else
        {
            Serial.println("Error = 4");
            Serial.print("CMPS not Connected or Found at address 0x");
            Serial.println(Set.CMPS14_ADDRESS, HEX);
            Set.IMUType = 0;
            Set.sendIMUPGN = 0;
        }
    }
    Serial.println();
}


void readCMPS() {
    //Data is stored to ring buffer
    byte nextIMUDataRingCount = (IMUDataRingCount + 1) % 10;
    
    Wire.beginTransmission(Set.CMPS14_ADDRESS);
    Wire.write(0x1C);
    Wire.endTransmission();

    Wire.requestFrom(Set.CMPS14_ADDRESS, 3);
    while (Wire.available() < 3);

    IMURoll[nextIMUDataRingCount] = int16_t(Wire.read() << 8 | Wire.read());
    if (Set.IMUinvertRoll) { IMURoll[nextIMUDataRingCount] *= -1; }

    // the heading x10
    Wire.beginTransmission(Set.CMPS14_ADDRESS);
    Wire.write(0x02);
    Wire.endTransmission();

    Wire.requestFrom(Set.CMPS14_ADDRESS, 3);
    while (Wire.available() < 3);

    IMUHeading[nextIMUDataRingCount] = Wire.read() << 8 | Wire.read();

    // 3rd byte pitch
    IMUPitch[nextIMUDataRingCount] = Wire.read();

    IMUDataRingCount = nextIMUDataRingCount;
}

//-------------------------------------------------------------------------------------------------

/*
void do_CMPS_traffic(void* pvParameters) {
    Serial.println("started new task on core 0: CMPS read 20 Hz");
    for (;;) { // MAIN LOOP
        if (Set.IMUType == 2) {
            task_CMPS_reader_running = true;
            readCMPS();
            vTaskDelay(49);
        }//2

        else {
            task_CMPS_reader_running = false;
            delay(1);
            vTaskDelete(NULL);
            delay(1);
        }
    }
}
*/

/*
void init_CMPS_serial() {
    Serial.println();
    Serial2.begin(9600, SERIAL_8N1, Set.SDA, Set.SCL);// This is the baud rate specified by the datasheet
    while (!Serial2)
        delay(10);
    delay(100);
    Serial2.write(0x12);
    delay(50);
    if (Serial2.available()) { // connect to the sensor over hardware serial
        Serial.println("CMPS14 found! Running in serial mode.");
        Serial.println();
        Serial2.read();
    }
    else {
        Serial.println("Could not find CMPS in serial mode at serial2 pins"); Serial.println();
        Set.IMUType = 0;
        Set.sendIMUPGN = 0;
    }
}

void readCMPSserial() {
    //Data is stored to ring buffer
    byte nextIMUDataRingCount = (IMUDataRingCount + 1) % 10;

    Serial2.write(0x12);

    unsigned long timeout = millis() + 100;
    while (Serial2.available() < 3) {
        now = millis();
        vTaskDelay(1);
        if (now > timeout) { Serial.println(Serial2.available()); Serial.print(millis()); Serial.print(" "); Serial.println(timeout); Serial.println("timeout reading CMPS14 serial"); break; }
    }

    IMUHeading[nextIMUDataRingCount] = int16_t(Serial2.read() << 8 | Serial2.read());

    IMUPitch[nextIMUDataRingCount] = Serial2.read();

    IMURoll[nextIMUDataRingCount] = Serial2.read();
    if (Set.IMUinvertRoll) { IMURoll[nextIMUDataRingCount] *= -1; }

    IMUDataRingCount = nextIMUDataRingCount;
    IMUnextReadTime = millis() + IMUCMPSreadTime;//read every 50ms
}
*/

