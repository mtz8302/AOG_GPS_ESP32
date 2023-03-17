//ESP32 programm for GPS receivers to send NMEA to AgOpenGPS or other program 

//Version für GPS Einheit in Klarsichtbox 206V + 6006

byte vers_nr = 54;
char VersionTXT[150] = " - 25. Feb 2023 by MTZ8302<br>(single GPS only, BNO085 serial, CMPS14 via I2C, WiFi to Ethernet bridge, multiple WiFi networks)";

//ESP sending $GGA+VTG and AgOpenGPS IMU PGN sentence via UDP to IP x.x.x.255 at port 9999 or via USB

//AgOpenGPS sending NTRIP via UDP to port 2233(or USB) -> ESP sends it to UBLOX via UART

//by Matthias Hammer (MTZ8302) and others 2023
//see GitHub https://github.com/mtz8302 and Youtube Ma Ha MTZ8302

//change stettings to your need. Afterwards you can change them via webinterface x.x.x.79 (192.168.1.79)
//if connection to your network fails an accesspoint is opened: webinterface 192.168.1.1

//use serial monitor at USB port, to get debug messages and IP for webinterface at ESP start.

//for easier setup:
//use webinterface, turn debugmode on and change GPIO pin until you get data from the UBlox receivers on USB serial monitor

//the settings below are written as defalt values and can be reloaded.
//So if changing settings set EEPROM_clear = true; (line ~109) - flash - boot - reset to EEPROM_clear = false - flash again to keep them as defauls


struct set {
    //connection plan:
    // ESP32--- Right F9P GPS pos --- Left F9P Heading-----Sentences
    //  RX1-27-------TX1--------------------------------UBX-Nav-PVT out   (=position+speed)
    //  TX1-16-------RX1--------------------------------RTCM in           (NTRIP comming from AOG to get absolute/correct postion

    // IO pins ESP32 side ----------------------------------------------------------------------------
    byte RX1 = 27;                    //right F9P TX1 GPS pos (or only one F9P)
    byte TX1 = 16;                    //right F9P RX1 GPS pos (or only one F9P)
    unsigned long GPS_baudrate = 115200;

    byte RX2 = 26;                    //BNO085
    byte TX2 = 14;                    //BNO085

    byte Eth_CS_PIN = 5;              //CS PIN with SPI Ethernet hardware  SPI config: MOSI 23 / MISO 19 / CLK18 / CS5

    uint8_t SDA = 21;	                  // I2C Pins DOS NOT WORK HERE don't know why, so use only default pins
    uint8_t SCL = 22;

    byte Button_WiFi_rescan_PIN = 4;  //Button to rescan/reconnect WiFi networks / push to GND

    byte LEDWiFi_PIN = 2;      // WiFi Status LED 0 = off
    byte LEDWiFi_ON_Level = HIGH;    //HIGH = LED on high, LOW = LED on low

    unsigned long USB_baudrate = 115200;

    //Network---------------------------------------------------------------------------------------------
    //tractors WiFi or mobile hotspots
    char ssid1[24] = "Matthias A53 5G";           // WiFi network Client name
    char password1[24] = "";                 // WiFi network password//Accesspoint name and password
    char ssid2[24] = "Fendt_260V_Tablet";// WiFi network Client name
    char password2[24] = "Fendt_260V";                 // WiFi network password//Accesspoint name and password
    char ssid3[24] = "Deutz_6006_Tablet";           // WiFi network Client name
    char password3[24] = "Deutz_6006";                 // WiFi network password//Accesspoint name and password
    char ssid4[24] = "Fendt_209V";       // WiFi network Client name
    char password4[24] = "";                 // WiFi network password//Accesspoint name and password
    char ssid5[24] = "WLANHammer";    // WiFi network Client name
    char password5[24] = "";                 // WiFi network password//Accesspoint name and password

    char ssid_ap[24] = "Fendt_260V_Tablet";  // name of Access point, if no WiFi found, NO password!!
    char password_ap[24] = "Fendt_260V";  // name of Access point, if no WiFi found, NO password!!

    int timeoutRouter = 20;//100                //time (s) to search for existing WiFi, than starting Accesspoint 

    byte timeoutWebIO = 10;                 //time (min) afterwards webinterface is switched off
/*
    // Ntrip Caster Data
    char NtripHost[40] = "www.sapos-bw-ntrip.de";//  "80.154.101.74";    // Server IP or URL
    int  NtripPort = 2101;                // Server Port
    char NtripMountpoint[40] = "VRS_3_3G_BW";   // Mountpoint
    char NtripUser[40] = "";     // Username
    char NtripPassword[40] = "";    // Password

    byte NtripSendWhichGGASentence = 2; // 0 = No Sentence will be sended / 1 = send fixed GGA Sentence from belowb/ 2 = GGA from GPS position will be sended

    char NtripFixGGASentence[100] = "$GPGGA, 162029.60, 4855.51531, N, 00915.91969, E, 2, 17, 1.55, 0250.17, M, , , , * 0E"; //hc create via www.nmeagen.org

    byte NtripGGASendRate = 10;         // time in seconds between GGA Packets

*/
    //WiFi
    byte WiFi_myip[4] = { 192, 168, 1, 79 };     // Roofcontrol module 
    byte WiFi_gwip[4] = { 192, 168, 1, 1 };      // Gateway IP only used if Accesspoint created
    byte WiFi_ipDest_ending = 255;//ending of IP address to send UDP data to
    byte mask[4] = { 255, 255, 255, 0 };
    byte myDNS[4] = { 8, 8, 8, 8 };         //optional

    //Ethernet
    byte Eth_myip[4] = { 192, 168, 5, 80 };     // Roofcontrol module 
    byte Eth_ipDest_ending = 255;//ending of IP address to send UDP data to
    byte Eth_mac[6] = { 0x90,0xA2,0xDA,0x10,0xB3,0x1C }; //usb autosteer 0x70, 0x69, 0x69, 0x2D, 0x30, 0x31
    bool Eth_static_IP = true;					// false = use DHPC and set last number to 80 (x.x.x.80) / true = use IP as set above
    
    bool bridge_AOG_UDP_Eth_to_WiFi = true;
   
    unsigned int PortGPSToAOG = 5544;             //this is port of this module: Autosteer = 5577 IMU = 5566 GPS = 
    unsigned int PortFromAOG = 8888;            //port to listen for AOG
    unsigned int AOGNtripPort = 2233;       //port NTRIP data from AOG comes in
    unsigned int PortDestination = 9999;    //Port of AOG that listens
/*
    //Antennas position
    double AntDist = 74.0;                //cm distance between Antennas
    double AntHight = 228.0;              //cm hight of Antenna
    double virtAntLeft = 37.0;           //cm to move virtual Antenna to the left (was renamed, keep your settings, name of direction was wrong)
    double virtAntForew = 0;            //cm to move virtual Antenna foreward
    double headingAngleCorrection = 90;

    double AntDistDeviationFactor = 1.4;  // factor (>1), of whom lenght vector from both GPS units can max differ from AntDist before stop heading calc
    byte checkUBXFlags = 1;               //UBX sending quality flags, when used with RTK sometimes 
    byte filterGPSposOnWeakSignal = 1;    //filter GPS Position on weak GPS signal

    byte GPSPosCorrByRoll = 1;            // 0 = off, 1 = correction of position by roll (AntHight must be > 0)
    double DualRollAngleCorrection = 0.0;

    byte MaxHeadChangPerSec = 30;         // NOT USED at the moment - degrees that heading is allowed to change per second
*/
    byte DataTransVia = 10;                //transfer data via 0 = USB / 7 = WiFi UDP / 8 = WiFi UDP 2x / 10 = Ethernet UDP

    byte NtripClientBy = 1;               //NTRIP client 0:off /1: listens for AOG NTRIP to UDP (WiFi/Ethernet) or USB serial /2: use ESP32 WiFi NTIRP client

    //example: for dual GPS: sendOGI 1, all other 0. For single antenna: VTG,GGA = 1; OGI,HDT = 0, if you have CMPS14 IMUPGN = 1
  //  byte sendOGI = 0;                     //1: send NMEA message 0: off
 //   byte sendVTG = 1;                     //1: send NMEA message 0: off
  //  byte sendGGA = 1;                     //1: send NMEA message 0: off
   // byte sendHDT = 0;                     //1: send NMEA message 0: off HDT=Trimble heading sentence
    byte sendIMUPGN = 1;                  //for CMPS14: 1: send IMU message 0: off
    byte AgIOHeartbeat_answer = 1;			//0: don't answer (default for WiFi), IP is send in any case	


    //IMU
    byte IMUType = 1;                     //0: none; 1: BNO085 in serial mode; 2: CMPS14 via I2C
    bool swapRollPitch = true;           //false: roll = X-axis, true: roll = Y-axis
    float IMUHeadingCorrection = 0.0;
    float IMURollCorrection = 0.0;
    int CMPS14_ADDRESS = 0x60;   // Address of CMPS14 shifted right one bit for arduino wire library

    bool debugmode = false;
   /*     bool debugmodeUBX = false;
    bool debugmodeHeading = false;
   bool debugmodeVirtAnt = false;
   bool debugmodeFilterPos = false;
    bool debugmodeNTRIP = false;
    bool debugmodeRAW = false;
*/
}; set Set;


bool EEPROM_clear = true;  //set to true when changing settings to write them as default values: true -> flash -> boot -> false -> flash again

//AgIO communcation
uint8_t AgIO_msgHeader[] = { 128,129,127};
byte AgIO_heartbeat = 200;
byte AgIO_subnet_chng = 201;//subnet change from AgIO, only used with Ethernet, WiFi always DHCP
byte AgIO_ScanRequest = 202;//AgIO asking for IP -> Ethernet/WiFi GPS sends IP 
uint8_t helloFromIMU[] = { 128, 129, 121, 121, 5, 0, 0, 0, 0, 0, 71 }; //hello from IMU sent back


// WiFistatus LED 
// blink times: searching WIFI: blinking 4x faster; connected: blinking as times set; data available: light on; no data for 2 seconds: blinking
unsigned int LED_WIFI_time = 0;
unsigned int LED_WIFI_pulse = 2000;   //light on in ms 
unsigned int LED_WIFI_pause = 1500;   //light off in ms
boolean LED_WIFI_ON = false;



//WIFI+Ethernet
unsigned long now = 0, NtripDataTime = 0, NtripDataMsgTime = 0, WebIOTimeOut = 0, WiFi_network_search_timeout = 0;//WiFi_lost_time = 0,
byte Eth_connect_step, WiFi_connect_step = 10, WiFi_STA_connect_call_nr = 1, WiFi_netw_nr = 0, my_WiFi_Mode = 0;   // WIFI_STA = 1 = Workstation  WIFI_AP = 2  = Accesspoint
IPAddress WiFi_ipDestination, Eth_ipDestination; //set in network.ino
bool  task_Eth_NTRIP_running = false, WebIORunning = false, WiFiUDPRunning = false, task_WiFiConnectRunning = false;
unsigned int packetLength = 0;
byte incommingBytes[512];
const byte BridgeArraySize = 10;
byte BridgeData[BridgeArraySize][512];
unsigned int BridgePacketLength[BridgeArraySize] = { 0,0,0,0,0,0,0,0,0,0};
byte BridgeDataRingCountIn = 0, BridgeDateRingCountOut = 0;

//NMEA
//bool newGGA = false, newVTG = false;
//byte OGIdigit = 0, GGAdigit = 0, VTGdigit = 0, HDTdigit = 0;

/*
// ai, 07.10.2020: use the GGA Message to determine Fix-Quality
bool bNMEAstarted = false, bGGAexists = false;
String sNMEA;
int i, iPos;
char cFixQualGGA;
// END ai, 07.10.2020: use the GGA Message to determine Fix-Quality

*/
//heading + roll
float rollIMURAW = 0;//rollToAOG = 0.0, 
float HeadingIMURAW = 0, HeadingIMUcorr = 0;


//CMPS14
bool CMPS14_NewData = false, task_CMPS_reader_running = false;

//IMU PGN - 211 - 0xD3
byte IMUToAOG[] = { 0x80,0x81,0x7D,0xD3,8, 0,0,0,0,0,0,0,0, 15 };
int IMUToAOGSize = sizeof(IMUToAOG);
uint16_t IMUHeading[10], IMURoll[10];
byte IMUDataRingCount = 0;

//BNO
unsigned long BNO_lastReadTime = 0;

#include <AsyncUDP.h>
//#include <HTTP_Method.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <ETH.h>
#include <Ethernet.h>
//#include <EthernetUdp.h>
//#include <EthernetWebServer.h>
//#include <EthernetHttpClient.h>
#include <base64.h>
#include <Wire.h>
#include "zAOG_ESP32Ping.h"
#include "zAOG_ping.h"
#include "zAOG_BNO_RVC.h"




//instances----------------------------------------------------------------------------------------
BNO_rvc rvc = BNO_rvc();
AsyncUDP WiFiUDPFromAOG;
AsyncUDP WiFiUDPToAOG;
AsyncUDP WiFiUDPBridgeToAOG;
AsyncUDP WiFi_udpNtrip;
EthernetUDP Eth_udpRoof;
EthernetUDP Eth_udpNtrip;
WebServer WiFi_Server(80);
//WebServer Eth_Server(80);
WiFiClient WiFi_Ntrip_cl;

TaskHandle_t taskHandle_WiFi_connect;
TaskHandle_t taskHandle_Eth_connect;
TaskHandle_t taskHandle_Eth_NTRIP;
TaskHandle_t taskHandle_CMPS14;
TaskHandle_t taskHandle_WebIO;
//TaskHandle_t taskHandle_EthWebIO;
//TaskHandle_t taskHandle_LEDBlink;




// SETUP ------------------------------------------------------------------------------------------

void setup()
{
    delay(200);//waiting for stable power
    delay(50);//

    //start serials
    Serial.begin(Set.USB_baudrate);
    delay(50);
    //GPS: serial 1
    Serial1.begin(Set.GPS_baudrate, SERIAL_8N1, Set.RX1, Set.TX1);
    delay(10);
    Serial.println();//new line

#if useLED_BUILTIN
    pinMode(LED_BUILTIN, OUTPUT);
#endif
    if (Set.LEDWiFi_PIN != 255) { pinMode(Set.LEDWiFi_PIN, OUTPUT); }
    pinMode(Set.Button_WiFi_rescan_PIN, INPUT_PULLUP);

    restoreEEprom();
    delay(150);

    Serial.print("ESP code Version Nr ");
    Serial.print(vers_nr);
    Serial.println(VersionTXT);

    //init BNO085: serial 2
    if (Set.IMUType == 1) { init_BNO_serial(); }

    //init CMPS
    if (Set.IMUType == 2) { delay(100); init_CMPS14(); delay(100); }
    
    //start Ethernet
    if (Set.DataTransVia == 10) {
        Eth_connect_step = 10;
        xTaskCreate(Eth_handle_connection, "Core1EthConnectHandle", 3072, NULL, 1, &taskHandle_Eth_connect);
        delay(500);
    }
    else { Eth_connect_step = 255; }
        
    //start WiFi
    WiFi_connect_step = 10;//step 10 = begin of starting a WiFi connection
	xTaskCreate(WiFi_handle_connection, "WiFiConnectHandle", 3072, NULL, 1, &taskHandle_WiFi_connect);
	delay(500);


    vTaskDelay(5000); //waiting for other tasks

    NtripDataTime = millis();
}

// MAIN loop  -------------------------------------------------------------------------------------------

void loop()
{
    //serial BNO
    now = millis();
    if ((now + 10) > BNO_lastReadTime) {

        //Data is stored to IMUPGN but send when Serial1 = GPS data comes in
        if (Set.IMUType == 1) {
            BNO_rvcData bnoData;
            if (rvc.read(&bnoData))
            {   
                byte nextIMUDataRingCount = (IMUDataRingCount + 1) % 10;
                //the heading x10
                if (!bnoData.yawX10 == 0) {
                    IMUHeading[nextIMUDataRingCount] = bnoData.yawX10;

                //roll
                    if (Set.swapRollPitch) {
                        IMURoll[nextIMUDataRingCount] = bnoData.pitchX10;// +IMURoll[IMUDataRingCount];
                    }
                    else {
                        IMURoll[nextIMUDataRingCount] = bnoData.rollX10;
                    }
                    IMUDataRingCount = nextIMUDataRingCount;
                }
            }
        }//serial BNO
    }//timed BNO loop 100Hz

    //GPS data?
    while (Serial1.available()) {
        //send IMU data, when new GPS data is there
        if (Set.sendIMUPGN == 1) {
            //put IMU data from ringbuffer to IMU PGN
            byte fitIMUDataRingCount = (IMUDataRingCount + 8) % 10;  // + 6 = 4 back = 40 ms back

            IMUToAOG[6] = highByte(IMUHeading[fitIMUDataRingCount]);
            IMUToAOG[5] = lowByte(IMUHeading[fitIMUDataRingCount]);

         //   int angle16 = IMUToAOG[6];                 // Calculate 16 bit angle
          //  angle16 <<= 8;
         //   angle16 += IMUToAOG[5];

            HeadingIMURAW = float(IMUHeading[fitIMUDataRingCount]) / 10;
            HeadingIMUcorr = HeadingIMURAW + Set.IMUHeadingCorrection;

            IMUToAOG[8] = highByte(IMURoll[fitIMUDataRingCount]);
            IMUToAOG[7] = lowByte(IMURoll[fitIMUDataRingCount]);
            rollIMURAW = float(IMURoll[fitIMUDataRingCount]) / 10;
            //add the checksum
            int CRCtoAOG = 0;
            for (byte i = 2; i < IMUToAOGSize - 1; i++)
            {
                CRCtoAOG = (CRCtoAOG + IMUToAOG[i]);
            }
            IMUToAOG[IMUToAOGSize - 1] = CRCtoAOG;
            //send
            if (Set.DataTransVia < 5) {//use USB
                Serial.write(IMUToAOG, IMUToAOGSize);
            }
            else
                if (Set.DataTransVia < 10) {//WiFi
                    if (WiFi_connect_step == 0) {
                        WiFiUDPToAOG.writeTo(IMUToAOG, IMUToAOGSize, WiFi_ipDestination, Set.PortDestination);
                    }
                }
                else {
                    //use Ethernet DataTransVia >= 10
                    if (Eth_connect_step == 0) {
                        Eth_udpRoof.beginPacket(Eth_ipDestination, Set.PortDestination);
                        Eth_udpRoof.write(IMUToAOG, IMUToAOGSize);
                        Eth_udpRoof.endPacket();
                    }
                }
        }

        //GPS data
        packetLength = Serial1.available();
        packetLength = Serial1.read(incommingBytes, packetLength);
        if (Set.DataTransVia < 5) {//use USB
            Serial.write(incommingBytes, packetLength);
        }
        else
            if (Set.DataTransVia < 10) {//WiFi
                if (WiFi_connect_step == 0) {
                    //send WiFi UDP // WiFi UDP NTRIP via AsyncUDP: only called once, works with .onPacket
                    WiFiUDPToAOG.writeTo(incommingBytes, packetLength, WiFi_ipDestination, Set.PortDestination);
                }
            }
            else {
                //use Ethernet DataTransVia >= 10
                if (Eth_connect_step == 0) {
                    Eth_udpRoof.beginPacket(Eth_ipDestination, Set.PortDestination);
                    Eth_udpRoof.write(incommingBytes, packetLength);
                    Eth_udpRoof.endPacket();
                }
            }
    }

    //USB data: NTRIP or UCenter commands
    while (Serial.available()) {
        packetLength = Serial.available();
        packetLength = Serial.read(incommingBytes, packetLength);
        Serial1.write(incommingBytes, packetLength);
        NtripDataTime = millis();
        if (Set.debugmode) { Serial.println("got NTRIP data via USB"); }
    }

    //Ethernet data: NTRIP or AgOpenGPS data or AgIO heart beat
    packetLength = Eth_udpRoof.parsePacket();
    if (packetLength) {
        //if (Set.debugmode) { Serial.print("got UDP data via Ethernet lenght: "); Serial.println(packetLength); }
        Eth_udpRoof.read(incommingBytes, packetLength);
        // Eth_udpRoof.flush();
         //AgIO message?
        if ((incommingBytes[0] == AgIO_msgHeader[0]) && (incommingBytes[1] == AgIO_msgHeader[1]) &&
            (incommingBytes[2] == AgIO_msgHeader[2])) {
            if ((Set.IMUType > 0) && (incommingBytes[3] == AgIO_heartbeat) && (Set.AgIOHeartbeat_answer == 1))
            {//AgIO heartbeat
                Eth_udpRoof.beginPacket(Eth_ipDestination, Set.PortDestination);
                Eth_udpRoof.write(helloFromIMU, sizeof(helloFromIMU));
                Eth_udpRoof.endPacket();
            }
            else {
                if (incommingBytes[3] == AgIO_subnet_chng) {
                    Set.Eth_myip[0] = incommingBytes[7];
                    Set.Eth_myip[1] = incommingBytes[8];
                    Set.Eth_myip[2] = incommingBytes[9];
                    Ethernet.setLocalIP(Set.Eth_myip);
                    Serial.print("changing Ethernet IP address as requested by AgIO to ");
                    delay(1000);
                    Serial.println(Ethernet.localIP());
                    EEprom_write_all();
                }
                else {
                    if (incommingBytes[3] == AgIO_ScanRequest) {
                        if (Set.debugmode) { Serial.println("got AgIO IP scan request via Ethernet"); }
                        uint8_t scanReply[] = { 128, 129, 120, 203, 7,
                            Set.Eth_myip[0],Set.Eth_myip[1],Set.Eth_myip[2],Set.Eth_myip[3],0,0,0, 23 };
                        //checksum
                        int16_t CK_A = 0;
                        for (uint8_t i = 2; i < sizeof(scanReply) - 1; i++)
                        {
                            CK_A = (CK_A + scanReply[i]);
                        }
                        scanReply[sizeof(scanReply) - 1] = CK_A;
                        Eth_udpRoof.beginPacket(Eth_ipDestination, Set.PortDestination);
                        Eth_udpRoof.write(scanReply, sizeof(scanReply));
                        Eth_udpRoof.endPacket();

                        if (Set.IMUType > 0) {
                            uint8_t scanReply[] = { 128, 129, 121, 203, 7,
                            Set.Eth_myip[0],Set.Eth_myip[1],Set.Eth_myip[2],Set.Eth_myip[3],0,0,0, 23 };
                            //checksum
                            int16_t CK_A = 0;
                            for (uint8_t i = 2; i < sizeof(scanReply) - 1; i++)
                            {
                                CK_A = (CK_A + scanReply[i]);
                            }
                            scanReply[sizeof(scanReply) - 1] = CK_A;
                            Eth_udpRoof.beginPacket(Eth_ipDestination, Set.PortDestination);
                            Eth_udpRoof.write(scanReply, sizeof(scanReply));
                            Eth_udpRoof.endPacket();
                        }
                    }
                }
            }
        }//AgIO message

        //AOG data to WiFi eg for section control
        if ((Set.bridge_AOG_UDP_Eth_to_WiFi) && (WiFi_connect_step == 0)) {
            //send WiFi UDP tp Port 8888 = as from AOG
            WiFiUDPToAOG.writeTo(incommingBytes, packetLength, WiFi_ipDestination, Set.PortFromAOG);
        }
    }  // end of Ethernet packet

    vTaskDelay(2);

    //WiFi to Ethernet bridge + NTRIP via Ethernet
    if (Eth_connect_step == 0) {
        //WiFi to Ethernet bridge
        for (;;) {
            if (BridgeDateRingCountOut == BridgeDataRingCountIn) { break; }
            Eth_udpRoof.beginPacket(Eth_ipDestination, Set.PortDestination);
            Eth_udpRoof.write(BridgeData[BridgeDateRingCountOut], BridgePacketLength[BridgeDateRingCountOut]);
            Eth_udpRoof.endPacket();
            //Serial.print("Sending bridge packet #"); Serial.println(BridgeDateRingCountOut);
            BridgeDateRingCountOut = (BridgeDateRingCountOut + 1) % BridgeArraySize;
        }

        //NTRIP via Ethernet
        unsigned int EthNTRIPPacketLenght = Eth_udpNtrip.parsePacket();
        char Eth_NTRIP_packetBuffer[512];
        //Serial.println("checking for NTRIP data");
        if (EthNTRIPPacketLenght) {
            if (Set.debugmode) {
                Serial.print("got NTRIP data via Ethernet lenght: "); Serial.println(EthNTRIPPacketLenght);
            }
            Eth_udpNtrip.read(Eth_NTRIP_packetBuffer, EthNTRIPPacketLenght);
            //Eth_udpNtrip.flush();
            Serial1.write(Eth_NTRIP_packetBuffer, EthNTRIPPacketLenght);
            NtripDataTime = millis();
        }
    }


    //check WiFi connection
    if ((Set.DataTransVia > 5) && (Set.DataTransVia < 10) && (WiFi_connect_step == 0)) {
        if (my_WiFi_Mode == WIFI_STA) {
            if (WiFi.status() != WL_CONNECTED) {
                WiFi_STA_connect_call_nr = 0;//to enable DHCP for WiFi
                WiFi_connect_step = 1;//ping
                my_WiFi_Mode = 0;//set 0 to end Ntrip task
                WiFiUDPRunning = false;
                if (!task_WiFiConnectRunning) {
                    //start WiFi
                    xTaskCreate(WiFi_handle_connection, "WiFiConnectHandle", 3072, NULL, 1, &taskHandle_WiFi_connect);
                    delay(200);
                }
            }
        }
        //WiFi rescann button pressed?
        if (!digitalRead(Set.Button_WiFi_rescan_PIN)) {
            WiFi_STA_connect_call_nr = 0;//to enable DHCP for WiFi
            WiFi_connect_step = 3;
            my_WiFi_Mode = 0;//set 0 to end Ntrip task
            WiFiUDPRunning = false;
            if (!task_WiFiConnectRunning) {
                //start WiFi
                xTaskCreate(WiFi_handle_connection, "WiFiConnectHandle", 3072, NULL, 1, &taskHandle_WiFi_connect);
                delay(500);
            }
        }
    }//WiFi reconnect
}

//-------------------------------------------------------------------------------------------------
//9. Maerz 2021

void WiFi_LED_blink(void* pvParameters)
{
    unsigned long now, saveTime = 0;
    byte blkSpeed;

    for (;;) {

        now = millis();
/*
        if ((Set.IMUType) && (now > saveTime + 600000)) {//10min time to write cmps calibration
            saveTime = now;
            EEprom_write_all();
            //Serial.println("savetime");
            now = millis();
        }
*/
        if (WiFi_connect_step > 0) { blkSpeed = 2; }
        else {
            if (Set.NtripClientBy > 0) {
                if (now > (NtripDataTime + 30000))
                {
                    blkSpeed = 0;
                    if (now > NtripDataMsgTime + 30000) {
                        NtripDataMsgTime = millis();
                        if (Set.NtripClientBy == 2) { Serial.println("no NTRIP data from ESP32 NTRIP client for more than 30s"); }
                        else {
                            Serial.print("no NTRIP for more than 30s from ");
                            if (Set.DataTransVia < 5) { Serial.println("USB"); }
                            else
                                if (Set.DataTransVia < 10) { Serial.println("WiFi"); }
                                else
                                    if (Set.DataTransVia >= 10) { Serial.println("Ethernet"); }
                        }
                    }
                }
                else {
                    //all OK = LED on                
                    blkSpeed = 255;
                    LED_WIFI_ON = true;
                    digitalWrite(Set.LEDWiFi_PIN, Set.LEDWiFi_ON_Level);
#if useLED_BUILTIN
                    digitalWrite(LED_BUILTIN, HIGH);
#endif
                    vTaskDelay(3000);
                }
            }
        }

        if (blkSpeed != 255) {
            if (!LED_WIFI_ON) {
                LED_WIFI_time = now;
                LED_WIFI_ON = true;
#if useLED_BUILTIN
                digitalWrite(LED_BUILTIN, HIGH);
#endif
                digitalWrite(Set.LEDWiFi_PIN, Set.LEDWiFi_ON_Level);

                vTaskDelay(LED_WIFI_pause >> blkSpeed);

            }
            if (LED_WIFI_ON) {
                LED_WIFI_time = now;
                LED_WIFI_ON = false;
#if useLED_BUILTIN
                digitalWrite(LED_BUILTIN, LOW);
#endif
                digitalWrite(Set.LEDWiFi_PIN, !Set.LEDWiFi_ON_Level);

                vTaskDelay(LED_WIFI_pulse >> blkSpeed);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
/*
void WiFi_LED_blink(byte blkSpeed)   //8. Maerz 2020
{
    unsigned long now = millis();
    if (!LED_WIFI_ON) {
        if (now > (LED_WIFI_time + (LED_WIFI_pause >> blkSpeed))) {
            LED_WIFI_time = now;
            LED_WIFI_ON = true;
#if useLED_BUILTIN
            digitalWrite(LED_BUILTIN, HIGH);
#endif
            digitalWrite(Set.LEDWiFi_PIN, Set.LEDWiFi_ON_Level);
        }
    }
    if (LED_WIFI_ON) {
        if (now > (LED_WIFI_time + (LED_WIFI_pulse >> blkSpeed))) {
            LED_WIFI_time = now;
            LED_WIFI_ON = false;
#if useLED_BUILTIN
            digitalWrite(LED_BUILTIN, LOW);
#endif
            digitalWrite(Set.LEDWiFi_PIN, !Set.LEDWiFi_ON_Level);
        }
    }
}*/
