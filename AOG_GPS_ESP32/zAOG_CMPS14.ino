void init_BNO_serial() {
    Serial.println();
    Serial2.begin(115200, SERIAL_8N1, Set.RX2, Set.TX2);// This is the baud rate specified by the datasheet
    while (!Serial2)
        delay(10);

    if (!rvc.begin(&Serial2)) { // connect to the sensor over hardware serial
        Serial.println("Could not find BNO08x in RVC mode at serial pins");
        Set.IMUType = 0;
        Set.sendIMUPGN = 0;
    }
    else { Serial.println("BNO08x found! Running in serial RVC mode."); }
}

//-------------------------------------------------------------------------------------------------

void init_CMPS14() {
    //init wire for CMPS
    delay(100);
    //if (!Wire.begin(Set.SDA, Set.SCL, 400000)) {
    if (Set.debugmode) { Serial.println("Init I2C = wire connection"); }
    if (!Wire.begin()) {
        Serial.println("error INIT wire, CMPS14 will not work");
        Set.IMUType = 0;
    }
    else {
        delay(50);

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
                Serial.println("CMPS14 Ok.");
            }
            xTaskCreate(do_CMPS_traffic, "Core1CMPSHandle", 3072, NULL, 1, &taskHandle_CMPS14);
            delay(500);
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
}


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

void readCMPS() {

    bool roll_neg;
    unsigned int angle16;
    int roll16;

   // HeadingIMURAW = 0.0;
   // rollIMURAW = 0.0;
    Wire.beginTransmission(Set.CMPS14_ADDRESS);  //starts communication with CMPS14
    Wire.write(0x02);                     //Sends the register we wish to start reading from
    Wire.endTransmission(Set.CMPS14_ADDRESS);

    // Request 2 bytes from the CMPS14
    // this will give us the 8 bit bearing,
    // both bytes of the 16 bit bearing, pitch and roll
    Wire.requestFrom(Set.CMPS14_ADDRESS, 2);

    if (Wire.available() <= 2)     // Wait for all bytes to come back
    {
        // Read back the 2 bytes
        //the heading x10
        IMUToAOG[6] = Wire.read();
        IMUToAOG[5] = Wire.read();

        angle16 = IMUToAOG[6];                 // Calculate 16 bit angle
        angle16 <<= 8;
        angle16 += IMUToAOG[5];

        HeadingIMURAW = float(angle16) / 10;
        HeadingIMUcorr = HeadingIMURAW + Set.IMUHeadingCorrection;
    }

    Wire.beginTransmission(Set.CMPS14_ADDRESS);  //starts communication with CMPS14
    Wire.write(0x1C);                     //Sends the register we wish to start reading from
    Wire.endTransmission(Set.CMPS14_ADDRESS);
    Wire.requestFrom(Set.CMPS14_ADDRESS, 2);

    if (Wire.available() <= 2)
    {
        //the roll x10
        IMUToAOG[8] = Wire.read();
        IMUToAOG[7] = Wire.read();

        roll16 = IMUToAOG[8];
        roll16 <<= 8;
        roll16 += IMUToAOG[7];
        if (bitRead(roll16, 15)) { roll_neg = true; }
        else { roll_neg = false; }
        rollIMURAW = float(roll16) / 10;
        if (roll_neg) { rollIMURAW -= 6554; }
        CMPS14_NewData = true;
    }
}
