void doSerialNTRIP() {
	byte incomingByte = 0;
	while (Serial.available())
	{
		incomingByte = Serial.read();
		Serial1.write(incomingByte);
	}
}


//-------------------------------------------------------------------------------------------------


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
		nextUBXcount2 = (UBXRingCount2 + 1) % sizeOfUBXArray;
		incomByte2 = Serial2.read();
		//if (GPSSet.debugmodeUBX) { Serial.print("incom Byte2: "); Serial.print(incomByte2); }

		//UBX comming?
		if (UBXDigit2 < 2) {
			if (incomByte2 == UBX_HEADER[UBXDigit2]) {
				UBXDigit2++;
				//if (GPSSet.debugmodeUBX) { Serial.print("UBX2 RelPosNED started: digit  "); Serial.println(UBXDigit2 - 1); }
			}
			else
				UBXDigit2 = 0;
		}
		else {
			//add incoming Byte to UBX
			((unsigned char*)(&UBXRelPosNED[nextUBXcount2]))[UBXDigit2 - 2] = incomByte2;
			UBXDigit2++;
			//if (GPSSet.debugmodeUBX) { Serial.print(incomByte2); Serial.print(": incoming byte number: "); Serial.println(UBXDigit2); }

			if (UBXDigit2 == 5) {

				if (!((UBXRelPosNED[nextUBXcount2].cls == 0x01) && (UBXRelPosNED[nextUBXcount2].id == 0x3C))) {
					//wrong sentence				
					/*if (GPSSet.debugmodeUBX) {
					Serial.print("UBX2 wrong sentence: digit  ");
					Serial.print(UBXDigit2 - 1);
					Serial.print("cls  ");
					Serial.print(UBXRelPosNED[nextUBXcount2].cls, HEX);
					Serial.print("id  ");
					Serial.println(UBXRelPosNED[nextUBXcount2].id, HEX);
				}*/
					UBXDigit2 = 0;
				}
				else { if (GPSSet.debugmodeUBX) { Serial.println("UBX RelPosNED found"); } }
			}//5
			else
			{
				if (UBXDigit2 == 7) {//lenght
					UBXLenght2 = UBXRelPosNED[nextUBXcount2].len + 8;//+2xheader,2xclass,2xlenght,2xchecksum
					//if (debugmode) { Serial.print("UBXLenght2: "); Serial.println(UBXLenght2); }
				}
				else
				{
					if (UBXDigit2 == UBXLenght2) { //UBX complete
						UBXchecksum2[0] = 0;
						UBXchecksum2[1] = 0;
						for (int i = 0; i < (UBXLenght2 - 4); i++) {
							UBXchecksum2[0] += ((unsigned char*)(&UBXRelPosNED[nextUBXcount2]))[i];
							UBXchecksum2[1] += UBXchecksum2[0];
						}
						/*	if (GPSSet.debugmodeUBX) {
								Serial.print("UBX2 Checksum0 expected "); Serial.print(UBXchecksum2[0]);
								Serial.print("  incomming Checksum0: "); Serial.println(UBXRelPosNED[nextUBXcount2].CK0);
								Serial.print("UBX2 Checksum1 expected "); Serial.print(UBXchecksum2[1]);
								Serial.print("  incomming Checksum1: "); Serial.println(UBXRelPosNED[nextUBXcount2].CK1);
							}
							*/
						if ((UBXRelPosNED[nextUBXcount2].CK0 == UBXchecksum2[0]) && (UBXRelPosNED[nextUBXcount2].CK1 == UBXchecksum2[1])) {
							//checksum OK
							if (GPSSet.checkUBXFlags) {
								if (bitRead(UBXRelPosNED[nextUBXcount2].flags, 8)) {
									//flag: heading OK
									existsUBXRelPosNED = true;
									UBXRingCount2 = nextUBXcount2;
									UBXDigit2 = 0;
									UBXLenght2 = 100;
									if (GPSSet.debugmodeUBX) {
										Serial.print("got RelPosNED. Heading: "); Serial.print((UBXRelPosNED[nextUBXcount2].relPosHeading * 0.00001), 2);
										Serial.print(" down vector (cm): "); Serial.println((float(UBXRelPosNED[nextUBXcount2].relPosD) + (float(UBXRelPosNED[nextUBXcount2].relPosHPD) * 0.01)), 2);
									}
									if (GPSSet.debugmodeRAW) {
										Serial.print("SerIn: RelPosNED heading down dist flags"); Serial.print(",");
										Serial.print(UBXRelPosNED[UBXRingCount2].relPosHeading); Serial.print(",");
										Serial.print((float(UBXRelPosNED[nextUBXcount2].relPosD) + (float(UBXRelPosNED[nextUBXcount2].relPosHPD) * 0.01)), 2); Serial.print(",");
										Serial.print(UBXRelPosNED[UBXRingCount2].relPosLength); Serial.print(",");
										Serial.print(UBXRelPosNED[UBXRingCount2].flags); Serial.print(",");
									}
								}
								else { if (GPSSet.debugmodeUBX) { Serial.println("UBXRelPosNED flag heading: NOT valid, checksum OK; sentence NOT used"); } }
							}
							else {//don't check UBX flags, checksum ok
								existsUBXRelPosNED = true;
								UBXRingCount2 = nextUBXcount2;
								UBXDigit2 = 0;
								UBXLenght2 = 100;
								if (GPSSet.debugmodeUBX) {
									Serial.print("got RelPosNED. Heading: "); Serial.print((UBXRelPosNED[nextUBXcount2].relPosHeading * 0.00001), 2);
									Serial.print(" down vector (cm): "); Serial.println((float(UBXRelPosNED[nextUBXcount2].relPosD) + (float(UBXRelPosNED[nextUBXcount2].relPosHPD) * 0.01)), 2);
								}
								if (GPSSet.debugmodeRAW) {
									Serial.print("SerIn: RelPosNED heading down dist flags"); Serial.print(",");
									Serial.print(UBXRelPosNED[UBXRingCount2].relPosHeading); Serial.print(",");
									Serial.print(UBXRelPosNED[UBXRingCount2].relPosD); Serial.print("."); Serial.print(UBXRelPosNED[nextUBXcount2].relPosHPD); Serial.print(",");
									Serial.print(UBXRelPosNED[UBXRingCount2].relPosLength); Serial.print(",");
									Serial.print(UBXRelPosNED[UBXRingCount2].flags); Serial.print(",");
								}
							}
						}
						else { //checksum wrong							
							UBXDigit2 = 0;
							UBXLenght2 = 100;
							if (GPSSet.debugmodeUBX) { Serial.println("UBX2 RelPosNED checksum invalid"); }
						}
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
		//if (GPSSet.debugmodeUBX) { Serial.print("incom Byte: "); Serial.print(incomByte1); Serial.print("UBXRingCount1: "); Serial.print(UBXRingCount1); Serial.print(" nextUBXCount1: "); Serial.println(nextUBXcount1); }

		// ai, 07.10.2020: use the GGA Message to determine Fix-Quality
		if (incomByte1 == '$') {
			bNMEAstarted = true;
			sNMEA = "";
		}
		if (bNMEAstarted == true) {
			sNMEA.concat((char)incomByte1); // add the char to the NMEA message
			if (incomByte1 == 10) { // ASCII(10) <LF> (Linefeed) ends the message
				bNMEAstarted = false;
				if (sNMEA.substring(3, 6) == "GGA") { // GGA Message found
				  // the fix quality is the char after the sixth ',' - so look for sixth ','
					iPos = -1;
					i = 0;
					do {
						iPos = sNMEA.indexOf(',', iPos + 1); // find position of next ','
						if (iPos > -1) { i++; }
					} while ((i < 6) && (iPos > -1));
					if (iPos > -1) { cFixQualGGA = sNMEA.charAt(iPos + 1); bGGAexists = true; }
				}
			}
		}
		// END ai, 07.10.2020: use the GGA Message to determine Fix-Quality


		//UBX comming?
		if (UBXDigit1 < 2) {
			if (incomByte1 == UBX_HEADER[UBXDigit1]) {
				UBXDigit1++;
				//if (GPSSet.debugmodeUBX) { Serial.print("UBX1 started: digit  "); Serial.println(UBXDigit1 - 1); }
			}
			else
				UBXDigit1 = 0;
		}
		else {
			//add incoming Byte to UBX
			((unsigned char*)(&UBXPVT1[nextUBXcount1]))[UBXDigit1 - 2] = incomByte1;
			UBXDigit1++;
			//if (GPSSet.debugmodeUBX) { Serial.print(incomByte1); Serial.print(": incoming byte number: "); Serial.println(UBXDigit1); }

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

							if (GPSSet.debugmodeRAW) {
								Serial.print("SerIn: #lstUBX #newUBX newUBX lat lon"); Serial.print(",");
								Serial.print(UBXRingCount1); Serial.print(",");
								Serial.print(nextUBXcount1); Serial.print(",");
								Serial.print(UBXPVT1[nextUBXcount1].lat); Serial.print(",");
								Serial.print(UBXPVT1[nextUBXcount1].lon); Serial.print(",");
								Serial.print("SerIn: RelPosNED heading down dist flags"); Serial.print(",");
								Serial.print(UBXRelPosNED[UBXRingCount2].relPosHeading); Serial.print(",");
								Serial.print(UBXRelPosNED[UBXRingCount2].relPosD); Serial.print("."); Serial.print(UBXRelPosNED[nextUBXcount2].relPosHPD); Serial.print(",");
								Serial.print(UBXRelPosNED[UBXRingCount2].relPosLength); Serial.print(",");
								Serial.print(UBXRelPosNED[UBXRingCount2].flags); Serial.print(",");
							}

							UBXRingCount1 = nextUBXcount1;
							UBXLenght1 = 100;
							if (GPSSet.debugmodeUBX) {
								Serial.print("got UBX1 PVT lat: "); Serial.print(UBXPVT1[nextUBXcount1].lat);
								Serial.print(" lon: "); Serial.println(UBXPVT1[nextUBXcount1].lon);
							}
						}
						else { if (GPSSet.debugmodeUBX) { Serial.println("UBX1 PVT checksum invalid"); } }

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

