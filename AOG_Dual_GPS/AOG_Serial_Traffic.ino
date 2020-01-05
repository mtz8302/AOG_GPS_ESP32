void doSerialNTRIP() {
	byte incomingByte = 0;
	while (Serial.available())
	{
		incomingByte = Serial.read();
		Serial1.write(incomingByte);
	}
}





void getUBX() {
	bool checksumOk1 = false, checksumOk2 = false;
	byte incomByte1 = 0, incomByte2 = 0, idx1 = 0, idx2 = 0;
	word CRC1 = 0, CRC2 = 0;
	static unsigned char UBXchecksum1[2] = { 0,0 };
	static unsigned char UBXchecksum2[2] = { 0, 0 };
	byte nextUBXcount1 = 0, nextUBXcount2 = 0;

	//dual GPS: serial 2
	while (Serial2.available())
	{
		incomByte2 = Serial2.read();
		//if (debugmodeUBX) { Serial.print("incom Byte2: "); Serial.print(incomByte2); }

		//UBX comming?
		if (UBXDigit2 < 2) {
			if (incomByte2 == UBX_HEADER[UBXDigit2]) {
				UBXDigit2++;
				//if (debugmodeUBX) { Serial.print("UBX2 RelPosNED started: digit  "); Serial.println(UBXDigit2 - 1); }
			}
			else
				UBXDigit2 = 0;
		}
		else {
			//add incoming Byte to UBX
			((unsigned char*)(&UBXRelPosNED))[UBXDigit2 - 2] = incomByte2;
			UBXDigit2++;
			//if (debugmodeUBX) { Serial.print(incomByte2); Serial.print(": incoming byte number: "); Serial.println(UBXDigit2); }

			if (UBXDigit2 == 5) {

				if (!((UBXRelPosNED.cls == 0x01) && (UBXRelPosNED.id == 0x3C))) {
					//wrong sentence				
					/*if (debugmode) {
					Serial.print("UBX2 wrong sentence: digit  ");
					Serial.print(UBXDigit2 - 1);
					Serial.print("cls  ");
					Serial.print(UBXRelPosNED.cls);
					Serial.print("id  ");
					Serial.println(UBXRelPosNED.id);
				}*/
					UBXDigit2 = 0;
				}
				//else { if (debugmode) { Serial.println("UBX PVT1 found"); } }
			}//5
			else
			{
				if (UBXDigit2 == 7) {//lenght
					UBXLenght2 = UBXRelPosNED.len + 8;//+2xheader,2xclass,2xlenght,2xchecksum
					//if (debugmode) { Serial.print("UBXLenght2: "); Serial.println(UBXLenght2); }
				}
				else
				{
					if (UBXDigit2 == UBXLenght2) { //UBX complete
						UBXchecksum2[0] = 0;
						UBXchecksum2[1] = 0;
						for (int i = 0; i < (UBXLenght2 - 4); i++) {
							UBXchecksum2[0] += ((unsigned char*)(&UBXRelPosNED))[i];
							UBXchecksum2[1] += UBXchecksum2[0];
						}
					/*	if (debugmodeUBX) {
							Serial.print("UBX2 Checksum0 expected "); Serial.print(UBXchecksum2[0]);
							Serial.print("  incomming Checksum0: "); Serial.println(UBXRelPosNED.CK0);
							Serial.print("UBX2 Checksum1 expected "); Serial.print(UBXchecksum2[1]);
							Serial.print("  incomming Checksum1: "); Serial.println(UBXRelPosNED.CK1);
						}
						*/
						if ((UBXRelPosNED.CK0 == UBXchecksum2[0]) && (UBXRelPosNED.CK1 == UBXchecksum2[1])) {
							//sentence OK
							existsUBXRelPosNED = true;
							UBXDigit2 = 0;
							UBXLenght2 = 100;
							if (debugmodeUBX) {
								if (debugmodeUBX) {
									Serial.print("got RelPosNED. Heading: "); Serial.print((UBXRelPosNED.relPosHeading * 0.00001), 2);
									Serial.print(" down vector (cm): "); Serial.println((float(UBXRelPosNED.relPosD) + (float(UBXRelPosNED.relPosHPD) * 0.01)), 2);
								}
							}
						}
						else { if (debugmodeUBX) { Serial.println("UBX2 RelPosNED checksum invalid"); } }

					}//UBX complete
					else
					{//too long
						if (UBXDigit2 > (108)) {
							UBXDigit2 = 0;
							UBXLenght2 = 100;
						}
					}//UBX complete
				}//7
			}//5
		}//>2
	}//while serial 2 available

//serial1 = main if only one
	while (Serial1.available())
	{
		nextUBXcount1 = (UBXRingCount1 + 1) % sizeOfUBXArray;
		
		incomByte1 = Serial1.read();
		//if (debugmodeUBX) { Serial.print("incom Byte: "); Serial.print(incomByte1); Serial.print("UBXRingCount1: "); Serial.print(UBXRingCount1); Serial.print(" nextUBXCount1: "); Serial.println(nextUBXcount1); }

		//UBX comming?
		if (UBXDigit1 < 2) {
			if (incomByte1 == UBX_HEADER[UBXDigit1]) {
				UBXDigit1++;
				//if (debugmodeUBX) { Serial.print("UBX1 started: digit  "); Serial.println(UBXDigit1 - 1); }
			}
			else
				UBXDigit1 = 0;
		}
		else {
			//add incoming Byte to UBX
			((unsigned char*)(&UBXPVT1[nextUBXcount1]))[UBXDigit1 - 2] = incomByte1;
			UBXDigit1++;
				//if (debugmodeUBX) { Serial.print(incomByte1); Serial.print(": incoming byte number: "); Serial.println(UBXDigit1); }

			if (UBXDigit1 == 5) {
				/*if (debugmode) {
					Serial.print("UBX1 wrong sentence: digit  ");
					Serial.print(UBXDigit1 - 1);
					Serial.print("cls  ");
					Serial.print(UBXPVT1[nextUBXcount1].cls);
					Serial.print("id  ");
					Serial.println(UBXPVT1[nextUBXcount1].id);
				}*/
				if (!((UBXPVT1[nextUBXcount1].cls == 0x01) && (UBXPVT1[nextUBXcount1].id == 0x07))) {
					//wrong sentence
					UBXDigit1 = 0;
				}
				//else { if (debugmode) { Serial.println("UBX PVT1 found"); } }
			}//5
			else
			{
				if (UBXDigit1 == 7) {//lenght
					UBXLenght1 = UBXPVT1[nextUBXcount1].len + 8;//+2xheader,2xclass,2xlenght,2xchecksum
					//if (debugmode) { Serial.print("UBXLenght1: "); Serial.println(UBXLenght1); }
				}
				else
				{
					if (UBXDigit1 == UBXLenght1) { //UBX complete
						UBXchecksum1[0] = 0;
						UBXchecksum1[1] = 0;
						for (int i = 0; i < (UBXLenght1 - 4); i++) {
							UBXchecksum1[0] += ((unsigned char*)(&UBXPVT1[nextUBXcount1]))[i];
							UBXchecksum1[1] += UBXchecksum1[0];
						}
						/*if (debugmode) {
							Serial.print("UBX Checksum0 expected "); Serial.print(UBXchecksum1[0]);
							Serial.print("  incomming Checksum0: "); Serial.println(UBXPVT1[nextUBXcount1].CK0);
							Serial.print("UBX Checksum1 expected "); Serial.print(UBXchecksum1[1]);
							Serial.print("  incomming Checksum1: "); Serial.println(UBXPVT1[nextUBXcount1].CK1);
						}
						*/
						if ((UBXPVT1[nextUBXcount1].CK0 == UBXchecksum1[0]) && (UBXPVT1[nextUBXcount1].CK1 == UBXchecksum1[1])) {
							UBXDigit1 = 0;
							UBXRingCount1 = nextUBXcount1;
							UBXLenght1 = 100;
							if (debugmodeUBX) {
								Serial.print("got UBX1 PVT lat: "); Serial.print(UBXPVT1[nextUBXcount1].lat);
								Serial.print(" lon: "); Serial.println(UBXPVT1[nextUBXcount1].lon);
							}
						}
						else { if (debugmodeUBX) { Serial.println("UBX1 PVT checksum invalid"); } }

					}//UBX complete
					else
					{//too long
						if (UBXDigit1 > (108)) {
							UBXDigit1 = 0;
							UBXLenght1 = 100;
						}
					}//UBX complete
				}//7
			}//5
		}//>2
	}//while serial 1 available
}//void getUBX

