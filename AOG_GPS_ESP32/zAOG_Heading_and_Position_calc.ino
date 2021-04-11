void headingRollCalc() {

	rollRelPosNEDPresent = false;
	dualGPSHeadingPresent = false;
	filterGPSpos = false;
	add360ToRelPosNED = false;
	add360ToVTG = false;

	if (existsUBXRelPosNED) {
		CMPS14CorrFactor = 0.0;//default, only correct CMPS14 heading with GPS, if GPS is good
		//check if all values are vaild
		if ((bitRead(UBXRelPosNED[UBXRingCount2].flags, 0)) || (Set.checkUBXFlags == 0)) {//gnssFixOK?
			if (bitRead(UBXRelPosNED[UBXRingCount2].flags, 2) || (Set.checkUBXFlags == 0)) {//1 if relative position components and moving baseline are valid
			//RelPosNED OK: heading + roll calc												 

				if ((UBXRelPosNED[UBXRingCount2].relPosLength > (Set.AntDist / Set.AntDistDeviationFactor)) && (UBXRelPosNED[UBXRingCount2].relPosLength < (Set.AntDist * Set.AntDistDeviationFactor)))
				{//check if vector lenght between antennas is in range = indicator of heading/roll quality
					

					dualAntNoValueCount = 0;//reset watchdog

					if ((UBXRelPosNED[UBXRingCount2].relPosLength > (Set.AntDist / ((Set.AntDistDeviationFactor / 4) + 0.75))) && (UBXRelPosNED[UBXRingCount2].relPosLength < (Set.AntDist * ((Set.AntDistDeviationFactor / 4) + 0.75))))
					{	//check if vector lenght between antennas is in range = indicator of heading/roll quality							
						//for roll calc only 1/4 deviation !!

				//signal perfect: deviation less than 1/4
						//set filters for heading
						if (UBXPVT1[UBXRingCount1].gSpeed > 500) //driving at least 1.8km/h
						{
							headVTGVarProcess = VarProcessMedi; //set Kalman filter
							if (drivDirect < 2) {//forewards or unknown
								if (UBXRelPosNED[UBXRingCount2].relPosHeading != 0) {//RelPosNED ok								
									HeadingQualFactor = 0.8;//0.8
									HeadingQuotaVTG = 0.0;
									HeadingQuotaCMPS = 0.0;//0
									if ((Set.CMPS14_present) && (UBXRelPosNED[UBXRingCount2].flags == 311)) 
									{ CMPS14CorrFactor = 0.002; }//0,03 = 350 loops to be at 0.008 degree
								}
								else {//RelPosNED fails
									if (Set.CMPS14_present) { HeadingQualFactor = 0.7; }
									else { HeadingQualFactor = 0.5; }
									HeadingQuotaVTG = 1.0;
									HeadingQuotaCMPS = 1.0;
								}
							}
							else { //backwards
								HeadingQualFactor = 0.8;//0,4
								HeadingQuotaVTG = 0.0;
								HeadingQuotaCMPS = 0.2;//0
							}
						}
						else {//low speed
							HeadingQualFactor = 0.6;//0,4
							HeadingQuotaCMPS = 0.2;
							HeadingQuotaVTG = 0.0;//0
							//headVarProcess = VarProcessMedi;  //set Kalman filter
							headVTGVarProcess = VarProcessSlow; //set Kalman filter
						}

						if (((Set.headingAngleCorrection > 70) && (Set.headingAngleCorrection < 110))
							|| ((Set.headingAngleCorrection > 250) && (Set.headingAngleCorrection < 290)))
						{//ant left+right: headingCorrectionAngle 90 or 270
				//roll		
							if (((UBXPVT1[UBXRingCount1].gSpeed > 3000) && (abs(UBXRelPosNED[UBXRingCount2].relPosD) < (Set.AntDist * 3)))
								|| ((UBXPVT1[UBXRingCount1].gSpeed <= 3000) && (abs(UBXRelPosNED[UBXRingCount2].relPosD) < (Set.AntDist * 2)))) {//50% = 26.7° max
								//rollRelPosNED = (atan2((double(UBXRelPosNED[UBXRingCount2].relPosD) + (double(UBXRelPosNED[UBXRingCount2].relPosHPD) / 100)), Set.AntDist)) / PI180;
								rollRelPosNED = (atan2((double(UBXRelPosNED[UBXRingCount2].relPosD) + (double(UBXRelPosNED[UBXRingCount2].relPosHPD) * 0.01)),
									(double(UBXRelPosNED[UBXRingCount2].relPosLength) + (double(UBXRelPosNED[UBXRingCount2].relPosHPLength) * 0.01)))) / PI180;
								rollRelPosNED += Set.DualRollAngleCorrection;
								noRollCount = 0;
								rollRelPosNEDPresent = true;
								if (CMPS14CorrFactor > 0.0001) {
									Set.CMPS14RollCorrection = (0.005 * (rollCMPS - rollRelPosNED))
										+ (0.995 * Set.CMPS14RollCorrection);//0,02 = diff 0.11 degrees after 400 loops
								}
								//rollVarProcess = VarProcessVeryFast; //set Kalman filter fast
								if (Set.debugmodeHeading) { 
									Serial.print("roll GPS: "); Serial.print(rollRelPosNED); 
									Serial.print(" roll CMPS: "); Serial.print(rollCMPS);
									Serial.print(" roll CMPS corr: "); Serial.println(rollCMPS - Set.CMPS14RollCorrection);
								}
							}
							else {
								//rollVarProcess = VarProcessSlow;
								rollRelPosNED = rollRelPosNED * 0.95;
								noRollCount++;
							}
						}
						if (Set.debugmodeFilterPos) { Serial.println("perfect Signal"); }
					}
					else
						//signal medium
					{
						if (UBXPVT1[UBXRingCount1].gSpeed > 500) { //driving at least 1.8km/h
							if (drivDirect < 2)//forewards or unknown
							{
								if (Set.CMPS14_present) { HeadingQualFactor = 0.8; }
								else { HeadingQualFactor = 0.4; }
								HeadingQuotaCMPS = 0.7;
								HeadingQuotaVTG = double(UBXPVT1[UBXRingCount1].gSpeed) / double(8000);//10000    // 24.03.2020
								if (HeadingQuotaVTG > 1.0) { HeadingQuotaVTG = 1.0; }//at x km/h use only VTG
							}
							else {
								if (Set.CMPS14_present) { HeadingQualFactor = 0.8; }
								else { HeadingQualFactor = 0.35; }
								HeadingQuotaCMPS = 0.7;
								HeadingQuotaVTG = 0.0;
							}
						}
						else {
							if (Set.CMPS14_present) { HeadingQualFactor = 0.7; }
							else {HeadingQualFactor = 0.35;}
							HeadingQuotaCMPS = 0.7;
							HeadingQuotaVTG = 0.0;
						}
						headVTGVarProcess = VarProcessSlow; //set Kalman filter
						noRollCount++;
						//start filter GPS pos, set Kalman values
						if (Set.filterGPSposOnWeakSignal == 1) {
							filterGPSpos = true;
							latVarProcess = VarProcessVeryFast; //set Kalman filter
							lonVarProcess = VarProcessVeryFast;
						}
						//roll
						if (Set.CMPS14_present) {rollRelPosNEDPresent = false;}
						else {
							if (noRollCount < 50) { rollRelPosNEDPresent = true; rollRelPosNED *= 0.93; }//19.03.2021
							else { noRollCount = 55; rollRelPosNEDPresent = false; }//prevent overflow
						}

						if (Set.debugmodeFilterPos) { Serial.println("medium Signal"); }
						if (Set.debugmode) {
							Serial.println("poor quality of GPS signal: NO roll calc, heading calc OK");
							Serial.print("Antenna distance set: "); Serial.print(Set.AntDist); Serial.print("  Ant. dist from GPS: "); Serial.println(UBXRelPosNED[UBXRingCount2].relPosLength);
						}
					}//else of deviation perfect	


					//deviation good or perfect
				//heading
					HeadingRelPosNED = (double(UBXRelPosNED[UBXRingCount2].relPosHeading) * 0.00001) + Set.headingAngleCorrection;
					if (HeadingRelPosNED >= 360) { HeadingRelPosNED -= 360; }

					//calc CMPS14 heading correction
					if (CMPS14CorrFactor > 0.0001) {
						double Diff = HeadingRelPosNED - HeadingCMPS;
						if (Diff < 0) { Diff += 360; }
						Set.CMPS14HeadingCorrection = (CMPS14CorrFactor * (Diff))
							+ ((1 - CMPS14CorrFactor) * Set.CMPS14HeadingCorrection);
					}
					if (Set.debugmodeRAW) {
						Serial.print("GPSRelPosNEDHead CMPSheadCorrFaktor SetCPMS14HeadCorr CMPS14Headraw CMPS14Headcorr"); Serial.print(",");
						Serial.print(HeadingRelPosNED, 3); Serial.print(",");
						Serial.print(CMPS14CorrFactor, 5); Serial.print(",");
						Serial.print(Set.CMPS14HeadingCorrection, 3); Serial.print(",");
						Serial.print(HeadingCMPS, 3); Serial.print(","); 
						Serial.print(HeadingCMPScorr, 3); Serial.print(",");
					}

					dualGPSHeadingPresent = true;

					if (Set.debugmodeHeading) {
						Serial.print("Antenna distance set: "); Serial.print(Set.AntDist); Serial.print("  Ant. dist from GPS: "); Serial.println(UBXRelPosNED[UBXRingCount2].relPosLength);
					}
					if (Set.debugmodeRAW) {
						//Serial.print("MaxHeadingDiff,"); Serial.print(HeadingDiff);
						Serial.print("AntDist AntDisGPS,");
						Serial.print(Set.AntDist); Serial.print(","); 
						Serial.print(UBXRelPosNED[UBXRingCount2].relPosLength); Serial.print(",");
					}
				}

				//very poor signal quality
				else {
					rollRelPosNED = rollRelPosNED * 0.9;//go slowly to 0
					dualAntNoValueCount++;
			//		if (dualAntNoValueCount < dualAntNoValueMax) { dualGPSHeadingPresent = true; }
			//		else { if (dualAntNoValueCount > 200) { dualAntNoValueCount = 220; } }
					if (Set.CMPS14_present) { HeadingQualFactor = 0.95; }
					else { HeadingQualFactor = 0.5; }
					HeadingQuotaCMPS = 1.0;
					//set Kalman filter for VTG heading
					if (UBXPVT1[UBXRingCount1].gSpeed > 100) //driving at least 0.36km/h
						if (UBXPVT1[UBXRingCount1].gSpeed > 1000) //driving at least 3.6km/h
						{
							headVTGVarProcess = VarProcessVerySlow;
						}
						else { headVTGVarProcess = VarProcessVerySlow * 0.1; }
					else { headVTGVarProcess = VarProcessVerySlow * 0.001; }


					HeadingQuotaVTG = double(UBXPVT1[UBXRingCount1].gSpeed) / double(3000);
					if (HeadingQuotaVTG > 1.0) { HeadingQuotaVTG = 1.0; }

					if (Set.filterGPSposOnWeakSignal == 1) {
						filterGPSpos = true;
						latVarProcess = VarProcessFast; //set Kalman filter
						lonVarProcess = VarProcessFast;
					}
					if (Set.debugmodeFilterPos) { Serial.println("very weak Signal, or only 1 Antenna"); }
					if (Set.debugmode || Set.debugmodeHeading) {
						Serial.println("poor quality of GPS signal, or only 1 Antenna: NO heading/roll calc");
						Serial.print("Antenna distance set: "); Serial.print(Set.AntDist); Serial.print("  Ant. dist from GPS: "); Serial.println(UBXRelPosNED[UBXRingCount2].relPosLength);
					}
					if (Set.debugmodeRAW) { Serial.print(",,,,,,,,,"); }
				}
			}

			//do this, if GNSFix is OK:

			//HeadingVTG Kalman filter go to cos for filtering to avoid 360-0° jump
			//cosHeadVTG = cos((HeadingVTG * 0.6) + (UBXPVT1[UBXRingCount1].headMot * 0.000004 * PI180));
			cosHeadVTG = cos((UBXPVT1[UBXRingCount1].headMot * 0.00001 * PI180));
			headVTGK = cosHeadVTG;//input
			if (abs(cosHeadVTG) > 0.98) { headVTGPc = headVTGP + (headVTGVarProcess * 10); }//"open" filter in 356-4 deg region
			else { headVTGPc = headVTGP + headVTGVarProcess; }
			headVTGG = headVTGPc / (headVTGPc + headVTGVar);
			headVTGP = (1 - headVTGG) * headVTGPc;
			headVTGXp = headVTGXe;
			headVTGZp = headVTGXp;
			headVTGXe = (headVTGG * (headVTGK - headVTGZp)) + headVTGXp;//result

			cosHeadVTG = headVTGXe;

			//go back to degree
			HeadingVTGOld = HeadingVTG;//bak old value for constrain
			if (UBXPVT1[UBXRingCount1].headMot <= 18000000)
			{
				HeadingVTG = acos(headVTGXe) / PI180;
			}
			else { HeadingVTG = double(360.0) - (acos(headVTGXe) / PI180); }

			if (Set.debugmodeHeading) { Serial.print("VTG heading (only from Ant R) filtered: "); Serial.println(HeadingVTG); }

			if (Set.debugmodeRAW) {
				Serial.print("headVTG filtHeadVTG,");
				Serial.print(float(UBXPVT1[UBXRingCount1].headMot) * 0.00001); Serial.print(",");
				//Serial.print(cosHeadVTG, 4); Serial.print(",");
				Serial.print(HeadingVTG); Serial.print(",");
			}

			//driving direction calcs done here, after HeadingVTG and dual heading was filtered
			if (dualAntNoValueCount < (dualAntNoValueMax * 3)) {
				if (UBXPVT1[UBXRingCount1].gSpeed > 100) {//driving at least 0.36 km/h
					drivDirect = 2;     //set to backwards
					if (abs(HeadingRelPosNED - HeadingVTG) <= 60) {  //40 330 20         // almost same direction = forewards
						drivDirect = 1;
					}
					else {
						if ((HeadingRelPosNED > 305) && (HeadingVTG < 55)) {//300 60 not working
							drivDirect = 1;
							add360ToVTG = true;
						}
						if ((HeadingRelPosNED < 55) && (HeadingVTG > 305)) {
							drivDirect = 1;
							add360ToRelPosNED = true;
						}
					}
				}
				else { //too slow
					drivDirect = 0;
				}
			}
			else { drivDirect = 0; }
			if (Set.debugmodeHeading) {
				Serial.print("driving direction: "); Serial.println(drivDirect);
				Serial.print("Heading quota VTG :"); Serial.println(HeadingQuotaVTG);
			}

		}
		else { if (Set.debugmode) { Serial.println("UBX RelPosNED flag: relative position not valid ->  NO heading + roll calc"); } }
	}

	//single Antenna
	else {
		if (Set.CMPS14_present) { HeadingQualFactor = 0.95; }
		else { HeadingQualFactor = 0.5; }
		drivDirect = 0;// 0 = unknown
		HeadingQuotaCMPS = 1.0;
		HeadingQuotaVTG = 1.0;
		//set Kalman filter for VTG heading
		if (UBXPVT1[UBXRingCount1].gSpeed > 100) //driving at least 0.36km/h
			if (UBXPVT1[UBXRingCount1].gSpeed > 1000) //driving at least 3.6km/h
			{
				//Serial.print("GPS fix quality: "); Serial.println(byte(cFixQualGGA));
				if ((bGGAexists) && (byte(cFixQualGGA) < 51)) { headVTGVarProcess = VarProcessVerySlow; }//GPS quality <2
				else { headVTGVarProcess = VarProcessSlow; }
			}
			else { headVTGVarProcess = VarProcessVerySlow; }
		else { headVTGVarProcess = VarProcessVerySlow * 0.1; }

		//HeadingVTG Kalman filter go to cos for filtering to avoid 360-0° jump
		cosHeadVTG = cos((UBXPVT1[UBXRingCount1].headMot * 0.00001 * PI180));
		headVTGK = cosHeadVTG;//input
		if (abs(cosHeadVTG) > 0.98) { headVTGPc = headVTGP + (headVTGVarProcess * 10); }//"open" filter in 356-4 deg region
		else { headVTGPc = headVTGP + headVTGVarProcess; }
		headVTGG = headVTGPc / (headVTGPc + headVTGVar);
		headVTGP = (1 - headVTGG) * headVTGPc;
		headVTGXp = headVTGXe;
		headVTGZp = headVTGXp;
		headVTGXe = (headVTGG * (headVTGK - headVTGZp)) + headVTGXp;//result
		cosHeadVTG = headVTGXe;
		//go back to degree
		HeadingVTGOld = HeadingVTG;//bak old value for constrain
		if (UBXPVT1[UBXRingCount1].headMot <= 18000000)
		{
			HeadingVTG = acos(headVTGXe) / PI180;
		}
		else { HeadingVTG = double(360.0) - (acos(headVTGXe) / PI180); }

		if (Set.CMPS14_present) {

			//driving direction calcs done here, after HeadingVTG was filtered
			if (UBXPVT1[UBXRingCount1].gSpeed > 150) {//driving at least 0.5 km/h
				drivDirect = 2;     //set to backwards
				if (abs(HeadingCMPScorr - HeadingVTG) <= 60) {  //40 330 20         // almost same direction = forewards
					drivDirect = 1;
				}
				else {
					if ((HeadingCMPScorr > 305) && (HeadingVTG < 55)) {
						drivDirect = 1;
					}
					if ((HeadingCMPScorr < 55) && (HeadingVTG > 305)) {
						drivDirect = 1;
					}
				}
			}
			else { //too slow
				drivDirect = 0;
			}
		}

		if ((Set.debugmodeHeading) || (Set.debugmodeVirtAnt)) { Serial.println("UBX RelPosNED not present (single Antenna), or checksums invalid"); }
	}//end single antenna

	if (Set.debugmodeRAW) {
		Serial.print("RelPosNedRollcorrected CMPSRollCorrected,");
		Serial.print(rollRelPosNED); Serial.print(",");
		Serial.print(rollCMPS- Set.CMPS14RollCorrection); Serial.print(",");
	}


	//roll filter before sending to AOG, if roll corrected position is send
	if (Set.GPSPosCorrByRoll) { rollToAOG = (rollToAOG * 0.7) + (rollRelPosNED * 0.3); }
	else { rollToAOG = rollRelPosNED; }
/*
	//heading
	if (UBXPVT1[UBXRingCount1].gSpeed > 5) {//prevent /0
		HeadingDiff = 2.5 + (double(Set.MaxHeadChangPerSec) * (UBXPVT1[UBXRingCount1].iTOW - UBXPVT1[(UBXRingCount1 + sizeOfUBXArray - 1) % sizeOfUBXArray].iTOW)) / (double(UBXPVT1[UBXRingCount1].gSpeed) * 0.72);
	}
	else { HeadingDiff = 200; }

	HeadingMin = HeadingVTGOld - HeadingDiff;
	HeadingMax = HeadingVTGOld + HeadingDiff;

	if (Set.debugmodeRAW) {
		Serial.print("HeadingDiff VTGHeadmin VTGHeadMax,");
		Serial.print(HeadingDiff); Serial.print(",");
		Serial.print(HeadingMin); Serial.print(",");
		Serial.print(HeadingMax); Serial.print(",");
	}
		if (Set.debugmodeHeading) {
			Serial.print("VTGLimits: HeadMin: "); Serial.print(HeadingMin); Serial.print(" HeadMax: ");
			Serial.print(HeadingMax); Serial.print(" Heading VTG: "); Serial.println(HeadingVTG);
		}
	//if ((HeadingVTG > HeadingDiff) && (HeadingVTG < (360 - HeadingDiff)) && (UBXPVT1[UBXRingCount1].gSpeed > 1000)) {
		if ((HeadingVTG > HeadingDiff) && (HeadingVTG < (360 - HeadingDiff)) && (abs(HeadingVTGOld - HeadingVTG) < 150) && (UBXPVT1[UBXRingCount1].gSpeed > 1000)) {
			if (Set.debugmodeRAW) {
				Serial.print("VTGconstrain,");
				//Serial.print(HeadingVTG); Serial.print(",");
			}
			HeadingVTG = constrain(HeadingVTG, HeadingMin, HeadingMax);
			//Serial.println("VTGcontrain");
			if (Set.debugmodeRAW) { Serial.print(HeadingVTG); Serial.print(","); }
		}
		else {
			if (Set.debugmodeRAW) { Serial.print("VTG NO constrain,,"); }
			//Serial.println("no VTG Contrain");
		}

*/
	HeadingMixBak = HeadingMix;

	if (existsUBXRelPosNED) {
		if (Set.CMPS14_present) {
			//if (drivDirect < 2) {//calc HeadingMix with CMPS and RelPosNED heading
			if ((abs(HeadingRelPosNED - HeadingCMPScorr)) <= 60) {
				HeadingMix = ((HeadingRelPosNED * (1.0 - HeadingQuotaCMPS)) + (HeadingCMPScorr * HeadingQuotaCMPS));
			}
			else {
				if ((HeadingCMPScorr > 300) && (HeadingRelPosNED < 60)) {
					HeadingMix = (((HeadingRelPosNED + 360) * (1.0 - HeadingQuotaCMPS)) + (HeadingCMPScorr * HeadingQuotaCMPS));
				}
				if ((HeadingCMPScorr < 60) && (HeadingRelPosNED > 300)) {
					HeadingMix = ((HeadingRelPosNED * (1.0 - HeadingQuotaCMPS)) + ((HeadingCMPScorr + 360) * HeadingQuotaCMPS));
				}
			}
			if (HeadingMix > 360) {
				HeadingMix -= 360;
			}
			//}
			//else { HeadingMix = HeadingRelPosNED; }//backwards
		}
		else {
			if (drivDirect < 2) {//calc HeadingMix with VTG and RelPosNED heading
				if ((abs(HeadingRelPosNED - HeadingVTG)) <= 60) {
					HeadingMix = ((HeadingRelPosNED * (1.0 - HeadingQuotaVTG)) + (HeadingVTG * HeadingQuotaVTG));
				}
				else {
					if (add360ToVTG) {
						HeadingMix = ((HeadingRelPosNED * (1.0 - HeadingQuotaVTG)) + ((HeadingVTG + 360) * HeadingQuotaVTG));
					}
					if (add360ToRelPosNED) {
						HeadingMix = (((HeadingRelPosNED + 360) * (1.0 - HeadingQuotaVTG)) + (HeadingVTG * HeadingQuotaVTG));
					}
				}
				if (HeadingMix > 360) {
					HeadingMix -= 360;
				}
			}
			else { HeadingMix = HeadingRelPosNED; }//backwards
		}
	}
	else { //single Antenna
		if (Set.CMPS14_present) { HeadingMix = HeadingCMPScorr; }
		else { HeadingMix = HeadingVTG; }
	}
	if (abs(HeadingMixBak - HeadingMix) <= 20) {   // use old and new HeadingMix values 
		HeadingMix = (HeadingMixBak * (double(1) - HeadingQualFactor)) + (HeadingMix * HeadingQualFactor);
	}
	else {
		if ((HeadingMixBak > 340) && (HeadingMix < 20)) { HeadingMix += 360; }
		if ((HeadingMixBak < 20) && (HeadingMix > 340)) { HeadingMixBak += 360; }
		HeadingMix = ((HeadingMixBak * (double(1) - HeadingQualFactor)) + (HeadingMix * HeadingQualFactor));
	}
	if (HeadingMix > 360) { HeadingMix -= 360; }

	if (Set.debugmodeRAW) {
		Serial.print("VTGQuota HeadQualFac HeadingVTG HeadingRelPos HeadingMixUnLim,");
		Serial.print(HeadingQuotaVTG); Serial.print(",");
		Serial.print(HeadingQualFactor); Serial.print(",");
		Serial.print(HeadingVTG); Serial.print(",");
		Serial.print(HeadingRelPosNED); Serial.print(",");
		Serial.print(HeadingMix); Serial.print(",");
	}
	/*
		HeadingMin = HeadingMixBak - HeadingDiff;
		HeadingMax = HeadingMixBak + HeadingDiff;
		if ((HeadingMixBak > HeadingDiff) && (HeadingMixBak < (360-HeadingDiff))) {//360 to 0
			HeadingMix = constrain(HeadingMix, HeadingMin, HeadingMax);
			if (Set.debugmodeHeading) {
				Serial.print("MixLimits: HeadMin: "); Serial.print(HeadingMin); Serial.print(" HeadMax: ");
				Serial.print(HeadingMax); Serial.print(" Heading Mix: "); Serial.println(HeadingMix);
			}
			if (Set.debugmodeRAW) {Serial.print("constrain used,");}
		}
		else { if (Set.debugmodeRAW) { Serial.print("NO constrain,"); } }
	*/
	if (Set.debugmodeHeading) {
		//	Serial.print("headDiffMin HeadDiffMax HeadingMix,");
		//	Serial.print(HeadingMin); Serial.print(",");
		//	Serial.print(HeadingMax); Serial.print(",");
		//	Serial.print(HeadingMix); Serial.print(",");
		//Serial.print("NoRollCount DualAntNoValueCount,");
		//Serial.print(noRollCount); Serial.print(",");
		//Serial.print(dualAntNoValueCount); Serial.print(",");
		Serial.print("VTGQuota HeadQualFac HeadingVTG HeadingRelPos HeadingMixUnLim,");
		Serial.print(HeadingQuotaVTG); Serial.print(",");
		Serial.print(HeadingQualFactor); Serial.print(",");
		Serial.print(HeadingVTG); Serial.print(",");
		Serial.print(HeadingRelPosNED); Serial.print(",");
		Serial.println(HeadingMix);
		Serial.print("CMPSheadraw corrFaktor SetCMPSHeadcorr headCMPScorr CMPSQuota,");
		Serial.print(HeadingCMPS); Serial.print(",");
		Serial.print(CMPS14CorrFactor); Serial.print(",");
		Serial.print(Set.CMPS14HeadingCorrection); Serial.print(",");
		Serial.print(HeadingCMPScorr, 3); Serial.print(",");
		Serial.println(HeadingQuotaCMPS); 
	}
	if (Set.debugmodeRAW) {
		//	Serial.print("headDiffMin HeadDiffMax HeadingMix,");
		//	Serial.print(HeadingMin); Serial.print(",");
		//	Serial.print(HeadingMax); Serial.print(",");
		//	Serial.print(HeadingMix); Serial.print(",");
		//Serial.print("NoRollCount DualAntNoValueCount,");
		//Serial.print(noRollCount); Serial.print(",");
		//Serial.print(dualAntNoValueCount); Serial.print(",");
		Serial.print("CMPSheadraw corrFaktor CMPSHeadcorr CMPSQuota,");
		Serial.print(HeadingCMPS); Serial.print(",");
		Serial.print(CMPS14CorrFactor); Serial.print(",");
		Serial.print(Set.CMPS14HeadingCorrection); Serial.print(",");
		Serial.print(HeadingQuotaCMPS); Serial.print(",");
		Serial.print("CMPSrollraw CMPSRollcorr,");
		Serial.print(rollCMPS); Serial.print(",");
		Serial.print(Set.CMPS14RollCorrection); Serial.print(",");
	}
}



//-------------------------------------------------------------------------------------------------

void virtualAntennaPoint() {//roll is corrected here
	double virtLatRad = double(UBXPVT1[UBXRingCount1].lat) / double(10000000) * PI180;
	double virtLonRad = double(UBXPVT1[UBXRingCount1].lon) / double(10000000) * PI180;
	double virtLatRadTemp = 0.0, virtAntHeadingDiff = 0.0, headingRad = 0.0, WayByRadius = 0.0;

	double toLeft = 0;
	virtAntPosPresent = false;

	if (Set.GPSPosCorrByRoll == 1) { //calculate virtual Antenna point using roll
		//use GPS roll or CMPS?
		if (rollRelPosNEDPresent) { toLeft = tan((rollRelPosNED * PI180)) * Set.AntHight; }
		else { if (Set.CMPS14_present) { toLeft = tan((rollCMPS - Set.CMPS14RollCorrection * PI180)) * Set.AntHight; } }

		if (Set.debugmodeVirtAnt) {
			Serial.print("rollRelPosNEDPresent: "); Serial.print(rollRelPosNEDPresent);
			Serial.print(" roll CMPS "); Serial.print(rollCMPS - Set.CMPS14RollCorrection, 2);
			Serial.print(" roll RelPosNed: "); Serial.print(rollRelPosNED, 2); Serial.print(" to left by roll: ");
			Serial.println(toLeft);
		}
		if (Set.debugmodeRAW) {
			Serial.print("rollRelPosNEDPresent rollRelPosNed rollCMPS toLeftRoll toLeftRollandDistError toLeftFinal SetToLeft SetForew,");
			Serial.print(rollRelPosNEDPresent); Serial.print(",");
			Serial.print(rollRelPosNED); Serial.print(",");
			Serial.print(rollCMPS-Set.CMPS14RollCorrection); Serial.print(",");
			Serial.print(toLeft); Serial.print(",");
		}
		toLeft += (double(UBXRelPosNED[UBXRingCount2].relPosLength) + (double(UBXRelPosNED[UBXRingCount2].relPosHPLength) * 0.01) - Set.AntDist) * 0.5;//correct half of antenna distance error
		if (Set.debugmodeRAW) {
			Serial.print(toLeft); Serial.print(",");
		}
		toLeft = toLeft - Set.virtAntLeft;
		if (Set.debugmodeRAW) {
			Serial.print(toLeft); Serial.print(",");
			Serial.print(Set.virtAntLeft); Serial.print(",");
			Serial.print(Set.virtAntForew); Serial.print(",");
		}

	}
	else {
		toLeft = 0 - Set.virtAntLeft;
		if (Set.debugmodeRAW) { Serial.print(",,,,,,,"); }
	}

	if (Set.virtAntForew != 0) {
		if (toLeft != 0) {
			//shift Antenna foreward and to the left: direction to move point: heading-atan(foreward/right)
			virtAntHeadingDiff = (atan2(toLeft, Set.virtAntForew)) / PI180;
			//distance to move  cm  / mean radius of earth cm (WGS84 6.371.000,8m)
			double distAnt = sqrt((toLeft * toLeft) + (Set.virtAntForew * Set.virtAntForew));
			WayByRadius = distAnt / 637100080;
			if (Set.debugmodeRAW) {
				Serial.print("distToMove SetForew virtAntHeadDiff,");
				Serial.print(distAnt,4); Serial.print(",");
				Serial.print(Set.virtAntForew); Serial.print(",");
				Serial.print(virtAntHeadingDiff); Serial.print(",");
				//Serial.print("WayByRadius,");
				//Serial.print(WayByRadius,10); Serial.print(",");
			}
			if (Set.debugmodeVirtAnt) {
				Serial.print("virt Ant: real dist to move"); Serial.print(distAnt);
				Serial.print(" WayByRadius: "); Serial.println(WayByRadius, 10);
				Serial.print(" angle corr by: "); Serial.println(virtAntHeadingDiff, 2);
			}
		}
		else {
			//shift Antenna only foreward
			virtAntHeadingDiff = 0.0;
			//distance to move  cm  / mean radius of earth cm (WGS84 6.371.000,8m)
			WayByRadius = Set.virtAntForew / 637100080;

		}
	}
	else {
		if (toLeft != 0) {
			//shift Antenna only left (needed for roll compensation)
			virtAntHeadingDiff = 90.0;
			WayByRadius = toLeft / 637100080;
			if (Set.debugmodeRAW) {
				Serial.print("distToMove(l) SetForew virtAntHeadDiff,");
				Serial.print(toLeft, 4); Serial.print(",");
				Serial.print(Set.virtAntForew); Serial.print(",");
				Serial.print(virtAntHeadingDiff); Serial.print(",");
				//Serial.print("WayByRadius,");
				//Serial.print(WayByRadius,10); Serial.print(",");
			}
			if (Set.debugmodeVirtAnt) {
				Serial.print("virt Ant: real dist to move (only left) "); Serial.print(toLeft);
				Serial.print(" WayByRadius: "); Serial.println(WayByRadius, 10);
				Serial.print(" angle corr by: "); Serial.println(virtAntHeadingDiff, 2);
			}
			if (Set.debugmodeVirtAnt) {
				Serial.print("effective dist to move virt Ant: "); Serial.print(toLeft);
				Serial.print(" WayToRaduis: "); Serial.println(WayByRadius, 12);
			}
		}
	}//direction+way



	if ((virtLatRad != 0.0) && (virtLonRad != 0.0)) {
		//all calculations in radians:  decimal * PI / 180
		//headingRad = HeadingMix + virtAntHeadingDiff;
		if ((Set.CMPS14_present) && (HeadingQuotaCMPS > 0.7)) {
			headingRad = HeadingCMPScorr + virtAntHeadingDiff;
		}
		else { headingRad = HeadingRelPosNED + virtAntHeadingDiff; }
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
		if (Set.debugmodeRAW) {
			Serial.print("virtLat virtLon,");
			Serial.print(virtLat,7); Serial.print(",");
			Serial.print(virtLon,7); Serial.print(",");
		}
		if (Set.debugmodeVirtAnt) {
			Serial.println("Virtual Antenna point calc:");
			Serial.print("UBX1 Lat: "); Serial.print((double(UBXPVT1[UBXRingCount1].lat) / 10000000), 7); Serial.print(" virtLat: "); Serial.print(virtLat, 7);
			Serial.print("UBX1 Lon: "); Serial.print((double(UBXPVT1[UBXRingCount1].lon) / 10000000), 7); Serial.print(" virtLon: "); Serial.println(virtLon, 7);
			Serial.print("virt Point head: "); Serial.print((headingRad), 3); Serial.print(" GPS mix head: "); Serial.print(HeadingMix, 3);
			Serial.print(" rollRelPosNed: "); Serial.println(rollRelPosNED, 3);
		}
	}
	else { if (Set.debugmode||Set.debugmodeVirtAnt) { Serial.println("No virtual Antenna point: lat/lon = 0"); } }
}

//-------------------------------------------------------------------------------------------------


void filterPosition() {
	//input to the kalman filter
	if (virtAntPosPresent) { latK = virtLat; lonK = virtLon; }
	else { latK = double(UBXPVT1[UBXRingCount1].lat) * 0.0000001; lonK = double(UBXPVT1[UBXRingCount1].lon) * 0.0000001; }

	if (Set.debugmodeHeading || Set.debugmodeFilterPos) { Serial.print(" lat: "); Serial.print(latK, 7); }

	//Kalman filter
	latPc = latP + latVarProcess;
	latG = latPc / (latPc + latVar);
	latP = (1 - latG) * latPc;
	latXp = latXe;
	latZp = latXp;
	latXe = (latG * (latK - latZp)) + latXp;

	

	//Kalman filter
	lonPc = lonP + lonVarProcess;
	lonG = lonPc / (lonPc + lonVar);
	lonP = (1 - lonG) * lonPc;
	lonXp = lonXe;
	lonZp = lonXp;
	lonXe = (lonG * (lonK - lonZp)) + lonXp;

	if (Set.debugmodeRAW) {
		Serial.print("virtLat virtLon filtVirtLat FiltVirtLog,");
		Serial.print(virtLat,8); Serial.print(",");
		Serial.print(virtLon,8); Serial.print(",");
		Serial.print(latXe,8); Serial.print(",");
		Serial.print(lonXe,8); Serial.print(",");
	}

	virtLon = lonXe;//result
	virtLat = latXe;//result
	if (Set.debugmodeHeading || Set.debugmodeFilterPos) {
		Serial.print(" lat filtered: "); Serial.print(virtLat, 7);
		Serial.print(" lon: "); Serial.print(lonK, 7); Serial.print(" lon filtered: "); Serial.println(virtLon, 7);
	}
}
