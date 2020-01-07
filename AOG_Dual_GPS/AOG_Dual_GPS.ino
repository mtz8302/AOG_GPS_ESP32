//ESP32 programm for 2 UBLOX receivers sending UBXPVT + UBXRelPosNED via UART to ESP32
//ESP sending $PAOGI sentence via UDP to IP x.x.x.255 at port 9999 or via USB
//AOG sending NTRIP via UDP to port 2233(or USB) -> ESP sends it to UBLOX via UART

//calculates heading, roll and virtual antenna position

//alpha version Matthias Hammer (MTZ8302) 5. Jan 2020

//to do: Web interface, Ethernet integration
//to think about: will also work with 2 receivers sending GGA, so not only fo UBlox, code is in "MISC"

#define useWiFi 1 //0 disables WiFi = use USB so no debugmessages!!
struct set {
    // IO pins ----------------------------------------------------------------------------------------
    int8_t RX0 = 3;//USB
    int8_t TX0 = 1;//USB

    int8_t RX1 = 27;//6;  //simpleRTK TX(xbee) = RX(f9p)
    int8_t TX1 = 16;//5; //simpleRTK RX(xbee) = TX(f9p)

    int8_t RX2 = 25;//3;  //simpleRTK TX1 2. Antenna
    int8_t TX2 = 17;//4;  //simpleRTK RX1 2. Antenna

    //int8_t LED_PIN_WIFI = 2;//21//32   // WiFi Status LED

    byte AOGNtrip = 1;  // AOG NTRIP client 0:off 1:on listens to UDP port or USB serial

    byte GPSPosCorrByRoll = 1;  // 0 = off, 1 = correction of position by roll (AntHight must be > 0)
    float rollAngleCorrection = 0.0;

    float headingAngleCorrection = 89.6;
    float AntDist = 68.0;           //cm distance between Antennas
    float AntHight = 225.0;         //cm hight of Antenna
    double virtAntRight = 34.0;   //cm to move virtual Antenna to the right
    double virtAntForew = 0.0;   //cm to move virtual Antenna foreward

#if useWiFi
    //WiFi---------------------------------------------------------------------------------------------
    //tractors WiFi
    char ssid[24] = "Fendt_209V";    // WiFi network Client name
    char password[24] =  "";      // WiFi network password//Accesspoint name and password

    const char* ssid_ap = "GPS_unit_ESP_Net";// name of Access point, if no WiFi found
    const char* password_ap = "";
    //Accessoint is active for time (seconds) 0 = always on, timer resets everytime a client connects
    unsigned long timeoutRouter = 65;//time (s) to search for existing WiFi, than starting Accesspoint 
    //const int AP_active_time = 600;

    //static IP
    byte myIP[4] = { 192, 168, 1, 79 };  // Roofcontrol module 
    byte gwIP[4] = { 192, 168, 1, 1 };   // Gateway IP also used if Accesspoint created
    byte mask[4] = { 255, 255, 255, 0 };
    byte myDNS[4] = { 8, 8, 8, 8 };      //optional

    unsigned int portMy = 5544;       //this is port of this module: Autosteer = 5577 IMU = 5566 GPS = 
    unsigned int portAOG = 8888;      // port to listen for AOG
    unsigned int AOGNtripPort = 2233; //port NTRIP data from AOG comes in
    unsigned int portDestination = 9999;  // Port of AOG that listens
#endif
}; set GPSSet;

bool debugmode = false;
bool debugmodeUBX = false;
bool debugmodeHeading = false;
bool debugmodeVirtAnt = false;

bool filterGPSpos = false; //filter GPS Position mostly not necessary

//Kalman filter roll
double rollK, rollPc, rollG, rollXp, rollZp, rollXe;
double rollP = 1.0;
double rollVar = 0.1; // variance, smaller: faster, less filtering
double rollVarProcess = 0.3;// 0.0005;// 0.0003;  smaller: faster, less filtering
//Kalman filter heading
double headK, headPc, headG, headXp, headZp, headXe;
double headP = 1.0;
double headVar = 0.1; // variance, smaller, more faster filtering
double headVarProcess = 0.1;// 0.001;//  smaller: faster, less filtering
//Kalman filter lat
double latK, latPc, latG, latXp, latZp, latXe;
double latP = 1.0;
double latVar = 0.1; // variance, smaller, more faster filtering
double latVarProcess = 0.5;//  smaller: faster, less filtering
//Kalman filter lon
double lonK, lonPc, lonG, lonXp, lonZp, lonXe;
double lonP = 1.0;
double lonVar = 0.1; // variance, smaller, more faster filtering
double lonVarProcess = 0.5;//  smaller: faster, less filtering

#if useWiFi
//WIFI
IPAddress IPToAOG(192, 168, 1, 255);//IP address to send UDP data to
byte myIPEnding = 79;             //ending of IP adress x.x.x.79 
byte my_WiFi_Mode = 0;  // WIFI_STA = 1 = Workstation  WIFI_AP = 2  = Accesspoint
#endif

//UBX
byte UBXRingCount1 = 0, UBXDigit1 = 0, UBXDigit2 = 0, OGIfromUBX = 0;
short UBXLenght1 = 100, UBXLenght2 = 100;
const unsigned char UBX_HEADER[] = { 0xB5, 0x62 };//all UBX start with this
bool isUBXPVT1 = false,  isUBXRelPosNED = false, existsUBXRelPosNED = false;

unsigned long NtripDataTime = 0, now = 0;
double virtLat = 0.0, virtLon = 0.0;//virtual Antenna Position

//NMEA
byte OGIBuffer[90];
bool newOGI = false; 
byte OGIdigit = 0;

//heading + roll
const byte GPSHeadingArraySize = 30;
double GPSHeading[GPSHeadingArraySize];
byte headRingCount = 0;
const double PI180 = PI / 180;
bool dualGPSHeadingPresent = false, virtAntPresent = false, rollPresent = false;
float roll = 0.0;
byte rollRingCont = 0;
byte dualAntNoValue = 0, dualAntNoValueMax = 15;// if dual Ant value not valid for xx times, send position without correction/heading/roll


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
const byte sizeOfUBXArray = 30;
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
    unsigned char res2;
    char relPosHPN;
    char relPosHPE;
    char relPosHPD;
    char relPosHPLength;
    unsigned long accN;
    unsigned long accE;
    unsigned long accD;
    unsigned long accLength;
    unsigned long accHeading;
    unsigned char res3;
    unsigned long flags;
    unsigned char CK0;
    unsigned char CK1;
};
NAV_RELPOSNED UBXRelPosNED;

#if useWiFi
#include <AsyncUDP.h>
//#include <WiFiUdp.h>
//#include <WiFiType.h>
#include <WiFiSTA.h>
#include <WiFiServer.h>
//#include <WiFiScan.h>
//#include <WiFiMulti.h>
//#include <WiFiGeneric.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WiFi.h>
//#include <ETH.h>


//instances----------------------------------------------------------------------------------------
AsyncUDP udpRoof;
AsyncUDP udpNtrip;
#endif

void setup()
{
    delay(200);
    delay(200);
    delay(50);
	//start serials
    Serial.begin(115200);
    delay(50);
    Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1);
    delay(10);
    Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2);
    delay(10);

    Serial.println();//new line

#if useWiFi
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
        Serial.println(IPToAOG); 
        Serial.print("UDP writing to port: ");
        Serial.println(GPSSet.portDestination);
        Serial.print("UDP writing from port: ");
        Serial.println(GPSSet.portMy);
    }
    delay(100);
#endif
}

void loop()
{
    getUBX();//read serials    

    if (UBXRingCount1 != OGIfromUBX)//new UXB exists
    {
        headingRollCalc();
        if ((dualGPSHeadingPresent) && (rollPresent)) {
            //virtual Antenna point?
            if ((GPSSet.virtAntForew != 0) || (GPSSet.virtAntRight != 0) ||
                ((GPSSet.GPSPosCorrByRoll == 1) && (GPSSet.AntHight > 0)))
            {//all data there
                virtualAntennaPoint();
            }
            buildOGI();
            newOGI = true;
        }
        else
        {
            dualAntNoValue++;//watchdog
            if (dualAntNoValue > dualAntNoValueMax) {//no new values exist, so send only pos
                if ((debugmodeHeading)||(debugmodeVirtAnt)) { Serial.println("no dual Antenna values, no heading/roll, watchdog: send only Pos"); }

                buildOGI();
                newOGI = true;
            }
        }
    }
#if !useWiFi
    if (GPSSet.AOGNtrip == 1) { doSerialNTRIP(); } //gets USB NTRIP and sends to serial 1   
    if (newOGI) {
        for (byte n = 0; n < (OGIdigit -1); n++)
        {
            Serial.write(OGIBuffer[n]);
        }
        Serial.println();
        newOGI = false;    }
#endif
#if useWiFi
    if (GPSSet.AOGNtrip == 1) { doUDPNtrip(); } //gets UDP NTRIP and sends to serial 1 

    if (newOGI) {
        udpRoof.writeTo(OGIBuffer, OGIdigit, IPToAOG, GPSSet.portDestination);
        newOGI = false; 
    }
#endif
}
