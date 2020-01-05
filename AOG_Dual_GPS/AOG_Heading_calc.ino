void headingRollCalc() {




	rollPresent = false;
	dualGPSHeadingPresent = false;

	if (existsUBXRelPosNED) {
		//check if all values are vaild
		if (bitRead(UBXRelPosNED.flags, 0)) {//gnssFixOK?
			if (bitRead(UBXRelPosNED.flags, 2)) {//1 if relative position components and moving baseline are valid

//RelPosNED OK: heading + roll calc	
				dualAntNoValue = 0;//reset watchdog

				headRingCount = (headRingCount + 1) % GPSHeadingArraySize;
				GPSHeading[headRingCount] = double(UBXRelPosNED.relPosHeading) * 0.00001;
				GPSHeading[headRingCount] = GPSHeading[headRingCount] + GPSSet.headingAngleCorrection;
				if (GPSHeading[headRingCount] >= 360) { GPSHeading[headRingCount] -= 360; }
				if (GPSHeading[headRingCount] < 0) { GPSHeading[headRingCount] += 360; }

				if (debugmodeHeading) { Serial.print("heading calc: "); Serial.print(GPSHeading[headRingCount]); }
				//Kalman filter heading
				//input to the kalman filter
				headK = GPSHeading[headRingCount];

				//Kalman filter
				headPc = headP + headVarProcess;
				headG = headPc / (headPc + headVar);
				headP = (1 - headG) * headPc;
				headXp = headXe;
				headZp = headXp;
				headXe = (headG * (headK - headZp)) + headXp;

				GPSHeading[headRingCount] = headXe;
				dualGPSHeadingPresent = true;
				if (debugmodeHeading) { Serial.print("heading filterd: "); Serial.print(GPSHeading[headRingCount]); }

				//roll
				roll = (atan2((double(UBXRelPosNED.relPosD) + (double(UBXRelPosNED.relPosHPD) / 100)), GPSSet.AntDist)) / PI180;
				if (debugmodeHeading) { Serial.print("roll calc: "); Serial.print(roll); }

				//input to the kalman filter
				rollK = roll;

				//Kalman filter
				rollPc = rollP + rollVarProcess;
				rollG = rollPc / (rollPc + rollVar);
				rollP = (1 - rollG) * rollPc;
				rollXp = rollXe;
				rollZp = rollXp;
				rollXe = (rollG * (rollK - rollZp)) + rollXp;

				roll = rollXe;
				rollPresent = true;
				if (debugmodeHeading) { Serial.print(" roll filtered: "); Serial.println(roll); }
			}
			else { if (debugmode) { Serial.println("UBX RelPosNED flag: relative position not valid ->  NO heading + roll calc"); } }
		}
		else { if (debugmode) { Serial.println("UBX RelPosNED flag: DGPS fix not valid ->  NO heading + roll calc"); } }
	}
}

//-------------------------------------------------------------------------------------------------

void virtualAntennaPoint() {
	double virtLatRad = double(UBXPVT1[UBXRingCount1].lat) / 10000000 * PI180;
	double virtLonRad = double(UBXPVT1[UBXRingCount1].lon) / 10000000 * PI180;
	double virtLatRadTemp = 0.0, virtAntHeadingDiff = 0.0, headingRad = 0.0, WayToRadius = 0.0;

	float toRight = 0, rollAbs = 0;
	bool rollNeg = false;

	if (GPSSet.GPSPosCorrByRoll == 1) { //calculate vitual Antenna point using roll
		toRight = tan((roll * PI180)) * GPSSet.AntHight;
		if (debugmodeVirtAnt) {
			Serial.print("roll°: "); Serial.print(roll, 2); Serial.print(" to right by roll: ");
			Serial.println(toRight);
		}
		toRight = GPSSet.virtAntRight + toRight;
	}
	else {
		toRight = GPSSet.virtAntRight;
	}

	if (GPSSet.virtAntForew != 0) {
		if (toRight != 0) {
			//shift Antenna foreward and to the right: direction to move poit: heading-atan(foreward/right)
			virtAntHeadingDiff = (atan2(toRight, GPSSet.virtAntForew)) / PI180;
			//distance to move  cm  / mean radius of earth cm (WGS84 6.371.000,8m)
			double distAnt = sqrt((toRight * toRight) + (GPSSet.virtAntForew * GPSSet.virtAntForew));
			WayToRadius = distAnt / 637100080;
			if (debugmodeVirtAnt) {
				Serial.print("virt Ant: real dist to move"); Serial.print(distAnt);
				Serial.print(" WayToRadius: "); Serial.println(WayToRadius, 10);
				Serial.print(" angle corr by: "); Serial.println(virtAntHeadingDiff, 2);
			}
		}
		else {
			//shift Antenna only foreward
			virtAntHeadingDiff = 0.0;
			//distance to move  cm  / mean radius of earth cm (WGS84 6.371.000,8m)
			WayToRadius = GPSSet.virtAntForew / 637100080;

		}
	}
	else {
		if (toRight != 0) {
			//shift Antenna only right (needed for roll compensation)
			virtAntHeadingDiff = 90;
			WayToRadius = toRight / 637100080;
			if (debugmodeVirtAnt) {
				Serial.print("effective dist to move virt Ant: "); Serial.print(toRight);
				Serial.print(" WayToRaduis: "); Serial.println(WayToRadius, 12);
			}
		}
	}//direction+way

	virtAntPresent = false;

	if ((virtLatRad != 0.0) && (virtLonRad != 0)) {
		//all calculations in radians:  decimal * PI / 180
		headingRad = GPSHeading[headRingCount] + virtAntHeadingDiff;
		if (headingRad > 360) { headingRad -= 360; }
		else {
			if (headingRad < 0) { headingRad += 360; }
		}
		headingRad *= PI180;
		virtLatRadTemp = asin((sin(virtLatRad) * cos(WayToRadius)) + (cos(virtLatRad) * sin(WayToRadius) * cos(headingRad)));
		virtLonRad = virtLonRad + atan2((sin(headingRad) * sin(WayToRadius) * cos(virtLatRad)), (cos(WayToRadius) - (sin(virtLatRad) * sin(virtLatRadTemp))));
		virtLatRad = virtLatRadTemp;
		//radians to dec
		virtLat = virtLatRad / PI180;
		virtLon = virtLonRad / PI180;

		if (filterGPSpos) {
			if (debugmodeHeading) {Serial.print("lat: "); Serial.print(virtLat, 7);}
			//input to the kalman filter
			latK = virtLat;

			//Kalman filter
			latPc = latP + latVarProcess;
			latG = latPc / (latPc + latVar);
			latP = (1 - latG) * latPc;
			latXp = latXe;
			latZp = latXp;
			latXe = (latG * (latK - latZp)) + latXp;

			virtLat = latXe;
			if (debugmodeHeading) {
				Serial.print(" lat filtered: "); Serial.print(virtLat, 7);
				Serial.print(" lon: "); Serial.print(virtLon,7);
			}

			//input to the kalman filter
			lonK = virtLon;

			//Kalman filter
			lonPc = lonP + lonVarProcess;
			lonG = lonPc / (lonPc + lonVar);
			lonP = (1 - lonG) * lonPc;
			lonXp = lonXe;
			lonZp = lonXp;
			lonXe = (lonG * (lonK - lonZp)) + lonXp;

			virtLon = lonXe;
			if (debugmodeHeading) { Serial.print(" lon filtered: "); Serial.println(virtLon,7); }
		}
		virtAntPresent = true;

		if (debugmodeVirtAnt) {
			Serial.println("Virtual Antenna point calc:");
			Serial.print("UBX1 Lat: "); Serial.print((double(UBXPVT1[UBXRingCount1].lat) / 10000000), 7); Serial.print(" virtLat: "); Serial.print(virtLat, 7);
			Serial.print("UBX1 Lon: "); Serial.print((double(UBXPVT1[UBXRingCount1].lon) / 10000000), 7); Serial.print(" virtLon: "); Serial.println(virtLon, 7);
			Serial.print("virt Point head: "); Serial.print((headingRad), 3); Serial.print(" GPS head: "); Serial.print(GPSHeading[headRingCount], 3);
			Serial.print(" roll: "); Serial.println(roll, 3);
		}
	}
	else { if (debugmode) { Serial.println("No virtual Antenna point: lat/lon = 0"); } }
}

//-------------------------------------------------------------------------------------------------


