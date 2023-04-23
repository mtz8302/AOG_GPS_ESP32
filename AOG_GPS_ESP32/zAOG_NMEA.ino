// Feb 2023 by Jaap
// Conversion to Hexidecimal
const char* asciiHex = "0123456789ABCDEF";

// GGA
char fixTime[12];
char latitude[15];
char latNS[3];
char longitude[15];
char lonEW[3];
char fixQuality[2];
char numSats[4];
char HDOP[5];
char altitude[12];
char ageDGPS[10];

// VTG
char vtgHeading[12] = { };
char speedKnots[10] = { };

//-------------------------------------------------------------------------------------------------
// If odd characters showed up.

void errorHandler()
{
	//nothing at the moment
}

//-------------------------------------------------------------------------------------------------

void VTG_Handler()
{
	// vtg heading
	parser.getArg(0, vtgHeading);

	// vtg Speed knots
	parser.getArg(4, speedKnots);	

	newVTG = true;
}

//-------------------------------------------------------------------------------------------------

void GGA_Handler() //Rec'd GGA
{
	// fix time
	parser.getArg(0, fixTime);

	// latitude
	parser.getArg(1, latitude);
	parser.getArg(2, latNS);

	// longitude
	parser.getArg(3, longitude);
	parser.getArg(4, lonEW);

	// fix quality
	parser.getArg(5, fixQuality);

	// satellite #
	parser.getArg(6, numSats);

	// HDOP
	parser.getArg(7, HDOP);

	// altitude
	parser.getArg(8, altitude);

	// time of last DGPS update
	parser.getArg(12, ageDGPS);

	newGGA = true;

	BuildPANDA();
	
}

//-------------------------------------------------------------------------------------------------

void BuildPANDA(void)
{
	strcpy(PANDA, "");

	//if (useDual) strcat(PANDA, "$PAOGI,");else 
	strcat(PANDA, "$PANDA,");

	strcat(PANDA, fixTime);
	strcat(PANDA, ",");

	strcat(PANDA, latitude);
	strcat(PANDA, ",");

	strcat(PANDA, latNS);
	strcat(PANDA, ",");

	strcat(PANDA, longitude);
	strcat(PANDA, ",");

	strcat(PANDA, lonEW);
	strcat(PANDA, ",");

	// 6
	strcat(PANDA, fixQuality);
	strcat(PANDA, ",");

	strcat(PANDA, numSats);
	strcat(PANDA, ",");

	strcat(PANDA, HDOP);
	strcat(PANDA, ",");

	strcat(PANDA, altitude);
	strcat(PANDA, ",");

	//10
	strcat(PANDA, ageDGPS);
	strcat(PANDA, ",");

	//11
	strcat(PANDA, speedKnots);
	strcat(PANDA, ",");

	//taking the IMU data that fits	
	byte fitIMUDataRingCount;
	if (IMUnoData >= IMUDataTimeShift) { fitIMUDataRingCount = IMUDataRingCount; }//reading IMU fails often, so take newest data
	else { fitIMUDataRingCount = (IMUDataRingCount + IMUDataTimeShift + IMUnoData) % 10; } //IMU time shift: GPS data is about 100ms old, so use older IMU data to be syncron

	if (Set.IMUType == 1) { IMUDataTimeShift = IMUDataTimeShiftBNO; }
	else if (Set.IMUType == 2) { IMUDataTimeShift = IMUDataTimeShiftCMPS; }

	IMUnoData = 0;

	//12
	itoa(IMUHeading[fitIMUDataRingCount], imuHeading, 10);
	strcat(PANDA, imuHeading);
	strcat(PANDA, ",");

	//13
	itoa(IMURoll[fitIMUDataRingCount], imuRoll, 10);
	strcat(PANDA, imuRoll);
	strcat(PANDA, ",");

	//14
	itoa(IMUPitch[fitIMUDataRingCount], imuPitch, 10);
	strcat(PANDA, imuPitch);
	strcat(PANDA, ",");

	//15
	itoa(IMUYawRate[fitIMUDataRingCount], imuYawRate, 10);
	strcat(PANDA, imuYawRate);

	strcat(PANDA, "*");

	CalculateChecksum();

	strcat(PANDA, "\r\n");

	newPANDA = true;
}

//-------------------------------------------------------------------------------------------------

void CalculateChecksum(void)
{
	int16_t sum = 0;
	int16_t inx = 0;
	char tmp;

	// The checksum calc starts after '$' and ends before '*'
	for (inx = 1; inx < 200; inx++)
	{
		tmp = PANDA[inx];

		// * Indicates end of data and start of checksum
		if (tmp == '*')
		{
			break;
		}

		sum ^= tmp;    // Build checksum
	}

	byte chk = (sum >> 4);
	char hex[2] = { asciiHex[chk], 0 };
	strcat(PANDA, hex);

	chk = (sum % 16);
	char hex2[2] = { asciiHex[chk], 0 };
	strcat(PANDA, hex2);
}

/*
  $PANDA
  (1) Time of fix

  position
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
  (11) Speed in knots

  FROM IMU:
  (12) Heading in degrees
  (13) Roll angle in degrees(positive roll = right leaning - right down, left up)

  (14) Pitch angle in degrees(Positive pitch = nose up)
  (15) Yaw Rate in Degrees / second

  CHKSUM
*/

/*
  $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M ,  ,*47
   0     1      2      3    4      5 6  7  8   9    10 11  12 13  14
		Time      Lat       Lon     FixSatsOP Alt
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
	 08           Number of satellites being tracked
	 0.9          Horizontal dilution of position
	 545.4,M      Altitude, Meters, above mean sea level
	 46.9,M       Height of geoid (mean sea level) above WGS84
					  ellipsoid
	 (empty field) time in seconds since last DGPS update
	 (empty field) DGPS station ID number
	  47          the checksum data, always begins with


  $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
  0      1    2   3      4    5      6   7     8     9     10   11
		Time      Lat        Lon       knots  Ang   Date  MagV

  Where:
	 RMC          Recommended Minimum sentence C
	 123519       Fix taken at 12:35:19 UTC
	 A            Status A=active or V=Void.
	 4807.038,N   Latitude 48 deg 07.038' N
	 01131.000,E  Longitude 11 deg 31.000' E
	 022.4        Speed over the ground in knots
	 084.4        Track angle in degrees True
	 230394       Date - 23rd of March 1994
	 003.1,W      Magnetic Variation
	  6A          The checksum data, always begins with

  $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48

	VTG          Track made good and ground speed
	054.7,T      True track made good (degrees)
	034.4,M      Magnetic track made good
	005.5,N      Ground speed, knots
	010.2,K      Ground speed, Kilometers per hour
	 48          Checksum
*/





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


