//ESP32 programm for UBLOX receivers to send NMEA to AgOpenGPS or other program 
//Version 16. August 2020 send data 2x

//works with 1 or 2 receivers

//1 receiver to send position from UBXPVT message to ESP32
//2 receivers to get position, roll and heading from UBXPVT + UBXRelPosNED via UART to ESP32

//ESP sending $PAOGI or $GGA+VTG+HDT sentence via UDP to IP x.x.x.255 at port 9999 or via USB

//AgOpenGPS sending NTRIP via UDP to port 2233(or USB) -> ESP sends it to UBLOX via UART

//filters roll, heading and on weak GPS signal, position with filter parameters changing dynamic on GPS signal quality

//by Matthias Hammer (MTZ8302) 12.Juli.2020

//change stettings to your need. Afterwards you can change them via webinterface x.x.x.79 (192.168.1.79)
//if connection to your network fails an accesspoint is opened: webinterface 192.168.1.1

//use serial monitor at USB port, to get sebug messages and IP for webinterface at ESP start.

//for easier setup:
//use webinterface, turn debugmodeUBX on and change GPIO pin until you get data from the UBlox receivers on USB serial monitor

//the settings below are written as defalt values and can be reloaded.
//So if changing settings set EEPROM_clear = true; (line ~109) - flash - boot - reset to EEPROM_clear = false - flash again to keep them as defauls

#define HardwarePlatform 0      //0 = runs on ESP32, 1 = runs on Arduino Mega

struct set {
    //connection plan:
    // ESP32--- Right F9P GPS pos --- Left F9P Heading-----Sentences
    //  RX1-27-------TX1--------------------------------UBX-Nav-PVT out   (=position+speed)
    //  TX1-16-------RX1--------------------------------RTCM in           (NTRIP comming from AOG to get absolute/correct postion
    //  RX2-25-----------------------------TX1----------UBX-RelPosNED out (=position relative to other Antenna)
    //  TX2-17-----------------------------RX1----------
    //               TX2-------------------RX2----------RTCM 1077+1087+1097+1127+1230+4072.0+4072.1 (activate in right F9P = NTRIP for relative positioning)
  
    // IO pins ----------------------------------------------------------------------------------------
    byte RX1 = 27;              //right F9P TX1 GPS pos
    byte TX1 = 16;              //right F9P RX1 GPS pos

    byte RX2 = 25;              //left F9P TX1 Heading
    byte TX2 = 17;              //left F9P RX1 Heading

    byte LEDWiFi_PIN = 2;      // WiFi Status LED 0 = off
    byte LEDWiFi_ON_Level = HIGH;    //HIGH = LED on high, LOW = LED on low#

    //WiFi---------------------------------------------------------------------------------------------
#if HardwarePlatform == 0
    //tractors WiFi
    char ssid[24] = "Fendt_209V";           // WiFi network Client name
    char password[24] = "";                 // WiFi network password//Accesspoint name and password

    char ssid_ap[24] = "GPS_unit_F9P_Net";  // name of Access point, if no WiFi found, NO password!!
    int timeoutRouter = 10;                //time (s) to search for existing WiFi, than starting Accesspoint 

    byte timeoutWebIO = 10;                 //time (min) afterwards webinterface is switched off

    //static IP
    byte myip[4] = { 192, 168, 1, 79 };     // Roofcontrol module 
    byte gwip[4] = { 192, 168, 1, 1 };      // Gateway IP also used if Accesspoint created
    byte mask[4] = { 255, 255, 255, 0 };
    byte myDNS[4] = { 8, 8, 8, 8 };         //optional
    byte ipDestination[4] = { 192, 168, 1, 255 };//IP address to send UDP data to
    byte myIPEnding = 79;             //ending of IP adress x.x.x.79 

    unsigned int portMy = 5544;             //this is port of this module: Autosteer = 5577 IMU = 5566 GPS = 
    unsigned int portAOG = 8888;            //port to listen for AOG
    unsigned int AOGNtripPort = 2233;       //port NTRIP data from AOG comes in
    unsigned int portDestination = 9999;    //Port of AOG that listens
#endif

    //Antennas position
    double AntDist = 74.0;                //cm distance between Antennas
    double AntHight = 228.0;              //cm hight of Antenna
    double virtAntRight = 42.0;           //cm to move virtual Antenna to the right
    double virtAntForew = 60.0;            //cm to move virtual Antenna foreward
    double headingAngleCorrection = 90;

    double AntDistDeviationFactor = 1.2;  // factor (>1), of whom lenght vector from both GPS units can max differ from AntDist before stop heading calc
    byte checkUBXFlags = 1;               //UBX sending quality flags, when used with RTK sometimes 
    byte filterGPSposOnWeakSignal = 1;    //filter GPS Position on weak GPS signal
   
    byte GPSPosCorrByRoll = 1;            // 0 = off, 1 = correction of position by roll (AntHight must be > 0)
    double rollAngleCorrection = 0.0; 

    byte MaxHeadChangPerSec = 30;         // degrees that heading is allowed to change per second
   
    byte DataTransVia = 8;                //transfer data via 0 = USB / 1 = USB 10 byte 2x / 7 = UDP / 8 = UDP 2x

    byte AOGNtrip = 1;                    // AOG NTRIP client 0:off 1:on listens to UDP port or USB serial
    byte sendOGI = 1;                     //1: send NMEA message 0: off
    byte sendVTG = 0;                     //1: send NMEA message 0: off
    byte sendGGA = 0;                     //1: send NMEA message 0: off
    byte sendHDT = 0;                     //1: send NMEA message 0: off


    bool debugmode = false;
    bool debugmodeUBX = false;
    bool debugmodeHeading = false;
    bool debugmodeVirtAnt = false;
    bool debugmodeFilterPos = false;
    bool debugmodeRAW = false;

}; set GPSSet;


bool EEPROM_clear = false;  //set to true when changing settings to write them as default values: true -> flash -> boot -> false -> flash again



// WiFistatus LED 
// blink times: searching WIFI: blinking 4x faster; connected: blinking as times set; data available: light on; no data for 2 seconds: blinking
unsigned int LED_WIFI_time = 0;
unsigned int LED_WIFI_pulse = 1400;   //light on in ms 
unsigned int LED_WIFI_pause = 700;   //light off in ms
boolean LED_WIFI_ON = false;
unsigned long NtripDataTime = 0, now = 0, WebIOTimeOut = 0;
bool WebIORunning = true;


//Kalman filter roll
double rollK, rollPc, rollG, rollXp, rollZp, rollXe;
double rollP = 1.0;
double rollVar = 0.1; // variance, smaller: faster, less filtering
double rollVarProcess = 0.3;// 0.0005;// 0.0003;  bigger: faster, less filtering// replaced by fast/slow depending on GPS quality
//Kalman filter heading
double headK, headPc, headG, headXp, headZp, headXe;
double headP = 1.0;
double headVar = 0.1; // variance, smaller, more faster filtering
double headVarProcess = 0.1;// 0.001;//  bigger: faster, less filtering// replaced by fast/slow depending on GPS quality
//Kalman filter heading
double headVTGK, headVTGPc, headVTGG, headVTGXp, headVTGZp, headVTGXe;
double headVTGP = 1.0;
double headVTGVar = 0.1; // variance, smaller, more faster filtering
double headVTGVarProcess = 0.01;// 0.001;//  bigger: faster, less filtering// replaced by fast/slow depending on GPS quality
//Kalman filter heading
double headMixK, headMixPc, headMixG, headMixXp, headMixZp, headMixXe;
double headMixP = 1.0;
double headMixVar = 0.1; // variance, smaller, more faster filtering
double headMixVarProcess = 0.1;// 0.001;//  bigger: faster, less filtering// replaced by fast/slow depending on GPS quality
//Kalman filter lat
double latK, latPc, latG, latXp, latZp, latXe;
double latP = 1.0;
double latVar = 0.1; // variance, smaller, more faster filtering
double latVarProcess = 0.3;//  replaced by fast/slow depending on GPS qaulity
//Kalman filter lon
double lonK, lonPc, lonG, lonXp, lonZp, lonXe;
double lonP = 1.0;
double lonVar = 0.1; // variance, smaller, more faster filtering
double lonVarProcess = 0.3;// replaced by fast/slow depending on GPS quality

double VarProcessVeryFast = 0.2;//0,3  used, when GPS signal is weak, no roll, but heading OK
double VarProcessFast = 0.08;//0,15  used, when GPS signal is weak, no roll, but heading OK
double VarProcessMedi = 0.02;//0,08 used, when GPS signal is  weak, no roll no heading
double VarProcessSlow = 0.001;//  0,004used, when GPS signal is  weak, no roll no heading
double VarProcessVerySlow = 0.0001;//0,03  used, when GPS signal is  weak, no roll no heading
bool filterGPSpos = false;

double HeadingQuotaVTG = 0.5;

#if HardwarePlatform == 0
//WIFI
IPAddress ipDestination; //set in network.ino
byte my_WiFi_Mode = 0;  // WIFI_STA = 1 = Workstation  WIFI_AP = 2  = Accesspoint
#endif

//UBX
byte UBXRingCount1 = 0, UBXRingCount2 = 0, UBXDigit1 = 0, UBXDigit2 = 0, OGIfromUBX = 0;
short UBXLenght1 = 100, UBXLenght2 = 100;
constexpr unsigned char UBX_HEADER[] = { 0xB5, 0x62 };//all UBX start with this
bool isUBXPVT1 = false,  isUBXRelPosNED = false, existsUBXRelPosNED = false;


double virtLat = 0.0, virtLon = 0.0;//virtual Antenna Position

//NMEA
byte OGIBuffer[90], HDTBuffer[20], VTGBuffer[50], GGABuffer[80];
bool newOGI = false, newHDT = false, newGGA = false, newVTG = false;
byte OGIdigit = 0, GGAdigit = 0, VTGdigit = 0, HDTdigit = 0;

//heading + roll
double HeadingRelPosNED = 0, cosHeadRelPosNED = 1, HeadingVTG = 0, HeadingVTGOld = 0, cosHeadVTG = 1, HeadingMix = 0, cosHeadMix = 1;
double HeadingDiff = 0, HeadingMax = 0, HeadingMin = 0, HeadingMixBak = 0, HeadingQualFactor = 0.5;
byte noRollCount = 0,  drivDirect = 0;
constexpr double PI180 = PI / 180;
bool dualGPSHeadingPresent = false, rollPresent = false, virtAntPosPresent = false, add360ToRelPosNED = false, add360ToVTG = false;
double roll = 0.0, rollToAOG = 0.0;
byte dualAntNoValueCount = 0, dualAntNoValueMax = 20;// if dual Ant value not valid for xx times, send position without correction/heading/roll


// Variables ------------------------------
struct NAV_PVT {
    unsigned char cls;
    unsigned char id;
    unsigned short len;
    unsigned long iTOW;  //GPS time ms
    unsigned short year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
    unsigned char valid;
    unsigned long tAcc;
    long nano;
    unsigned char fixType;//0 no fix....
    unsigned char flags;
    unsigned char flags2;
    unsigned char numSV; //number of sats
    long lon;   //deg * 10^-7
    long lat;   //deg * 10^-7
    long height;
    long hMSL;  //heigt above mean sea level mm
    unsigned long hAcc;
    unsigned long vAcc;
    long velN;
    long velE;
    long velD;
    long gSpeed; //Ground Speed mm/s
    long headMot;
    unsigned long sAcc;
    unsigned long headAcc;
    unsigned short pDOP;
    unsigned char flags3;
    unsigned char reserved1;
    long headVeh;
    long magDec;//doesnt fit, checksum was 4 bytes later, so changes from short to long
    unsigned long magAcc;
    unsigned char CK0;
    unsigned char CK1;
};
constexpr byte sizeOfUBXArray = 3;
NAV_PVT UBXPVT1[sizeOfUBXArray];


struct NAV_RELPOSNED {
    unsigned char cls;
    unsigned char id;
    unsigned short len;
    unsigned char ver;
    unsigned char res1;
    unsigned short refStID;
    unsigned long iTOW;
    long relPosN;
    long relPosE;
    long relPosD;
    long relPosLength;
    long relPosHeading;
    long res2;//    unsigned char res2;
    char relPosHPN;
    char relPosHPE;
    char relPosHPD;
    char relPosHPLength;
    unsigned long accN;
    unsigned long accE;
    unsigned long accD;
    unsigned long accLength;
    unsigned long accHeading;
    long res3; // unsigned char res3;
    unsigned long flags;
    unsigned char CK0;
    unsigned char CK1;
};
NAV_RELPOSNED UBXRelPosNED[sizeOfUBXArray];

#if HardwarePlatform == 0
#include <AsyncUDP.h>
//#include <WiFiUdp.h>
//#include <WiFiSTA.h>
//#include <WiFiServer.h>
#include <HTTP_Method.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
//#include <WiFiAP.h>
#include <WiFi.h>
#include <EEPROM.h>


//instances----------------------------------------------------------------------------------------
AsyncUDP udpRoof;
AsyncUDP udpNtrip;
WebServer server(80);

#endif

// SETUP ------------------------------------------------------------------------------------------

void setup()
{
    delay(200);
    delay(200);
    delay(50);
	//start serials
    Serial.begin(115200);
    delay(50);
#if HardwarePlatform == 0
    Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1);
    delay(10);
    Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2);
    delay(10);
#endif
#if HardwarePlatform == 1
    Serial1.begin(115200);
    delay(10);
    Serial2.begin(115200);
    delay(10);
#endif
    Serial.println();//new line

    if (GPSSet.LEDWiFi_PIN != 0) { pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); }

#if HardwarePlatform == 1
    GPSSet.DataTransVia = 0;//set data via USB
#endif


#if HardwarePlatform == 0
    restoreEEprom();
    delay(10);

    //start WiFi
    WiFi_Start_STA();
    if (my_WiFi_Mode == 0) {// if failed start AP
        WiFi_Start_AP(); 
        delay(100); 
    }
    delay(200);

    //init UPD listening to AOG
	if (udpNtrip.listen(GPSSet.AOGNtripPort))
	{
		Serial.print("NTRIP UDP Listening to port: ");
		Serial.println(GPSSet.AOGNtripPort);
		Serial.println();
	}
    delay(50);
    if (udpRoof.listen(GPSSet.portMy))
    {
        Serial.print("UDP writing to IP: ");
        Serial.println(ipDestination);
        Serial.print("UDP writing to port: ");
        Serial.println(GPSSet.portDestination);
        Serial.print("UDP writing from port: ");
        Serial.println(GPSSet.portMy);
    }
    delay(200);
  
    //start Server for Webinterface
    StartServer();

    delay(50);
    WebIOTimeOut = millis() + (long(GPSSet.timeoutWebIO) * 60000);
#endif

}

// MAIN loop  -------------------------------------------------------------------------------------------

void loop()
{

	getUBX();//read serials    

	if (UBXRingCount1 != OGIfromUBX)//new UXB exists
	{//Serial.println("new UBX to process");
		headingRollCalc();
		if (existsUBXRelPosNED) {
			//virtual Antenna point?
			if ((GPSSet.virtAntForew != 0) || (GPSSet.virtAntRight != 0) ||
				((GPSSet.GPSPosCorrByRoll == 1) && (GPSSet.AntHight > 0)))
			{//all data there
				virtualAntennaPoint();
			}
		}
		else //only 1 Antenna
		{
			virtAntPosPresent = false;
			if ((GPSSet.debugmodeHeading) || (GPSSet.debugmodeVirtAnt)) { Serial.println("no dual Antenna values so not virtual Antenna point calc"); }
		}

		//filter position: set kalman variables
		//0: no fix 1: GPS only -> filter slow, else filter fast, but filter due to no roll compensation
		if (UBXPVT1[UBXRingCount1].fixType <= 1) { latVarProcess = VarProcessSlow; lonVarProcess = VarProcessSlow; filterGPSpos = true; }
		else { if (!dualGPSHeadingPresent) { latVarProcess = VarProcessFast; lonVarProcess = VarProcessFast; filterGPSpos = true; } }
        //filterGPSPosition might set false an Kalman variables set, if signal is perfect (in void HeadingRollCalc)
        
        if (GPSSet.filterGPSposOnWeakSignal == 0) { filterGPSpos = false; }
		
        filterPosition();//runs allways to fill kalman variables

		if (GPSSet.sendGGA) { buildGGA(); }
		if (GPSSet.sendVTG) { buildVTG(); }
		if (GPSSet.sendHDT) { buildHDT(); }
		buildOGI();//should be build anyway, to decide if new data came in

	}

    //transfer data via 0 = USB / 1 = USB 10 byte 2x / 7 = UDP / 8 = UDP 2x
	if (GPSSet.DataTransVia < 5) {//use USB
		if (GPSSet.AOGNtrip == 1) { doSerialNTRIP(); } //gets USB NTRIP and sends to serial 1  
        
        //send 2x
        if (GPSSet.DataTransVia == 1) {
            if ((newOGI) && (GPSSet.sendOGI == 1)) {
                for (byte n = 0; n < (OGIdigit - 1); n++) { Serial.write(OGIBuffer[n]); }
                Serial.println();
            }
            if (newVTG) {
                for (byte n = 0; n < (VTGdigit - 1); n++) { Serial.write(VTGBuffer[n]); }
                Serial.println();
            }
            if (newHDT) {
                for (byte n = 0; n < (HDTdigit - 1); n++) { Serial.write(HDTBuffer[n]); }
                Serial.println();
            }
            if (newGGA) {
                for (byte n = 0; n < (GGAdigit - 1); n++) { Serial.write(GGABuffer[n]); }
                Serial.println();
            }
            delay(5);
        }//end send 2x

        //send USB
        if ((newOGI) && (GPSSet.sendOGI == 1)) {
			for (byte n = 0; n < (OGIdigit - 1); n++) { Serial.write(OGIBuffer[n]); }
			Serial.println();
			newOGI = false;
		}
		if (newVTG) {
			for (byte n = 0; n < (VTGdigit - 1); n++) { Serial.write(VTGBuffer[n]); }
			Serial.println();
			newVTG = false;
		}
		if (newHDT) {
			for (byte n = 0; n < (HDTdigit - 1); n++) { Serial.write(HDTBuffer[n]); }
			Serial.println();
			newHDT = false;
		}
		if (newGGA) {
			for (byte n = 0; n < (GGAdigit - 1); n++) { Serial.write(GGABuffer[n]); }
			Serial.println();
			newGGA = false;
		}
	}

#if HardwarePlatform == 0
	if (GPSSet.DataTransVia > 5) {//use WiFi
		if (GPSSet.AOGNtrip == 1) { doUDPNtrip(); } //gets UDP NTRIP and sends to serial 1 

		if ((newOGI) && (GPSSet.sendOGI == 1)) {
			udpRoof.writeTo(OGIBuffer, OGIdigit, ipDestination, GPSSet.portDestination);
            if (GPSSet.DataTransVia == 8) { delay(5); udpRoof.writeTo(OGIBuffer, OGIdigit, ipDestination, GPSSet.portDestination);}
            if (GPSSet.debugmodeRAW) {
                Serial.print("millis,"); Serial.print(millis()); Serial.print(",");
                Serial.print("UBXRingCount1 OGIfromUBX PAOGI,");
                Serial.print(UBXRingCount1); Serial.print(",");
                Serial.print(OGIfromUBX); Serial.print(",");
                Serial.print("DualGPSPres RollPres VirtAntPres DrivDir FilterPos,");
                Serial.print(dualGPSHeadingPresent); Serial.print(",");
                Serial.print(rollPresent); Serial.print(",");
                Serial.print(virtAntPosPresent); Serial.print(",");
                Serial.print(drivDirect); Serial.print(",");
                Serial.print(filterGPSpos); Serial.print(",");
                Serial.print("PVThead RelPosNEDhead,");
                Serial.print(UBXPVT1[UBXRingCount1].headMot); Serial.print(",");
                Serial.print(UBXRelPosNED[UBXRingCount2].relPosHeading); Serial.print(",");
                Serial.print("PVTlat PVTlon,");
                Serial.print(UBXPVT1[UBXRingCount1].lat); Serial.print(",");
                Serial.print(UBXPVT1[UBXRingCount1].lon); Serial.print(",");
                for (byte N = 0; N < OGIdigit; N++) {Serial.write(OGIBuffer[N]);}
            }
			newOGI = false;
		}
		if (newGGA) {
			udpRoof.writeTo(GGABuffer, GGAdigit, ipDestination, GPSSet.portDestination);
            if (GPSSet.DataTransVia == 8) { delay(5); udpRoof.writeTo(OGIBuffer, OGIdigit, ipDestination, GPSSet.portDestination); }
			newGGA = false;
		}
		if (newVTG) {
			udpRoof.writeTo(VTGBuffer, VTGdigit, ipDestination, GPSSet.portDestination);
            if (GPSSet.DataTransVia == 8) { delay(5); udpRoof.writeTo(OGIBuffer, OGIdigit, ipDestination, GPSSet.portDestination); }
			newVTG = false;
		}
		if (newHDT) {
			udpRoof.writeTo(HDTBuffer, HDTdigit, ipDestination, GPSSet.portDestination);
            if (GPSSet.DataTransVia == 8) { delay(5); udpRoof.writeTo(OGIBuffer, OGIdigit, ipDestination, GPSSet.portDestination); }
			newHDT = false;
		}
	}
    now = millis();
    if ((GPSSet.AOGNtrip == 1) && (GPSSet.LEDWiFi_PIN != 0)) {
       
        if (now > (NtripDataTime + 3000)) {
            if ((LED_WIFI_ON) && (now > (LED_WIFI_time + LED_WIFI_pulse))) {
                digitalWrite(GPSSet.LEDWiFi_PIN, !GPSSet.LEDWiFi_ON_Level);
                LED_WIFI_ON = false;
                LED_WIFI_time = millis();
            }
            if ((!LED_WIFI_ON) && (now > (LED_WIFI_time + LED_WIFI_pause))){
                digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level);
                LED_WIFI_ON = true;
                LED_WIFI_time = millis();
            }
        }
    }

    if (WebIORunning) {       
        server.handleClient(); //does the Webinterface
        if ((now > WebIOTimeOut) && (GPSSet.timeoutWebIO != 255)) {
            WebIORunning = false;
            server.close();
            if (GPSSet.debugmode) { Serial.println("switching off Webinterface"); }
        }
    }
#endif
    
}
