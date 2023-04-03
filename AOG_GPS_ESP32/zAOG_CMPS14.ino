
void readCMPS() {

    bool roll_neg;
    unsigned int angle16;
    int roll16;

    HeadingCMPS = 0.0;
    rollCMPS = 0.0;
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

        HeadingCMPS = float(angle16) / 10;
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
        rollCMPS = float(roll16) / 10;
        if (roll_neg) { rollCMPS -= 6554; }
    }
    
    if (Set.sendIMUPGN) {
        //add the checksum
        int CRCtoAOG = 0;
        for (byte i = 2; i < IMUToAOGSize - 1; i++)
        {
            CRCtoAOG = (CRCtoAOG + IMUToAOG[i]);
        }
        IMUToAOG[IMUToAOGSize - 1] = CRCtoAOG;
    }
}
