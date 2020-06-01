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

