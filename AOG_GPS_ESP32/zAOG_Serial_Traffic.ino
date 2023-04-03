// Mrz 2023 stolen from Jaaps code

//checks ESP32 Serial1 for UBlox F9P chip: sends UBX request at different baudrates and wait for answer
void GPSGetBaudrate() {
	//UBX packet to send and ich GPS answers
	byte mon_rate[] = { 0xB5, 0x62, 0x06, 0x08, 0x00, 0x00, 0x00, 0x00 };
	byte mon_rate_size = sizeof(mon_rate);
	//add checksum
	for (int i = 2; i < (mon_rate_size - 2); i++) {
		mon_rate[mon_rate_size - 2] += ((unsigned char*)(&mon_rate))[i];
		mon_rate[mon_rate_size - 1] += mon_rate[mon_rate_size - 2];
	}

	// Check baudrate
	bool communicationSuccessfull = false, checkActualBaudrate = true;
	unsigned long baudrate = 0;

	Serial.println("Checking for GPS serial baudrate.");

	for (byte i = 0; i < nrBaudrates; i++)
	{
		if (checkActualBaudrate) {//first run, baudrate allready set is checked, then start with first in array
			baudrate = baudrates[Set.GPS_baudrate_Nr];

			Serial.print(F("Checking baudrate: "));
			Serial.println(baudrate);
		}
		else {
			baudrate = baudrates[i];

			Serial.print(F("Checking baudrate: "));
			Serial.println(baudrate);

			//Serial1.begin(Set.GPS_baudrate, SERIAL_8N1, Set.RX1, Set.TX1);
			Serial1.updateBaudRate(baudrate);
			delay(20);
			vTaskDelay(50);
		}

		// first send dumb data to make sure its on
		Serial1.write(0xFF);

		// Clear
		while (Serial1.available() > 0)
		{
			Serial1.read();
		}

		// Send request
		Serial1.write(mon_rate, 8);

		unsigned long millis_read = millis();
		const unsigned long UART_TIMEOUT = 1000;
		int ubxFrameCounter = 0;
		bool isUbx = false;
		uint8_t incoming = 0;

		uint8_t requestedClass = 6;// packetCfg.cls;
		uint8_t requestedID = 8;// packetCfg.id;

		uint8_t packetBufCls = 0;
		uint8_t packetBufId = 0;

		do
		{
			while (Serial1.available() > 0)
			{
				incoming = Serial1.read();

				//Serial.print(incoming); Serial.print(" ");

				if (!isUbx && incoming == 0xB5) // UBX binary frames start with 0xB5, aka ?
				{
					ubxFrameCounter = 0;
					isUbx = true;
				}

				if (isUbx)
				{
					// Decide what type of response this is
					if ((ubxFrameCounter == 0) && (incoming != 0xB5))      // ISO micro
					{
						isUbx = false;                                            // Something went wrong. Reset.
					}
					else if ((ubxFrameCounter == 1) && (incoming != 0x62)) // ASCII 'b'
					{
						isUbx = false;                                            // Something went wrong. Reset.
					}
					else if (ubxFrameCounter == 1 && incoming == 0x62)
					{
						// Serial.println("UBX_SYNCH_2");
						// isUbx should be still true
					}
					else if (ubxFrameCounter == 2) // Class
					{
						// Record the class in packetBuf until we know what to do with it
						packetBufCls = incoming; // (Duplication)
					}
					else if (ubxFrameCounter == 3) // ID
					{
						// Record the ID in packetBuf until we know what to do with it
						packetBufId = incoming; // (Duplication)

						// We can now identify the type of response
						// If the packet we are receiving is not an ACK then check for a class and ID match
						if (packetBufCls != 0x05)
						{
							// This is not an ACK so check for a class and ID match
							if ((packetBufCls == requestedClass) && (packetBufId == requestedID))
							{
								// This is not an ACK and we have a class and ID match
								communicationSuccessfull = true;
								//
								if (!checkActualBaudrate) { Set.GPS_baudrate_Nr = i; }
							}
							else
							{
								// This is not an ACK and we do not have a class and ID match
								// so we should keep diverting data into packetBuf and ignore the payload
								isUbx = false;
							}
						}
					}
				}

				// Finally, increment the frame counter
				ubxFrameCounter++;
			}
		} while (millis() - millis_read < UART_TIMEOUT);

		if (communicationSuccessfull)
		{
			break;
		}
		if (checkActualBaudrate) { checkActualBaudrate = false; i = 0; }//first run, baudrate allready set was checked, then start with first in array
	}

	if (communicationSuccessfull)
	{
		Serial.println();
		Serial.print(F("Found u-blox GPS reciever at baudrate: "));
		Serial.println(baudrate);
		GPSBaudrateValid = true;
	}
	else {
		Serial.println("u-blox GNSS not detected. Please check wiring.");
		GPSBaudrateValid = false;
	}
	vTaskDelay(5);
}

	/*	SerialAOG.write(aogSerialCmdBuffer, 6);
		SerialAOG.print(F("Found reciever at baudrate: "));
		SerialAOG.println(baudrate);

		// Let the configuring program know it can proceed
		SerialAOG.println("!AOGOK");
	}
	else
	{
		SerialAOG.println(F("u-blox GNSS not detected. Please check wiring."));
	}*/

