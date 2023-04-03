//ESP32 programm for GPS receivers to send NMEA to AgOpenGPS or other program 

//Version für GPS Einheit in Klarsichtbox 206V + 6006

byte vers_nr = 60;
char VersionTXT[150] = " - 3. Apr 2023 by MTZ8302<br>PANDA (single GPS only, BNO085 RVC serial, CMPS14 via I2C, WiFi to Ethernet bridge, multiple WiFi networks)";

//ESP sending $GGA+VTG and AgOpenGPS IMU PGN sentence via UDP to IP x.x.x.255 at port 9999 or via USB

//AgOpenGPS sending NTRIP via UDP to port 2233(or USB) -> ESP sends it to UBLOX via UART

//code copied from everywere in the internet and combined by Matthias Hammer (MTZ8302) and others 2023
//see GitHub https://github.com/mtz8302 and Youtube Ma Ha MTZ8302

//change stettings to your need. Afterwards you can change them via webinterface x.x.x.79 (192.168.1.79)
//if connection to your network fails an accesspoint is opened: webinterface 192.168.1.1

//use serial monitor at USB port, to get debug messages and IP for webinterface at ESP start.

//the settings below are written as defalt values and can be reloaded.
//So if changing settings set EEPROM_clear = true; (line ~109) - flash - boot - reset to EEPROM_clear = false - flash again to keep them as defauls

struct set {
    //connection plan:
    //  ESP32 --- F9P GPS pos ---Sentences
    //  RX1-27-------TX1-----------NMEA GGA and VTG out   (=position+speed)
    //  TX1-16-------RX1----------------RTCM in           (NTRIP comming from AOG to get absolute/correct postion

    // IO pins ESP32 side ----------------------------------------------------------------------------
    byte RX1 = 27;                    //F9P TX1 GPS pos
    byte TX1 = 14;                    //F9P RX1 GPS pos
    byte GPS_baudrate_Nr = 4;         //4 = 115 200 = recommended  

    byte RX2 = 26;                    //BNO085
    byte TX2 = 0;                     //BNO085

    byte Eth_CS_PIN = 5;              //CS PIN with SPI Ethernet hardware  SPI config: MOSI 23 / MISO 19 / CLK18 / CS 5
    byte Eth_INT_PIN = 10;            //interupt pin not used at the moment, but needed if async ethernet lib will be used

    uint8_t SDA = 21;	              // I2C Pins DOS NOT WORK HERE don't know why, so use only default pins
    uint8_t SCL = 22;

    byte Button_WiFi_rescan_PIN = 4;  //Button to rescan/reconnect WiFi networks / push to GND

    byte LEDWiFi_PIN = 15;            // WiFi Status LED 0 = off
    byte LEDWiFi_ON_Level = HIGH;     //HIGH = LED on high, LOW = LED on low

    byte USB_baudrate_Nr = 4;         //4 = 115 200 = recommended

    //Network---------------------------------------------------------------------------------------------
    //tractors WiFi or mobile hotspots
    char ssid1[24] = "";                     // WiFi network Client name
    char password1[24] = "";                 // WiFi network password//Accesspoint name and password
    char ssid2[24] = "";                     // WiFi network Client name
    char password2[24] = "";                 // WiFi network password//Accesspoint name and password
    char ssid3[24] = "";                     // WiFi network Client name
    char password3[24] = "";                 // WiFi network password//Accesspoint name and password
    char ssid4[24] = "";                     // WiFi network Client name
    char password4[24] = "";                 // WiFi network password//Accesspoint name and password
    char ssid5[24] = "";                     // WiFi network Client name
    char password5[24] = "";                 // WiFi network password//Accesspoint name and password

    char ssid_ap[24] = "Fendt_260V_Tablet";  // name of Access point, if no WiFi found, or no SSID1 .. SSID5 set !!
    char password_ap[24] = "Fendt_260V";     // password for Access point, if no WiFi found, or no SSID1 .. SSID5 set !!

    int timeoutRouter = 100;                 // time (s) to search for existing WiFi, than starting Accesspoint 

    byte timeoutWebIO = 10;                  // time (min) afterwards webinterface is switched off

//WiFi
    byte WiFi_myip[4] = { 192, 168, 1, 79 };     // Roofcontrol module 
    byte WiFi_gwip[4] = { 192, 168, 1, 1 };      // Gateway IP only used if Accesspoint created
    byte WiFi_ipDest_ending = 255;               // ending of IP address to send UDP data to
    byte mask[4] = { 255, 255, 255, 0 };
    byte myDNS[4] = { 8, 8, 8, 8 };              //optional

    //Ethernet
    byte Eth_myip[4] = { 192, 168, 5, 80 };     // Roofcontrol module 
    byte Eth_ipDest_ending = 255;               // ending of IP address to send UDP data to
    byte Eth_mac[6] = { 0x90,0xA2,0xDA,0x10,0xB3,0x1C }; //usb autosteer 0x70, 0x69, 0x69, 0x2D, 0x30, 0x31
    bool Eth_static_IP = true;					// false = use DHPC and set last number to 80 (x.x.x.80) / true = use IP as set above


    unsigned int PortGPSToAOG = 5544;          //this is port of this module: Autosteer = 5577 IMU = 5566 GPS = 
    unsigned int PortFromAOG = 8888;           //port to listen for AOG
    unsigned int AOGNtripPort = 2233;          //port NTRIP data from AOG comes in
    unsigned int PortDestination = 9999;       //Port of AOG that listens

    byte DataTransVia = 10;              //transfer data via 0 = USB / 7 = WiFi UDP / 10 = Ethernet UDP

    byte WiFi_AOG_data_bridge = true;    // other modules can log into ESP32 WiFi and data is forewarded to AGO via Ethernet or USB
    bool bridgeGPStoUSB = false;         // connects GPS to USB for UCenter use

    byte NtripClientBy = 1;              // NTRIP client 0:off /1: listens for AOG NTRIP to UDP (WiFi/Ethernet) or USB serial /2: use ESP32 WiFi NTIRP client

    byte sendPANDA = 1;                  // 1: send NMEA message 0: off
    byte sendIMUPGN = 1;                 // for CMPS14/BNO: 1: send IMU message 0: off, irrelevant if PANDA is used, as included in PANDA
    byte AgIOHeartbeat_answer = 1;		 // 0: don't answer (default for WiFi), IP is send in any case	


    //IMU
    byte IMUType = 1;                     // 0: none; 1: BNO085 in serial mode; 2: CMPS14 via I2C
    bool IMUswapRollPitch = false;        // false: roll = X-axis, true: roll = Y-axis
    bool IMUinvertRoll = false;
    float IMURollCorrection = 0.0;

    float CMPSHeadingCorrection = 0.0;
    int CMPS14_ADDRESS = 0x60;           // Address of CMPS14 shifted right one bit for arduino wire library

    bool debugmode = false;

}; set Set;


bool EEPROM_clear = false;  //set to true when changing settings to write them as default values: true -> flash -> boot -> false -> flash again

// Baudrates for serial
unsigned long baudrates[]
{
  9600,//0
  19200,//1
  38400,//2
  57600,//3
  115200,//4
  230400,//5
  460800,//6
};
const uint32_t nrBaudrates = sizeof(baudrates) / sizeof(baudrates[0]);
bool GPSBaudrateValid = false;

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
unsigned long now = 0, NtripDataTime = 0, NtripDataMsgTime = 0, WebIOLastUsePlus3 = 0, WebIOTimeOut = 0, WiFi_network_search_timeout = 0, halfSecLoopTime = 0;//WiFi_lost_time = 0,
byte Eth_connect_step, WiFi_connect_step = 10, WiFi_STA_connect_call_nr = 1, WiFi_netw_nr = 0, my_WiFi_Mode = 0;   // WIFI_STA = 1 = Workstation  WIFI_AP = 2  = Accesspoint
IPAddress WiFi_ipDestination, Eth_ipDestination; //set in network.ino
bool  WebIORunning = false, WiFiUDPRunning = false, task_WiFiConnectRunning = false;
unsigned int packetLength = 0;
byte incommingBytes[254];
char PANDA[100];// the new PANDA sentence buffer
bool newPANDA = false;
const byte BridgeArraySize = 10;
byte BridgeData9999[BridgeArraySize][254], BridgeData8888[BridgeArraySize][254];
unsigned int BridgePacket9999Length[BridgeArraySize] = { 0,0,0,0,0,0,0,0,0,0 }, BridgePacket8888Length[BridgeArraySize] = { 0,0,0,0,0,0,0,0,0,0 };
byte BridgeData9999RingCountIn = 0, BridgeData9999RingCountOut = 0, BridgeData8888RingCountIn = 0, BridgeData8888RingCountOut = 0;

//NMEA
bool newGGA = false, newVTG = false;

//IMU heading + roll
float rollIMURAW = 0;
float HeadingIMURAW = 0, HeadingIMUcorr = 0;
char imuHeading[6], imuRoll[6], imuPitch[6], imuYawRate[6];//for PANDA sentence
short IMUHeading[10], IMURoll[10], IMUPitch[10], IMUYawRate[10];//ringbuffer: store 10 values for time fit with GPS data
byte IMUDataRingCount = 0, IMUDataTimeShift = 0, IMUnoData = 0;
const byte IMUDataTimeShiftCMPS = 9, IMUDataTimeShiftBNO = 8;//+8 = -2 as ring of 10; take older data for PANDA sentence, as GPS data is about 80ms old and IMU data is 100Hz -> 4=40ms
const byte IMUBNOreadTime = 20;  //time ms to read IMU IMUCMPSreadTime = 49;
unsigned long IMUnextReadTime = 0;
bool task_CMPS_reader_running = false;

//IMU PGN - 211 - 0xD3
byte IMUToAOG[] = { 0x80,0x81,0x7D,0xD3,8, 0,0,0,0,0,0,0,0, 15 };
int IMUToAOGSize = sizeof(IMUToAOG);



#include <AsyncUDP.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <ETH.h>
#include <Ethernet.h>
#include <base64.h>
#include <Wire.h>
#include "zAOG_ESP32Ping.h"
#include "zAOG_ping.h"
#include "zAOG_BNO_RVC.h"
#include "zNMEAParser.h"


//instances----------------------------------------------------------------------------------------
BNO_rvc rvc = BNO_rvc();
AsyncUDP WiFiUDPFromAOG;
AsyncUDP WiFiUDPToAOG;
AsyncUDP WiFiUDPBridgeToAOG;
AsyncUDP WiFi_udpNtrip;
EthernetUDP Eth_udpRoof;
EthernetUDP Eth_udpNtrip;
WebServer WiFi_Server(80);
WiFiClient WiFi_Ntrip_cl;


/* A parser is declared with 3 handlers at most */
NMEAParser<2> parser;

TaskHandle_t taskHandle_WiFi_connect;
TaskHandle_t taskHandle_Eth_connect;
TaskHandle_t taskHandle_CMPS14;
TaskHandle_t taskHandle_WebIO;
TaskHandle_t taskHandle_EthWebIO;


// SETUP ------------------------------------------------------------------------------------------

void setup()
{
    delay(200);//waiting for stable power
    delay(50);//

    //start serials
    Serial.begin(baudrates[Set.GPS_baudrate_Nr]);
    delay(50);
    //GPS: serial 1
    Serial1.begin(baudrates[Set.GPS_baudrate_Nr], SERIAL_8N1, Set.RX1, Set.TX1);
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
    Serial.println();

    //init NMEA parser, the dash means wildcard
    parser.setErrorHandler(errorHandler);
    parser.addHandler("G-GGA", GGA_Handler);
    parser.addHandler("G-VTG", VTG_Handler);

    //init BNO085: serial 2
    if (Set.IMUType == 1) { init_BNO_serial(); }

    //init CMPS via I2C
    if (Set.IMUType == 2) { delay(100); init_CMPS14(); delay(100); IMUnextReadTime = 4294967294; }

    delay(100);

    //no IMU, or IMU not found 
    if (Set.IMUType == 0) {
        for (byte a = 0; a < 10; a++) {
            IMUHeading[a] = 65535;
            IMURoll[a] = 0;// 8888;
            IMUPitch[a] = 0;
            IMUYawRate[a] = 0;
        }
        itoa(65535, imuHeading, 10);       //65535 is max value to stop AgOpen using IMU in Panda
        IMUnextReadTime = 4294967294;
        Set.sendIMUPGN = 0;
    }

    //start Ethernet
    if (Set.DataTransVia == 10) {
        Eth_connect_step = 10;
        xTaskCreate(Eth_handle_connection, "Core1EthConnectHandle", 3072, NULL, 1, &taskHandle_Eth_connect);
        delay(500);
    }
    else { Eth_connect_step = 255; }

    //start WiFi
    if ((strlen(Set.ssid1) == 0) && (strlen(Set.ssid2) == 0) && (strlen(Set.ssid3) == 0) && (strlen(Set.ssid4) == 0) && (strlen(Set.ssid5) == 0)) {
        WiFi_connect_step = 50;
    } //step 50 = start WiFi access point    
    else { WiFi_connect_step = 10; } //step 10 = begin of starting a WiFi connection

    xTaskCreate(WiFi_handle_connection, "WiFiConnectHandle", 3072, NULL, 1, &taskHandle_WiFi_connect);
    delay(500);

    vTaskDelay(5000); //waiting for other tasks

    NtripDataTime = millis();
    if (!Set.DataTransVia == 7) { Set.WiFi_AOG_data_bridge = false; } //only usefull, if data tranfer via Ethernet or USB
}

// MAIN loop  -------------------------------------------------------------------------------------------

void loop()
{
    //IMU handle. if no IMU IMUnextReadTime is very high, so no read
    now = millis();
    if (now > IMUnextReadTime) {//CMPS is read in extra task
        readBNO();//serial BNO (RVC mode)
    }//timed IMU loop

    //GPS data?
    while (Serial1.available()) {
        packetLength = Serial1.available();
        packetLength = Serial1.read(incommingBytes, packetLength);

        //pass data to USB for config the F9P with UCenter
        if (Set.bridgeGPStoUSB == true) { Serial.write(incommingBytes, packetLength); }

        if (Set.sendPANDA == 1) {
            for (int i = 0; i < packetLength; i++) {
                parser << char(incommingBytes[i]);

                //if GGA is comming, it will send PANDA after the GGA parsing void
            }
            if (Set.DataTransVia < 5) {//use USB
                Serial.write(PANDA, strlen(PANDA));
            }
            else
                if (Set.DataTransVia < 10) {//WiFi
                    if (WiFi_connect_step == 0) {
                        byte tempBytes[100], tempLeng = strlen(PANDA);
                        for (int i = 0; i < tempLeng; i++) {
                            tempBytes[i] = byte(PANDA[i]);
                        }
                        WiFiUDPToAOG.writeTo(tempBytes, tempLeng, WiFi_ipDestination, Set.PortDestination);
                    }
                }
                else {
                    //use Ethernet DataTransVia >= 10
                    if (Eth_connect_step == 0) {
                        Eth_udpRoof.beginPacket(Eth_ipDestination, Set.PortDestination);
                        Eth_udpRoof.write(PANDA, strlen(PANDA));
                        Eth_udpRoof.endPacket();
                    }
                }
        }//PANDA
        else {//send GPS data as it comes from receiver
            //send IMU data, when new GPS data is there
            if (Set.sendIMUPGN == 1) {
                //put IMU data from ringbuffer to IMU PGN
                byte fitIMUDataRingCount = (IMUDataRingCount + IMUDataTimeShift) % 10; //IMU time shift: GPS data is about 100ms old, so use older IMU data to be syncron
                if (Set.IMUType == 1) { IMUDataTimeShift = IMUDataTimeShiftBNO; }
                else if (Set.IMUType == 2) { IMUDataTimeShift = IMUDataTimeShiftCMPS; }

                IMUToAOG[6] = highByte(IMUHeading[fitIMUDataRingCount]);
                IMUToAOG[5] = lowByte(IMUHeading[fitIMUDataRingCount]);

                //   int angle16 = IMUToAOG[6];                 // Calculate 16 bit angle
                 //  angle16 <<= 8;
                //   angle16 += IMUToAOG[5];

                HeadingIMURAW = float(IMUHeading[fitIMUDataRingCount]) / 10;
                HeadingIMUcorr = HeadingIMURAW + Set.CMPSHeadingCorrection;

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


            //send Serial1 (GPS) data
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
    }

    //USB data: NTRIP or UCenter commands or AgOpenGPS data for section control etc
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
        if ((Set.WiFi_AOG_data_bridge) && (WiFi_connect_step == 0)) {
            //send WiFi UDP tp Port 8888 = as from AOG
            WiFiUDPToAOG.writeTo(incommingBytes, packetLength, WiFi_ipDestination, Set.PortFromAOG);
        }
    }  // end of Ethernet packet

    vTaskDelay(2);//2

    //WiFi to Ethernet bridge + NTRIP via Ethernet
    if (Eth_connect_step == 0) {
        //WiFi to Ethernet bridge
        for (;;) {
            if (BridgeData9999RingCountOut == BridgeData9999RingCountIn) { break; }
            Eth_udpRoof.beginPacket(Eth_ipDestination, Set.PortDestination);
            Eth_udpRoof.write(BridgeData9999[BridgeData9999RingCountOut], BridgePacket9999Length[BridgeData9999RingCountOut]);
            Eth_udpRoof.endPacket();
            BridgeData9999RingCountOut = (BridgeData9999RingCountOut + 1) % BridgeArraySize;
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

    if (now > halfSecLoopTime) {
        halfSecLoopTime = now + 500;
        vTaskDelay(3);//give more time for other tasks
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
}

//-------------------------------------------------------------------------------------------------
//9. Maerz 2021

void WiFi_LED_blink(void* pvParameters)
{
    unsigned long now, saveTime = 0;
    byte blkSpeed;

    for (;;) {

        now = millis();
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


