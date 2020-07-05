#if HardwarePlatform == 0
//--------------------------------------------------------------
//  EEPROM Data Handling
//--------------------------------------------------------------
#define EEPROM_SIZE 512
#define EE_ident1 0xED  // Marker Byte 0 + 1
#define EE_ident2 0x43


//--------------------------------------------------------------
//  Restore EEprom Data
//--------------------------------------------------------------
void restoreEEprom() {
	Serial.println("read values from EEPROM");

	if (EEprom_empty_check() == 1 || EEPROM_clear) { //first start?
		EEprom_write_all();     //write default data
	}
	if (EEprom_empty_check() == 2) { //data available
		EEprom_read_all();
	}
	if (GPSSet.debugmode) { EEprom_show_memory(); }
}

//--------------------------------------------------------------
byte EEprom_empty_check() {

	if (!EEPROM.begin(EEPROM_SIZE))
	{
		Serial.println("failed to initialise EEPROM"); delay(1000);
		return false;//0
	}
	if ((EEPROM.read(0) != EE_ident1) || (EEPROM.read(1) != EE_ident2))
		return true;  // 1 = is empty

	if ((EEPROM.read(0) == EE_ident1) && (EEPROM.read(1) == EE_ident2))
		return 2;     // data available

}
//--------------------------------------------------------------

void EEprom_write_all() {
	//first time = empty or clear EEPROM data?
	byte tempbyt = EEprom_empty_check();

	EEPROM.write(0, EE_ident1);
	EEPROM.write(1, EE_ident2);
	EEPROM.write(2, 0); // reset Restart blocker
	EEPROM.put(3, GPSSet);
	EEPROM.commit();


	if ((tempbyt == 0) || (tempbyt == 1) || (EEPROM_clear)) {
		EEPROM.put((4 + sizeof(GPSSet)), GPSSet);
		Serial.print("rewriting EEPROM + write 2. set at #"); Serial.println(4 + sizeof(GPSSet));
		delay(2);
		EEPROM.commit();
	}//write 2. time with defaults to be able to reload them  
}

//--------------------------------------------------------------

void EEprom_block_restart() {
	if (EEPROM.read(2) == 0) {//prevents from restarting, when webpage is reloaded. Is set to 0, when other ACTION than restart is called
		EEPROM.write(2, 1);
		EEPROM.commit();
	}
}

//--------------------------------------------------------------


void EEprom_unblock_restart() {
	if (EEPROM.read(2) != 0) {
		EEPROM.write(2, 0); // reset Restart blocker
		delay(2);
		EEPROM.commit();
	}
}

//--------------------------------------------------------------

void EEprom_read_all() {

	EEPROM.get(3, GPSSet);

}
//--------------------------------------------------------------
void EEprom_read_default() {
	EEPROM.get(4 + sizeof(GPSSet), GPSSet);
	Serial.print("load default value from EEPROM at #"); Serial.println(4 + sizeof(GPSSet));
}
//--------------------------------------------------------------
void EEprom_show_memory() {
	byte c2 = 0, data_;
	Serial.print(EEPROM_SIZE, 1);
	Serial.println(" bytes read from Flash . Values are:");
	for (int i = 0; i < EEPROM_SIZE; i++)
	{
		data_ = byte(EEPROM.read(i));
		if (data_ < 0x10) Serial.print("0");
		Serial.print(data_, HEX);
		if (c2 == 15) {
			Serial.print(" ");
		}
		else if (c2 >= 31) {
			Serial.println(); //NL
			c2 = -1;
		}
		else Serial.print(" ");
		c2++;
	}
}

#endif




//parser for 2x UBX PVT (not only RelPosNED)

/*	while (Serial2.available())
	{
		incomByte2 = Serial2.read();

		//UBX comming?
		if (UBXDigit2 < 2) {
			if (incomByte2 == UBX_HEADER[UBXDigit2]) {
				UBXDigit2++;
			}
			else
				UBXDigit2 = 0;
		}
		else {
			//add incoming byte
			if (isUBXRelPosNED) { 
				//add to RelPosNED sentense				
				if (GPSSet.debugmodeUBX) { Serial.print("UBX2 digit  "); Serial.print(UBXDigit2); Serial.print(" incom Byte: "); Serial.println(incomByte2); }
				((unsigned char*)(&UBXRelPosNED))[UBXDigit2 - 2] = incomByte2;
			}
			else {
				//add to PVT2 sentense (default, on digit 5 decided)
				((unsigned char*)(&UBXPVT2[nextUBXcount2]))[UBXDigit2 - 2] = incomByte2;
			}
			UBXDigit2++;


			if (UBXDigit2 == 5) {
				if (UBXPVT2[nextUBXcount2].cls == 0x01)
				{
					if (UBXPVT2[nextUBXcount2].id == 0x07) {
						//PVT sentence
						isUBXPVT2 = true;
						if (GPSSet.debugmodeUBX) { Serial.println("UBX PVT2 started on serial 2"); }
					}
					else 
					{
						isUBXPVT2 = false;
						if (UBXPVT2[nextUBXcount2].id == 0x3C) {
							//RelPosNED sentence
							isUBXRelPosNED = true;
							UBXRelPosNED[UBXRingCount2].cls = 0x01;
							UBXRelPosNED[UBXRingCount2].id = 0x3C;
							if (GPSSet.debugmodeUBX) { Serial.println("UBX RelPosNED started on serial 2"); }
						}
						else {//no sentence of interrest
							UBXDigit2 = 0;
							isUBXRelPosNED = false;
						}
					}
				}

			} //digit 5
			else
				if (UBXDigit2 == 7) {//lenght
					if (isUBXRelPosNED) { UBXLenght2 = UBXRelPosNED[UBXRingCount2].len+8; }//+2xheader,2xclass,2xlenght,2xchecksum
					else { UBXLenght2 = UBXPVT2[nextUBXcount2].len + 8; }//+2xheader,2xclass,2xlenght,2xchecksum
					if (GPSSet.debugmodeUBX) { Serial.print("UBXLenght2: "); Serial.println(UBXLenght2); }
				}
				else
				{
					if (UBXDigit2 == UBXLenght2) { //UBX complete
						//calculate checksum
						UBXchecksum2[0] = 0;
						UBXchecksum2[1] = 0;
						for (int i = 0; i < (UBXLenght2 - 4); i++) {
							if (isUBXRelPosNED) { UBXchecksum2[0] += ((unsigned char*)(&UBXRelPosNED))[i]; }
							else { UBXchecksum2[0] += ((unsigned char*)(&UBXPVT2[nextUBXcount2]))[i]; }
							UBXchecksum2[1] += UBXchecksum2[0];
						}
						if (isUBXRelPosNED) {//compare checksum
							if ((UBXRelPosNED[UBXRingCount2].CK0 == UBXchecksum2[0]) && (UBXRelPosNED[UBXRingCount2].CK1 == UBXchecksum2[1])) {
								UBXDigit2 = 0;
								isUBXRelPosNED = false;
								existsUBXRelPosNED = true;//if exists heading and roll calc with RelPosNED
								UBXLenght2 = 100;
								if (GPSSet.debugmodeUBX) {
									Serial.print("got RelPosNED. Heading: "); Serial.print((UBXRelPosNED[UBXRingCount2].relPosHeading * 0.00001), 2);
									Serial.print(" down vector (cm): "); Serial.println((float(UBXRelPosNED[UBXRingCount2].relPosD) + (float(UBXRelPosNED[UBXRingCount2].relPosHPD) * 0.01)), 2);
								}
							}
							else {
								UBXDigit1 = 0;
								isUBXRelPosNED = false;
								UBXLenght1 = 100;
								if (GPSSet.debugmodeUBX) {
									Serial.println("UBX2 RelPosNED checksum invalid");
									Serial.print("UBX Checksum0 expected "); Serial.print(UBXchecksum1[0]);
									Serial.print("  incomming Checksum0: "); Serial.println(UBXRelPosNED[UBXRingCount2].CK0);
									Serial.print("UBX Checksum1 expected "); Serial.print(UBXchecksum1[1]);
									Serial.print("  incomming Checksum1: "); Serial.println(UBXRelPosNED[UBXRingCount2].CK1);
								}
							}
						}
						else {
							if (isUBXPVT2) {//compare checksum
								if ((UBXPVT2[nextUBXcount2].CK0 == UBXchecksum2[0]) && (UBXPVT2[nextUBXcount2].CK1 == UBXchecksum2[1])) {
									UBXDigit2 = 0;
									isUBXPVT2 = false;
									UBXRingCount2 = nextUBXcount2;
									UBXLenght2 = 100;
									if (GPSSet.debugmodeUBX) {
										Serial.print("got UBX2 PVT lat: "); Serial.print(UBXPVT2[nextUBXcount2].lat);
										Serial.print(" lon: "); Serial.println(UBXPVT2[nextUBXcount2].lon);
									}
								}
								else {
									UBXDigit1 = 0;
									isUBXPVT2 = false;
									UBXLenght1 = 100;
									if (GPSSet.debugmodeUBX) { Serial.println("UBX2 RelPosNED checksum invalid"); }
								}
							}//PVT
						}//else RelPosNED

					}//UBX complete
				}//7
			if (UBXDigit2 > (105)) //too long
			{
				UBXDigit2 = 0;
				UBXLenght2 = 100;
			}
		}//>2
	}//end serial 2 
/

*/

//-------------------------------------------------------------------------------------------------

//parser for UBX and NMEA
//checksum validation for GGA not working (commended out)
/*
//------------------------------------------------------------------------------------------
//Read serial GPS data
//-------------------------------------------------------------------------------------------
void Serial_Traffic() {
	bool checksumOk1 = false, checksumOk2 = false;
	byte idx1 = 0, idx2 = 0;
	word CRC1 = 0, CRC2 = 0;
	static unsigned char UBXchecksum1[2] = { 0,0 };
	static unsigned char UBXchecksum2[2] = { 0, 0 };
	byte nextUBXcount1 = 0, nextUBXcount2 = 0;

	//mtz8302 dual GPS: serial 2
	if (GPSSet.dualGPS > 0) {
		nextUBXcount2 = (UBXcount2 + 1) % 25;

		while (Serial2.available())
		{
			c2 = Serial2.read();

			//UBX comming?
			if (UBXDigit2 < 2) {
				if (c2 == UBX_HEADER[UBXDigit2])
					UBXDigit2++;
				else
					UBXDigit2 = 0;
			}
			else {
				if ((UBXDigit2) < 49) { //add to sentence
					if (isUBXRelPosNED) {
						((unsigned char*)(&UBXRelPosNED))[UBXDigit2 - 2] = c2;
					}
					else
					{
						((unsigned char*)(&UBXPosLLH2[nextUBXcount2]))[UBXDigit2 - 2] = c2;
					}

					UBXDigit2++;
				}

				if (UBXDigit2 == 5) {
					if (UBXPosLLH2[nextUBXcount2].cls == 0x01) {
						if (UBXPosLLH2[nextUBXcount2].id == 0x02) {
							//POS LLH sentence
							isUBXPosLLH = true;
						}
						if (UBXPosLLH2[nextUBXcount2].id == 0x3C) {
							//POS RelPosNED sentence
							isUBXRelPosNED = true;
							UBXRelPosNED[UBXRingCount2].cls = 0x01;
							UBXRelPosNED[UBXRingCount2].id = 0x3C;
						}
					}
					else //no POS sentence
					{
						UBXDigit2 = 0;
						isUBXPosLLH = false;
						isUBXRelPosNED = false;
					}

				} //digit 5
				else
					if ((isUBXPosLLH) && (UBXDigit2 == (36))) { //UBX POS LLH complete
						UBXchecksum2[0] = 0;
						UBXchecksum2[1] = 0;
						for (int i = 0; i < 32; i++) {
							UBXchecksum2[0] += ((unsigned char*)(&UBXPosLLH2[nextUBXcount2]))[i];
							UBXchecksum2[1] += UBXchecksum2[0];
						}
						if ((UBXPosLLH2[nextUBXcount2].CK0 == UBXchecksum2[0]) && (UBXPosLLH2[nextUBXcount2].CK1 == UBXchecksum2[1])) {
							UBXDigit2 = 0;
							isUBXPosLLH = false;
							UBXpresent2 = true;
							GGATime2 = millis();
							UBXcount2 = nextUBXcount2;
							if (debugmode) Serial.println("got UBX2 Nav PosLLH");
						}
						else { if (debugmode) { Serial.println("UBX2 Nav PosLLH checksum invalid"); } }
					}
					else
						if ((isUBXRelPosNED) && (UBXDigit2 == (49))) { //UBX POS LLH complete
							UBXDigit2 = 0;
							isUBXRelPosNED = false;
							if (debugmode) Serial.println("got UBX2 Nav RelPosNED");
						}


				if (UBXDigit2 > (50)) //too long
				{
					UBXDigit2 = 0;
				}
			}//end UBX



			//NMEA comming?
			if (c2 == '$')      //if character is $=0x24 start new sentence
			{
				newSentence2 = true; // only store sentence, if time is over
				gpsBuffer2[0] = '/0';
				i2 = 0;
			}

			if (c2 == 0x0D && newSentence2)   //if character is CR, build UDP send buffer
			{
				char Sent_Buffer2[] = "???";
				gpsBuffer2[i2++] = 0x0D;  //last-1 byte =CR
				gpsBuffer2[i2++] = 0x0A;  //last-1 byte =CR
				Sent_Buffer2[0] = (char)gpsBuffer2[3];
				Sent_Buffer2[1] = (char)gpsBuffer2[4];
				Sent_Buffer2[2] = (char)gpsBuffer2[5];

				//GGA message complete
				if (strcmp(Sent_Buffer2, "GGA") == 0) {
					/*
						//checksum
						checksumOk2 = false;
						for (idx2 = 1; idx2 <= i2 - 4; idx2++) {
							if (gpsBuffer2[idx2] == 0x2A) { //'*'
								break;
							}
							CRC2 = CRC2 ^ gpsBuffer2[idx2];
						}
						if (gpsBuffer2[idx2++] > 57) //ASCII 48 = 0 ASCII A = 65
						{
							if (gpsBuffer2[idx2] == byte((CRC2 / 16) + 57))
							{
								checksumOk2 = true;
							}
						}
						else
						{
							if (gpsBuffer2[idx2] == byte((CRC2 / 16) + 48)) { checksumOk2 = true; }
						}

						if (gpsBuffer2[idx2++] > 57)//ASCII 48 = 0 ASCII A = 65 = 48 + 9
						{
							if ((gpsBuffer2[idx2] == byte((CRC2 % 16) + 57)) && checksumOk2)
							{
								checksumOk2 = true;
							}
						}
						else
						{
							if ((gpsBuffer2[idx2] == byte((CRC2 % 16) + 48)) && checksumOk2) { checksumOk2 = true; }
						}//end checksum


						if (checksumOk2)
						{
					*/	/*	for (byte b = 0; b <= i2; b++) {
						GGABuffer2[b] = gpsBuffer2[b];
					}
					GGATime2 = millis();
					/*		if (debugmode) { Serial.print(" NMEA from GPS 2 checksum OK "); Serial.println(Sent_Buffer2); }
						}
						else
						{
							if (debugmode) { Serial.print("NMEA from GPS 2 checksum invalid "); Serial.println(Sent_Buffer2); }
						}
	*/
	/*			}//if GGA
				i2 = 0;
				newSentence2 = false;
			}//NMEA complete

			if (newSentence2 && i2 < 100)//too long
			{
				gpsBuffer2[i2++] = c2;
				//if (debugmode) Serial.print(gpsBuffer2[i2]);
			}
		}//end serial 2 
	}//end if (GPSSet.dualGPS > 0)


  //serial1 = main if only one
	while (Serial1.available())
	{
		nextUBXcount1 = (UBXcount1 + 1) % 25;
		c1 = Serial1.read();
		//  Serial.print(c1);

		//UBX comming?
		if (UBXDigit1 < 2) {
			if (c1 == UBX_HEADER[UBXDigit1])
				UBXDigit1++;
			else
				UBXDigit1 = 0;
		}
		else {
			if ((UBXDigit1) <= 36) {
				((unsigned char*)(&UBXPosLLH1[nextUBXcount1]))[UBXDigit1 - 2] = c1;
			}

			UBXDigit1++;

			if (UBXDigit1 == 5) {
				if (!((UBXPosLLH1[nextUBXcount1].cls == 0x01) && (UBXPosLLH1[nextUBXcount1].id == 0x02))) {
					//wrong sentence
					UBXDigit1 = 0;
				}
			}//5

			if (UBXDigit1 == (36)) { //UBX POS LLH complete
				UBXchecksum1[0] = 0;
				UBXchecksum1[1] = 0;
				for (int i = 0; i < 32; i++) {
					UBXchecksum1[0] += ((unsigned char*)(&UBXPosLLH1[nextUBXcount1]))[i];
					UBXchecksum1[1] += UBXchecksum1[0];
				}
				/*		if (debugmode) {
							Serial.print("UBX Checksum0 expected "); Serial.print(UBXchecksum1[0]);
							Serial.print("  incomming Checksum0: "); Serial.println(UBXPosLLH1[nextUBXcount1].CK0);
							Serial.print("UBX Checksum1 expected "); Serial.print(UBXchecksum1[1]);
							Serial.print("  incomming Checksum1: "); Serial.println(UBXPosLLH1[nextUBXcount1].CK1);
						}*/

	/*			if ((UBXPosLLH1[nextUBXcount1].CK0 == UBXchecksum1[0]) && (UBXPosLLH1[nextUBXcount1].CK1 == UBXchecksum1[1])) {
					UBXDigit1 = 0;
					//	UBXDigit1 = 0;
					UBXpresent1 = true;
					GGATime1 = millis();
					//	Serial.print(UBXPosLLH1[nextUBXcount1].iTOW); Serial.print("  "); Serial.println(GGATime1);
					UBXcount1 = nextUBXcount1;

					if (debugmode) Serial.println("got UBX1 Nav PosLLH");

				}
				else { if (debugmode) { Serial.println("UBX1 checksum invalid"); } }
			}//36
			else
				if (UBXDigit1 > (36)) {
					UBXDigit1 = 0;
				}
		}//end UBX

		//NMEA comming?	
		if ((c1 == '$') && (UBXDigit1 < 3))      //if character is $=0x24 and no UBX sentence start new sentence
		{
			newSentence1 = true;
			gpsBuffer1[0] = '/0';
			i1 = 0;
		}

		if (c1 == 0x0D && newSentence1)   //if character is CR, build UDP send buffer
		{
			char Sent_Buffer1[] = "???";
			gpsBuffer1[i1++] = 0x0D;  //last-1 byte =CR
			gpsBuffer1[i1++] = 0x0A;  //last-1 byte =CR
			Sent_Buffer1[0] = (char)gpsBuffer1[3];
			Sent_Buffer1[1] = (char)gpsBuffer1[4];
			Sent_Buffer1[2] = (char)gpsBuffer1[5];

			if (strcmp(Sent_Buffer1, "RMC") == 0 || strcmp(Sent_Buffer1, "GGA") == 0 || strcmp(Sent_Buffer1, "VTG") == 0 || strcmp(Sent_Buffer1, "ZDA") == 0)
			{
				//mtz8302 dual GPS

			/*
				//checksum
				for (idx1 = 1; idx1 <= i1-4; idx1++) {
					if (gpsBuffer1[idx1] == 0x2A) { //'*'
							break;
						}
						CRC1 = CRC1 ^ gpsBuffer1[idx1];
				}
				checksumOk1 = false;
				if (gpsBuffer1[idx1++] > 57) //ASCII 48 = 0 ASCII A = 65
				{

					Serial.print("idx1: "); Serial.print(idx1); Serial.print(" gpsBuffer1[idx1] :"); Serial.print(gpsBuffer1[idx1]); Serial.print(" CRC1/16 : "); Serial.println(byte(CRC1 / 16));

					if (gpsBuffer1[idx1] == byte((CRC1 / 16) + 57))
					{
						checksumOk1 = true;
					}
				}
				else
				{
					Serial.print("else idx1: "); Serial.print(idx1); Serial.print(" gpsBuffer1[idx1] :"); Serial.print(gpsBuffer1[idx1]); Serial.print(" CRC1/16 : "); Serial.println(byte(CRC1 / 16));

					if (gpsBuffer1[idx1] == byte((CRC1 / 16) + 48)) { checksumOk1 = true; }
				}

				//Serial.print("IDX1 "); Serial.print(idx1); Serial.print(" gpsBuffer1[idx1]: "); Serial.println(gpsBuffer1[idx1]);
				if (gpsBuffer1[idx1++] > 57)//ASCII 48 = 0 ASCII A = 65 = 48 + 9
				{

					Serial.print("idx1: "); Serial.print(idx1); Serial.print(" gpsBuffer1[idx1] :"); Serial.print(gpsBuffer1[idx1]); Serial.print(" CRC1%16 : "); Serial.println(byte(CRC1 % 16));

					if ((gpsBuffer1[idx1] == byte((CRC1 % 16) + 57)) && checksumOk1)
					{
						checksumOk1 = true;
					}
				}
				else
				{
					Serial.print("else idx1: "); Serial.print(idx1); Serial.print(" gpsBuffer1[idx1] :"); Serial.print(gpsBuffer1[idx1]); Serial.print(" CRC1%16 : "); Serial.println(byte(CRC1 % 16));

					if ((gpsBuffer1[idx1] == byte((CRC1 % 16) + 48)) && checksumOk1) { checksumOk1 = true; }
				}

				//if (debugmode) {
					if (checksumOk1)
					{
						Serial.print(" NMEA from GPS 1 checksum OK "); Serial.println(Sent_Buffer1);
					}
					else
					{
						Serial.print("NMEA from GPS 1 checksum invalid "); Serial.println(Sent_Buffer1);
					Serial.print("idx1:-1 "); Serial.print(idx1-1); Serial.print(" gpsBuffer1[idx1-1] :"); Serial.print(gpsBuffer1[idx1-1]); Serial.print(" CRC1/16 : "); Serial.println(byte(CRC1 / 16));
					Serial.print("idx1: "); Serial.print(idx1); Serial.print(" gpsBuffer1[idx1] :"); Serial.print(gpsBuffer1[idx1]); Serial.print(" CRC1%16 : "); Serial.println(byte(CRC1 % 16));

					}
				//}

				if (checksumOk1) {
	*/
	/*			if (strcmp(Sent_Buffer1, "GGA") == 0) {
					for (byte n = 0; n <= i1; n++) {
						GGABuffer1[n] = gpsBuffer1[n];
						if (GPSSet.sendGGAsentence == 2) { lastSentence[n] = gpsBuffer1[n]; }
					}
					if (GPSSet.sendGGAsentence == 2) { repeat_ser = millis(); } //Reset timer
					GGATime1 = millis();
					if (GPSSet.dualGPS > 0) {
						delay(1);
						HeadingCalc(); //heading based on position of 2 Antennas
						if (GPSHeadingPresent) {
							//						if ((GPSSet.filterQuota > 0) && (GPSSet.filterLastItems > 0)) { filter_movement(); }//!! first call HeadingCalc() !! filters movements and changes GGA
							if ((GPSSet.dualGPS == 1) || (GPSSet.dualGPS == 3)) {
								//							buildHDT();	//!! first call HeadingCalc() !!   builds $GNHDT sentence based on 2 Antenna heading 
								newHDT = true;
							}
							if ((GPSSet.dualGPS == 5)) {
								//							buildOGI();//call heading calc first!!
							}
						}
					}
				}
				if (strcmp(Sent_Buffer1, "VTG") == 0) {
					VTGTime = millis();
					for (byte n = 0; n <= i1 + 1; n++) {
						VTGBuffer[n] = gpsBuffer1[n];
					}
					if ((GPSHeadingPresent) && ((GPSSet.dualGPS == 2) || (GPSSet.dualGPS == 4))) {
						//					changeVTG();  //!! first call HeadingCalc() !!  puts 2 Antenna heading into existing $GNVTG sentence
					}

				}
				switch (GPSSet.send_UDP_AOG) {
				case 1:
					//mtz8302
					if (my_WiFi_Mode != 0) {
						if (GPSSet.dualGPS == 5) {
							//send OGI only if new GGA position came in
							if (strcmp(Sent_Buffer1, "GGA") == 0) {
								udpRoof.writeTo(OGIBuffer, OGIlenght, ipDestination, portDestination);
							}
						}
						else {
							if ((newHDT) && (strcmp(Sent_Buffer1, "VTG"))) {
								//if new vtg then send hdt too
								udpRoof.writeTo(HDTBuffer, int(19), ipDestination, portDestination);
								delay(2);
							}
							udpRoof.writeTo(gpsBuffer1, i1, ipDestination, portDestination);
						}
					}
					//send also via BT if no Tractor WIFI = AP mode
#if (useBluetooth)
					if ((my_WiFi_Mode == WIFI_AP) || (my_WiFi_Mode == 0)) {
						if (GPSSet.dualGPS == 5) {
							if (strcmp(Sent_Buffer1, "GGA") == 0) { //send OGI only if new GGA came in
								for (byte n = 0; n < OGIlenght; n++) {
									SerialBT.print((char)OGIBuffer[n]);
								}
							}
							SerialBT.println();
						}
						else {
							for (byte n = 0; n < i1; n++) {  //print gpsBuffer to Bluetooth
								SerialBT.print((char)gpsBuffer1[n]);
							}
							SerialBT.println();
							if (newHDT) {
								for (byte n = 0; n <= 18; n++) {  //print gpsBuffer to Bluetooth
									SerialBT.print((char)HDTBuffer[n]);
								}
								SerialBT.println();
							}
						}
					}
#endif 
					newHDT = false;
					break;
				case 2:
#if (useBluetooth)
					if (GPSSet.dualGPS == 5) {
						if (strcmp(Sent_Buffer1, "GGA") == 0) {//send OGI only if new GGA came in
							for (byte n = 0; n < OGIlenght; n++) {
								SerialBT.print((char)OGIBuffer[n]);
							}
						}
						SerialBT.println();
					}
					else {
						for (byte n = 0; n < i1; n++) {  //print gpsBuffer to Bluetooth
							SerialBT.print((char)gpsBuffer1[n]);
						}
						SerialBT.println();
						if (newHDT) {
							for (byte n = 0; n <= 18; n++) {  //print gpsBuffer to Bluetooth
								SerialBT.print((char)HDTBuffer[n]);
							}
							SerialBT.println();
						}
					}
#endif 
					newHDT = false;
					break;
				}
				//	}//checksumOK
			} //sentence GGA/VTG/RCM
			i1 = 0;
			newSentence1 = false;
		}//sentence was ready and send

		if (newSentence1 && i1 < 100)
		{
			gpsBuffer1[i1++] = c1;
			// if (debugmode) Serial.print(gpsBuffer1[i1]);
		}
	}//while serial 1 available

}
*/





/*
struct NAV_POSLLH {
    unsigned char cls;
    unsigned char id;
    unsigned short len;
    unsigned long iTOW;
    long lon;
    long lat;
    long height;
    long hMSL;
    unsigned long hAcc;
    unsigned long vAcc;
    unsigned char  CK0;
    unsigned char  CK1;
};
NAV_POSLLH UBXPosLLH1[50], UBXPosLLH2[50];
*/


/*
//heading calc based on 2 positions (no RelPosNED needed)

void headingRollCalc() {
	double LatDec1 = 0, LatDec2 = 0, LonDec1 = 0, LonDec2 = 0, Hight1 = 0, Hight2 = 0;
	bool UBXTimeFit = false;
	byte UBXIdx1 = 0, UBXIdx2 = 0;

	//compare Sat Time
	if (UBXPVT1[UBXRingCount1].iTOW == UBXPVT2[UBXRingCount2].iTOW) {
		UBXTimeFit = true;
		UBXIdx1 = UBXRingCount1;
		UBXIdx2 = UBXRingCount2;
	}
	/*else //check, if the last sentence is of the same sat time
	{
		UBXIdx1 = (UBXRingCount1 + sizeOfUBXArray - 1) % sizeOfUBXArray;  //last one
		UBXIdx2 = (UBXRingCount2 + sizeOfUBXArray - 1) % sizeOfUBXArray;  //last one
		if (UBXPVT1[UBXIdx1].iTOW == UBXPVT2[UBXRingCount2].iTOW) {
			UBXTimeFit = true;
			UBXIdx2 = UBXRingCount2;
		}
		else
		{
			if (UBXPVT1[UBXRingCount1].iTOW == UBXPVT2[UBXIdx2].iTOW) {
				UBXTimeFit = true;
				UBXIdx1 = UBXRingCount1;
			}
			else
			{
				if (UBXPVT1[UBXIdx1].iTOW == UBXPVT2[UBXIdx2].iTOW) {
					UBXTimeFit = true;
				}
			}
		}
	}*/
/*
if (UBXTimeFit) {
	LatDec1 = (UBXPVT1[UBXIdx1].lat) * (double(0.0000001));
	LonDec1 = (UBXPVT1[UBXIdx1].lon) * (double(0.0000001));
	Hight1 = UBXPVT1[UBXIdx1].hMSL;
	LatDec2 = (UBXPVT2[UBXIdx2].lat) * (double(0.0000001));
	LonDec2 = (UBXPVT2[UBXIdx2].lon) * (double(0.0000001));
	Hight2 = UBXPVT2[UBXIdx2].hMSL;

	if (debugmode) { Serial.println("UBX present, newer than 0.5 sec and satellite time of both antennas fit: heading + roll calc"); }

	//check if all values are there
	if ((LatDec1 == 0.0) || (LonDec1 == 0.0) || (LatDec2 == 0.0) || (LonDec2 == 0.0)) {
		dualGPSHeadingPresent = false;
		if (GPSSet.RollDevice == 1) { rollPresent = false; }
		if (debugmode) {
			Serial.println(); Serial.println("NO heading calc: Lat or Long value 0");
		}
	}
	//heading calc: good values: calc heading + roll
	else {

		double DLongRad = (LonDec1 - LonDec2) * PI180;
		double y = sin(DLongRad) * cos(LonDec2 * PI180);
		double x = (cos(LatDec2 * PI180) * sin(LatDec1 * PI180))
			- (sin(LatDec2 * PI180) * cos(LatDec1 * PI180));
		double Heading = ((180.0 / PI) * atan2(y, x));
		if (Heading < 0) { Heading = 360.0 + Heading; }

		byte headRingCountOld = headRingCount;
		headRingCount = (headRingCount + 1) % GPSHeadingArraySize;//ringcounter: 0-29

		HeadingRelPosNED = GPSSet.headingAngleCorrection + Heading;
		if (HeadingRelPosNED > 360) { HeadingRelPosNED -= 360; }

		if (GPSSet.debugmodeHeading) {
			Serial.print("UBX1 used for heading/roll #(UBXIdx1): "); Serial.print(UBXIdx1);
			Serial.print(" UBXIdx2: "); Serial.println(UBXIdx2);
			Serial.print("GPS heading: "); Serial.print(HeadingRelPosNED, 3);
			Serial.print(" headRingCount: "); Serial.println(headRingCount);
			Serial.print("lat1: "); Serial.print((UBXPVT1[UBXIdx1].lat));
			Serial.print(" lon1: "); Serial.print((UBXPVT1[UBXIdx1].lon));
			Serial.print("lat2: "); Serial.print((UBXPVT2[UBXIdx2].lat));
			Serial.print(" lon2: "); Serial.println((UBXPVT2[UBXIdx2].lon));
		}
		//!!never runs?
		//try to filter wrong UBX positions: if heading changes too much, pos is wrong
		if ((HeadingRelPosNED[headRingCountOld] + GPSSet.headingMaxChange) < HeadingRelPosNED) {
			if ((HeadingRelPosNED[headRingCountOld] - GPSSet.headingMaxChange) > HeadingRelPosNED) {
				if (GPSSet.debugmodeHeading) {
					Serial.print("heading old: "); Serial.print(HeadingRelPosNED[headRingCountOld], 2);
					Serial.print(" heading new: "); Serial.println(HeadingRelPosNED, 2);
					Serial.print("headRingCountOld: "); Serial.print(headRingCountOld);
					Serial.print(" headRingCount: "); Serial.print(headRingCount);
					Serial.print("UBX[Idx of heading calc] Pos lat: "); Serial.print((UBXPVT1[UBXIdx1].lat / 10000000), 7);
					Serial.print(" lon: "); Serial.println((UBXPVT1[UBXIdx1].lon / 10000000), 7);
					Serial.println("heading changes too fast, drop actuall UBX sentence");
					//UBX1 is used for position so kick this one use UBXidx as used for heading calc
				}
				UBXRingCount1 = (UBXIdx1 + sizeOfUBXArray - 1) % sizeOfUBXArray;
			}
		}

		dualGPSHeadingPresent = true;
		dualAntNoValueCount = 0;//reset watchdog

		if (GPSSet.RollDevice == 1) {

			//roll calculation only if antennas are at left+right
			if (((GPSSet.headingAngleCorrection > 70) && (GPSSet.headingAngleCorrection < 110)) ||
				((GPSSet.headingAngleCorrection > 250) && (GPSSet.headingAngleCorrection < 290)))
			{
				float rollOld = roll;
				bool rollInvert = false;
				double hightDiff = double((UBXPVT1[UBXIdx1].hMSL - UBXPVT2[UBXIdx2].hMSL)) / 10;//cm
				if (hightDiff < 0) {
					hightDiff *= -1;
					rollInvert = true;
				}
				roll = atan2(hightDiff, GPSSet.AntDist) / PI180;
				if (rollInvert) { roll *= -1; }				//	roll = roll * -1; // left tilt should be negative 
				float rollmax = rollOld + GPSSet.rollMaxChange;
				float rollmin = rollOld - GPSSet.rollMaxChange;

				//not realy a filter, but discards extremes
				roll = constrain(roll, rollmin, rollmax); // attenuation of the max, min roll value

				rollPresent = true;
				if (GPSSet.debugmodeHeading) {
					Serial.print("GPS H�he 1 (mm): "); Serial.print(UBXPVT1[UBXIdx1].hMSL);
					Serial.print(" GPS H�he 2 (mm): "); Serial.println(UBXPVT2[UBXIdx2].hMSL);
					Serial.print(" Antenna dist (cm): "); Serial.print(GPSSet.AntDist, 1);
					Serial.print(" Antenna hight (cm): "); Serial.print(GPSSet.AntHight, 1);
					Serial.print("Roll from GPS: "); Serial.println(roll, 4);
				}
			}
		}

	}
	/*	if (debugmode) {
			byte lastheadRingCount = 0;
			if (headRingCount == 0) { lastheadRingCount = 29; }
			else { lastheadRingCount = headRingCount - byte(1); }
			Serial.print("new Heading: "); Serial.print(HeadingRelPosNED);
			Serial.print(" new headRingCount: "); Serial.print(headRingCount);
			Serial.print(" last Heading: "); Serial.print(HeadingRelPosNED[lastheadRingCount]);
			Serial.print(" lastheadRingCount: "); Serial.println(lastheadRingCount);
		}
		*/
/*
}//UBX time fit
else {
	if (debugmode) { Serial.println("UBX present, but satellite time of antennas don't fit: NO heading/roll calc"); }
	dualGPSHeadingPresent = false;
	if (GPSSet.RollDevice == 1) { rollPresent = false; }
	dualAntNoValueCount++;//increase watchdog
}//UBX1+2 time don't fit
}
*/

/*
//old:
void virtualAntennaPoint() {
	double virtLatRad = double(UBXPVT1[UBXRingCount1].lat) / 10000000 * PI180;
	double virtLonRad = double(UBXPVT1[UBXRingCount1].lon) / 10000000 * PI180;
	double virtLatRadTemp = 0.0;
	double heading = HeadingRelPosNED;
	double WayToRadius = 0.0;

	optimizedPosPresent = false;

	if ((virtLatRad != 0.0) && (virtLonRad != 0)) {
		//shift Antenna to the right
		WayToRadius = GPSSet.virtAntRight / 637100080;//cm mean radius of earth WGS84 6.371.000,8m
		if (WayToRadius != 0.0)
		{
			double headingTemp = heading + 90;//move to right
			if (headingTemp > 360) { headingTemp = headingTemp - 360; }
			virtLatRadTemp = asin((sin(virtLatRad) * cos(WayToRadius)) + (cos(virtLatRad) * sin(WayToRadius) * cos(headingTemp)));
			virtLonRad = virtLonRad + atan2((sin(headingTemp) * sin(WayToRadius) * cos(virtLatRad)), (cos(WayToRadius) - (sin(virtLatRad) * sin(virtLatRadTemp))));
			virtLatRad = virtLatRadTemp;
			optimizedPosPresent = true;
		}
		//shift Antenna foreward
		WayToRadius = GPSSet.virtAntForew / 637100080;//cm mean radius of earth WGS84 6.371.000,8m
		if (WayToRadius != 0.0) {
			//move point x mm in heading direction
			virtLatRadTemp = asin((sin(virtLatRad) * cos(WayToRadius)) + (cos(virtLatRad) * sin(WayToRadius) * cos(heading)));
			virtLonRad = virtLonRad + atan2((sin(heading) * sin(WayToRadius) * cos(virtLatRad)), (cos(WayToRadius) - (sin(virtLatRad) * sin(virtLatRadTemp))));
			virtLatRad = virtLatRadTemp;
			optimizedPosPresent = true;
		}
		if (optimizedPosPresent) {
			//radians to dec
			virtLat = virtLatRad / PI180;
			virtLon = virtLonRad / PI180;
		}

		if (debugmode) {
			Serial.print("UBX1 Lat: "); Serial.print((double(UBXPVT1[UBXRingCount1].lat) / 10000000), 7); Serial.print("  virtual Lat: "); Serial.print(virtLat, 7);
			Serial.print("UBX1 Lon: "); Serial.print((double(UBXPVT1[UBXRingCount1].lon) / 10000000), 7); Serial.print("  virtual Lon: "); Serial.println(virtLon, 7);
		}
	}
}
*/
