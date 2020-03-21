#if HardwarePlatform == 0
// Wifi variables & definitions
#define MAX_PACKAGE_SIZE 2048
char HTML_String[24000];
char HTTP_Header[150];

// Allgemeine Variablen

int Aufruf_Zaehler = 0;

#define ACTION_SET_SSID              1  
#define ACTION_SET_OUTPUT_TYPE       2  // also adress at EEPROM
#define ACTION_SET_loadDefault       3
#define ACTION_SET_Msg               4
#define ACTION_SET_debugmode         5
#define ACTION_SET_AOGNTRIP         12
#define ACTION_SET_RESTART          13
#define ACTION_SET_GPIO             14
#define ACTION_SET_GPSPosCorrByRoll 15
#define ACTION_SET_RollAngCorr      16
#define ACTION_SET_MixedHeading     17
#define ACTION_SET_AntDist          18
#define ACTION_SET_VirtAntPoi       19
#define  ACTION_SET_HeadAngCorr     20
#define ACTION_SET_AntDistDevFact   21
#define ACTION_SET_DataTransfVia    22
#define ACTION_SET_WiFiLEDon        23

int action;

//-------------------------------------------------------------------------------------------------
// 11. Maerz 2020

void doWebInterface() {

    unsigned long my_timeout;

    // Check if a client has connected
    client_page = server.available();

    if (!client_page)  return;

    Serial.println("New Client:");           // print a message out the serial port

    my_timeout = millis() + 250L;
    while (!client_page.available() && (millis() < my_timeout)) { delay(10); }
    delay(10);
    if (millis() > my_timeout)
    {
        Serial.println("Client connection timeout!");
        client_page.flush();
        client_page.stop();
        return;
    }

    //---------------------------------------------------------------------
    //htmlPtr = 0;
    char c;
    if (client_page) {                        // if you get a client,
      //Serial.print("New Client.\n");                   // print a message out the serial port
        String currentLine = "";                // make a String to hold incoming data from the client
        while (client_page.connected()) {       // loop while the client's connected
            delay(0);
            if (client_page.available()) {        // if there's bytes to read from the client,
                char c = client_page.read();        // read a byte, then
                Serial.print(c);                             // print it out the serial monitor
                if (c == '\n') {                    // if the byte is a newline character

                  // if the current line is blank, you got two newline characters in a row.
                  // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0) {

                        make_HTML01();  // create Page array
                       //---------------------------------------------------------------------
                       // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                       // and a content-type so the client knows what's coming, then a blank line:
                        strcpy(HTTP_Header, "HTTP/1.1 200 OK\r\n");
                        strcat(HTTP_Header, "Content-Length: ");
                        strcati(HTTP_Header, strlen(HTML_String));
                        strcat(HTTP_Header, "\r\n");
                        strcat(HTTP_Header, "Content-Type: text/html\r\n");
                        strcat(HTTP_Header, "Connection: close\r\n");
                        strcat(HTTP_Header, "\r\n");

                        client_page.print(HTTP_Header);
                        delay(20);
                        send_HTML();
                        // break out of the while loop:
                        break;
                    }
                    else {    // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                { // if you got anything else but a carriage return character,
                    currentLine += c;      // add it to the end of the currentLine
                    if (currentLine.endsWith("HTTP"))
                    {
                        if (currentLine.startsWith("GET "))
                        {
                            currentLine.toCharArray(HTML_String, currentLine.length());
                            Serial.println(); //NL
                            exhibit("Request : ", HTML_String);
                            process_Request();
                        }
                    }
                }//end else
            } //end client available
        } //end while client.connected
        // close the connection:
        client_page.stop();
        Serial.print("Pagelength : ");
        Serial.print((long)strlen(HTML_String));
        Serial.print("   --> Client Disconnected\n");
    }// end if client 
}


//-------------------------------------------------------------------------------------------------
// Process given values
//-------------------------------------------------------------------------------------------------
void process_Request()
{
    int myIndex;

    if (Find_Start("/?", HTML_String) < 0 && Find_Start("GET / HTTP", HTML_String) < 0)
    {
        //nothing to process
        return;
    }
    action = Pick_Parameter_Zahl("ACTION=", HTML_String);

    if (action != ACTION_SET_RESTART) { EEprom_unblock_restart(); }
    if (action == ACTION_SET_loadDefault) {
        EEprom_read_default();
        delay(5);
    }
    // WiFi access data
    if (action == ACTION_SET_SSID) {

        myIndex = Find_End("SSID_MY=", HTML_String);
        if (myIndex >= 0) {
            for (int i = 0; i < 24; i++) GPSSet.ssid[i] = 0x00;
            Pick_Text(GPSSet.ssid, &HTML_String[myIndex], 24);
            exhibit("SSID  : ", GPSSet.ssid);
        }
        myIndex = Find_End("Password_MY=", HTML_String);
        if (myIndex >= 0) {
            for (int i = 0; i < 24; i++) GPSSet.password[i] = 0x00;
            Pick_Text(GPSSet.password, &HTML_String[myIndex], 24);
            exhibit("Password  : ", GPSSet.password);
            
        }
        int tempint = Pick_Parameter_Zahl("timeoutRout=", HTML_String);
        if ((tempint > 20) && (tempint < 1000)) { GPSSet.timeoutRouter = tempint; }
        EEprom_write_all();
    }
    if (action == ACTION_SET_GPSPosCorrByRoll) {
        if (Pick_Parameter_Zahl("GPSPosCorrByRoll=", HTML_String) == 0) GPSSet.GPSPosCorrByRoll = 0;
        if (Pick_Parameter_Zahl("GPSPosCorrByRoll=", HTML_String) == 1) GPSSet.GPSPosCorrByRoll = 1;
        if (GPSSet.debugmode) {
            Serial.println();
            Serial.print("Set GPSPosCorrByRoll to "); Serial.println(GPSSet.GPSPosCorrByRoll);
            Serial.println();
            delay(500);
        }
        exhibit("AOG NTRIP client  : ", GPSSet.GPSPosCorrByRoll);
        EEprom_write_all();
    }
    if (action == ACTION_SET_AntDist) {
        int tempint = Pick_Parameter_Zahl("AntDist=", HTML_String); //get interger value
        //Serial.println(Pick_Parameter_Zahl("RollAngleCorr=", HTML_String));
        if (tempint >= 0) {//value transmited
            GPSSet.AntDist = tempint;
            if (GPSSet.debugmode) {
                Serial.println();
                Serial.print("AntDist "); Serial.println(tempint);
                Serial.println();
                delay(500);
            }
        }
        tempint = Pick_Parameter_Zahl("AntHight=", HTML_String); //get interger value
//Serial.println(Pick_Parameter_Zahl("RollAngleCorr=", HTML_String));
        if (tempint >= 0) {//value transmited
            GPSSet.AntHight = tempint;
            if (GPSSet.debugmode) {
                Serial.println();
                Serial.print("AntHight "); Serial.println(tempint);
                Serial.println();
                delay(500);
            }
        }
        EEprom_write_all();
    }
    if (action == ACTION_SET_VirtAntPoi) {
        int tempint = Pick_Parameter_Zahl("AntRight=", HTML_String); //get interger value
        if (tempint > 0) {//value transmited
            GPSSet.virtAntRight = tempint;
        }
        else {//neg or 0
            tempint = Pick_Parameter_Zahl("AntRight=-", HTML_String); //get interger value
            if (tempint >= 0) { GPSSet.virtAntRight = 0 - tempint; }
        }
        if (GPSSet.debugmode) {
            Serial.println();
            Serial.print("AntRight "); Serial.println(tempint);
            Serial.println();
            delay(500);
        }
        tempint = Pick_Parameter_Zahl("AntForew=", HTML_String); //get interger value
        if (tempint > 0) {//value transmited
            GPSSet.virtAntForew = tempint;
        }
        else {//neg or 0
            tempint = Pick_Parameter_Zahl("AntForew=-", HTML_String); //get interger value
            if (tempint >= 0) { GPSSet.virtAntForew = 0 - tempint; }
        }
        if (GPSSet.debugmode) {
            Serial.println();
            Serial.print("AntForew "); Serial.println(tempint);
            Serial.println();
            delay(500);
        }
        EEprom_write_all();
    }
    if (action == ACTION_SET_AntDistDevFact) {
        float tempfl = Pick_Parameter_Zahl("AntDistDevFact=1.", HTML_String); //gets digits#
        if (tempfl > 0) {
            if (tempfl < 10) { tempfl = (tempfl * 0.1) + 1; }//ex: 1,3 not 1,03
            else { tempfl = (tempfl * 0.01) + 1; }
            GPSSet.AntDistDeviationFactor = tempfl;

            if (GPSSet.debugmode) {
                Serial.println();
                Serial.print("AntDisDevFact "); Serial.println(tempfl);
                Serial.println();
                delay(500);
            }
        }
        if (Pick_Parameter_Zahl("UBXFlagCheck=", HTML_String) == 0) GPSSet.checkUBXFlags = 0;
        if (Pick_Parameter_Zahl("UBXFlagCheck=", HTML_String) == 1) GPSSet.checkUBXFlags = 1;
        if (Pick_Parameter_Zahl("GPSPosFilter=", HTML_String) == 0) GPSSet.filterGPSposOnWeakSignal = 0;
        if (Pick_Parameter_Zahl("GPSPosFilter=", HTML_String) == 1) GPSSet.filterGPSposOnWeakSignal = 1;
        EEprom_write_all();
    }
    if (action == ACTION_SET_HeadAngCorr) {
        float tempfl = Pick_Parameter_Zahl("HeadAngleCorr=", HTML_String); //get interger value
        //Serial.println(Pick_Parameter_Zahl("RollAngleCorr=", HTML_String));
        char tempstr[20] = "HeadAngleCorr=";
        strcati(tempstr, int(tempfl));
        strcat(tempstr, ".");
        int tempint = Pick_Parameter_Zahl(tempstr, HTML_String);//get .x
        if (tempint > 0) { tempfl += tempint * 0.1; }//dezimal exists
        GPSSet.headingAngleCorrection = tempfl;
        if (GPSSet.debugmode) {
            Serial.println();
            Serial.print("HeadAngCorr "); Serial.println(tempfl);
            Serial.println();
            delay(500);
        }
        EEprom_write_all();
    }
    if (action == ACTION_SET_RollAngCorr) {
        float tempfl = Pick_Parameter_Zahl("RollAngleCorr=", HTML_String); //get interger value
        //Serial.println(Pick_Parameter_Zahl("RollAngleCorr=", HTML_String));
        bool rollNeg = false;
        char tempstr[20] = "RollAngleCorr=";
        if (tempfl <= 0) {
            rollNeg = true;
            strcat(tempstr, "-");
            tempfl = Pick_Parameter_Zahl("RollAngleCorr=-", HTML_String); //get interger value 
            if (tempfl == -1) { tempfl = 0; }//-1 = not found
           // Serial.println(Pick_Parameter_Zahl("RollAngleCorr=-", HTML_String));
        }
        strcati(tempstr, int(tempfl));
        strcat(tempstr, ".");
        int tempint = Pick_Parameter_Zahl(tempstr, HTML_String);//get .x
        //Serial.println(tempint);
        if (tempint > 0) { tempfl += tempint * 0.1; }//dezimal exists
        if (rollNeg) { tempfl *= -1; }
        GPSSet.rollAngleCorrection = tempfl;
        if (GPSSet.debugmode) {
            Serial.println();
            Serial.print("RollAngCorr "); Serial.println(tempfl);
            Serial.println();
            delay(500);
        }
        EEprom_write_all();
    }
    if (action == ACTION_SET_MixedHeading) {
        //int temp = Pick_Parameter_Zahl("MixHead=", HTML_String);
        if (Pick_Parameter_Zahl("MixHead=", HTML_String) == 0) GPSSet.useMixedHeading = 0;
        if (Pick_Parameter_Zahl("MixHead=", HTML_String) == 1) GPSSet.useMixedHeading = 1;
        EEprom_write_all();
    }
    if (action == ACTION_SET_AOGNTRIP) {
        //int temp = Pick_Parameter_Zahl("AOGNTRIP=", HTML_String);
        if (Pick_Parameter_Zahl("AOGNTRIP=", HTML_String) == 0) GPSSet.AOGNtrip = 0;
        if (Pick_Parameter_Zahl("AOGNTRIP=", HTML_String) == 1) GPSSet.AOGNtrip = 1;
        if (GPSSet.debugmode) {
            Serial.println();
            Serial.print("Set AOGNTRIP to "); Serial.println(GPSSet.AOGNtrip);
            Serial.println();
            delay(500);
        }
        exhibit("AOG NTRIP client  : ", GPSSet.AOGNtrip);
        EEprom_write_all();
    }
    if (action == ACTION_SET_Msg) {
        bool noMsg = true;
        //int temp = Pick_Parameter_Zahl("AOGNTRIP=", HTML_String);
        if (Pick_Parameter_Zahl("seOGI=", HTML_String) == 1) { GPSSet.sendOGI = 1; noMsg = false; }
            else { GPSSet.sendOGI = 0; }
        if (Pick_Parameter_Zahl("seGGA=", HTML_String) == 1) { GPSSet.sendGGA = 1; noMsg = false; }
        else { GPSSet.sendGGA = 0; }
        if (Pick_Parameter_Zahl("seVTG=", HTML_String) == 1) { GPSSet.sendVTG = 1; noMsg = false; }
        else { GPSSet.sendVTG = 0; }
        if (Pick_Parameter_Zahl("seHDT=", HTML_String) == 1) { GPSSet.sendHDT = 1; noMsg = false; }
        else { GPSSet.sendHDT = 0; }
        if (noMsg) { GPSSet.sendOGI = 1; }
        if (GPSSet.debugmode) {
            Serial.println();
            Serial.print("Set PAOGI to "); Serial.println(GPSSet.sendOGI);
            Serial.println();
            delay(500);
        }

        EEprom_write_all();
    }
    if (action == ACTION_SET_DataTransfVia) {
        //int temp = Pick_Parameter_Zahl("AOGNTRIP=", HTML_String);
        if (Pick_Parameter_Zahl("DataTransfVia=", HTML_String) == 0) GPSSet.DataTransVia = 0;
        if (Pick_Parameter_Zahl("DataTransfVia=", HTML_String) == 1) GPSSet.DataTransVia = 1;
        if (GPSSet.debugmode) {
            Serial.println();
            Serial.print("Set AOGNTRIP to "); Serial.println(GPSSet.AOGNtrip);
            Serial.println();
            delay(500);
        }
        EEprom_write_all();
    }
    if (action == ACTION_SET_debugmode)
    {
        byte tempby = Pick_Parameter_Zahl("debugmode=", HTML_String);
        if (tempby == 1) {
            GPSSet.debugmode = true;
        }
        else { GPSSet.debugmode = false; }//no value back from page = 0

        tempby = Pick_Parameter_Zahl("debugmUBX=", HTML_String);
        if (tempby == 1) {
            GPSSet.debugmodeUBX = true;
        }
        else { GPSSet.debugmodeUBX = false; }//no value back from page = 0

        tempby = Pick_Parameter_Zahl("debugmHead=", HTML_String);
        if (tempby == 1) {
            GPSSet.debugmodeHeading = true;
        }
        else { GPSSet.debugmodeHeading = false; }//no value back from page = 0

        tempby = Pick_Parameter_Zahl("debugmVirtAnt=", HTML_String);
        if (tempby == 1) {
            GPSSet.debugmodeVirtAnt = true;
        }
        else { GPSSet.debugmodeVirtAnt = false; }//no value back from page = 0

        tempby = Pick_Parameter_Zahl("debugmFiltPos=", HTML_String);
        if (tempby == 1) {
            GPSSet.debugmodeFilterPos = true;
        }
        else { GPSSet.debugmodeFilterPos = false; }//no value back from page = 0
        tempby = Pick_Parameter_Zahl("debugmRAW=", HTML_String);
        if (tempby == 1) {
            GPSSet.debugmodeRAW = true;
        }
        else { GPSSet.debugmodeRAW = false; }//no value back from page = 0
        EEprom_write_all();
    }

	if (action == ACTION_SET_WiFiLEDon) {
		//int temp = Pick_Parameter_Zahl("AOGNTRIP=", HTML_String);
		if (Pick_Parameter_Zahl("WiFiLEDon=", HTML_String) == 0) {
			GPSSet.LEDWiFi_ON_Level = 0;
			if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, 0); }
			else { digitalWrite(GPSSet.LEDWiFi_PIN, 1); }
		}
		if (Pick_Parameter_Zahl("WiFiLEDon=", HTML_String) == 1) {
			GPSSet.LEDWiFi_ON_Level = 1;
			if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, 1); }
			else { digitalWrite(GPSSet.LEDWiFi_PIN, 0); }
		}
		EEprom_write_all();
	}
    if (action == ACTION_SET_GPIO) {
        //Serial.println(Pick_Parameter_Zahl("LED=", HTML_String));
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 2) { GPSSet.LEDWiFi_PIN = 2; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 4) { GPSSet.LEDWiFi_PIN = 4; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 5) { GPSSet.LEDWiFi_PIN = 5; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT);  if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); }EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 13) { GPSSet.LEDWiFi_PIN = 13; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 14) { GPSSet.LEDWiFi_PIN = 14; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT);  if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); }EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 15) { GPSSet.LEDWiFi_PIN = 15; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT);  if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); }EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 16) { GPSSet.LEDWiFi_PIN = 16; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT);  if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); }EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 17) { GPSSet.LEDWiFi_PIN = 17; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 18) { GPSSet.LEDWiFi_PIN = 18; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 19) { GPSSet.LEDWiFi_PIN = 19; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 20) { GPSSet.LEDWiFi_PIN = 20; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 21) { GPSSet.LEDWiFi_PIN = 21; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 22) { GPSSet.LEDWiFi_PIN = 22; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 23) { GPSSet.LEDWiFi_PIN = 23; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 24) { GPSSet.LEDWiFi_PIN = 24; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 25) { GPSSet.LEDWiFi_PIN = 25; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 26) { GPSSet.LEDWiFi_PIN = 26; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 27) { GPSSet.LEDWiFi_PIN = 27; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 28) { GPSSet.LEDWiFi_PIN = 28; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT);  if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); }EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 29) { GPSSet.LEDWiFi_PIN = 29; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 30) { GPSSet.LEDWiFi_PIN = 30; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT);  if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); }EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 31) { GPSSet.LEDWiFi_PIN = 31; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT);  if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); }EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 32) { GPSSet.LEDWiFi_PIN = 32; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT); if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); } EEprom_write_all(); }
        if (Pick_Parameter_Zahl("LED=", HTML_String) == 33) { GPSSet.LEDWiFi_PIN = 33; pinMode(GPSSet.LEDWiFi_PIN, OUTPUT);  if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); }EEprom_write_all(); }
        //Serial.println();
        //Serial.print("set pin for WiFi LED to: "); Serial.println(GPSSet.LEDWiFi_PIN); Serial.println();
        //delay(500);
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 2) {
            GPSSet.TX1 = 2; 
            Serial1.end(); 
            delay(20); 
            Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); 
            delay(5); 
            EEprom_write_all();
        }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 3) { GPSSet.TX1 = 3; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 4) { GPSSet.TX1 = 4; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 5) { GPSSet.TX1 = 5; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 13) { GPSSet.TX1 = 13; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 14) { GPSSet.TX1 = 14; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 15) { GPSSet.TX1 = 15; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 16) { GPSSet.TX1 = 16; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 17) { GPSSet.TX1 = 17; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 18) { GPSSet.TX1 = 18; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 19) { GPSSet.TX1 = 19; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 20) { GPSSet.TX1 = 20; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 21) { GPSSet.TX1 = 21; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 22) { GPSSet.TX1 = 22; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 23) { GPSSet.TX1 = 23; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 24) { GPSSet.TX1 = 24; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 25) { GPSSet.TX1 = 25; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 26) { GPSSet.TX1 = 26; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 27) { GPSSet.TX1 = 27; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 28) { GPSSet.TX1 = 28; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 29) { GPSSet.TX1 = 29; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 30) { GPSSet.TX1 = 30; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 31) { GPSSet.TX1 = 31; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 32) { GPSSet.TX1 = 32; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX1=", HTML_String) == 33) { GPSSet.TX1 = 33; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
    
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 2) {
            GPSSet.RX1 = 2;
            Serial1.end();
            delay(20);
            Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1);
            delay(5);
            EEprom_write_all();
        }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 3) { GPSSet.RX1 = 3; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 4) { GPSSet.RX1 = 4; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 5) { GPSSet.RX1 = 5; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 13) { GPSSet.RX1 = 13; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 14) { GPSSet.RX1 = 14; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 15) { GPSSet.RX1 = 15; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 16) { GPSSet.RX1 = 16; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 17) { GPSSet.RX1 = 17; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 18) { GPSSet.RX1 = 18; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 19) { GPSSet.RX1 = 19; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 20) { GPSSet.RX1 = 20; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 21) { GPSSet.RX1 = 21; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 22) { GPSSet.RX1 = 22; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 23) { GPSSet.RX1 = 23; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 24) { GPSSet.RX1 = 24; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 25) { GPSSet.RX1 = 25; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 26) { GPSSet.RX1 = 26; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 27) { GPSSet.RX1 = 27; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 28) { GPSSet.RX1 = 28; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 29) { GPSSet.RX1 = 29; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 30) { GPSSet.RX1 = 30; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 31) { GPSSet.RX1 = 31; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 32) { GPSSet.RX1 = 32; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX1=", HTML_String) == 33) { GPSSet.RX1 = 33; Serial1.end(); delay(20); Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1); delay(5); EEprom_write_all(); }
    

        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 2) {
            GPSSet.TX2 = 2;
            Serial2.end();
            delay(20);
            Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2);
            delay(5);
            EEprom_write_all();
        }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 3) { GPSSet.TX2 = 3; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 4) { GPSSet.TX2 = 4; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 5) { GPSSet.TX2 = 5; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 13) { GPSSet.TX2 = 13; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 14) { GPSSet.TX2 = 14; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 15) { GPSSet.TX2 = 15; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 16) { GPSSet.TX2 = 16; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 17) { GPSSet.TX2 = 17; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 18) { GPSSet.TX2 = 18; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 19) { GPSSet.TX2 = 19; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 20) { GPSSet.TX2 = 20; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 21) { GPSSet.TX2 = 21; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 22) { GPSSet.TX2 = 22; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 23) { GPSSet.TX2 = 23; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 24) { GPSSet.TX2 = 24; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 25) { GPSSet.TX2 = 25; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 26) { GPSSet.TX2 = 26; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 27) { GPSSet.TX2 = 27; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 28) { GPSSet.TX2 = 28; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 29) { GPSSet.TX2 = 29; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 30) { GPSSet.TX2 = 30; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 31) { GPSSet.TX2 = 31; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 32) { GPSSet.TX2 = 32; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("TX2=", HTML_String) == 33) { GPSSet.TX2 = 33; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }

        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 2) {
            GPSSet.RX2 = 2;
            Serial2.end();
            delay(20);
            Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2);
            delay(5);
            EEprom_write_all();
        }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 3) { GPSSet.RX2 = 3; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 4) { GPSSet.RX2 = 4; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 5) { GPSSet.RX2 = 5; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 13) { GPSSet.RX2 = 13; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 14) { GPSSet.RX2 = 14; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 15) { GPSSet.RX2 = 15; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 16) { GPSSet.RX2 = 16; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 17) { GPSSet.RX2 = 17; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 18) { GPSSet.RX2 = 18; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 19) { GPSSet.RX2 = 19; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 20) { GPSSet.RX2 = 20; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 21) { GPSSet.RX2 = 21; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 22) { GPSSet.RX2 = 22; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 23) { GPSSet.RX2 = 23; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 24) { GPSSet.RX2 = 24; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 25) { GPSSet.RX2 = 25; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 26) { GPSSet.RX2 = 26; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 27) { GPSSet.RX2 = 27; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 28) { GPSSet.RX2 = 28; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 29) { GPSSet.RX2 = 29; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 30) { GPSSet.RX2 = 30; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 31) { GPSSet.RX2 = 31; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 32) { GPSSet.RX2 = 32; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }
        if (Pick_Parameter_Zahl("RX2=", HTML_String) == 33) { GPSSet.RX2 = 33; Serial2.end(); delay(20); Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2); delay(5); EEprom_write_all(); }

    } 
	if (action == ACTION_SET_RESTART) {
		EEprom_block_restart();//prevents from restarting, when webpage is reloaded. Is set to 0, when other ACTION than restart is called
		delay(2000);
		ESP.restart();
	}
}

//-------------------------------------------------------------------------------------------------
// HTML Seite 01 aufbauen
//-------------------------------------------------------------------------------------------------
void make_HTML01() {

    strcpy(HTML_String, "<!DOCTYPE html>");
    strcat(HTML_String, "<html>");
    strcat(HTML_String, "<head>");
    strcat(HTML_String, "<title>GPS roof unit Config Page</title>");
    strcat(HTML_String, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0;\" />\r\n");
    //strcat( HTML_String, "<meta http-equiv=\"refresh\" content=\"10\">");
    strcat(HTML_String, "<style>divbox {background-color: lightgrey;width: 200px;border: 5px solid red;padding:10px;margin: 10px;}</style>");
    strcat(HTML_String, "</head>");
    strcat(HTML_String, "<body bgcolor=\"#ffcc00\">");//ff9900
    strcat(HTML_String, "<font color=\"#000000\" face=\"VERDANA,ARIAL,HELVETICA\">");
    strcat(HTML_String, "<h1>GPS roof unit ESP32 for single or dual antenna</h1>");
    strcat(HTML_String, "for 1 or 2 UBlox receivers connected (1: UXB PVT 2: UBX RelPosNED)<br>");
    strcat(HTML_String,"supports data via USB/WiFi UDP, dual antenna, heading/roll filter and postion correction<br><br>");
    strcat(HTML_String, "<b>Transfers NTRIP data from AOG port 2233 to UBlox receiver</b><br><br>");
    strcat(HTML_String, "more settings like IPs, UPD ports... in setup zone of INO code<br>");
    strcat(HTML_String, "(Rev. 4.00 by MTZ8302 Webinterface by WEDER)<br><hr>");


    //---------------------------------------------------------------------------------------------  
    //load values of INO setup zone
    strcat(HTML_String, "<h2>Load default values of INO setup zone</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(270, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"2\">Only load default values, does NOT save them</td>");
    strcat(HTML_String, "<td><button style= \"width:150px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_loadDefault);
    strcat(HTML_String, "\">Load default values</button></td>");
    strcat(HTML_String, "</tr>");
    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //-----------------------------------------------------------------------------------------
    // WiFi Client Access Datastrcat( HTML_String, "<hr><h2>WiFi Network Client Access Data</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<h2>Network to send UDP data:</h2>");
    strcat(HTML_String, "</b>If access to networks fails, an accesspoint will be created:<br>SSID: <b>");
    strcat(HTML_String, GPSSet.ssid_ap);
    strcat(HTML_String, "</b>     with no password<br><br><table>");
    strcat(HTML_String, "<table>");
    set_colgroup(250, 300, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td><b>Network SSID:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" style= \"width:200px\" name=\"SSID_MY\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, GPSSet.ssid);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_SSID);
    strcat(HTML_String, "\">Apply and Save</button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td><b>Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" style= \"width:200px\" name=\"Password_MY\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, GPSSet.password);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\">time (s) trying to connect to network</td></tr>");
    strcat(HTML_String, "<td colspan=\"3\">after time has passed access point is opened</td></tr>");
    strcat(HTML_String, "<tr><td></td><td><input type = \"number\"  name = \"timeoutRout\" min = \"20\" max = \"1000\" step = \"1\" style= \"width:100px\" value = \"");// placeholder = \"");
    strcati(HTML_String, GPSSet.timeoutRouter);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
    strcat(HTML_String, "<tr><td colspan=\"2\"><b>Restart NTRIP client for changes to take effect</b></td>");
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_RESTART);
    strcat(HTML_String, "\">Restart</button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // Antenna distance/hight
    strcat(HTML_String, "<h2>Antenna position</h2>");
    //   strcat(HTML_String, "antennas must be left + right to be able to calculate roll<br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td>Antenna distance (cm)</td>");
    strcat(HTML_String, "<td><input type = \"number\"  name = \"AntDist\" min = \"0\" max = \"1000\" step = \"1\" style= \"width:100px\" value = \"");// placeholder = \"");
    strcati(HTML_String, int(GPSSet.AntDist));
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_AntDist);
    strcat(HTML_String, "\">Apply and Save</button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td>Antenna hight (cm)</td>");
    strcat(HTML_String, "<td><input type = \"number\"  name = \"AntHight\" min = \"0\" max = \"600\" step = \"1\" style= \"width:100px\" value = \"");// placeholder = \"");
    strcati(HTML_String, int(GPSSet.AntHight));
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // Antenna virtual position
    strcat(HTML_String, "<h2>Antenna virtual position</h2>");
    strcat(HTML_String, "Moves the antenna point to the right and foreward,<br>");
    strcat(HTML_String, "so you can eliminate antennas offset.<br>");
    strcat(HTML_String, "Don't move more than antenna distance.<br><br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td>move Antenna to the right (cm) (left: neg.)</td>");
    strcat(HTML_String, "<td><input type = \"number\"  name = \"AntRight\" min = \"-1000\" max = \"1000\" step = \"1\" style= \"width:100px\" value = \"");// placeholder = \"");
    strcati(HTML_String, int(GPSSet.virtAntRight));
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_VirtAntPoi);
    strcat(HTML_String, "\">Apply and Save</button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td>move Antenna foreward (cm) (backw.: neg.)</td>");
    strcat(HTML_String, "<td><input type = \"number\"  name = \"AntForew\" min = \"-1000\" max = \"1000\" step = \"1\" style= \"width:100px\" value = \"");// placeholder = \"");
    strcati(HTML_String, int(GPSSet.virtAntForew));
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // heading angle correction 
    strcat(HTML_String, "<h2>Heading angle correction</h2>");
    strcat(HTML_String, "Set to 90 if antenna for position is right and other left.<br><br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"number\"  name = \"HeadAngleCorr\" min = \" 0\" max = \"360\" step = \"0.1\" style= \"width:100px\" value = \"");// placeholder = \"");
    if (GPSSet.headingAngleCorrection < 10) { strcatf(HTML_String, GPSSet.headingAngleCorrection, 3, 1); }
    else {
        if (GPSSet.headingAngleCorrection < 100) { strcatf(HTML_String, GPSSet.headingAngleCorrection, 4, 1); }
        else { strcatf(HTML_String, GPSSet.headingAngleCorrection, 5, 1); }
    }
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_HeadAngCorr);
    strcat(HTML_String, "\">Apply and Save</button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // Roll angle correction 
    strcat(HTML_String, "<h2>Roll angle correction</h2>");
    strcat(HTML_String, "Antennas must be left + right to be able to calculate roll.<br><br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"number\"  name = \"RollAngleCorr\" min = \" - 45\" max = \"45\" step = \"0.1\" style= \"width:100px\" value = \"");// placeholder = \"");

    strcatf(HTML_String, GPSSet.rollAngleCorrection, 3, 1);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_RollAngCorr);
    strcat(HTML_String, "\">Apply and Save</button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // GPS position correction by roll 
    strcat(HTML_String, "<h2>Correct GPS position using roll</h2>");
    strcat(HTML_String, "Antennas must be left + right to be able to calculate roll.<br><br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"GPSPosCorrByRoll\" id=\"JZ\" value=\"0\"");
    if (GPSSet.GPSPosCorrByRoll == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">OFF</label></td>");
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_GPSPosCorrByRoll);
    strcat(HTML_String, "\">Apply and Save</button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"GPSPosCorrByRoll\" id=\"JZ\" value=\"1\"");
    if (GPSSet.GPSPosCorrByRoll == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">ON (default)</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // Mixed Heading
    strcat(HTML_String, "<h2>Mixed Heading</h2>");
    strcat(HTML_String, "Uses dual GPS heading and heading from position antenna on weak signal.<br><br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"MixHead\" id=\"JZ\" value=\"0\"");
    if (GPSSet.useMixedHeading == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">OFF</label></td>");
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_MixedHeading);
    strcat(HTML_String, "\">Apply and Save</button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"MixHead\" id=\"JZ\" value=\"1\"");
    if (GPSSet.useMixedHeading == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">ON (default)</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // heading calc antenna dist deviation + filter postion
    strcat(HTML_String, "<h2>GPS Signal quality check</h2>");        
    strcat(HTML_String,"<b>Max deviation for heading/roll calculation</b><br><br>");
    strcat(HTML_String, "If GPS signal is weak, heading and roll calc is wrong.<br>");
    strcat(HTML_String, "To check this, the antenna distance messured by GPS is compared with antenna distance from setup.<br>");
    strcat(HTML_String, "This factor is the max deviation for doing the calculations.<br>");
    strcat(HTML_String, "Example: Ant dist at tractor 100cm factor 1.2 = 100*1.2= 120cm; 100/1.2 = 83 cm.<br>"); 
    strcat(HTML_String, "If GPS antenna distance is less than 120 and more than 83 heading calcultaion is done.<br>");
    strcat(HTML_String, "Roll calculation is only done, if deviation is less than 1 / 4 of it.<br> <br>");
    strcat(HTML_String, "<b>factor: 1.1 - 1.99  recommended: 1.2 - 1.4. For new setups use 1.99 to test!</b><br>");
    
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"number\"  name = \"AntDistDevFact\" min = \" 1.1\" max = \"1.99\" step = \"0.01\" style= \"width:100px\" value = \"");// placeholder = \"");
    strcatf(HTML_String, GPSSet.AntDistDeviationFactor, 4, 2);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_AntDistDevFact);
    strcat(HTML_String, "\">Apply and Save</button></td>");
    strcat(HTML_String, "</tr>");
    strcat(HTML_String, "</table><br>");

    strcat(HTML_String, "<b>Check UBX flags for signal quality</b><br>");
    strcat(HTML_String, "With some bases for RTK, the flags sometimes fall back, so position will jump.<br>");
    strcat(HTML_String, "In this case turn check of UBX falgs OFF<br>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"UBXFlagCheck\" id=\"JZ\" value=\"0\"");
    if (GPSSet.checkUBXFlags == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">OFF</label></td>");;
    strcat(HTML_String, "</tr>");
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"UBXFlagCheck\" id=\"JZ\" value=\"1\"");
    if (GPSSet.checkUBXFlags == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">ON (default)</label></td></tr>");
    strcat(HTML_String, "</table><br>");

    strcat(HTML_String, "<b>Filter GPS position on weak signal</b><br>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"GPSPosFilter\" id=\"JZ\" value=\"0\"");
    if (GPSSet.filterGPSposOnWeakSignal == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">OFF</label></td>");;
    strcat(HTML_String, "</tr>");
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"GPSPosFilter\" id=\"JZ\" value=\"1\"");
    if (GPSSet.filterGPSposOnWeakSignal == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">ON (default)</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");




    //---------------------------------------------------------------------------------------------
    // Checkboxes Messages
    strcat(HTML_String, "<h2>Messages to send to AgOpenGPS</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

	strcat(HTML_String, "<tr>");
	strcat(HTML_String, "<td></td><td><input type=\"checkbox\" name=\"seOGI\" id = \"Part\" value = \"1\" ");
	if (GPSSet.sendOGI == 1) strcat(HTML_String, "checked ");
	strcat(HTML_String, "> ");
	strcat(HTML_String, "<label for =\"Part\"> send PAOGI</label>");
	strcat(HTML_String, "</td>");    
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_Msg);
    strcat(HTML_String, "\">Apply and Save</button></td>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type=\"checkbox\" name=\"seGGA\" id = \"Part\" value = \"1\" ");
    if (GPSSet.sendGGA == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> send GPGGA</label>");
    strcat(HTML_String, "</td></tr>");
    strcat(HTML_String, "<tr>");

    strcat(HTML_String, "<td></td><td><input type=\"checkbox\" name=\"seVTG\" id = \"Part\" value = \"1\" ");
    if (GPSSet.sendVTG == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> send GPVTG</label>");
    strcat(HTML_String, "</td></tr>");
    strcat(HTML_String, "<tr>");

    strcat(HTML_String, "<td></td><td><input type=\"checkbox\" name=\"seHDT\" id = \"Part\" value = \"1\" ");
    if (GPSSet.sendHDT == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> send GPHDT</label>");
    strcat(HTML_String, "</td></tr>");


    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");


    //---------------------------------------------------------------------------------------------  
    // NTRIP from AOG 
    strcat(HTML_String, "<h2>NTRIP from AOG</h2>");
    strcat(HTML_String, "Pass NTRIP data from AOG NTRIP client to GPS receiver<br>");
    strcat(HTML_String,"Using WiFi UDP set AOG NTRIP port to 2233 <form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"AOGNTRIP\" id=\"JZ\" value=\"0\"");
    if (GPSSet.AOGNtrip == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">OFF</label></td>");
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_AOGNTRIP);
    strcat(HTML_String, "\">Apply and Save</button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"AOGNTRIP\" id=\"JZ\" value=\"1\"");
    if (GPSSet.AOGNtrip == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">ON (default)</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // Data transfer via USB/Wifi 
    strcat(HTML_String, "<h2>USB or WiFi data transfer</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"DataTransfVia\" id=\"JZ\" value=\"0\"");
    if (GPSSet.DataTransVia == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">USB</label></td>");
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_DataTransfVia);
    strcat(HTML_String, "\">Apply and Save</button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"DataTransfVia\" id=\"JZ\" value=\"1\"");
    if (GPSSet.DataTransVia == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">WiFi (UDP) (default)</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //-------------------------------------------------------------
    // Checkboxes debugmode
    strcat(HTML_String, "<h2>Debugmode</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr> <td colspan=\"2\">debugmode sends messages to USB serial</td>");
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_debugmode);
    strcat(HTML_String, "\">Apply and Save</button></td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"2\"><input type=\"checkbox\" name=\"debugmode\" id = \"Part\" value = \"1\" ");
    if (GPSSet.debugmode == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode on</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" name=\"debugmUBX\" id = \"Part\" value = \"1\" ");
    if (GPSSet.debugmodeUBX == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode UBX on (good to check, if data from UBlox comes to ESP)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" name=\"debugmHead\" id = \"Part\" value = \"1\" ");
    if (GPSSet.debugmodeHeading == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode Heading on (see Antenna distance real and from GPS ->indicator of GPS signal quality)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" name=\"debugmVirtAnt\" id = \"Part\" value = \"1\" ");
    if (GPSSet.debugmodeVirtAnt == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode Virtual Antenna on (see, how position is moved by ESP)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" name=\"debugmFiltPos\" id = \"Part\" value = \"1\" ");
    if (GPSSet.debugmodeFilterPos == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode Filter Position on (position is filtered on weak GPS signal)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" name=\"debugmRAW\" id = \"Part\" value = \"1\" ");
    if (GPSSet.debugmodeRAW == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode RAW data on (sends lots of data as comma separated values)</label>");
    strcat(HTML_String, "</td></tr>");
         
    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");


    //---------------------------------------------------------------------------------------------  
    // WiFi LED light on high/low 
    strcat(HTML_String, "<h2>WiFi LED light on</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"WiFiLEDon\" id=\"JZ\" value=\"0\"");
    if (GPSSet.LEDWiFi_ON_Level == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">LOW</label></td>");
    strcat(HTML_String, "<td><button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_WiFiLEDon);
    strcat(HTML_String, "\">Apply and Save</button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" name=\"WiFiLEDon\" id=\"JZ\" value=\"1\"");
    if (GPSSet.LEDWiFi_ON_Level == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">HIGH</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // GPIO PINs selection
    strcat(HTML_String, "<h2>GPIO to GPS UART </h2>");
    strcat(HTML_String, "<br>");
    strcat(HTML_String, "<b>RX1 at ESP32 = TX1 at GPS board</b><br>");

    strcat(HTML_String, "<b>select # of GPIO pin at ESP32</b><br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(100, 100, 100, 100, 200);
    strcat(HTML_String, "<tr><td>");
    strcat(HTML_String, "<button style= \"width:120px\" name=\"ACTION\" value=\"");
    strcati(HTML_String, ACTION_SET_GPIO);
    strcat(HTML_String, "\">Apply and Save</button></td></tr>");
    strcat(HTML_String, "<td><b>RX1</b></td>");
    strcat(HTML_String, "<td><b>TX1</b></td>");
    strcat(HTML_String, "<td><b>RX2</b> </td>");
    strcat(HTML_String, "<td><b>TX2</b></td>");
    strcat(HTML_String, "<td><b>pin for WiFi indication LED</b></td>");
    strcat(HTML_String, "</tr>");
    for (int i = 2; i < 34; i++) {
        //skip not usabel GPIOs
        if (i == 3) { i++; } //3: 6-11: not use! USB 12: ESP wouldn't boot
        if (i == 6) { i = 13; }

        strcat(HTML_String, "<tr>");

        strcat(HTML_String, "<td><input type = \"radio\" name=\"RX1\" id=\"GPIORX1\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (GPSSet.RX1 == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");
        strcat(HTML_String, "</td>");

        strcat(HTML_String, "<td><input type = \"radio\" name=\"TX1\" id=\"GPIOTX1\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (GPSSet.TX1 == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" name=\"RX2\" id=\"GPIORX2\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (GPSSet.RX2 == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" name=\"TX2\" id=\"GPIOTX2\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (GPSSet.TX2 == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" name=\"LED\" id=\"GPIOLED\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (GPSSet.LEDWiFi_PIN == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "</tr>");
    }
    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form><hr>");
}
//--------------------------------------------------------------------------
void send_not_found() {

	Serial.print("\nSend Not Found\n");

	client_page.print("HTTP/1.1 404 Not Found\r\n\r\n");
	delay(20);
	//client_page.stop();
}

//--------------------------------------------------------------------------
void send_HTML() {
	char my_char;
	int  my_len = strlen(HTML_String);
	int  my_ptr = 0;
	int  my_send = 0;

	//--------------------------------------------------------------------------
	// in Portionen senden
	while ((my_len - my_send) > 0) {
		my_send = my_ptr + MAX_PACKAGE_SIZE;
		if (my_send > my_len) {
			client_page.print(&HTML_String[my_ptr]);
			delay(10);

			//Serial.println(&HTML_String[my_ptr]);

			my_send = my_len;
		}
		else {
			my_char = HTML_String[my_send];
			// Auf Anfang eines Tags positionieren
			while (my_char != '<') my_char = HTML_String[--my_send];
			HTML_String[my_send] = 0;
			client_page.print(&HTML_String[my_ptr]);
			delay(10);

			//Serial.println(&HTML_String[my_ptr]);

			HTML_String[my_send] = my_char;
			my_ptr = my_send;
		}
	}
	//client_page.stop();
}

//----------------------------------------------------------------------------------------------
void set_colgroup(int w1, int w2, int w3, int w4, int w5) {
	strcat(HTML_String, "<colgroup>");
	set_colgroup1(w1);
	set_colgroup1(w2);
	set_colgroup1(w3);
	set_colgroup1(w4);
	set_colgroup1(w5);
	strcat(HTML_String, "</colgroup>");

}
//------------------------------------------------------------------------------------------
void set_colgroup1(int ww) {
	if (ww == 0) return;
	strcat(HTML_String, "<col width=\"");
	strcati(HTML_String, ww);
	strcat(HTML_String, "\">");
}


//---------------------------------------------------------------------
void strcatf(char* tx, float f, byte leng, byte dezim) {
	char tmp[8];

	dtostrf(f, leng, dezim, tmp);//f,6,2,tmp
	strcat(tx, tmp);
}
//---------------------------------------------------------------------
//void strcatl(char* tx, long l) {
  //char tmp[sizeof l];
  //memcpy(tmp, l, sizeof l);
  //strcat (tx, tmp);
//}

//---------------------------------------------------------------------
void strcati(char* tx, int i) {
	char tmp[8];

	itoa(i, tmp, 10);
	strcat(tx, tmp);
}

//---------------------------------------------------------------------
void strcati2(char* tx, int i) {
	char tmp[8];

	itoa(i, tmp, 10);
	if (strlen(tmp) < 2) strcat(tx, "0");
	strcat(tx, tmp);
}

//---------------------------------------------------------------------
int Pick_Parameter_Zahl(const char* par, char* str) {
	int myIdx = Find_End(par, str);

	if (myIdx >= 0) return  Pick_Dec(str, myIdx);
	else return -1;
}
//---------------------------------------------------------------------
int Find_End(const char* such, const char* str) {
	int tmp = Find_Start(such, str);
	if (tmp >= 0)tmp += strlen(such);
	return tmp;
}

//---------------------------------------------------------------------
int Find_Start(const char* such, const char* str) {
	int tmp = -1;
	int ww = strlen(str) - strlen(such);
	int ll = strlen(such);

	for (int i = 0; i <= ww && tmp == -1; i++) {
		if (strncmp(such, &str[i], ll) == 0) tmp = i;
	}
	return tmp;
}
//---------------------------------------------------------------------
int Pick_Dec(const char* tx, int idx) {
	int tmp = 0;

	for (int p = idx; p < idx + 5 && (tx[p] >= '0' && tx[p] <= '9'); p++) {
		tmp = 10 * tmp + tx[p] - '0';
	}
	return tmp;
}
//----------------------------------------------------------------------------
int Pick_N_Zahl(const char* tx, char separator, byte n) {//never used?

	int ll = strlen(tx);
	int tmp = -1;
	byte anz = 1;
	byte i = 0;
	while (i < ll && anz < n) {
		if (tx[i] == separator)anz++;
		i++;
	}
	if (i < ll) return Pick_Dec(tx, i);
	else return -1;
}

//---------------------------------------------------------------------
int Pick_Hex(const char* tx, int idx) {
	int tmp = 0;

	for (int p = idx; p < idx + 5 && ((tx[p] >= '0' && tx[p] <= '9') || (tx[p] >= 'A' && tx[p] <= 'F')); p++) {
		if (tx[p] <= '9')tmp = 16 * tmp + tx[p] - '0';
		else tmp = 16 * tmp + tx[p] - 55;
	}

	return tmp;
}

//---------------------------------------------------------------------
void Pick_Text(char* tx_ziel, char* tx_quelle, int max_ziel) {

	int p_ziel = 0;
	int p_quelle = 0;
	int len_quelle = strlen(tx_quelle);

	while (p_ziel < max_ziel && p_quelle < len_quelle && tx_quelle[p_quelle] && tx_quelle[p_quelle] != ' ' && tx_quelle[p_quelle] != '&') {
		if (tx_quelle[p_quelle] == '%') {
			tx_ziel[p_ziel] = (HexChar_to_NumChar(tx_quelle[p_quelle + 1]) << 4) + HexChar_to_NumChar(tx_quelle[p_quelle + 2]);
			p_quelle += 2;
		}
		else if (tx_quelle[p_quelle] == '+') {
			tx_ziel[p_ziel] = ' ';
		}
		else {
			tx_ziel[p_ziel] = tx_quelle[p_quelle];
		}
		p_ziel++;
		p_quelle++;
	}

	tx_ziel[p_ziel] = 0;
}
//---------------------------------------------------------------------
char HexChar_to_NumChar(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 55;
	return 0;
}
//---------------------------------------------------------------------
void exhibit(const char* tx, int v) {
	Serial.print(tx);
	Serial.print(v, 1);
}
//---------------------------------------------------------------------
void exhibit(const char* tx, unsigned int v) {
	Serial.print(tx);
	Serial.print((int)v, 1);
}
//---------------------------------------------------------------------
void exhibit(const char* tx, unsigned long v) {
	Serial.print(tx);
	Serial.print((long)v, 1);
}
//---------------------------------------------------------------------
void exhibit(const char* tx, const char* v) {
	Serial.print(tx);
	Serial.print(v);
}
#endif