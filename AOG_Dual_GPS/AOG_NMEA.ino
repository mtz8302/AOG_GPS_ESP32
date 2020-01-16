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
(1 , 2) 123519 Fix taken at 1219 UTC
(3 , 4) 4807.038,N Latitude 48 deg 07.038' N
(5, 6) 01131.000,E Longitude 11 deg 31.000' E
(7) 1 Fix quality: 0 = invalid
1 = GPS fix (SPS)
2 = DGPS fix
3 = PPS fix
4 = Real Time Kinematic
5 = Float RTK
6 = estimated (dead reckoning) (2.3 feature)
7 = Manual input mode
8 = Simulation mode
(8) 08 Number of satellites being tracked
(9) 0.9 Horizontal dilution of position
(10, 11) 545.4,M Altitude, Meters, above mean sea level
(12) 1.2 time in seconds since last DGPS update

From RMC or VTG:
(13) 022.4 Speed over the ground in knots
(14) 084.4 Track angle in degrees True

//remark MTZ8302: in AOG heading is only parsed once?? so one word is less in AOG!
FROM IMU:
(15) XXX.xx IMU Heading in degrees True
(16) XXX.xx Roll angle in degrees (What is a positive roll, left leaning - left down, right up?)
(17) XXX.xx Pitch angle in degrees (Positive pitch = nose up)
(18) XXX.xx Yaw Rate in Degrees / second
(19) T/F IMU status - Valid IMU Fusion

*CHKSUM
*/


//----------------------------------------------------------------------------------------------------------------------------------------------------



void buildOGI() {
	byte CRC = 0, OGIItem = 1;
	byte GPStrueNorth[3], GPSRollDigit[3];



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

	OGIBuffer[OGIdigit++] = 0x2C;//,

	//lat: xx min min . min/10 .. 4.5 digits
	long Lat = 0;
	if (virtAntPresent) { Lat = long(virtLat * 10000000); }
	else { Lat = UBXPVT1[UBXRingCount1].lat; }
	//if (debugmode) { Serial.print("UBX1 Lat (deg *10^-7): "); Serial.println(Lat); }
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
	if (virtAntPresent) { Lon = long(virtLon * 10000000); }
	else { Lon = UBXPVT1[UBXRingCount1].lon; }
	//if (debugmode) { Serial.print("UBX1 Lon (deg *10^-7): "); Serial.println(Lon); }
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

	OGIBuffer[OGIdigit++] = UBXPVT1[UBXRingCount1].fixType + 48;//fix type
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
	OGIBuffer[OGIdigit++] = 0x4D;//M
	OGIBuffer[OGIdigit++] = 0x2C;//,

    //shoud be age of NTRIP, not included in UBX PVT 
	OGIBuffer[OGIdigit++] = 0x2C;//,

	//ground speed knots
	long speed1000Knotes = (1.9438445 * UBXPVT1[UBXRingCount1].gSpeed);
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


	//remark MTZ8302: in AOG heading is only parsed once?? so one word is less in AOG! in OGI description: VTG heading, IMU heading,

	//GPS dual heading
	if (dualGPSHeadingPresent) {
		if (debugmode) { Serial.print("GPS Heading to OGI present: "); Serial.println(GPSHeading[headRingCount]); }
		double tempGPSHead = GPSHeading[headRingCount];
		temp = byte(tempGPSHead/100);
		tempGPSHead = tempGPSHead - temp*100;
		OGIBuffer[OGIdigit++] = temp + 48;
		temp = byte(tempGPSHead / 10);
		tempGPSHead = tempGPSHead - temp * 10;
		OGIBuffer[OGIdigit++] = temp + 48;
		temp = byte(tempGPSHead);
		tempGPSHead = tempGPSHead - temp;
		OGIBuffer[OGIdigit++] = temp + 48;
		OGIBuffer[OGIdigit++] = 0x2E;//.
		temp = byte(tempGPSHead*10);
		OGIBuffer[OGIdigit++] = temp + 48;
	}
	OGIBuffer[OGIdigit++] = 0x2C;//,

	//roll
	if (rollPresent) {
		double tempRoll = roll;
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
	}

	OGIBuffer[OGIdigit++] = 0x2C;//,
	//pitch
	OGIBuffer[OGIdigit++] = 0x2C;//,
	//yaw
	OGIBuffer[OGIdigit++] = 0x2C;//,
	//angualr velocity
	OGIBuffer[OGIdigit++] = 0x2C;//,
	//IMU valid?
/*	if (GPSHeadingPresent) {
		OGIBuffer[OGIdigit++] = 0x54;//T
	}
	else {
		OGIBuffer[OGIdigit++] = 0x46;//F 
	}
*/
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

/*
	if (debugmode) {
		Serial.println("build OGI from PVT:");
		for (byte n = 0; n < OGIdigit; n++) {
			Serial.print(char(OGIBuffer[n]));
		}
		Serial.println();
	}
	*/
}