#if HardwarePlatform == 0
// Wifi variables & definitions
char HTML_String[50000];
int action;
long temLong = 0;
double temDoub = 0;

#define ACTION_LoadDefaultVal   1
#define ACTION_RESTART          2


//-------------------------------------------------------------------------------------------------
//10. Mai 2020

void StartServer() {

    /*return index page which is stored in serverIndex */
    server.on("/", HTTP_GET, []() {
        make_HTML01();
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", HTML_String);
        if (GPSSet.debugmode) { Serial.println("Webpage root"); }
        process_Request();
        make_HTML01();
        WebIOTimeOut = millis() + (long(GPSSet.timeoutWebIO) * 60000);

        Serial.print("millis: "); Serial.print(millis());
        Serial.print(" timeout WebIO: "); Serial.println(WebIOTimeOut);

        server.sendHeader("Connection", "close");
        server.send(200, "text/html", HTML_String);
        });
    server.on("/root", HTTP_GET, []() { //needed for 404 not found redirect
        make_HTML01();
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", HTML_String);
        if (GPSSet.debugmode) { Serial.println("Webpage root"); }
        process_Request();
        make_HTML01();
        WebIOTimeOut = millis() + (long(GPSSet.timeoutWebIO) * 60000);

        Serial.print("millis: "); Serial.print(millis());
        Serial.print(" timeout WebIO: "); Serial.println(WebIOTimeOut);

        server.sendHeader("Connection", "close");
        server.send(200, "text/html", HTML_String);
        });
    server.on("/serverIndex", HTTP_GET, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", serverIndex);
        WebIOTimeOut = millis() + 1200000;//20 min
        });
    /*handling uploading firmware file */
    server.on("/update", HTTP_POST, []() {
        WebIOTimeOut = millis() + 1200000;//20 min
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
        }, []() {
            HTTPUpload& upload = server.upload();
            if (upload.status == UPLOAD_FILE_START) {
                Serial.printf("Update: %s\n", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE) {
                /* flashing firmware to ESP*/
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_END) {
                if (Update.end(true)) { //true to set the size to the current progress
                    Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                }
                else {
                    Update.printError(Serial);
                }
            }
        });

    server.onNotFound(handleNotFound);

    server.begin();
}

//---------------------------------------------------------------------
// Process given values 10. Mai 2020
//---------------------------------------------------------------------
void process_Request()
{
    action = 0;
    if (GPSSet.debugmode) { Serial.print("From webinterface: number of arguments: "); Serial.println(server.args()); }
    for (byte n = 0; n < server.args(); n++) {
        if (GPSSet.debugmode) {
            Serial.print("argName "); Serial.print(server.argName(n));
            Serial.print(" val: "); Serial.println(server.arg(n));
        }
        if (server.argName(n) == "ACTION") {
            action = int(server.arg(n).toInt());
            if (GPSSet.debugmode) { Serial.print("Action found: "); Serial.println(action); }
        }
        if (action != ACTION_RESTART) { EEprom_unblock_restart(); }
        if (action == ACTION_LoadDefaultVal) {
            if (GPSSet.debugmode) { Serial.println("load default settings from EEPROM"); }
            EEprom_read_default();
            delay(2);
        }
        //save changes
        if (server.argName(n) == "Save") {
            if (GPSSet.debugmode) { Serial.println("Save button pressed in webinterface"); }
            EEprom_write_all();
        }

        if (server.argName(n) == "SSID_MY") {
            for (int i = 0; i < 24; i++) GPSSet.ssid[i] = 0x00;
            int tempInt = server.arg(n).length() + 1;
            server.arg(n).toCharArray(GPSSet.ssid, tempInt);
        }
        if (server.argName(n) == "Password_MY") {
            for (int i = 0; i < 24; i++) GPSSet.password[i] = 0x00;
            int tempInt = server.arg(n).length() + 1;
            server.arg(n).toCharArray(GPSSet.password, tempInt);
        }
        if (server.argName(n) == "timeoutRout") {
            temLong = server.arg(n).toInt();
            if ((temLong >= 20) && (temLong <= 1000)) { GPSSet.timeoutRouter = int(temLong); }
        }
        if (server.argName(n) == "timeoutWebIO") {
            temLong = server.arg(n).toInt();
            if ((temLong >= 2) && (temLong <= 255)) { GPSSet.timeoutWebIO = byte(temLong); }
        }
        
        if (server.argName(n) == "GPSPosCorrByRoll") {
            if (server.arg(n) == "true") { GPSSet.GPSPosCorrByRoll = 1; }
            else { GPSSet.GPSPosCorrByRoll = 0; }
        }

        if (server.argName(n) == "AntDist") {
            temDoub = server.arg(n).toDouble();
            if ((temDoub <= 1000) && (temDoub >= 0)) { GPSSet.AntDist = temDoub; }
        }
        if (server.argName(n) == "AntHight") {
            temDoub = server.arg(n).toDouble();
            if ((temDoub <= 600) && (temDoub >= 0)) { GPSSet.AntHight = temDoub; }
        }
        if (server.argName(n) == "AntRight") {
            temDoub = server.arg(n).toDouble();
            if ((temDoub <= 1000) && (temDoub >= -1000)) { GPSSet.virtAntRight = temDoub; }
        }
        if (server.argName(n) == "AntForew") {
            temDoub = server.arg(n).toDouble();
            if ((temDoub <= 1000) && (temDoub >= -1000)) { GPSSet.virtAntForew = temDoub; }
        }
        if (server.argName(n) == "AntDistDevFact") {
            temDoub = server.arg(n).toDouble();
            if ((temDoub <= 1.99) && (temDoub >= 1.0)) { GPSSet.AntDistDeviationFactor = temDoub; }
        }

        if (server.argName(n) == "UBXFlagCheck") {
            if (server.arg(n) == "true") { GPSSet.checkUBXFlags = 1; }
            else { GPSSet.checkUBXFlags = 0; }
        }
        if (server.argName(n) == "GPSPosFilter") {
            if (server.arg(n) == "true") { GPSSet.filterGPSposOnWeakSignal = 1; }
            else { GPSSet.filterGPSposOnWeakSignal = 0; }
        }

        if (server.argName(n) == "HeadAngleCorr") {
            temDoub = server.arg(n).toDouble();
            if ((temDoub < 360) && (temDoub >= 0)) { GPSSet.headingAngleCorrection = temDoub; }
        }
        if (server.argName(n) == "maxHeadChang") {
            temLong = server.arg(n).toInt();
            if ((temLong < 100) && (temLong >= 2)) { GPSSet.MaxHeadChangPerSec = byte(temLong); }
        }
        if (server.argName(n) == "RollAngleCorr") {
            temDoub = server.arg(n).toDouble();
            if ((temDoub < 360) && (temDoub >= 0)) { GPSSet.rollAngleCorrection = temDoub; }
        }

        if (server.argName(n) == "AOGNTRIP") {
            if (server.arg(n) == "true") { GPSSet.AOGNtrip = 1; }
            else { GPSSet.AOGNtrip = 0; }
        }

        if (server.argName(n) == "seOGI") {
            if (server.arg(n) == "true") { GPSSet.sendOGI = 1; }
            else {
                if (GPSSet.sendGGA == 0) { GPSSet.sendOGI = 1; }
                else {
                    GPSSet.sendOGI = 0;//only switch off, if GGA is send
                }
            }
        }
        if (server.argName(n) == "seGGA") {
            if (server.arg(n) == "true") { GPSSet.sendGGA = 1;; }
            else { GPSSet.sendGGA = 0; }
        }
        if (server.argName(n) == "seVTG") {
            if (server.arg(n) == "true") { GPSSet.sendVTG = 1; }
            else { GPSSet.sendVTG = 0; }
        }
        if (server.argName(n) == "seHDT") {
            if (server.arg(n) == "true") { GPSSet.sendHDT = 1; }
            else { GPSSet.sendHDT = 0; }
        }
        if (server.argName(n) == "DataTransfVia") {
            temLong = server.arg(n).toInt();
            if ((temLong <= 2) && (temLong >= 0)) { GPSSet.DataTransVia = byte(temLong); }
        }

        if (server.argName(n) == "debugmode") {
            if (server.arg(n) == "true") { GPSSet.debugmode = true; }
            else { GPSSet.debugmode = false; }
        }
        if (server.argName(n) == "debugmUBX") {
            if (server.arg(n) == "true") { GPSSet.debugmodeUBX = true; }
            else { GPSSet.debugmodeUBX = false; }
        }
        if (server.argName(n) == "debugmHead") {
            if (server.arg(n) == "true") { GPSSet.debugmodeHeading = true; }
            else { GPSSet.debugmodeHeading = false; }
        }
        if (server.argName(n) == "debugmVirtAnt") {
            if (server.arg(n) == "true") { GPSSet.debugmodeVirtAnt = true; }
            else { GPSSet.debugmodeVirtAnt = false; }
        }
        if (server.argName(n) == "debugmFiltPos") {
            if (server.arg(n) == "true") { GPSSet.debugmodeFilterPos = true; }
            else { GPSSet.debugmodeFilterPos = false; }
        }
        if (server.argName(n) == "debugmRAW") {
            if (server.arg(n) == "true") { GPSSet.debugmodeRAW = true; }
            else { GPSSet.debugmodeRAW = false; }
        }

        if (server.argName(n) == "WiFiLEDon") {
            temLong = server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                GPSSet.LEDWiFi_ON_Level = byte(temLong);
                if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); }
                else { digitalWrite(GPSSet.LEDWiFi_PIN, !GPSSet.LEDWiFi_ON_Level); }
            }
        }

        if (server.argName(n) == "LED") {
            temLong = server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                GPSSet.LEDWiFi_PIN = byte(temLong);
                pinMode(GPSSet.LEDWiFi_PIN, OUTPUT);
                if (LED_WIFI_ON) { digitalWrite(GPSSet.LEDWiFi_PIN, GPSSet.LEDWiFi_ON_Level); }
                else { digitalWrite(GPSSet.LEDWiFi_PIN, !GPSSet.LEDWiFi_ON_Level); }
            }
        }
        if (server.argName(n) == "TX1") {
            temLong = server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                GPSSet.TX1 = byte(temLong);
                Serial1.end();
                delay(20);
                Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1);
                delay(5);
            }
        }
        if (server.argName(n) == "RX1") {
            temLong = server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                GPSSet.RX1 = byte(temLong);
                Serial1.end();
                delay(20);
                Serial1.begin(115200, SERIAL_8N1, GPSSet.RX1, GPSSet.TX1);
                delay(5);
            }
        }
        if (server.argName(n) == "TX2") {
            temLong = server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                GPSSet.TX2 = byte(temLong);
                Serial2.end();
                delay(20);
                Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2);
                delay(5);
            }
        }
        if (server.argName(n) == "RX2") {
            temLong = server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                GPSSet.RX2 = byte(temLong);
                Serial2.end();
                delay(20);
                Serial2.begin(115200, SERIAL_8N1, GPSSet.RX2, GPSSet.TX2);
                delay(5);
            }
        }

        if (action == ACTION_RESTART) {
            Serial.println("reboot ESP32: selected by webinterface");
            EEprom_block_restart();//prevents from restarting, when webpage is reloaded. Is set to 0, when other ACTION than restart is called
            delay(2000);
            ESP.restart();
        }
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
    strcat(HTML_String, "<style>divbox {background-color: lightgrey;width: 200px;border: 5px solid red;padding:10px;margin: 10px;}</style>");
    strcat(HTML_String, "</head>");
    strcat(HTML_String, "<body bgcolor=\"#ffcc00\">");//ff9900
    strcat(HTML_String, "<font color=\"#000000\" face=\"VERDANA,ARIAL,HELVETICA\">");
    strcat(HTML_String, "<h1>GPS roof unit ESP32 for single or dual antenna</h1>");
    strcat(HTML_String, "for 1 or 2 UBlox receivers connected (1: UXB PVT 2: UBX RelPosNED)<br>");
    strcat(HTML_String,"supports data via USB/WiFi UDP, dual antenna, heading/roll filter and postion correction<br><br>");
    strcat(HTML_String, "<b>Transfers NTRIP data from AOG port ");
    strcati(HTML_String, GPSSet.AOGNtripPort);
    strcat(HTML_String," to UBlox receiver</b><br><br>");
    strcat(HTML_String, "more settings like IPs, UPD ports... in setup zone of INO code<br>");
    strcat(HTML_String, "(Rev. 4.3 - 5. July 2020 by MTZ8302 Webinterface by WEDER)<br><hr>");


    //---------------------------------------------------------------------------------------------  
    //load values of INO setup zone
    strcat(HTML_String, "<h2>Load default values of INO setup zone</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(270, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"2\">Only load default values, does NOT save them</td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?ACTION=");
    strcati(HTML_String, ACTION_LoadDefaultVal);
    strcat(HTML_String, "')\" style= \"width:150px\" value=\"Load default values\"></button></td>");
    strcat(HTML_String, "</tr>");
    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //-----------------------------------------------------------------------------------------
    // WiFi Client Access Data

    strcat(HTML_String, "<h2>WiFi Network Client Access Data</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "</b>If access to networks fails, an accesspoint will be created:<br>SSID: <b>");
    strcat(HTML_String, GPSSet.ssid_ap);
    strcat(HTML_String, "</b>     with no password<br><br><table>");
    set_colgroup(250, 300, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td><b>Network SSID:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?SSID_MY='+this.value)\" style= \"width:200px\" name=\"SSID_MY\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, GPSSet.ssid);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td><b>Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_MY='+this.value)\" style= \"width:200px\" name=\"Password_MY\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, GPSSet.password);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\">time, trying to connect to network</td></tr>");
    strcat(HTML_String, "<td colspan=\"3\">after time has passed access point is opened</td></tr>");
    strcat(HTML_String, "<tr><td><b>Timeout (s):</b></td><td><input type = \"number\" onchange=\"sendVal('/?timeoutRout='+this.value)\" name = \"timeoutRout\" min = \"20\" max = \"1000\" step = \"1\" style= \"width:200px\" value = \"");// placeholder = \"");
    strcati(HTML_String, GPSSet.timeoutRouter);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
    strcat(HTML_String, "<tr><td colspan=\"2\"><b>Restart NTRIP client for changes to take effect</b></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?ACTION=");
    strcati(HTML_String, ACTION_RESTART);
    strcat(HTML_String, "')\" style= \"width:120px\" value=\"Restart\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //-----------------------------------------------------------------------------------------
    // timeout webinterface

    strcat(HTML_String, "<h2>Webinterface timeout</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<b>Webinterface needs lots of calculation time, so if switched off, GPS runs better.</b><br>");
    strcat(HTML_String, "After this time (minutes) from restart, or last usage, webinterface is turned off.<br><br>");
    strcat(HTML_String, "Set to 255 to keep active.<br><br><table>");
    set_colgroup(250, 300, 150, 0, 0);

    strcat(HTML_String, "<tr><td><b>Webinterface timeout (min)</b></td><td><input type = \"number\"  onchange=\"sendVal('/?timeoutWebIO='+this.value)\" name = \"timeoutWebIO\" min = \"2\" max = \"255\" step = \"1\" style= \"width:200px\" value = \"");// placeholder = \"");
    strcati(HTML_String, GPSSet.timeoutWebIO);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
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
    strcat(HTML_String, "<td><input type = \"number\" onchange=\"sendVal('/?AntDist='+this.value)\" name = \"AntDist\" min = \"0\" max = \"1000\" step = \"1\" style= \"width:100px\" value = \"");// placeholder = \"");
    strcati(HTML_String, int(GPSSet.AntDist));
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td>Antenna hight (cm)</td>");
    strcat(HTML_String, "<td><input type = \"number\" onchange=\"sendVal('/?AntHight='+this.value)\" name = \"AntHight\" min = \"0\" max = \"600\" step = \"1\" style= \"width:100px\" value = \"");// placeholder = \"");
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
    strcat(HTML_String, "<td><input type = \"number\" onchange=\"sendVal('/?AntRight='+this.value)\" name = \"AntRight\" min = \"-1000\" max = \"1000\" step = \"1\" style= \"width:100px\" value = \"");// placeholder = \"");
    strcati(HTML_String, int(GPSSet.virtAntRight));
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td>move Antenna foreward (cm) (backw.: neg.)</td>");
    strcat(HTML_String, "<td><input type = \"number\" onchange=\"sendVal('/?AntForew='+this.value)\" name = \"AntForew\" min = \"-1000\" max = \"1000\" step = \"1\" style= \"width:100px\" value = \"");// placeholder = \"");
    strcati(HTML_String, int(GPSSet.virtAntForew));
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // heading angle correction 
    strcat(HTML_String, "<h2>Dual heading angle correction/max heading change</h2>");
    
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr><td colspan=\"3\">Set to 90 if antenna for position is right and other left.</td></tr>");
    strcat(HTML_String, "<tr><td>Heading angle correction for dual GPS</td><td><input type = \"number\" onchange=\"sendVal('/?HeadAngleCorr='+this.value)\" name = \"HeadAngleCorr\" min = \" 0\" max = \"360\" step = \"0.1\" style= \"width:100px\" value = \"");// placeholder = \"");
    if (GPSSet.headingAngleCorrection < 10) { strcatf(HTML_String, GPSSet.headingAngleCorrection, 3, 1); }
    else {
        if (GPSSet.headingAngleCorrection < 100) { strcatf(HTML_String, GPSSet.headingAngleCorrection, 4, 1); }
        else { strcatf(HTML_String, GPSSet.headingAngleCorrection, 5, 1); }
    }
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td colspan=\"3\"><b>Heading values from dual GPS and position antenna (VTG heading).</td></tr>");
    strcat(HTML_String, "<td colspan=\"3\">Drive straight, when reloading webpage to get good VTG values.</b></td></tr>");
    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
    strcat(HTML_String, "<tr><td>Heading from dual GPS</td><td><divbox align=\"right\"><font size=\"+1\"><b>");
    if (HeadingRelPosNED < 10) { strcatf(HTML_String, HeadingRelPosNED, 3, 1); }
    else {
        if (HeadingRelPosNED < 100) { strcatf(HTML_String, HeadingRelPosNED, 4, 1); }
        else { strcatf(HTML_String, HeadingRelPosNED, 5, 1); }
    }
    strcat(HTML_String, "</b></font></divbox></td></tr>");
    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
    strcat(HTML_String, "<tr><td>Heading from VTG</td><td><divbox align=\"right\"><font size=\"+1\"><b>");
    if (HeadingVTG < 10) { strcatf(HTML_String, HeadingVTG, 3, 1); }
    else {
        if (HeadingVTG < 100) { strcatf(HTML_String, HeadingVTG, 4, 1); }
        else { strcatf(HTML_String, HeadingVTG, 5, 1); }
    }
    strcat(HTML_String, "</b></font></divbox></td>");
    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
    strcat(HTML_String, "<tr><td>Heading Mix</td><td><divbox align=\"right\"><font size=\"+1\"><b>");
    if (HeadingMix < 10) { strcatf(HTML_String, HeadingMix, 3, 1); }
    else {
        if (HeadingMix < 100) { strcatf(HTML_String, HeadingMix, 4, 1); }
        else { strcatf(HTML_String, HeadingMix, 5, 1); }
    }
    strcat(HTML_String, "</b></font></divbox></td>");
    //Refresh button
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"location.reload()\" style= \"width:120px\" value=\"Refresh\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td colspan=\"3\"><b>Max heading change per second. Limits dual GPS and VTG heading change.</b></td></tr>");
    strcat(HTML_String,"<tr><td colspan=\"3\">Limits heading change to avoid jumps. Max change (deg/s) at 5 km/h.</td> </tr>");
    strcat(HTML_String, "<tr><td colspan=\"3\">Angle is adjusted to speed: faster -> less change.</td> </tr>");
    strcat(HTML_String, "<tr><td>recommended 30-50 deg/s</td><td><input type = \"number\" onchange=\"sendVal('/?maxHeadChang='+this.value)\" name = \"maxHeadChang\" min = \" 2\" max = \"99\" step = \"1\" style= \"width:100px\" value = \"");// placeholder = \"");
    if (GPSSet.MaxHeadChangPerSec < 10) { strcati(HTML_String, GPSSet.MaxHeadChangPerSec); }
    else {
        if (GPSSet.MaxHeadChangPerSec < 100) { strcati(HTML_String, GPSSet.MaxHeadChangPerSec); }
    }
    strcat(HTML_String, "\"></td></tr>");


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
    strcat(HTML_String, "<td></td><td><input type = \"number\" onchange=\"sendVal('/?RollAngleCorr='+this.value)\" name = \"RollAngleCorr\" min = \" - 45\" max = \"45\" step = \"0.1\" style= \"width:100px\" value = \"");// placeholder = \"");

    strcatf(HTML_String, GPSSet.rollAngleCorrection, 3, 1);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // GPS position correction by roll 
    strcat(HTML_String, "<h2>Correct GPS position using roll</h2>");
    strcat(HTML_String, "Roll corrected position is send, it's like moving antenna over the ground.<br>");
    strcat(HTML_String, "The left and right movement caused by rocking tractor is eliminated.<br>");
    strcat(HTML_String, "Roll transfered to AOG is more filtered for sidehill draft gain.<br>");
    strcat(HTML_String, "Antennas must be left + right to be able to calculate roll.<br><br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    //checkbox
    strcat(HTML_String, "<tr><td>(default: ON)</td><td><input type=\"checkbox\" onclick=\"sendVal('/?GPSPosCorrByRoll='+this.checked)\" name=\"GPSPosCorrByRoll\" id = \"Part\" value = \"1\" ");
    if (GPSSet.GPSPosCorrByRoll == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> <b>send corrected position</b></label>");
    strcat(HTML_String, "</td>");    
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

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
    strcat(HTML_String, "<td></td><td><input type = \"number\" onchange=\"sendVal('/?AntDistDevFact='+this.value)\" name = \"AntDistDevFact\" min = \" 1.1\" max = \"1.99\" step = \"0.01\" style= \"width:100px\" value = \"");// placeholder = \"");
    strcatf(HTML_String, GPSSet.AntDistDeviationFactor, 4, 2);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");
    strcat(HTML_String, "</table><br><br>");

    strcat(HTML_String, "<b>Check UBX flags for signal quality</b><br>");
    strcat(HTML_String, "With some bases for RTK, the flags sometimes fall back, so position will jump.<br>");
    strcat(HTML_String, "In this case turn check of UBX falgs OFF<br>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);
   // strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
    //checkbox
    strcat(HTML_String, "<tr><td>(default: ON)</td><td><input type=\"checkbox\" onclick=\"sendVal('/?UBXFlagCheck='+this.checked)\" name=\"UBXFlagCheck\" id = \"Part\" value = \"1\" ");
    if (GPSSet.checkUBXFlags == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> <b>check UBX flags</b></label>");
    strcat(HTML_String, "</td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");
    strcat(HTML_String, "</table><br><br>");

    strcat(HTML_String, "<b>Filter GPS position on weak signal</b><br>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);
    strcat(HTML_String, "<tr>");
    //checkbox
    strcat(HTML_String, "<tr><td>(default: ON)</td><td><input type=\"checkbox\" onclick=\"sendVal('/?GPSPosFilter='+this.checked)\" name=\"GPSPosFilter\" id = \"Part\" value = \"1\" ");
    if (GPSSet.filterGPSposOnWeakSignal == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> <b>Filter GPS postition</b></label>");
    strcat(HTML_String, "</td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

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
	strcat(HTML_String, "<td></td><td><input type=\"checkbox\" onclick=\"sendVal('/?seOGI='+this.checked)\" name=\"seOGI\" id = \"Part\" value = \"1\" ");
	if (GPSSet.sendOGI == 1) strcat(HTML_String, "checked ");
	strcat(HTML_String, "> ");
	strcat(HTML_String, "<label for =\"Part\"> send PAOGI</label>");
	strcat(HTML_String, "</td>");    
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type=\"checkbox\" onclick=\"sendVal('/?seGGA='+this.checked)\" name=\"seGGA\" id = \"Part\" value = \"1\" ");
    if (GPSSet.sendGGA == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> send GPGGA</label>");
    strcat(HTML_String, "</td></tr>");
    strcat(HTML_String, "<tr>");

    strcat(HTML_String, "<td></td><td><input type=\"checkbox\" onclick=\"sendVal('/?seVTG='+this.checked)\" name=\"seVTG\" id = \"Part\" value = \"1\" ");
    if (GPSSet.sendVTG == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> send GPVTG</label>");
    strcat(HTML_String, "</td></tr>");
    strcat(HTML_String, "<tr>");

    strcat(HTML_String, "<td></td><td><input type=\"checkbox\" onclick=\"sendVal('/?seHDT='+this.checked)\" name=\"seHDT\" id = \"Part\" value = \"1\" ");
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
    strcat(HTML_String,"Using WiFi UDP set AOG NTRIP port to ");
    strcati(HTML_String, GPSSet.AOGNtripPort);
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    //checkbox
    strcat(HTML_String, "<tr><td>(default: ON)</td><td><input type = \"checkbox\" onclick=\"sendVal('/?AOGNTRIP='+this.checked)\" name=\"AOGNTRIP\" id = \"Part\" value = \"1\" ");
    if (GPSSet.AOGNtrip == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> <b>Use AOG NTRIP</b></label>");
    strcat(HTML_String, "</td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

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
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?DataTransfVia=0')\" name=\"DataTransfVia\" id=\"JZ\" value=\"0\"");
    if (GPSSet.DataTransVia == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">USB</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?DataTransfVia=1')\" name=\"DataTransfVia\" id=\"JZ\" value=\"1\"");
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
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"2\"><input type=\"checkbox\" onclick=\"sendVal('/?debugmode='+this.checked)\" name=\"debugmode\" id = \"Part\" value = \"1\" ");
    if (GPSSet.debugmode == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode on</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" onclick=\"sendVal('/?debugmUBX='+this.checked)\" name=\"debugmUBX\" id = \"Part\" value = \"1\" ");
    if (GPSSet.debugmodeUBX == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode UBX on (good to check, if data from UBlox comes to ESP)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" onclick=\"sendVal('/?debugmHead='+this.checked)\" name=\"debugmHead\" id = \"Part\" value = \"1\" ");
    if (GPSSet.debugmodeHeading == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode Heading on (see Antenna distance real and from GPS ->indicator of GPS signal quality)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" onclick=\"sendVal('/?debugmVirtAnt='+this.checked)\" name=\"debugmVirtAnt\" id = \"Part\" value = \"1\" ");
    if (GPSSet.debugmodeVirtAnt == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode Virtual Antenna on (see, how position is moved by ESP)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" onclick=\"sendVal('/?debugmFiltPos='+this.checked)\" name=\"debugmFiltPos\" id = \"Part\" value = \"1\" ");
    if (GPSSet.debugmodeFilterPos == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode Filter Position on (position is filtered on weak GPS signal)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" onclick=\"sendVal('/?debugmRAW='+this.checked)\" name=\"debugmRAW\" id = \"Part\" value = \"1\" ");
    if (GPSSet.debugmodeRAW == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode RAW data on (sends lots of data as comma separated values)</label>");
    strcat(HTML_String, "</td></tr>");
         
    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //-------------------------------------------------------------
    // firmware update
    strcat(HTML_String, "<h2>Firmware Update for ESP32</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr> <td colspan=\"3\">build a new firmware with Arduino IDE selecting</td> </tr>");
    strcat(HTML_String, "<tr> <td colspan=\"3\">Sketch -> Export compiled Binary</td> </tr>");
    strcat(HTML_String, "<tr> <td colspan=\"3\">upload this file via WiFi/Ethernet connection</td> </tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
    strcat(HTML_String, "<tr><td></td>");
    //button
    strcat(HTML_String, "<td><input type='submit' onclick='openUpload(this.form)' value='Open Firmware uploader'></td></tr>");

    strcat(HTML_String, "<script>");
    strcat(HTML_String, "function openUpload(form)");
    strcat(HTML_String, "{");
    strcat(HTML_String, "window.open('/serverIndex')");
    strcat(HTML_String, "}");
    strcat(HTML_String, "</script>");

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
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?WiFiLEDon=0')\" name=\"WiFiLEDon\" id=\"JZ\" value=\"0\"");
    if (GPSSet.LEDWiFi_ON_Level == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">LOW</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?WiFiLEDon=1')\" name=\"WiFiLEDon\" id=\"JZ\" value=\"1\"");
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

    strcat(HTML_String, "<b>select # of GPIO pin at ESP32</b><br><br>");
    strcat(HTML_String, "<form>");    
    strcat(HTML_String, "<input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button><br>");
    strcat(HTML_String, "<table>");
    set_colgroup(100, 100, 100, 100, 200);
    strcat(HTML_String, "<tr>");
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

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?RX1=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"RX1\" id=\"GPIORX1\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (GPSSet.RX1 == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");
        strcat(HTML_String, "</td>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?TX1=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"TX1\" id=\"GPIOTX1\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (GPSSet.TX1 == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?RX2=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"RX2\" id=\"GPIORX2\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (GPSSet.RX2 == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?TX2=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"TX2\" id=\"GPIOTX2\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (GPSSet.TX2 == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?LED=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"LED\" id=\"GPIOLED\" value=\"");
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


    //script to send values from webpage to ESP for process request
    strcat(HTML_String, "<script>");
    strcat(HTML_String, "function sendVal(ArgStr)");
    strcat(HTML_String, "{");
    strcat(HTML_String, "  var xhttp = new XMLHttpRequest();");
    strcat(HTML_String, "  xhttp.open(\"GET\",ArgStr, true);");
    strcat(HTML_String, "  xhttp.send();");
    strcat(HTML_String, " if (ArgStr == '/?ACTION=");
    strcati(HTML_String, ACTION_LoadDefaultVal);
    strcat(HTML_String, "') { window.setTimeout('location.reload()',400); }");
    strcat(HTML_String, "}");
    strcat(HTML_String, "</script>");

}


//-------------------------------------------------------------------------------------------------

void handleNotFound() {
    const char* notFound =
        "<!doctype html>"
        "<html lang = \"en\">"
        "<head>"""
        "<meta charset = \"utf - 8\">"
        "<meta http - equiv = \"x - ua - compatible\" content = \"ie = edge\">"
        "<meta name = \"viewport\" content = \"width = device - width, initial - scale = 1.0\">"
        "<title>Redirecting</title>"
        "</head>"
        "<body onload = \"redirect()\">"
        "<h1 style = \"text - align: center; padding - top: 50px; display: block; \"><br>404 not found<br><br>Redirecting to settings page in 3 secs ...</h1>"
        "<script>"
        "function redirect() {"
        "setTimeout(function() {"
        "    window.location.replace(\"/root\");"//new landing page
        "}"
        ", 3000);"
        "}"
        "</script>"
        "</body>"
        "</html>";

	server.sendHeader("Connection", "close");
	server.send(200, "text/html", notFound);
	if (GPSSet.debugmode) { Serial.println("redirecting from 404 not found to Webpage root"); }

/*
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
 */
}

//-------------------------------------------------------------------------------------------------

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
void strcati(char* tx, int i) {
    char tmp[8];

    itoa(i, tmp, 10);
    strcat(tx, tmp);
}

#endif