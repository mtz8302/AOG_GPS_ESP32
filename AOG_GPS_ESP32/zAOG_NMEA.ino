//$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M ,  ,*47
//   0     1      2      3    4      5 6  7  8   9    10 11  12 13  14
//        Time      Lat       Lon

/*
GGA - essential fix data which provide 3D location and accuracy data.

$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47

Where:
GGA          Global Positioning System Fix Data
123519       Fix taken at 12:35:19 UTC
4807.038,N   Latitude 48 deg 07.038' N
01131.000,E  Longitude 11 deg 31.000' E
1            Fix quality: 0 = invalid
1 = GPS fix (SPS)
2 = DGPS fix
3 = PPS fix
4 = Real Time Kinematic
5 = Float RTK
6 = estimated (dead reckoning) (2.3 feature)
7 = Manual input mode
8 = Simulation mode
(7)    08           Number of satellites being tracked
(8)    0.9          Horizontal dilution of position
545.4,M      Altitude, Meters, above mean sea level
46.9,M       Height of geoid (mean sea level) above WGS84
ellipsoid
(empty field) time in seconds since last DGPS update
(empty field) DGPS station ID number
*47          the checksum data, always begins with *
*
*

$GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48
*
VTG          Track made good and ground speed
054.7,T      True track made good (degrees)
034.4,M      Magnetic track made good
005.5,N      Ground speed, knots
010.2,K      Ground speed, Kilometers per hour
*48          Checksum
*
$PAOGI
* * From GGA:
				
				$PAOGI
				(1) 123519 Fix taken at 1219 UTC
				Roll corrected position
				(2,3) 4807.038,N Latitude 48 deg 07.038' N
				(4,5) 01131.000,E Longitude 11 deg 31.000' E
				(6) 1 Fix quality:
					0 = invalid
					1 = GPS fix(SPS)
					2 = DGPS fix
					3 = PPS fix
					4 = Real Time Kinematic
					5 = Float RTK
					6 = estimated(dead reckoning)(2.3 feature)
					7 = Manual input mode
					8 = Simulation mode
				(7) Number of satellites being tracked
				(8) 0.9 Horizontal dilution of position
				(9) 545.4 Altitude (ALWAYS in Meters, above mean sea level)
				(10) 1.2 time in seconds since last DGPS update
				(11) 022.4 Speed over the ground in knots - can be positive or negative
				FROM AHRS:
				(12) Heading in degrees
				(13) Roll angle in degrees(positive roll = right leaning - right down, left up)
				(14) Pitch angle in degrees(Positive pitch = nose up)
				(15) Yaw Rate in Degrees / second
				* CHKSUM
*/


//----------------------------------------------------------------------------------------------------------------------------------------------------


void buildOGI() {
	byte CRC = 0;

	//create $PAOGI
	OGIBuffer[0] = 0x24;//$
	OGIBuffer[1] = 0x50;//P
	OGIBuffer[2] = 0x41;//A
	OGIBuffer[3] = 0x4F;//O
	OGIBuffer[4] = 0x47;//G
	OGIBuffer[5] = 0x49;//I
	OGIBuffer[6] = 0x2C;//,

	OGIdigit = 7;

	//allways +48 to get ASCII: "0" = 48

	// time stamp hhmmss
	byte time = UBXPVT1[UBXRingCount1].hour;
	OGIBuffer[OGIdigit++] = (time / 10) + 48;
	time = time % 10;
	OGIBuffer[OGIdigit++] = time + 48;
	time = UBXPVT1[UBXRingCount1].min;
	OGIBuffer[OGIdigit++] = (time / 10) + 48;
	time = time % 10;
	OGIBuffer[OGIdigit++] = time + 48;
	time = UBXPVT1[UBXRingCount1].sec;
	OGIBuffer[OGIdigit++] = (time / 10) + 48;
	time = time % 10;
	OGIBuffer[OGIdigit++] = time + 48;
	OGIBuffer[OGIdigit++] = 0x2E;//.
	long timel = UBXPVT1[UBXRingCount1].nano / 1000000;//ms
	OGIBuffer[OGIdigit++] = (timel / 100) + 48;
	timel = timel % 100;
	OGIBuffer[OGIdigit++] = (timel / 10) + 48;

	OGIBuffer[OGIdigit++] = 0x2C;//,

	//lat: xx min min . min/10 .. 4.5 digits
	long Lat = 0;
	if (filterGPSpos || virtAntPosPresent) { Lat = long(virtLat * 10000000); }
	else { Lat = UBXPVT1[UBXRingCount1].lat; }
	if (GPSSet.debugmodeRAW) { Serial.print("buildOGI tempLat temLon,"); Serial.print(Lat); Serial.print(",");}
	//N/S?
	byte Sign = 0x53;//S
	if (Lat > 0) {Sign = 0x4E;}//N	
	Lat = abs(Lat);
	OGIBuffer[OGIdigit++] = (Lat / 100000000)+48;
	Lat = Lat % 100000000;
	OGIBuffer[OGIdigit++] = (Lat / 10000000) + 48;
	Lat = Lat % 10000000;
	Lat = Lat * 0.6;//dec to min
	OGIBuffer[OGIdigit++] = (Lat / 1000000) + 48;
	Lat = Lat % 1000000;
	OGIBuffer[OGIdigit++] = (Lat / 100000) + 48;
	Lat = Lat % 100000;
	OGIBuffer[OGIdigit++] = 0x2E;//.
	OGIBuffer[OGIdigit++] = (Lat / 10000) + 48;
	Lat = Lat % 10000;
	OGIBuffer[OGIdigit++] = (Lat / 1000) + 48;
	Lat = Lat % 1000;
	OGIBuffer[OGIdigit++] = (Lat /100)+ 48;
	Lat = Lat % 100;
	OGIBuffer[OGIdigit++] = (Lat / 10) + 48;
	Lat = Lat % 10;	
	OGIBuffer[OGIdigit++] = Lat + 48;

	OGIBuffer[OGIdigit++] = 0x2C;//,
	//N/S
	OGIBuffer[OGIdigit++] = Sign;
	OGIBuffer[OGIdigit++] = 0x2C;//,

	//lon: xxx min min . min/10 .. 5.5 digits
	long Lon = 0 ;
	if (filterGPSpos || virtAntPosPresent) { Lon = long(virtLon * 10000000); }
	else { Lon = UBXPVT1[UBXRingCount1].lon; }
	if (GPSSet.debugmodeRAW) { Serial.print(Lon); Serial.print(","); }
	//E/W?
	if (Lon < 0){Sign = 0x57;}//W
	else{Sign= 0x45;	}//E
	Lon = abs(Lon);
	OGIBuffer[OGIdigit++] = (Lon / 1000000000) + 48;
	Lon = Lon % 1000000000;
	OGIBuffer[OGIdigit++] = (Lon / 100000000) + 48;
	Lon = Lon % 100000000;
	OGIBuffer[OGIdigit++] = (Lon / 10000000) + 48;
	Lon = Lon % 10000000;
	Lon = Lon * 0.6;//dec to min
	OGIBuffer[OGIdigit++] = (Lon / 1000000) + 48;
	Lon = Lon % 1000000;
	OGIBuffer[OGIdigit++] = (Lon / 100000) + 48;
	Lon = Lon % 100000;
	OGIBuffer[OGIdigit++] = 0x2E;//.
	OGIBuffer[OGIdigit++] = (Lon / 10000) + 48;
	Lon = Lon % 10000;
	OGIBuffer[OGIdigit++] = (Lon / 1000) + 48;
	Lon = Lon % 1000;
	OGIBuffer[OGIdigit++] = (Lon / 100) + 48;
	Lon = Lon % 100;
	OGIBuffer[OGIdigit++] = (Lon / 10) + 48;
	Lon = Lon % 10;
	OGIBuffer[OGIdigit++] = Lon + 48;

	OGIBuffer[OGIdigit++] = 0x2C;//,
	//E/W
	OGIBuffer[OGIdigit++] = Sign;
	OGIBuffer[OGIdigit++] = 0x2C;//,

	//fix type
	if ((bitRead(UBXPVT1[UBXRingCount1].flags, 1) == true) && (UBXPVT1[UBXRingCount1].fixType == 3)) { OGIBuffer[OGIdigit++] = 52; }//4 = RTK
	else { OGIBuffer[OGIdigit++] = UBXPVT1[UBXRingCount1].fixType + 48; }
	OGIBuffer[OGIdigit++] = 0x2C;//,

	byte temp = UBXPVT1[UBXRingCount1].numSV;//number of satellites
	OGIBuffer[OGIdigit++] = (temp / 10) + 48;
	temp = temp % 10;
	OGIBuffer[OGIdigit++] = temp + 48;
	OGIBuffer[OGIdigit++] = 0x2C;//,

	unsigned short pDOP = UBXPVT1[UBXRingCount1].pDOP;//Disolution of Position  might be wrong item and/or wrong scale
	OGIBuffer[OGIdigit++] = (pDOP / 100) + 48;
	pDOP = pDOP % 100;
	OGIBuffer[OGIdigit++] = 0x2E;//.
	OGIBuffer[OGIdigit++] = (pDOP / 10) + 48;
	pDOP = pDOP % 10;
	OGIBuffer[OGIdigit++] = pDOP + 48;
	OGIBuffer[OGIdigit++] = 0x2C;//,

	long templng = UBXPVT1[UBXRingCount1].hMSL;//hight main sea level mm
	OGIBuffer[OGIdigit++] = (templng / 1000000) + 48;
	templng = templng % 1000000;
	OGIBuffer[OGIdigit++] = (templng / 100000) + 48;
	templng = templng % 100000;
	OGIBuffer[OGIdigit++] = (templng / 10000) + 48;
	templng = templng % 10000;
	OGIBuffer[OGIdigit++] = (templng / 1000) + 48;
	templng = templng % 1000;
	OGIBuffer[OGIdigit++] = 0x2E;//.
	OGIBuffer[OGIdigit++] = (templng / 100) + 48;
	templng = templng % 100;
	OGIBuffer[OGIdigit++] = (templng / 10) + 48;
	OGIBuffer[OGIdigit++] = 0x2C;//,

    //shoud be age of NTRIP, not included in UBX PVT 
	OGIBuffer[OGIdigit++] = 0x2C;//,

	//ground speed knots
	if (drivDirect == 2) {//backwards, so write "-"
		OGIBuffer[OGIdigit++] = 0x2D;}
	long speed1000Knotes;
	if (UBXPVT1[UBXRingCount1].gSpeed > 30) { speed1000Knotes = (1.9438445 * UBXPVT1[UBXRingCount1].gSpeed); }
	else speed1000Knotes = 0;//send 0 if slower than 0,1km/h
	OGIBuffer[OGIdigit++] = (speed1000Knotes / 10000) + 48;
	speed1000Knotes = speed1000Knotes % 10000;
	OGIBuffer[OGIdigit++] = (speed1000Knotes / 1000)+48;
	speed1000Knotes = speed1000Knotes % 1000;
	OGIBuffer[OGIdigit++] = 0x2E;//.
	OGIBuffer[OGIdigit++] = (speed1000Knotes / 100)+48;
	speed1000Knotes = speed1000Knotes %100;
	OGIBuffer[OGIdigit++] = (speed1000Knotes / 10)+48;
	speed1000Knotes = speed1000Knotes%10;
	OGIBuffer[OGIdigit++] = speed1000Knotes+48;

	OGIBuffer[OGIdigit++] = 0x2C;//,


	//GPS dual heading
	double tempGPSHead;
	tempGPSHead = HeadingMix;
	temp = byte(tempGPSHead / 100);
	tempGPSHead = tempGPSHead - temp * 100;
	OGIBuffer[OGIdigit++] = temp + 48;
	temp = byte(tempGPSHead / 10);
	tempGPSHead = tempGPSHead - temp * 10;
	OGIBuffer[OGIdigit++] = temp + 48;
	temp = byte(tempGPSHead);
	tempGPSHead = tempGPSHead - temp;
	OGIBuffer[OGIdigit++] = temp + 48;
	OGIBuffer[OGIdigit++] = 0x2E;//.
	temp = byte(tempGPSHead * 10);
	OGIBuffer[OGIdigit++] = temp + 48;
	OGIBuffer[OGIdigit++] = 0x2C;//,

	//roll
	double tempRoll = 0 - roll;
	if (tempRoll < 0) {
		OGIBuffer[OGIdigit++] = 0x2D;//-
		tempRoll *= -1;
	}
	temp = byte(tempRoll / 10);
	tempRoll = tempRoll - temp * 10;
	OGIBuffer[OGIdigit++] = temp + 48;
	temp = byte(tempRoll);
	tempRoll = tempRoll - temp;
	OGIBuffer[OGIdigit++] = temp + 48;
	OGIBuffer[OGIdigit++] = 0x2E;//.
	temp = byte(tempRoll * 10);
	OGIBuffer[OGIdigit++] = temp + 48;
	OGIBuffer[OGIdigit++] = 0x2C;//,

	//pitch
	OGIBuffer[OGIdigit++] = 0x2C;//,
	//yaw
	OGIBuffer[OGIdigit++] = 0x2C;//,
	//angular velocity
	//...,

	OGIBuffer[OGIdigit++] = 0x2A;//*

	//checksum
	for (byte index = 1; index <= (OGIdigit); index++) {
		if (OGIBuffer[index] == 0x2A) {
			break;
		}
		CRC = CRC ^ OGIBuffer[index];
	}
	OGIBuffer[OGIdigit++] = byte(CRC / 16) + 48;
	if (OGIBuffer[OGIdigit - 1] > 57) { OGIBuffer[OGIdigit-1] += 7; }//ASCII 48 = 0 ASCII A = 65
	OGIBuffer[OGIdigit++] = byte(CRC % 16) + 48;
	if (OGIBuffer[OGIdigit - 1] > 57) { OGIBuffer[OGIdigit - 1] += 7; }//ASCII 48 = 0 ASCII A = 65
	OGIBuffer[OGIdigit++] = 0x0D;
	OGIBuffer[OGIdigit++] = 0x0A;	
	
	
	OGIfromUBX = UBXRingCount1;
	newOGI = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------


void buildHDT() {
	byte CRC = 0;
	byte tempbyt = 0;

	HDTBuffer[0] = 0x24;//$
	HDTBuffer[1] = 0x47;//G
	HDTBuffer[2] = 0x4E;//N
	HDTBuffer[3] = 0x48;//H
	HDTBuffer[4] = 0x44;//D
	HDTBuffer[5] = 0x54;//T
	HDTBuffer[6] = 0x2C;//,
	//heading
	double tempGPSHead;
	if (GPSSet.useMixedHeading) {
		if (GPSSet.debugmode) { Serial.print("mix Heading to OGI present: "); Serial.println(HeadingMix); }
		tempGPSHead = HeadingMix;
	}
	else { tempGPSHead = HeadingRelPosNED; }
	tempbyt = byte(tempGPSHead / 100);
	tempGPSHead = tempGPSHead - tempbyt * 100;
	HDTBuffer[7] = tempbyt + 48;
	tempbyt = byte(tempGPSHead / 10);
	tempGPSHead = tempGPSHead - tempbyt * 10;
	HDTBuffer[8] = tempbyt + 48;
	tempbyt = byte(tempGPSHead);
	tempGPSHead = tempGPSHead - tempbyt;
	HDTBuffer[9] = tempbyt + 48;
	HDTBuffer[10] = 0x2E;//.
	tempbyt = byte(tempGPSHead * 10);
	HDTBuffer[11] = tempbyt + 48;

	HDTBuffer[12] = 0x2C;//,
	HDTBuffer[13] = 0x54;//T
	HDTBuffer[14] = 0x2A;//*
	//checksum
	for (byte index = 1; index <= 13; index++) {
		CRC = CRC ^ HDTBuffer[index];
	}
	HDTBuffer[15] = byte(CRC / 16) + 48;
	if (HDTBuffer[15] > 57) { HDTBuffer[15] += 7; }//ASCII 48 = 0 ASCII A = 65
	HDTBuffer[16] = byte(CRC % 16) + 48;
	if (HDTBuffer[16] > 57) { HDTBuffer[16] += 7; }//ASCII 48 = 0 ASCII A = 65
	HDTBuffer[17] = 0x0D;
	HDTBuffer[18] = 0x0A;
	HDTdigit = 19;
	newHDT = true;
}


//----------------------------------------------------------------------------------------------------------------------------------------------------



void buildGGA() {
	byte CRC = 0;

	//create $GPGGA
	GGABuffer[0] = 0x24;//$
	GGABuffer[1] = 0x47;//G
	GGABuffer[2] = 0x50;//P
	GGABuffer[3] = 0x47;//G
	GGABuffer[4] = 0x47;//G
	GGABuffer[5] = 0x41;//A
	GGABuffer[6] = 0x2C;//,

	GGAdigit = 7;

	//allways +48 to get ASCII: "0" = 48

	// time stamp hhmmss
	byte time = UBXPVT1[UBXRingCount1].hour;
	GGABuffer[GGAdigit++] = (time / 10) + 48;
	time = time % 10;
	GGABuffer[GGAdigit++] = time + 48;
	time = UBXPVT1[UBXRingCount1].min;
	GGABuffer[GGAdigit++] = (time / 10) + 48;
	time = time % 10;
	GGABuffer[GGAdigit++] = time + 48;
	time = UBXPVT1[UBXRingCount1].sec;
	GGABuffer[GGAdigit++] = (time / 10) + 48;
	time = time % 10;
	GGABuffer[GGAdigit++] = time + 48;
	GGABuffer[GGAdigit++] = 0x2E;//.
	long timel = UBXPVT1[UBXRingCount1].nano / 1000000;//ms
	GGABuffer[GGAdigit++] = (timel / 100) + 48;
	timel = timel % 100;
	GGABuffer[GGAdigit++] = (timel / 10) + 48;

	GGABuffer[GGAdigit++] = 0x2C;//,

	//lat: xx min min . min/10 .. 4.5 digits
	long Lat = UBXPVT1[UBXRingCount1].lat;
//	if (filterGPSpos || virtAntPosPresent) { Lat = long(virtLat * 10000000); }
//	else { Lat = UBXPVT1[UBXRingCount1].lat; }
	//if (debugmode) { Serial.print("UBX1 Lat (deg *10^-7): "); Serial.println(Lat); }
	//N/S?
	byte Sign = 0x53;//S
	if (Lat > 0) { Sign = 0x4E; }//N	
	Lat = abs(Lat);
	GGABuffer[GGAdigit++] = (Lat / 100000000) + 48;
	Lat = Lat % 100000000;
	GGABuffer[GGAdigit++] = (Lat / 10000000) + 48;
	Lat = Lat % 10000000;
	Lat = Lat * 0.6;//dec to min
	GGABuffer[GGAdigit++] = (Lat / 1000000) + 48;
	Lat = Lat % 1000000;
	GGABuffer[GGAdigit++] = (Lat / 100000) + 48;
	Lat = Lat % 100000;
	GGABuffer[GGAdigit++] = 0x2E;//.
	GGABuffer[GGAdigit++] = (Lat / 10000) + 48;
	Lat = Lat % 10000;
	GGABuffer[GGAdigit++] = (Lat / 1000) + 48;
	Lat = Lat % 1000;
	GGABuffer[GGAdigit++] = (Lat / 100) + 48;
	Lat = Lat % 100;
	GGABuffer[GGAdigit++] = (Lat / 10) + 48;
	Lat = Lat % 10;
	GGABuffer[GGAdigit++] = Lat + 48;

	GGABuffer[GGAdigit++] = 0x2C;//,
	//N/S
	GGABuffer[GGAdigit++] = Sign;
	GGABuffer[GGAdigit++] = 0x2C;//,

	//lon: xxx min min . min/10 .. 5.5 digits
	long Lon = UBXPVT1[UBXRingCount1].lon;
//	if (filterGPSpos || virtAntPosPresent) { Lon = long(virtLon * 10000000); }
//	else { Lon = UBXPVT1[UBXRingCount1].lon; }
	//if (debugmode) { Serial.print("UBX1 Lon (deg *10^-7): "); Serial.println(Lon); }
	//E/W?
	if (Lon < 0) { Sign = 0x57; }//W
	else { Sign = 0x45; }//E
	Lon = abs(Lon);
	GGABuffer[GGAdigit++] = (Lon / 1000000000) + 48;
	Lon = Lon % 1000000000;
	GGABuffer[GGAdigit++] = (Lon / 100000000) + 48;
	Lon = Lon % 100000000;
	GGABuffer[GGAdigit++] = (Lon / 10000000) + 48;
	Lon = Lon % 10000000;
	Lon = Lon * 0.6;//dec to min
	GGABuffer[GGAdigit++] = (Lon / 1000000) + 48;
	Lon = Lon % 1000000;
	GGABuffer[GGAdigit++] = (Lon / 100000) + 48;
	Lon = Lon % 100000;
	GGABuffer[GGAdigit++] = 0x2E;//.
	GGABuffer[GGAdigit++] = (Lon / 10000) + 48;
	Lon = Lon % 10000;
	GGABuffer[GGAdigit++] = (Lon / 1000) + 48;
	Lon = Lon % 1000;
	GGABuffer[GGAdigit++] = (Lon / 100) + 48;
	Lon = Lon % 100;
	GGABuffer[GGAdigit++] = (Lon / 10) + 48;
	Lon = Lon % 10;
	GGABuffer[GGAdigit++] = Lon + 48;

	GGABuffer[GGAdigit++] = 0x2C;//,
	//E/W
	GGABuffer[GGAdigit++] = Sign;
	GGABuffer[GGAdigit++] = 0x2C;//,

	//fix type
	if ((bitRead(UBXPVT1[UBXRingCount1].flags, 1) == true) && (UBXPVT1[UBXRingCount1].fixType == 3)) { GGABuffer[GGAdigit++] = 52; }//4 = RTK
	else { GGABuffer[GGAdigit++] = UBXPVT1[UBXRingCount1].fixType + 48; }
	GGABuffer[GGAdigit++] = 0x2C;//,

	byte temp = UBXPVT1[UBXRingCount1].numSV;//number of satellites
	GGABuffer[GGAdigit++] = (temp / 10) + 48;
	temp = temp % 10;
	GGABuffer[GGAdigit++] = temp + 48;
	GGABuffer[GGAdigit++] = 0x2C;//,

	unsigned short pDOP = UBXPVT1[UBXRingCount1].pDOP;//Disolution of Position  might be wrong item and/or wrong scale
	GGABuffer[GGAdigit++] = (pDOP / 100) + 48;
	pDOP = pDOP % 100;
	GGABuffer[GGAdigit++] = 0x2E;//.
	GGABuffer[GGAdigit++] = (pDOP / 10) + 48;
	pDOP = pDOP % 10;
	GGABuffer[GGAdigit++] = pDOP + 48;
	GGABuffer[GGAdigit++] = 0x2C;//,

	long templng = UBXPVT1[UBXRingCount1].hMSL;//hight main sea level mm
	GGABuffer[GGAdigit++] = (templng / 1000000) + 48;
	templng = templng % 1000000;
	GGABuffer[GGAdigit++] = (templng / 100000) + 48;
	templng = templng % 100000;
	GGABuffer[GGAdigit++] = (templng / 10000) + 48;
	templng = templng % 10000;
	GGABuffer[GGAdigit++] = (templng / 1000) + 48;
	templng = templng % 1000;
	GGABuffer[GGAdigit++] = 0x2E;//.
	GGABuffer[GGAdigit++] = (templng / 100) + 48;
	templng = templng % 100;
	GGABuffer[GGAdigit++] = (templng / 10) + 48;
	GGABuffer[GGAdigit++] = 0x2C;//,
	GGABuffer[GGAdigit++] = 0x4D;//M
	GGABuffer[GGAdigit++] = 0x2C;//,
	//no hight above geoid
	GGABuffer[GGAdigit++] = 0x2C;//,	
	//so no M
	GGABuffer[GGAdigit++] = 0x2C;//,
	//shoud be age of NTRIP, not included in UBX PVT 
	GGABuffer[GGAdigit++] = 0x2C;//,
	//no station numer


	GGABuffer[GGAdigit++] = 0x2A;//*

	//checksum
	for (byte index = 1; index <= (GGAdigit); index++) {
		if (GGABuffer[index] == 0x2A) {
			break;
		}
		CRC = CRC ^ GGABuffer[index];
	}
	GGABuffer[GGAdigit++] = byte(CRC / 16) + 48;
	if (GGABuffer[GGAdigit - 1] > 57) { GGABuffer[GGAdigit - 1] += 7; }//ASCII 48 = 0 ASCII A = 65
	GGABuffer[GGAdigit++] = byte(CRC % 16) + 48;
	if (GGABuffer[GGAdigit - 1] > 57) { GGABuffer[GGAdigit - 1] += 7; }//ASCII 48 = 0 ASCII A = 65
	GGABuffer[GGAdigit++] = 0x0D;
	GGABuffer[GGAdigit++] = 0x0A;


	newGGA = true;

	/*
		if (debugmode) {
			Serial.println("build OGI from PVT:");
			for (byte n = 0; n < GGAdigit; n++) {
				Serial.print(char(GGABuffer[n]));
			}
			Serial.println();
		}
		*/
}


//----------------------------------------------------------------------------------------------------------------------------------------------------


void buildVTG() {
	byte tempbyt = 0, CRC = 0;

	//create $GPVTG
	VTGBuffer[0] = 0x24;//$
	VTGBuffer[1] = 0x47;//G
	VTGBuffer[2] = 0x50;//P
	VTGBuffer[3] = 0x56;//V
	VTGBuffer[4] = 0x54;//T
	VTGBuffer[5] = 0x47;//G
	VTGBuffer[6] = 0x2C;//,

	VTGdigit = 7;

	//allways +48 to get ASCII: "0" = 48
	double tempGPSHead;
//	if (GPSSet.useMixedHeading) {
//		if (GPSSet.debugmode) { Serial.print("mix Heading to OGI present: "); Serial.println(HeadingMix); }
		tempGPSHead = HeadingMix; //decided in Heading calc
/*	}
	else {
		if (dualGPSHeadingPresent) { tempGPSHead = HeadingRelPosNED; }
		else {
			if (GPSSet.debugmode) { Serial.print("VTG Heading to OGI present: "); Serial.println(HeadingVTG); }
			tempGPSHead = HeadingVTG;
		}
	}*/
	tempbyt = byte(tempGPSHead / 100);
	tempGPSHead = tempGPSHead - tempbyt * 100;
	VTGBuffer[VTGdigit++] = tempbyt + 48;
	tempbyt = byte(tempGPSHead / 10);
	tempGPSHead = tempGPSHead - tempbyt * 10;
	VTGBuffer[VTGdigit++] = tempbyt + 48;
	tempbyt = byte(tempGPSHead);
	tempGPSHead = tempGPSHead - tempbyt;
	VTGBuffer[VTGdigit++] = tempbyt + 48;
	VTGBuffer[VTGdigit++] = 0x2E;//.
	tempbyt = byte(tempGPSHead * 10);
	VTGBuffer[VTGdigit++] = tempbyt + 48;
	VTGBuffer[VTGdigit++] = 0x2C;//,
	VTGBuffer[VTGdigit++] = 0x54;//T
	VTGBuffer[VTGdigit++] = 0x2C;//,
	//no magnect heading
	VTGBuffer[VTGdigit++] = 0x2C;//,
	//no M
	VTGBuffer[VTGdigit++] = 0x2C;//,

	//ground speed knots
	if (drivDirect == 2) {//backwards, so write "-"
		VTGBuffer[VTGdigit++] = 0x2D;
	}
	long speed1000Knotes;
	if (UBXPVT1[UBXRingCount1].gSpeed > 30) { speed1000Knotes = (1.9438445 * UBXPVT1[UBXRingCount1].gSpeed); }
	else speed1000Knotes = 0;//send 0 if slower than 0,1km/h
	VTGBuffer[VTGdigit++] = (speed1000Knotes / 10000) + 48;
	speed1000Knotes = speed1000Knotes % 10000;
	VTGBuffer[VTGdigit++] = (speed1000Knotes / 1000) + 48;
	speed1000Knotes = speed1000Knotes % 1000;
	VTGBuffer[VTGdigit++] = 0x2E;//.
	VTGBuffer[VTGdigit++] = (speed1000Knotes / 100) + 48;
	speed1000Knotes = speed1000Knotes % 100;
	VTGBuffer[VTGdigit++] = (speed1000Knotes / 10) + 48;
	speed1000Knotes = speed1000Knotes % 10;
	VTGBuffer[VTGdigit++] = speed1000Knotes + 48;
	VTGBuffer[VTGdigit++] = 0x2C;//,
	VTGBuffer[VTGdigit++] = 0x4E;//N
	VTGBuffer[VTGdigit++] = 0x2C;//,

	//ground speed km/h
	long speed1000kmh = (3.6 * UBXPVT1[UBXRingCount1].gSpeed);
	VTGBuffer[VTGdigit++] = (speed1000kmh / 10000) + 48;
	speed1000kmh = speed1000kmh % 10000;
	VTGBuffer[VTGdigit++] = (speed1000kmh / 1000) + 48;
	speed1000kmh = speed1000kmh % 1000;
	VTGBuffer[VTGdigit++] = 0x2E;//.
	VTGBuffer[VTGdigit++] = (speed1000kmh / 100) + 48;
	speed1000kmh = speed1000kmh % 100;
	VTGBuffer[VTGdigit++] = (speed1000kmh / 10) + 48;
	speed1000kmh = speed1000kmh % 10;
	VTGBuffer[VTGdigit++] = speed1000kmh + 48;

	VTGBuffer[VTGdigit++] = 0x4B;//K


	VTGBuffer[VTGdigit++] = 0x2A;//*

	//checksum
	for (byte index = 1; index <= (VTGdigit); index++) {
		if (VTGBuffer[index] == 0x2A) {
			break;
		}
		CRC = CRC ^ VTGBuffer[index];
	}
	VTGBuffer[VTGdigit++] = byte(CRC / 16) + 48;
	if (VTGBuffer[VTGdigit - 1] > 57) { VTGBuffer[VTGdigit - 1] += 7; }//ASCII 48 = 0 ASCII A = 65
	VTGBuffer[VTGdigit++] = byte(CRC % 16) + 48;
	if (VTGBuffer[VTGdigit - 1] > 57) { VTGBuffer[VTGdigit - 1] += 7; }//ASCII 48 = 0 ASCII A = 65
	VTGBuffer[VTGdigit++] = 0x0D;
	VTGBuffer[VTGdigit++] = 0x0A;

	newVTG = true;
}
