void headingRollCalc() {

	rollPresent = false;
	dualGPSHeadingPresent = false;
	filterGPSpos = false;
	drivDirect = 0;//0=no value 1=foreward 2=backwards

	if (existsUBXRelPosNED) {
		//check if all values are vaild
		if ((bitRead(UBXRelPosNED.flags, 0)) || (GPSSet.checkUBXFlags == 0)) {//gnssFixOK?
			if (bitRead(UBXRelPosNED.flags, 2) || (GPSSet.checkUBXFlags == 0)) {//1 if relative position components and moving baseline are valid
			//RelPosNED OK: heading + roll calc												 

				if ((UBXRelPosNED.relPosLength > (GPSSet.AntDist / GPSSet.AntDistDeviationFactor)) && (UBXRelPosNED.relPosLength < (GPSSet.AntDist * GPSSet.AntDistDeviationFactor)))
				{
					//check if vector lenght between antennas is in range = indicator of heading/roll quality

					dualAntNoValue = 0;//reset watchdog

					if ((UBXRelPosNED.relPosLength > (GPSSet.AntDist / ((GPSSet.AntDistDeviationFactor / 3) + 0.667))) && (UBXRelPosNED.relPosLength < (GPSSet.AntDist * ((GPSSet.AntDistDeviationFactor / 3) + 0.667))))
					{	//check if vector lenght between antennas is in range = indicator of heading/roll quality							
						//for roll calc only 1/3 deviation !!
			//signal perfect: deviation less than 1/3

						headVarProcess = VarProcessFast;  //set Kalman filter
						headVTGVarProcess = VarProcessMedi; //set Kalman filter
						rollVarProcess = VarProcessFast; //set Kalman filter

						if (((GPSSet.headingAngleCorrection > 70) && (GPSSet.headingAngleCorrection < 110))
							|| ((GPSSet.headingAngleCorrection > 250) && (GPSSet.headingAngleCorrection < 290)))
						{//ant left+right: headingCorrectionAngle 90 or 270
				//roll
							roll = (atan2((double(UBXRelPosNED.relPosD) + (double(UBXRelPosNED.relPosHPD) / 100)), GPSSet.AntDist)) / PI180;
							roll -= GPSSet.rollAngleCorrection;
							noRollCount = 0;
							if (GPSSet.debugmodeHeading) { Serial.print("roll calc: "); Serial.print(roll); }
						}
						if (GPSSet.debugmodeFilterPos) { Serial.println("perfect Signal"); }
					}
					else
						//signal medium
					{
						headVarProcess = VarProcessSlow; //set Kalman filter
						headVTGVarProcess = VarProcessSlow; //set Kalman filter
						rollVarProcess = VarProcessVerySlow; //roll slowly to 0
						roll = 0.0;
						noRollCount++;
						//start filter GPS pos, set Kalman values
						if (GPSSet.filterGPSposOnWeakSignal == 1) {
							filterGPSpos = true;
							latVarProcess = VarProcessVeryFast; //set Kalman filter
							lonVarProcess = VarProcessVeryFast;
						}

						if (GPSSet.debugmodeFilterPos) { Serial.println("medium Signal"); }
						if (GPSSet.debugmode) {
							Serial.println("poor quality of GPS signal: NO roll calc, heading calc OK");
							Serial.print("Antenna distance set: "); Serial.print(GPSSet.AntDist); Serial.print("  Ant. dist from GPS: "); Serial.println(UBXRelPosNED.relPosLength);
						}
					}//end of deviation good or perfect					
				//heading
					headRingCount = (headRingCount + 1) % GPSHeadingArraySize;
					GPSHeading[headRingCount] = double(UBXRelPosNED.relPosHeading) * 0.00001;
					GPSHeading[headRingCount] = GPSHeading[headRingCount] + GPSSet.headingAngleCorrection;
					if (GPSHeading[headRingCount] >= 360) { GPSHeading[headRingCount] -= 360; }
					if (GPSHeading[headRingCount] < 0) { GPSHeading[headRingCount] += 360; }
					//go to cos for filtering to avoid 360-0° jump
					double cosHead = cos((GPSHeading[headRingCount] * PI180));
					if (GPSSet.debugmodeHeading) { Serial.print("heading calc: "); Serial.print(GPSHeading[headRingCount]); }
					//Kalman filter heading
					headK = cosHead;//input
					headPc = headP + headVarProcess;
					headG = headPc / (headPc + headVar);
					headP = (1 - headG) * headPc;
					headXp = headXe;
					headZp = headXp;
					headXe = (headG * (headK - headZp)) + headXp;//result
				//go back to degree
					if (GPSHeading[headRingCount] <= 180)
					{
						GPSHeading[headRingCount] = acos(headXe) / PI180;
					}
					else { GPSHeading[headRingCount] = 360 - (acos(headXe) / PI180); }

					dualGPSHeadingPresent = true;
					noHeadingCount = 0;

					if (GPSSet.debugmodeHeading) { Serial.print("heading filterd: "); Serial.print(GPSHeading[headRingCount]); }


					//filter roll
					rollK = roll;//input
					rollPc = rollP + rollVarProcess;
					rollG = rollPc / (rollPc + rollVar);
					rollP = (1 - rollG) * rollPc;
					rollXp = rollXe;
					rollZp = rollXp;
					rollXe = (rollG * (rollK - rollZp)) + rollXp;//result
					roll = rollXe;
					if (noRollCount < 8) { rollPresent = true; }
					else { noRollCount = 20; rollPresent = false; }//prevent overflow
					if (GPSSet.debugmodeHeading) {
						Serial.print(" roll filtered: "); Serial.println(roll);
						Serial.print("Antenna distance set: "); Serial.print(GPSSet.AntDist); Serial.print("  Ant. dist from GPS: "); Serial.println(UBXRelPosNED.relPosLength);
					}
				}

				//very poor signal quality, or one antenna: send only position
				else {
					//set Kalman filter for VTG heading
					if (UBXPVT1[UBXRingCount1].gSpeed > 100) //driving at least 0.36km/h
					{headVTGVarProcess = VarProcessVerySlow;}
					else { headVTGVarProcess = 0.00001; }

					noHeadingCount++;
					if (noHeadingCount < 9) { dualGPSHeadingPresent = true; }//use last heading 8x to calc virt position to avoid jumps
					if (GPSSet.filterGPSposOnWeakSignal == 1) {
						filterGPSpos = true;
						latVarProcess = VarProcessFast; //set Kalman filter
						lonVarProcess = VarProcessFast;
					}
					if (GPSSet.debugmodeFilterPos) { Serial.println("very weak Signal, or only 1 Antenna"); }
					if (GPSSet.debugmode || GPSSet.debugmodeHeading) {
						Serial.println("poor quality of GPS signal, or only 1 Antenna: NO heading/roll calc");
						Serial.print("Antenna distance set: "); Serial.print(GPSSet.AntDist); Serial.print("  Ant. dist from GPS: "); Serial.println(UBXRelPosNED.relPosLength);
					}
				}
			}

		//do this, if GNSFix is OK:

			//headingVTG Kalman filter go to cos for filtering to avoid 360-0° jump
			double cosHeadVTG = cos((UBXPVT1[UBXRingCount1].headMot * 0.00001 * PI180));
			headVTGK = cosHeadVTG;//input
			headVTGPc = headVTGP + headVTGVarProcess; //varProcess is set in relation to speed and signal quality
			headVTGG = headVTGPc / (headVTGPc + headVTGVar);
			headVTGP = (1 - headVTGG) * headVTGPc;
			headVTGXp = headVTGXe;
			headVTGZp = headVTGXp;
			headVTGXe = (headVTGG * (headVTGK - headVTGZp)) + headVTGXp;//result
			//go back to degree
			if (UBXPVT1[UBXRingCount1].headMot <= 18000000)
				{headingVTG = acos(headVTGXe) / PI180;	}
			else { headingVTG = 360 - (acos(headVTGXe) / PI180); }
			if (GPSSet.debugmodeHeading) { Serial.print("VTG heading (only from Ant R) filtered: "); Serial.println(headingVTG); }
		
			//driving direction calcs done here, after headingVTG and dual heading was filtered
			if (dualGPSHeadingPresent) {
				if (UBXPVT1[UBXRingCount1].gSpeed > 120) {//driving at least 0.43km/h
					//get heading of motion from postion GPS -> calc min/max value
					//for values from 359 to 0 min could get <0 and max could get >360
					//double temphead = UBXPVT1[UBXRingCount1].headMot * 0.00001;
					double temphead = headingVTG;
					double headmin = temphead + 330;
					if (headmin > 360) { headmin -= 360; }
					if (GPSHeading[headRingCount] < 30) { if (headmin > 330) { headmin -= 360; } }
					double headmax = temphead + 30;
					if (temphead < 330) { if (headmax > 360) { headmax -= 360; } }
					//driving forewards
					if (headmax < headmin) { headmax += 360; }
					if ((GPSHeading[headRingCount] > headmin) && (GPSHeading[headRingCount] < headmax)) {
						drivDirect = 1;
					}
					else {
						if (temphead > 330) { headmin -= 360; headmax -= 360; }
						if ((GPSHeading[headRingCount] > headmin) && (GPSHeading[headRingCount] < headmax)) { drivDirect = 1; }
					}
					//Serial.print("forew: min: "); Serial.print(headmin); Serial.print(" max: "); Serial.print(headmax); Serial.print(" heading: "); Serial.println(GPSHeading[headRingCount]);

					headmin = temphead + 150;
					if (headmin > 360) { headmin -= 360; }
					if (GPSHeading[headRingCount] < 30) { if (headmin > 330) { headmin -= 360; } }
					headmax = temphead + 210;
					if (headmax > 390) { headmax -= 360; }
					if (headmax < headmin) { headmax += 360; }
					//driving backwards?
					if ((GPSHeading[headRingCount] > headmin) && (GPSHeading[headRingCount] < headmax)) { drivDirect = 2; }
					else {
						if (GPSHeading[headRingCount] < 30) { headmin -= 360; headmax -= 360; }
						if ((GPSHeading[headRingCount] > headmin) && (GPSHeading[headRingCount] < headmax)) { drivDirect = 2; }
					}
					//Serial.print("backw: min: "); Serial.print(headmin); Serial.print(" max: "); Serial.print(headmax); Serial.print(" heading: "); Serial.println(GPSHeading[headRingCount]);
				}
				if (GPSSet.debugmodeHeading) { Serial.print("driving direction: "); Serial.println(drivDirect); }
			}
		}
		else { if (GPSSet.debugmode) { Serial.println("UBX RelPosNED flag: relative position not valid ->  NO heading + roll calc"); } }
	}
	else { if (GPSSet.debugmode) { Serial.println("UBX RelPosNED flag: DGPS fix not valid ->  NO heading + roll calc"); } }
}

//-------------------------------------------------------------------------------------------------

void virtualAntennaPoint() {
	double virtLatRad = double(UBXPVT1[UBXRingCount1].lat) / 10000000 * PI180;
	double virtLonRad = double(UBXPVT1[UBXRingCount1].lon) / 10000000 * PI180;
	double virtLatRadTemp = 0.0, virtAntHeadingDiff = 0.0, headingRad = 0.0, WayByRadius = 0.0;

	double toRight = 0, rollAbs = 0;
	bool rollNeg = false;
	virtAntPosPresent = false;

	if (rollPresent && (GPSSet.GPSPosCorrByRoll == 1)) { //calculate virtual Antenna point using roll
		toRight = tan((roll * PI180)) * GPSSet.AntHight;
		if (GPSSet.debugmodeVirtAnt) {
			Serial.print("roll degr: "); Serial.print(roll, 2); Serial.print(" to right by roll: ");
			Serial.println(toRight);
		}
		toRight = toRight - GPSSet.virtAntRight;
	}
	else {
		toRight = 0 - GPSSet.virtAntRight;
	}

	if (GPSSet.virtAntForew != 0) {
		if (toRight != 0) {
			//shift Antenna foreward and to the right: direction to move poit: heading-atan(foreward/right)
			virtAntHeadingDiff = (atan2(toRight, GPSSet.virtAntForew)) / PI180;
			//distance to move  cm  / mean radius of earth cm (WGS84 6.371.000,8m)
			double distAnt = sqrt((toRight * toRight) + (GPSSet.virtAntForew * GPSSet.virtAntForew));
			WayByRadius = distAnt / 637100080;
			if (GPSSet.debugmodeVirtAnt) {
				Serial.print("virt Ant: real dist to move"); Serial.print(distAnt);
				Serial.print(" WayByRadius: "); Serial.println(WayByRadius, 10);
				Serial.print(" angle corr by: "); Serial.println(virtAntHeadingDiff, 2);
			}
		}
		else {
			//shift Antenna only foreward
			virtAntHeadingDiff = 0.0;
			//distance to move  cm  / mean radius of earth cm (WGS84 6.371.000,8m)
			WayByRadius = GPSSet.virtAntForew / 637100080;

		}
	}
	else {
		if (toRight != 0) {
			//shift Antenna only right (needed for roll compensation)
			virtAntHeadingDiff = 90;
			WayByRadius = toRight / 637100080;
			if (GPSSet.debugmodeVirtAnt) {
				Serial.print("effective dist to move virt Ant: "); Serial.print(toRight);
				Serial.print(" WayToRaduis: "); Serial.println(WayByRadius, 12);
			}
		}
	}//direction+way



	if ((virtLatRad != 0.0) && (virtLonRad != 0.0)) {
		//all calculations in radians:  decimal * PI / 180
		headingRad = GPSHeading[headRingCount] + virtAntHeadingDiff;
		if (headingRad > 360) { headingRad -= 360; }
		else {
			if (headingRad < 0) { headingRad += 360; }
		}
		headingRad *= PI180;
		virtLatRadTemp = asin((sin(virtLatRad) * cos(WayByRadius)) + (cos(virtLatRad) * sin(WayByRadius) * cos(headingRad)));
		virtLonRad = virtLonRad + atan2((sin(headingRad) * sin(WayByRadius) * cos(virtLatRad)), (cos(WayByRadius) - (sin(virtLatRad) * sin(virtLatRadTemp))));
		virtLatRad = virtLatRadTemp;
		//radians to dec
		virtLat = virtLatRad / PI180;
		virtLon = virtLonRad / PI180;

		virtAntPosPresent = true;

		if (GPSSet.debugmodeVirtAnt) {
			Serial.println("Virtual Antenna point calc:");
			Serial.print("UBX1 Lat: "); Serial.print((double(UBXPVT1[UBXRingCount1].lat) / 10000000), 7); Serial.print(" virtLat: "); Serial.print(virtLat, 7);
			Serial.print("UBX1 Lon: "); Serial.print((double(UBXPVT1[UBXRingCount1].lon) / 10000000), 7); Serial.print(" virtLon: "); Serial.println(virtLon, 7);
			Serial.print("virt Point head: "); Serial.print((headingRad), 3); Serial.print(" GPS head: "); Serial.print(GPSHeading[headRingCount], 3);
			Serial.print(" roll: "); Serial.println(roll, 3);
		}
	}
	else { if (GPSSet.debugmode||GPSSet.debugmodeVirtAnt) { Serial.println("No virtual Antenna point: lat/lon = 0"); } }
}

//-------------------------------------------------------------------------------------------------


void filterPosition() {
	//input to the kalman filter
	if (virtAntPosPresent) { latK = virtLat; lonK = virtLon; }
	else { latK = long(UBXPVT1[UBXRingCount1].lat)* 0.0000001;lonK = long(UBXPVT1[UBXRingCount1].lon) * 0.0000001;}

	if (GPSSet.debugmodeHeading || GPSSet.debugmodeFilterPos) { Serial.print(" lat: "); Serial.print(latK, 7); }

	//Kalman filter
	latPc = latP + latVarProcess;
	latG = latPc / (latPc + latVar);
	latP = (1 - latG) * latPc;
	latXp = latXe;
	latZp = latXp;
	latXe = (latG * (latK - latZp)) + latXp;

	virtLat = latXe;//result

	//Kalman filter
	lonPc = lonP + lonVarProcess;
	lonG = lonPc / (lonPc + lonVar);
	lonP = (1 - lonG) * lonPc;
	lonXp = lonXe;
	lonZp = lonXp;
	lonXe = (lonG * (lonK - lonZp)) + lonXp;

	virtLon = lonXe;//result

	if (GPSSet.debugmodeHeading || GPSSet.debugmodeFilterPos) {
		Serial.print(" lat filtered: "); Serial.print(virtLat, 7);
		Serial.print(" lon: "); Serial.print(lonK, 7); Serial.print(" lon filtered: "); Serial.println(virtLon, 7);
	}
}
