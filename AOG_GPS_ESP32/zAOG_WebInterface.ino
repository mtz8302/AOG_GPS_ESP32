// Wifi variables & definitions
char HTML_String[25000];//55000 25.3.: 19603 bytes
int action;
long temLong = 0;
double temDoub = 0;
int temInt = 0;

#define ACTION_LoadDefaultVal   1
#define ACTION_RESTART          2
//-------------------------------------------------------------------------------------------------
/*
//-------------------------------------------------------------------------------------------------
//10. Mai 2020

void EthStartServer() {

    Eth_Server.on("/", HTTP_GET, []() {EthhandleRoot(); });

    //file selection for firmware update
    Eth_Server.on("/serverIndex", HTTP_GET, []() {
        Eth_Server.sendHeader("Connection", "close");
        Eth_Server.send(200, "text/html", serverIndex);
        });

    //handling uploading firmware file //
    Eth_Server.on("/update", HTTP_POST, []() {
        Eth_Server.sendHeader("Connection", "close");
        Eth_Server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
        }, []() {
            HTTPUpload& upload = Eth_Server.upload();
            if (upload.status == UPLOAD_FILE_START) {
                Serial.printf("Update: %s\n", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE) {
                // flashing firmware to ESP//
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

    Eth_Server.onNotFound(EthhandleNotFound);

    Eth_Server.begin();
}

//-------------------------------------------------------------------------------------------------
//7. Maerz 2021

void doEthWebinterface(void* pvParameters) {
    Serial.println("starting Ethernet Webinterface");
    for (;;) {
        Eth_Server.handleClient(); //does the Webinterface

        vTaskDelay(30);
        //}
        if ((now > WebIOTimeOut) && (Set.timeoutWebIO != 255)) {
            WebIORunning = false;
            Eth_Server.close();
            Serial.println("closing Ethernet Webinterface task");
            delay(1);
            vTaskDelete(NULL);
            delay(1);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//7. Maerz 2021

void EthhandleRoot() {
    make_HTML01();
    Eth_Server.sendHeader("Connection", "close");
    Eth_Server.send(200, "text/html", HTML_String);
    WebIOTimeOut = millis() + long((Set.timeoutWebIO * 60000));
    Serial.println("handling Eth root");
    if (Set.debugmode) {
        Serial.println("Webpage root"); Serial.print("Timeout WebIO: "); Serial.println(WebIOTimeOut);
    }
    //process_Request();
}



void EthhandleNotFound() {
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
        ", 5000);"
        "}"
        "</script>"
        "</body>"
        "</html>";

    Eth_Server.sendHeader("Connection", "close");
    Eth_Server.send(200, "text/html", notFound);
    if (Set.debugmode) { Serial.println("redirecting from 404 not found to Webpage root"); }

}
*/

//-------------------------------------------------------------------------------------------------
//7. Maerz 2021

void doWebinterface(void* pvParameters) {
    for (;;) {
        WiFi_Server.handleClient(); //does the Webinterface
        if (WebIOLastUsePlus3 < millis()) {//not called in the last 3 sec
            //Serial.println("Webinterface no client for 3 sec");
            vTaskDelay(1000);
        }
        else {
            vTaskDelay(20);
        }
        if ((now > WebIOTimeOut) && (Set.timeoutWebIO != 255)) {
            WebIORunning = false;
            WiFi_Server.close();
            Serial.println("closing Webinterface task");
            delay(1);
            vTaskDelete(NULL);
            delay(1);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//7. Maerz 2021

void handleRoot() {
    make_HTML01();
    WiFi_Server.sendHeader("Connection", "close");
    WiFi_Server.send(200, "text/html", HTML_String);
    WebIOLastUsePlus3 = 3000 + millis();
    WebIOTimeOut = WebIOLastUsePlus3 + long((Set.timeoutWebIO * 60000));
    if (Set.debugmode) {
        Serial.print("used size of HTML string: "); Serial.println(strlen(HTML_String));
        Serial.println("Webpage root");
        Serial.print("Timeout WebIO: "); Serial.println(WebIOTimeOut);
    }
    process_Request();
}

//-------------------------------------------------------------------------------------------------
//10. Mai 2020

void WiFiStartServer() {

    WiFi_Server.on("/", HTTP_GET, []() {handleRoot(); });

    //file selection for firmware update
    WiFi_Server.on("/serverIndex", HTTP_GET, []() {
        WiFi_Server.sendHeader("Connection", "close");
        WiFi_Server.send(200, "text/html", serverIndex);
        });

            //handling uploading firmware file //
    WiFi_Server.on("/update", HTTP_POST, []() {
        WiFi_Server.sendHeader("Connection", "close");
        WiFi_Server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
        }, []() {
            HTTPUpload& upload = WiFi_Server.upload();
            if (upload.status == UPLOAD_FILE_START) {
                Serial.printf("Update: %s\n", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE) {
                // flashing firmware to ESP//
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

    WiFi_Server.onNotFound(handleNotFound);

    WiFi_Server.begin();
}

//---------------------------------------------------------------------
// Process given values 10. Mai 2020
//---------------------------------------------------------------------
void process_Request()
{
    action = 0;
    if (Set.debugmode) { Serial.print("From webinterface: number of arguments: "); Serial.println(WiFi_Server.args()); }
    for (byte n = 0; n < WiFi_Server.args(); n++) {
        if (Set.debugmode) {
            Serial.print("argName "); Serial.print(WiFi_Server.argName(n));
            Serial.print(" val: "); Serial.println(WiFi_Server.arg(n));
        }
        if (WiFi_Server.argName(n) == "ACTION") {
            action = int(WiFi_Server.arg(n).toInt());
            if (Set.debugmode) { Serial.print("Action found: "); Serial.println(action); }
        }
        if (action != ACTION_RESTART) { EEprom_unblock_restart(); }
        if (action == ACTION_LoadDefaultVal) {
            if (Set.debugmode) { Serial.println("load default settings from EEPROM"); }
            EEprom_read_default();
            delay(2);
        }
        //save changes
        if (WiFi_Server.argName(n) == "Save") {
            if (Set.debugmode) { Serial.println("Save button pressed in webinterface"); }
            EEprom_write_all();
        }

        if (WiFi_Server.argName(n) == "SSID_MY1") {
            for (int i = 0; i < 24; i++) Set.ssid1[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.ssid1, temInt);
        }
        if (WiFi_Server.argName(n) == "Password_MY1") {
            for (int i = 0; i < 24; i++) Set.password1[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.password1, temInt);
        }
        if (WiFi_Server.argName(n) == "SSID_MY2") {
            for (int i = 0; i < 24; i++) Set.ssid2[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.ssid2, temInt);
        }
        if (WiFi_Server.argName(n) == "Password_MY2") {
            for (int i = 0; i < 24; i++) Set.password2[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.password2, temInt);
        }
        if (WiFi_Server.argName(n) == "SSID_MY3") {
            for (int i = 0; i < 24; i++) Set.ssid3[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.ssid3, temInt);
        }
        if (WiFi_Server.argName(n) == "Password_MY3") {
            for (int i = 0; i < 24; i++) Set.password3[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.password3, temInt);
        }
        if (WiFi_Server.argName(n) == "SSID_MY4") {
            for (int i = 0; i < 24; i++) Set.ssid4[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.ssid4, temInt);
        }
        if (WiFi_Server.argName(n) == "Password_MY4") {
            for (int i = 0; i < 24; i++) Set.password4[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.password4, temInt);
        }
        if (WiFi_Server.argName(n) == "SSID_MY5") {
            for (int i = 0; i < 24; i++) Set.ssid5[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.ssid5, temInt);
        }
        if (WiFi_Server.argName(n) == "Password_MY5") {
            for (int i = 0; i < 24; i++) Set.password5[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.password5, temInt);
        }
        if (WiFi_Server.argName(n) == "SSID_AP") {
            for (int i = 0; i < 24; i++) Set.ssid_ap[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.ssid_ap, temInt);
        }
        if (WiFi_Server.argName(n) == "Password_AP") {
            for (int i = 0; i < 24; i++) Set.password_ap[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.password_ap, temInt);
        }
        if (WiFi_Server.argName(n) == "timeoutRout") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 20) && (temLong <= 1000)) { Set.timeoutRouter = int(temLong); }
        }
        if (WiFi_Server.argName(n) == "timeoutWebIO") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 2) && (temLong <= 255)) { Set.timeoutWebIO = byte(temLong); }
        }
        if (WiFi_Server.argName(n) == "WiFiIP0") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.WiFi_myip[0] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "WiFiIP1") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.WiFi_myip[1] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "WiFiIP2") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.WiFi_myip[2] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "WiFiIP3") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.WiFi_myip[3] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "WiFiIPDest") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.WiFi_ipDest_ending = byte(temInt);
        }

        if (WiFi_Server.argName(n) == "EthIP0") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_myip[0] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthIP1") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_myip[1] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthIP2") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_myip[2] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthIP3") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_myip[3] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthStatIP") {
            temInt = WiFi_Server.arg(n).toInt();
            if (temInt == 1) { Set.Eth_static_IP = true; }
            else { Set.Eth_static_IP = false; }
        }
        if (WiFi_Server.argName(n) == "EthIPDest") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_ipDest_ending = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthMac0") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_mac[0] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthMac1") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_mac[1] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthMac2") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_mac[2] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthMac3") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_mac[3] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthMac4") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_mac[4] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthMac5") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_mac[5] = byte(temInt);
        }
        /*
                if (WiFi_Server.argName(n) == "GPSPosCorrByRoll") {
                    if (WiFi_Server.arg(n) == "true") { Set.GPSPosCorrByRoll = 1; }
                    else { Set.GPSPosCorrByRoll = 0; }
                }

                if (WiFi_Server.argName(n) == "AntDist") {
                    temDoub = WiFi_Server.arg(n).toDouble();
                    if ((temDoub <= 1000) && (temDoub >= 0)) { Set.AntDist = temDoub; }
                }
                if (WiFi_Server.argName(n) == "AntHight") {
                    temDoub = WiFi_Server.arg(n).toDouble();
                    if ((temDoub <= 600) && (temDoub >= 0)) { Set.AntHight = temDoub; }
                }
                if (WiFi_Server.argName(n) == "AntLeft") {
                    temDoub = WiFi_Server.arg(n).toDouble();
                    if ((temDoub <= 1000) && (temDoub >= -1000)) { Set.virtAntLeft = temDoub; }
                }
                if (WiFi_Server.argName(n) == "AntForew") {
                    temDoub = WiFi_Server.arg(n).toDouble();
                    if ((temDoub <= 1000) && (temDoub >= -1000)) { Set.virtAntForew = temDoub; }
                }
                if (WiFi_Server.argName(n) == "AntDistDevFact") {
                    temDoub = WiFi_Server.arg(n).toDouble();
                    if ((temDoub <= 5) && (temDoub >= 1.0)) { Set.AntDistDeviationFactor = temDoub; }
                }

                if (WiFi_Server.argName(n) == "UBXFlagCheck") {
                    if (WiFi_Server.arg(n) == "true") { Set.checkUBXFlags = 1; }
                    else { Set.checkUBXFlags = 0; }
                }
                if (WiFi_Server.argName(n) == "GPSPosFilter") {
                    if (WiFi_Server.arg(n) == "true") { Set.filterGPSposOnWeakSignal = 1; }
                    else { Set.filterGPSposOnWeakSignal = 0; }
                }
                if (WiFi_Server.argName(n) == "HeadAngleCorr") {
                    temDoub = WiFi_Server.arg(n).toDouble();
                    if ((temDoub < 360) && (temDoub >= 0)) { Set.headingAngleCorrection = temDoub; }
                }
                if (WiFi_Server.argName(n) == "maxHeadChang") {
                    temLong = WiFi_Server.arg(n).toInt();
                    if ((temLong < 100) && (temLong >= 2)) { Set.MaxHeadChangPerSec = byte(temLong); }
                }
                if (WiFi_Server.argName(n) == "RollAngleCorr") {
                    temDoub = WiFi_Server.arg(n).toDouble();
                    if ((temDoub < 45) && (temDoub >= -45)) { Set.DualRollAngleCorrection = temDoub; }
                }
              */  /*      if (WiFi_Server.argName(n) == "NtripHost") {
                          for (int i = 0; i < 40; i++) Set.NtripHost[i] = 0x00;
                          temInt = WiFi_Server.arg(n).length() + 1;
                          WiFi_Server.arg(n).toCharArray(Set.NtripHost, temInt);
                      }
                      if (WiFi_Server.argName(n) == "NtripMountpoint") {
                          for (int i = 0; i < 40; i++) Set.NtripMountpoint[i] = 0x00;
                          temInt = WiFi_Server.arg(n).length() + 1;
                          WiFi_Server.arg(n).toCharArray(Set.NtripMountpoint, temInt);
                      }
                      if (WiFi_Server.argName(n) == "NtripUser") {
                          for (int i = 0; i < 40; i++) Set.NtripUser[i] = 0x00;
                          temInt = WiFi_Server.arg(n).length() + 1;
                          WiFi_Server.arg(n).toCharArray(Set.NtripUser, temInt);
                      }
                      if (WiFi_Server.argName(n) == "NtripPassword") {
                          for (int i = 0; i < 40; i++) Set.NtripPassword[i] = 0x00;
                          temInt = WiFi_Server.arg(n).length() + 1;
                          WiFi_Server.arg(n).toCharArray(Set.NtripPassword, temInt);
                      }
                      if (WiFi_Server.argName(n) == "NtripFixGGASentence") {
                          for (int i = 0; i < 100; i++) Set.NtripFixGGASentence[i] = 0x00;
                          temInt = WiFi_Server.arg(n).length() + 1;
                          WiFi_Server.arg(n).toCharArray(Set.NtripFixGGASentence, temInt);
                      }
                      if (WiFi_Server.argName(n) == "NtripPort") {
                          temLong = WiFi_Server.arg(n).toInt();
                          if ((temLong <= 32000) && (temLong >= 0)) { Set.NtripPort = int(temLong); }
                      }
                      if (WiFi_Server.argName(n) == "NtripGGASendRate") {
                          temLong = WiFi_Server.arg(n).toInt();
                          if ((temLong <= 200) && (temLong >= 2)) { Set.NtripGGASendRate = byte(temLong); }
                      }
                      if (WiFi_Server.argName(n) == "NtripSendWhichGGASentence") {
                          Set.NtripSendWhichGGASentence = byte(WiFi_Server.arg(n).toInt());
                      }
                      if (WiFi_Server.argName(n) == "NtripClientBy") {
                          temLong = WiFi_Server.arg(n).toInt();
                          switch (temLong) {
                          case 0://NTRIP off
                              Set.NtripClientBy = 0;
                              break;
                          case 1://AOG WiFi NTRIP
                              Set.NtripClientBy = 1;
                              if (Set.DataTransVia > 5) {
                                  if (Set.DataTransVia < 10) {
                                      //start WiFI UDP
                                      if (WiFi_udpNtrip.listen(Set.AOGNtripPort))
                                      {
                                          Serial.print("NTRIP UDP Listening to port: ");
                                          Serial.println(Set.AOGNtripPort);
                                          Serial.println();
                                      }
                                      vTaskDelay(100);
                                      delay(10);//100
                                      // UDP NTRIP packet handling
                                      WiFi_udpNtrip.onPacket([](AsyncUDPPacket packet)
                                          {
                                              if (Set.debugmode) { Serial.println("got NTRIP data"); }
                                              for (unsigned int i = 0; i < packet.length(); i++)
                                              {
                                                  Serial1.write(packet.data()[i]);
                                              }
                                              NtripDataTime = millis();
                                          });  // end of onPacket call
                                      WiFiUDPRunning = true;
                                  }
                                  else {
                                      //Ethernet
                                      if (WiFiUDPRunning) { WiFi_udpNtrip.close(); WiFiUDPRunning = false; }
                                  }
                              }
                              break;
                          case 2://ESP32 NTRIP client
                              Set.NtripClientBy = 2;
                              if (!task_NTRIP_Client_running) {
                                  Ntrip_restart = 1;
                                  NtripDataTime = millis();
                                  Serial.println("creating NTRIP task on core 1");
                                  xTaskCreatePinnedToCore(NTRIP_Client_Code, "Core1", 3072, NULL, 1, &taskHandle_WiFi_NTRIP, 1);
                                  delay(500);
                              }
                              break;
                          }
                      }

                     if (WiFi_Server.argName(n) == "seOGI") {
                       if (WiFi_Server.arg(n) == "true") { Set.sendOGI = 1; }
                       else {
                           if (Set.sendGGA == 0) { Set.sendOGI = 1; }
                           else {
                               Set.sendOGI = 0;//only switch off, if GGA is send
                           }
                       }
                   } */
                   /*       if (WiFi_Server.argName(n) == "seGGA") {
                            if (WiFi_Server.arg(n) == "true") { Set.sendGGA = 1; }
                            else { Set.sendGGA = 0; }
                        }
                        if (WiFi_Server.argName(n) == "seVTG") {
                            if (WiFi_Server.arg(n) == "true") { Set.sendVTG = 1; }
                            else { Set.sendVTG = 0; }
                        }
                      if (WiFi_Server.argName(n) == "seHDT") {
                            if (WiFi_Server.arg(n) == "true") { Set.sendHDT = 1; }
                            else { Set.sendHDT = 0; }
                        }*/
        if (WiFi_Server.argName(n) == "sePANDA") {
            if (WiFi_Server.arg(n) == "1") { Set.sendPANDA = 1; }
            else { Set.sendPANDA = 0; }
        }
        if (WiFi_Server.argName(n) == "seIMU") {
            if (WiFi_Server.arg(n) == "true") { Set.sendIMUPGN = 1; }
            else { Set.sendIMUPGN = 0; }
        }
        if (WiFi_Server.argName(n) == "AgIOHeartBeat") {
            if (WiFi_Server.arg(n) == "true") { Set.AgIOHeartbeat_answer = 1; }
            else { Set.AgIOHeartbeat_answer = 0; }
        }

        if (WiFi_Server.argName(n) == "DataTransfVia") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong <= 20) && (temLong >= 0)) { Set.DataTransVia = byte(temLong); }
            if (Set.DataTransVia == 10) {
                if (Eth_connect_step == 255) {
                    Eth_connect_step = 10;
                    xTaskCreate(Eth_handle_connection, "Core1EthConnectHandle", 3072, NULL, 1, &taskHandle_Eth_connect);
                    delay(500);
                }
            }
            if (Set.NtripClientBy == 1) {
                if (Set.DataTransVia > 5) {
                    if (Set.DataTransVia < 10) {
                        //start WiFI UDP                
                        if (WiFi_udpNtrip.listen(Set.AOGNtripPort))
                        {
                            Serial.print("NTRIP UDP Listening to port: ");
                            Serial.println(Set.AOGNtripPort);
                            Serial.println();
                        }
                        vTaskDelay(100);
                        delay(10);//100
                        // UDP NTRIP packet handling
                        WiFi_udpNtrip.onPacket([](AsyncUDPPacket packet)
                            {
                                if (Set.debugmode) { Serial.println("got NTRIP data"); }
                                for (unsigned int i = 0; i < packet.length(); i++)
                                {
                                    Serial1.write(packet.data()[i]);
                                }
                                NtripDataTime = millis();
                            });  // end of onPacket call
                        WiFiUDPRunning = true;
                        /*               if ((Set.NtripClientBy == 2) && (!task_Eth_NTRIP_running)) {
                                           if (!task_NTRIP_Client_running) {
                                               Ntrip_restart = 1;
                                               NtripDataTime = millis();
                                               xTaskCreatePinnedToCore(NTRIP_Client_Code, "Core1", 3072, NULL, 1, &taskHandle_WiFi_NTRIP, 1);
                                               delay(500);
                                           }
                                       }
                                   }
                                   else {
                                       //Ethernet
                                       if (WiFiUDPRunning) { WiFi_udpNtrip.close(); WiFiUDPRunning = false; }
                                       */
                    }
                }
            }
        }

        if (WiFi_Server.argName(n) == "IMUtype") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong <= 20) && (temLong >= 0)) { Set.IMUType = byte(temLong); }
            if (Set.IMUType == 0) {    //no IMU
                for (byte a = 0; a < 10; a++) {
                    IMUHeading[a] = 65535;
                    IMURoll[a] = 0; //8888;
                    IMUPitch[a] = 0;
                    IMUYawRate[a] = 0;
                }
                itoa(65535, imuHeading, 10);       //65535 is max value to stop AgOpen using IMU in Panda
                IMUnextReadTime = 4294967294;
            }
            if (Set.IMUType == 1) { init_BNO_serial(); }
            if (Set.IMUType == 2) { init_CMPS14(); }
        }

        if (WiFi_Server.argName(n) == "IMUinvertRoll") {
            if (WiFi_Server.arg(n) == "true") { Set.IMUinvertRoll = true; }
            else { Set.IMUinvertRoll = false; }
        }

        if (WiFi_Server.argName(n) == "IMUswapRollPitch") {
            if (WiFi_Server.arg(n) == "true") { Set.IMUswapRollPitch = true; }
            else { Set.IMUswapRollPitch = false; }
        }

        if (WiFi_Server.argName(n) == "RollAngleCorrCMPS") {
            temDoub = WiFi_Server.arg(n).toDouble();
            if ((temDoub < 45) && (temDoub >= -45)) { Set.IMURollCorrection = temDoub; }
        }
        if (WiFi_Server.argName(n) == "HeadAngleCorrCMPS") {
            temDoub = WiFi_Server.arg(n).toDouble();
            if ((temDoub < 360) && (temDoub >= 0)) { Set.CMPSHeadingCorrection = temDoub; }
        }
        if (WiFi_Server.argName(n) == "GPS2USB") {
            if (WiFi_Server.arg(n) == "true") { Set.bridgeGPStoUSB = true; }
            else { Set.bridgeGPStoUSB = false; }
        }
        if (WiFi_Server.argName(n) == "BaudrGPS") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong <= 20) && (temLong >= 0)) {
                Set.GPS_baudrate_Nr = byte(temLong);
                Serial1.updateBaudRate(baudrates[Set.GPS_baudrate_Nr]);
                delay(50);
            }
        }
        if (WiFi_Server.argName(n) == "BaudrUSB") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong <= 20) && (temLong >= 0)) {
                Set.USB_baudrate_Nr = byte(temLong);
                Serial.updateBaudRate(baudrates[Set.USB_baudrate_Nr]);
                delay(50);
            }
        }
        if (WiFi_Server.argName(n) == "GPSGetBaud") {
            if (Set.debugmode) { Serial.println("Get GPS serial baudrate pressed in webinterface"); }
            GPSGetBaudrate();
        }

    /*    if (WiFi_Server.argName(n) == "IMUTimeShift") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong <= 20) && (temLong >= 0)) {
                IMUDataTimeShiftTest = 11 - byte(temLong);
            }
            if (IMUDataTimeShiftTest == 10) { IMUDataTimeShiftTest = 0; }
            Serial.print("IMU data time shift changed in webinterface to: "); Serial.println(IMUDataTimeShiftTest);
        }*/
        if (WiFi_Server.argName(n) == "debugmode") {
            if (WiFi_Server.arg(n) == "true") { Set.debugmode = true; }
            else { Set.debugmode = false; }
        }
        /*          if (WiFi_Server.argName(n) == "debugmUBX") {
                      if (WiFi_Server.arg(n) == "true") { Set.debugmodeUBX = true; }
                      else { Set.debugmodeUBX = false; }
                  }
                  if (WiFi_Server.argName(n) == "debugmHead") {
                      if (WiFi_Server.arg(n) == "true") { Set.debugmodeHeading = true; }
                      else { Set.debugmodeHeading = false; }
                  }
                  if (WiFi_Server.argName(n) == "debugmVirtAnt") {
                      if (WiFi_Server.arg(n) == "true") { Set.debugmodeVirtAnt = true; }
                      else { Set.debugmodeVirtAnt = false; }
                  }
                  if (WiFi_Server.argName(n) == "debugmFiltPos") {
                      if (WiFi_Server.arg(n) == "true") { Set.debugmodeFilterPos = true; }
                      else { Set.debugmodeFilterPos = false; }
                  }

                  if (WiFi_Server.argName(n) == "debugmNtrip") {
                      if (WiFi_Server.arg(n) == "true") { Set.debugmodeNTRIP = true; }
                      else { Set.debugmodeNTRIP = false; }
                  }
                  if (WiFi_Server.argName(n) == "debugmRAW") {
                      if (WiFi_Server.arg(n) == "true") { Set.debugmodeRAW = true; }
                      else { Set.debugmodeRAW = false; }
                  }
       */
        if (WiFi_Server.argName(n) == "WiFiLEDon") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.LEDWiFi_ON_Level = byte(temLong);
                if (LED_WIFI_ON) { digitalWrite(Set.LEDWiFi_PIN, Set.LEDWiFi_ON_Level); }
                else { digitalWrite(Set.LEDWiFi_PIN, !Set.LEDWiFi_ON_Level); }
            }
        }

        if (WiFi_Server.argName(n) == "LED") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.LEDWiFi_PIN = byte(temLong);
                pinMode(Set.LEDWiFi_PIN, OUTPUT);
                if (LED_WIFI_ON) { digitalWrite(Set.LEDWiFi_PIN, Set.LEDWiFi_ON_Level); }
                else { digitalWrite(Set.LEDWiFi_PIN, !Set.LEDWiFi_ON_Level); }
            }
        }
        if (WiFi_Server.argName(n) == "TX1") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.TX1 = byte(temLong);
                Serial1.end();
                delay(20);
                Serial1.begin(115200, SERIAL_8N1, Set.RX1, Set.TX1);
                delay(5);
            }
        }
        if (WiFi_Server.argName(n) == "RX1") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.RX1 = byte(temLong);
                Serial1.end();
                delay(20);
                Serial1.begin(115200, SERIAL_8N1, Set.RX1, Set.TX1);
                delay(5);
            }
        }
        if (WiFi_Server.argName(n) == "TX2") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.TX2 = byte(temLong);
                Serial2.end();
                delay(20);
                Serial2.begin(115200, SERIAL_8N1, Set.RX2, Set.TX2);
                delay(5);
            }
        }
        if (WiFi_Server.argName(n) == "RX2") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.RX2 = byte(temLong);
                Serial2.end();
                delay(20);
                Serial2.begin(115200, SERIAL_8N1, Set.RX2, Set.TX2);
                delay(5);
            }
        }
        if (WiFi_Server.argName(n) == "WiFiResc") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.LEDWiFi_PIN = byte(temLong);
                pinMode(Set.Button_WiFi_rescan_PIN, INPUT_PULLUP);
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
    strcat(HTML_String, "<body bgcolor=\"#66b3ff\">");//ff9900 ffcc00
    strcat(HTML_String, "<font color=\"#000000\" face=\"VERDANA,ARIAL,HELVETICA\">");
    strcat(HTML_String, "<h1>GPS roof unit for ESP32</h1>");
    strcat(HTML_String,"supports data via USB, WiFi/Ethernet UDP<br>possible IMU types: BNO085 (RVC serial mode) or CMPS14 (I2C)<br><br>");
    strcat(HTML_String, "<b>Transfers NTRIP data from AOG port ");
    strcati(HTML_String, Set.AOGNtripPort);
    strcat(HTML_String," to GPS receiver</b><br><br>");
    strcat(HTML_String, "more settings like IPs, UPD ports... in setup zone of INO code<br><br>Version: ");
    strcati(HTML_String, vers_nr);
    strcat(HTML_String, VersionTXT);
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    //load values of INO setup zone
    strcat(HTML_String, "<h2>Load default values of INO setup zone</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(270, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"2\">Button only loads default values, does NOT save them</td>");
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
    strcat(HTML_String, "<b>Data to access tractor WiFi or mobil phone hotspots.<br><br>");
    strcat(HTML_String, "If access to networks fails, an accesspoint will be created with the settings below.</b><br><br><table>");   
    strcat(HTML_String, "<form>");
    set_colgroup(250, 300, 150, 0, 0);

    strcat(HTML_String, "<tr><td><b>#1 Network SSID:</b></td>");
    strcat(HTML_String, "<td><input type=\"text\" onchange=\"sendVal('/?SSID_MY1='+this.value)\" style= \"width:200px\" name=\"SSID_MY1\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.ssid1);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td></tr>");

    strcat(HTML_String, "<tr><td><b>#1 Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_MY1='+this.value)\" style= \"width:200px\" name=\"Password_MY1\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.password1);
    strcat(HTML_String, "\"></td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td><b>#2 Network SSID:</b></td>");
    strcat(HTML_String, "<td><input type=\"text\" onchange=\"sendVal('/?SSID_MY2='+this.value)\" style= \"width:200px\" name=\"SSID_MY2\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.ssid2);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr><td><b>#2 Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_MY2='+this.value)\" style= \"width:200px\" name=\"Password_MY2\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.password2);
    strcat(HTML_String, "\"></td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td><b>#3 Network SSID:</b></td>");
    strcat(HTML_String, "<td><input type=\"text\" onchange=\"sendVal('/?SSID_MY3='+this.value)\" style= \"width:200px\" name=\"SSID_MY3\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.ssid3);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr><td><b>#3 Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_MY3='+this.value)\" style= \"width:200px\" name=\"Password_MY3\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.password3);
    strcat(HTML_String, "\"></td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td><b>#4 Network SSID:</b></td>");
    strcat(HTML_String, "<td><input type=\"text\" onchange=\"sendVal('/?SSID_MY4='+this.value)\" style= \"width:200px\" name=\"SSID_MY4\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.ssid4);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr><td><b>#4 Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_MY4='+this.value)\" style= \"width:200px\" name=\"Password_MY4\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.password4);
    strcat(HTML_String, "\"></td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td><b>#5 Network SSID:</b></td>");
    strcat(HTML_String, "<td><input type=\"text\" onchange=\"sendVal('/?SSID_MY5='+this.value)\" style= \"width:200px\" name=\"SSID_MY5\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.ssid5);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr><td><b>#5 Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_MY5='+this.value)\" style= \"width:200px\" name=\"Password_MY5\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.password5);
    strcat(HTML_String, "\"></td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\">time, trying to connect to networks from list above</td></tr>");
    strcat(HTML_String, "<td colspan=\"3\">after time has passed, access point is opened (SSID and password below</td></tr>");
    strcat(HTML_String, "<tr><td><b>Timeout (s):</b></td><td><input type = \"number\" onchange=\"sendVal('/?timeoutRout='+this.value)\" name = \"timeoutRout\" min = \"20\" max = \"1000\" step = \"1\" style= \"width:200px\" value = \"");// placeholder = \"");
    strcati(HTML_String, Set.timeoutRouter);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td><b>Access Point SSID:</b></td>");
    strcat(HTML_String, "<td><input type=\"text\" onchange=\"sendVal('/?SSID_AP='+this.value)\" style= \"width:200px\" name=\"SSID_AP\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.ssid_ap);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr><td><b>Access Point Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_AP='+this.value)\" style= \"width:200px\" name=\"Password_AP\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.password_ap);
    strcat(HTML_String, "\"></td></tr>");
    strcat(HTML_String, "<tr> <td colspan=\"3\"><b>IP for WebInterface in AP mode will be 192.168.1.1</b></td>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td colspan=\"2\"><b>Restart ESP32 roof unit</b></td>");
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
    strcati(HTML_String, Set.timeoutWebIO);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");


    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // Data transfer via USB/Wifi 
    strcat(HTML_String, "<h2>USB, WiFi or Ethernet data transfer</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    //transfer data via 0 = USB / 7 = WiFi UDP / 10 = Ethernet
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?DataTransfVia=0')\" name=\"DataTransfVia\" id=\"JZ\" value=\"0\"");
    if (Set.DataTransVia == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">USB at ");
    strcati(HTML_String, baudrates[Set.GPS_baudrate_Nr]);
    strcat(HTML_String, " baud</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");
    strcat(HTML_String, "><label for=\"JZ\">USB + mobile Hotspot for NTRIP</label></td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?DataTransfVia=7')\" name=\"DataTransfVia\" id=\"JZ\" value=\"7\"");
    if (Set.DataTransVia == 7)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">WiFi (UDP) (default)</label></td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td colspan=\"2\"><input type = \"radio\" onclick=\"sendVal('/?DataTransfVia=10')\" name=\"DataTransfVia\" id=\"JZ\" value=\"10\"");
    if (Set.DataTransVia == 10)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">Ethernet (UDP) Ethernet hardware needed!!</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

/*
    //---------------------------------------------------------------------------------------------  
    // NTRIP

    strcat(HTML_String, "<h2>NTRIP client</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    //transfer data via 0 = OFF / 1 = AOG NTRIP / 2 = ESP32 NTRIP client
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?NtripClientBy=0')\" name=\"NtripClientBy\" id=\"JZ\" value=\"0\"");
    if (Set.NtripClientBy == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">OFF</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td colspan=\"2\"><input type = \"radio\" onclick=\"sendVal('/?NtripClientBy=1')\" name=\"NtripClientBy\" id=\"JZ\" value=\"1\"");
    if (Set.NtripClientBy == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">AOG NTRIP client (for WiFi/Ethernet UDP set port 2233 in AOG)</label></td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td colspan=\"2\"><input type = \"radio\" onclick=\"sendVal('/?NtripClientBy=2')\" name=\"NtripClientBy\" id=\"JZ\" value=\"2\"");
    if (Set.NtripClientBy == 2)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">Roof unit NTRIP client (access data set here)</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

/*    //-----------------------------------------------------------------------------------------
    // NTRIP client data

    strcat(HTML_String, "<h2>ESP32 NTRIP Client Data</h2>");
    strcat(HTML_String, "<b>NTRIP client uses tractor WiFi or mobil phone hotspots.</b><br><br>");
    strcat(HTML_String, "<table><form>");
    set_colgroup(250, 300, 150, 0, 0);

    strcat(HTML_String, "<tr><td><b>NTRIP Host:</b></td>");
    strcat(HTML_String, "<td colspan=\"2\"><input type=\"text\" onchange=\"sendVal('/?NtripHost='+this.value)\" style= \"width:300px\" name=\"NtripHost\" maxlength=\"38\" Value =\"");
    strcat(HTML_String, Set.NtripHost);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr><td><b>NTRIP Mountpoint:</b></td>");
    strcat(HTML_String, "<td colspan=\"2\">");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?NtripMountpoint='+this.value)\" style= \"width:300px\" name=\"NtripMountpoint\" maxlength=\"38\" Value =\"");
    strcat(HTML_String, Set.NtripMountpoint);
    strcat(HTML_String, "\"></td></tr>");

    strcat(HTML_String, "<tr><td><b>NTRIP Port</b></td>");
    strcat(HTML_String, "<td><input type = \"number\" onchange=\"sendVal('/?NtripPort='+this.value)\" name = \"NtripPort\" min = \"0\" max = \"32000\" step = \"1\" style= \"width:100px\" value = \"");// placeholder = \"");
    strcati(HTML_String, Set.NtripPort);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td><b>User:</b></td>");
    strcat(HTML_String, "<td colspan=\"2\"><input type=\"text\" onchange=\"sendVal('/?NtripUser='+this.value)\" style= \"width:300px\" name=\"NtripUser\" maxlength=\"38\" Value =\"");
    strcat(HTML_String, Set.NtripUser);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr><td><b>Password:</b></td>");
    strcat(HTML_String, "<td colspan=\"2\"><input type=\"text\" onchange=\"sendVal('/?NtripPassword='+this.value)\" style= \"width:300px\" name=\"NtripPassword\" maxlength=\"38\" Value =\"");
    strcat(HTML_String, Set.NtripPassword);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td colspan=\"3\"><b>Position send to NTRIP server:</b></td>");
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?NtripSendWhichGGASentence=0')\" name=\"NtripSendWhichGGASentence\" id=\"JZ\" value=\"0\"");
    if (Set.NtripSendWhichGGASentence == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">OFF</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?NtripSendWhichGGASentence=1')\" name=\"NtripSendWhichGGASentence\" id=\"JZ\" value=\"1\"");
    if (Set.NtripSendWhichGGASentence == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">fixed GGA string</label></td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?NtripSendWhichGGASentence=2')\" name=\"NtripSendWhichGGASentence\" id=\"JZ\" value=\"2\"");
    if (Set.NtripSendWhichGGASentence == 2)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">actual position</label></td></tr>");

    strcat(HTML_String, "<tr><td><b>GGA position to send, if fixed position is used:</b></td></tr>");
    strcat(HTML_String, "<tr><td colspan=\"3\"><input type=\"text\" onchange=\"sendVal('/?NtripFixGGASentence='+this.value)\" style= \"width:550px\" name=\"NtripFixGGASentence\" maxlength=\"98\" Value =\"");
    strcat(HTML_String, Set.NtripFixGGASentence);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td><b>Time to send GGA position (every xx seconds):</b></td>");
    strcat(HTML_String, "<td><input type = \"number\" onchange=\"sendVal('/?NtripGGASendRate='+this.value)\" name = \"NtripGGASendRate\" min = \"2\" max = \"200\" step = \"1\" style= \"width:100px\" value = \"");// placeholder = \"");
    strcati(HTML_String, Set.NtripGGASendRate);
    strcat(HTML_String, "\"></td></tr>");

    strcat(HTML_String, "</table></form>");
    strcat(HTML_String, "<br><hr>");
*/
    //---------------------------------------------------------------------------------------------  
    // WiFi IP settings 
    strcat(HTML_String, "<h2>WiFi IP settings</h2><br>for Access Point always 192.168.1.1<br>");
    strcat(HTML_String, "<form>");
    //IP
    strcat(HTML_String, "<b>IP address for WiFi</b><br>When using DHCP the last number is set as IP from here, the first 3 numbers are set by DHCP.<br>");  
    strcat(HTML_String, "<b>Default for last number is 79, it's also the address of the Webinterface.</b>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 50, 50, 50, 50);
    strcat(HTML_String, "<tr><td>IP address</td><td><input type = \"number\"  onchange=\"sendVal('/?WiFiIP0='+this.value)\" name = \"WiFiIP\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.WiFi_myip[0]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?WiFiIP1='+this.value)\" name = \"WiFiIP\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.WiFi_myip[1]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?WiFiIP2='+this.value)\" name = \"WiFiIP\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.WiFi_myip[2]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?WiFiIP3='+this.value)\" name = \"WiFiIP\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.WiFi_myip[3]);
    strcat(HTML_String, "\"></td></table><br>");

    //IP destination    
    strcat(HTML_String, "<table>");
    set_colgroup(250, 300, 150, 0, 0);
    strcat(HTML_String, "<tr><td colspan=\"2\"><b>IP address of destination</b></td>");    
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr></table><table>");
    strcat(HTML_String,"Destination's IP address, the first 3 numbers are set by DHCP, or as above.<br>");
    strcat(HTML_String, "<b>Use 255 to send to every device in network (default).</b> Use IP of your Computer, if you don't have a router and fixed IPs");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 50, 50, 50, 50);
    strcat(HTML_String, "<tr><td>IP address destination</td><td>xxx</td><td>xxx</td><td>xxx<td><input type = \"number\"  onchange=\"sendVal('/?WiFiIPDest='+this.value)\" name = \"WiFiIPDest\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.WiFi_ipDest_ending);
    strcat(HTML_String, "\"></td></table><br><hr>");

    //---------------------------------------------------------------------------------------------  
    // Ethernet settings 
    strcat(HTML_String, "<h2>Ethernet settings</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    //use DHCP/static IP radio button
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?EthStatIP=0')\" name=\"EthStatIP\" id=\"JZ\" value=\"0\"");
    if (Set.Eth_static_IP == false)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">use DHCP (default)</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?EthStatIP=1')\" name=\"EthStatIP\" id=\"JZ\" value=\"1\"");
    if (Set.Eth_static_IP == true)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">use fixed IP from below</label></td></tr>");

    strcat(HTML_String, "</table><br>");

    //IP
    strcat(HTML_String, "<b>IP address for Ethernet</b><br>When using DHCP the last number is set as IP from here, the first 3 numbers are set by DHCP.<br>");
    strcat(HTML_String, "<b>Default for last number is 80.</b>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 50, 50, 50, 50);
    strcat(HTML_String, "<tr><td>IP address</td><td><input type = \"number\"  onchange=\"sendVal('/?EthIP0='+this.value)\" name = \"EthIP\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.Eth_myip[0]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?EthIP1='+this.value)\" name = \"EthIP\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.Eth_myip[1]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?EthIP2='+this.value)\" name = \"EthIP\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.Eth_myip[2]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?EthIP3='+this.value)\" name = \"EthIP\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.Eth_myip[3]);
    strcat(HTML_String, "\"></td></table><br>");

    //IP destination
    strcat(HTML_String, "<table>");
    set_colgroup(250, 300, 150, 0, 0);
    strcat(HTML_String, "<tr><td colspan=\"2\"><b>IP address of destination</b></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr></table><table>");
    strcat(HTML_String, "Destination's IP address, the first 3 numbers are set by DHCP, or as above.<br>");
    strcat(HTML_String, "<b>Use 255 to send to every device in network (default).</b> Use IP of your Computer, if you don't have a router and fixed IPs");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 50, 50, 50, 50);
    strcat(HTML_String, "<tr><td>IP address destination</td><td>xxx</td><td>xxx</td><td>xxx<td><input type = \"number\"  onchange=\"sendVal('/?EthIPDest='+this.value)\" name = \"EthIPDest\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.Eth_ipDest_ending);
    strcat(HTML_String, "\"></td></table><br>");

    //mac
    strcat(HTML_String, "<b>mac address of Ethernet hardware</b><br>Type in the mac address of you Ethernet shield.<br>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 50, 50, 50, 50, 50, 50);
    strcat(HTML_String, "<tr><td>mac address</td><td><input type = \"number\"  onchange=\"sendVal('/?EthMac0='+this.value)\" name = \"EthMac\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.Eth_mac[0]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?EthMac1='+this.value)\" name = \"EthMac\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.Eth_mac[1]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?EthMac2='+this.value)\" name = \"EthMac\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.Eth_mac[2]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?EthMac3='+this.value)\" name = \"EthMac\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.Eth_mac[3]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?EthMac4='+this.value)\" name = \"EthMac\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.Eth_mac[4]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?EthMac5='+this.value)\" name = \"EthMac\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.Eth_mac[5]);
    strcat(HTML_String, "\"></td></table>");


    strcat(HTML_String, "</form><br><hr>");
  
    //---------------------------------------------------------------------------------------------
    // IMU setup

    strcat(HTML_String, "<h2>IMU setup</h2>");

    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    //IMU: 0 = none; 1 = BNO085 in serial mode; 2 = CMPS14
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td>IMU type</td><td><input type = \"radio\" onclick=\"sendVal('/?IMUtype=0')\" name=\"IMUtype\" id=\"JZ\" value=\"0\"");
    if (Set.IMUType == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">none</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td colspan=\"2\"><input type = \"radio\" onclick=\"sendVal('/?IMUtype=1')\" name=\"IMUtype\" id=\"JZ\" value=\"1\"");
    if (Set.IMUType == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">BNO085 in serial mode (default)</label></td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td colspan=\"2\"><input type = \"radio\" onclick=\"sendVal('/?IMUtype=2')\" name=\"IMUtype\" id=\"JZ\" value=\"2\"");
    if (Set.IMUType == 2)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">CMPS14 at I2C address (dec) ");
    strcati(HTML_String, Set.CMPS14_ADDRESS); 
    strcat(HTML_String,"</label></td> </tr>");
    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br>");

    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(350, 200, 150, 0, 0);

    //checkbox
    strcat(HTML_String, "<tr><td>Swap roll and pitch (BNO085 only) </td><td><input type=\"checkbox\" onclick=\"sendVal('/?IMUswapRollPitch='+this.checked)\" name=\"IMUswapRollPitch\" id = \"Part\" value = \"1\" ");
    if (Set.IMUswapRollPitch) strcat(HTML_String, "checked ");
    strcat(HTML_String, "></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");
    strcat(HTML_String, "<tr><td>Invert roll </td><td><input type=\"checkbox\" onclick=\"sendVal('/?IMUinvertRoll='+this.checked)\" name=\"IMUinvertRoll\" id = \"Part\" value = \"1\" ");
    if (Set.IMUinvertRoll) strcat(HTML_String, "checked ");
    strcat(HTML_String, "></td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
    strcat(HTML_String, "<tr><td>Roll from IMU</td><td><divbox align=\"right\"><font size=\"+1\"><b>");
    if (bitRead(IMURoll[IMUDataRingCount],15)) { rollIMURAW = float(IMURoll[IMUDataRingCount] - 65535) / 10; }
    else{ rollIMURAW = float(IMURoll[IMUDataRingCount]) / 10; }
    if (rollIMURAW < 0) { rollIMURAW += 6553.5; }//
    if (rollIMURAW < 10) { strcatf(HTML_String, rollIMURAW, 3, 1); }
    else { strcatf(HTML_String, rollIMURAW, 4, 1); }
    strcat(HTML_String, "</b></font></divbox></td>");
    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");


    strcat(HTML_String, "<tr><td>Heading from IMU</td><td><divbox align=\"right\"><font size=\"+1\"><b>");
    HeadingIMURAW = float(IMUHeading[IMUDataRingCount]) / 10;
    if (HeadingIMURAW < 10) { strcatf(HTML_String, HeadingIMURAW, 3, 1); }
    else {
        if (HeadingIMURAW < 100) { strcatf(HTML_String, HeadingIMURAW, 4, 1); }
        else { strcatf(HTML_String, HeadingIMURAW, 5, 1); }
    }
    strcat(HTML_String, "</b></font></divbox></td>");
    //Refresh button
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"location.reload()\" style= \"width:120px\" value=\"Refresh\"></button></td>");
    strcat(HTML_String, "</b></font></divbox></td></tr>");
    strcat(HTML_String, "</table></form><br><hr>");
/*
    // heading angle correction 

    strcat(HTML_String, "<b>In combination with dual GPS, the angle correction will be done by the ESP when driving in good signal conditions</b><br><br>");
    strcat(HTML_String, "<b>CMPS14 heading and roll angle correction</b>");  
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(350, 200, 150, 0, 0);

    strcat(HTML_String, "<tr><td>heading angle correction</td><td><input type = \"number\" onchange=\"sendVal('/?HeadAngleCorrCMPS='+this.value)\" name = \"HeadAngleCorrCMPS\" min = \" 0\" max = \"360\" step = \"0.1\" style= \"width:100px\" value = \"");// placeholder = \"");
    if (Set.CMPSHeadingCorrection < 10) { strcatf(HTML_String, Set.CMPSHeadingCorrection, 3, 1); }
    else {
        if (Set.CMPSHeadingCorrection < 100) { strcatf(HTML_String, Set.CMPSHeadingCorrection, 4, 1); }
        else { strcatf(HTML_String, Set.CMPSHeadingCorrection, 5, 1); }
    }
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td>roll angle correction</td><td><input type = \"number\" onchange=\"sendVal('/?RollAngleCorrCMPS='+this.value)\" name = \"RollAngleCorrCMPS\" min = \" - 45\" max = \"45\" step = \"0.1\" style= \"width:100px\" value = \"");// placeholder = \"");

    if (Set.IMURollCorrection < 10) { strcatf(HTML_String, Set.IMURollCorrection, 3, 1); }
    else { strcatf(HTML_String, Set.IMURollCorrection, 4, 1); }
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");
*/
    //---------------------------------------------------------------------------------------------
    // Checkboxes Messages
    strcat(HTML_String, "<h2>Messages to send to AgOpenGPS</h2>");
    strcat(HTML_String, "If you have a BNO085 or CMPS14 IMU send $PANDA OR IMU PGN to AgOpenGPS.<br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

//send PANDA or org NMEA sentence radio button
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td colspan=\"2\"><input type = \"radio\" onclick=\"sendVal('/?sePANDA=1')\" name=\"PANDA\" id=\"JZ\" value=\"1\"");
    if (Set.sendPANDA == 1) strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">send $PANDA (default, used only by AgOpenGPS)</label></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?sePANDA=0')\" name=\"PANDA\" id=\"JZ\" value=\"0\"");
    if (Set.sendPANDA == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">send original NMEA sentence</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr><td></td><td colspan=\"2\"><input type=\"checkbox\" onclick=\"sendVal('/?seIMU='+this.checked)\" name=\"seIMU\" id = \"Part\" value = \"1\" ");
    if (Set.sendIMUPGN == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> send IMU PGN (NOT with PANDA, as included in PANDA)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td></td><td><input type=\"checkbox\" onclick=\"sendVal('/?AgIOHeartBeat='+this.checked)\" name=\"AgIOHeartBeat\" id = \"Part\" value = \"1\" ");
    if (Set.AgIOHeartbeat_answer == 1) { strcat(HTML_String, "checked "); }
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> send IMU heartbeat</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");    

    //---------------------------------------------------------------------------------------------
    // GPS to USB brigde
    strcat(HTML_String, "<h2>USB to GPS and GPS to USB bridge</h2>");
    strcat(HTML_String, "Activate to configure F9P GPS with UCenter via ESP32 USB port.<br>All data will be send from USB to UART1 of GPS system at 115.200 baud.<br><br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    strcat(HTML_String, "<tr><td></td><td><input type=\"checkbox\" onclick=\"sendVal('/?GPS2USB='+this.checked)\" name=\"GPS2USB\" id = \"Part\" value = \"1\" ");
    if (Set.bridgeGPStoUSB == 1) { strcat(HTML_String, "checked "); }
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> bridge GPS to USB and vice versa</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");

    strcat(HTML_String, "</tr></table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------
    // GPS Get Baudrate
    strcat(HTML_String, "<h2>ESP32 serial1 baudrate connected to GPS u-blox chip UART 1 baudrate</h2>");
    strcat(HTML_String, "If the baudrate between ESP32 and u-blox F9P does not fit, no data will be transferd.<br>Run get u-blox baudrate below, if UCenter is not working or no GPS data is comming.<br><br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(50, 180, 180, 0, 0);

    strcat(HTML_String, "<tr> <td colspan=\"2\">ESP32 serial1 to GPS baudrate</td><td>ESP32 USB baudrate</td></tr>");
    for (byte i = 0; i < nrBaudrates; i++) {//nrBaudrates
        strcat(HTML_String, "<tr><td>");
        if ((Set.GPS_baudrate_Nr == i) && GPSBaudrateValid) strcat(HTML_String, "u-blox verified");
        strcat(HTML_String, "</td><td><input type = \"radio\" onclick=\"sendVal('/?BaudrGPS=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"baudrate\" id=\"JZ\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (Set.GPS_baudrate_Nr == i) strcat(HTML_String, "CHECKED");
        strcat(HTML_String, "><label for=\"JZ\"> ");
        strcati(HTML_String, baudrates[i]);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?BaudrUSB=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"baudrateUSB\" id=\"JZ2\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (Set.USB_baudrate_Nr == i) strcat(HTML_String, "CHECKED");
        strcat(HTML_String, "><label for=\"JZ2\"> ");
        strcati(HTML_String, baudrates[i]);
        strcat(HTML_String, "</label></td></tr>");
    }
    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">Click get u-blox baudrate to check and<b> wait 15 seconds and reload the page!</b></td> </tr>");
    strcat(HTML_String, "<tr><td></td><td><input type= \"button\" onclick= \"sendVal('/?GPSGetBaud=true');setTimeout(location.reload.bind(location), 7000);\" style= \"width:200px\" value=\"get u-blox baudrate\"></button></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");

    strcat(HTML_String, "</tr></table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><br>If it worked klick the Save button!<br><hr>");

    //-------------------------------------------------------------
    // Checkboxes debugmode
    strcat(HTML_String, "<h2>Debugmode</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    strcat(HTML_String, "<tr> <td colspan=\"3\">debugmode sends messages to USB serial at ");
    strcati(HTML_String, baudrates[Set.GPS_baudrate_Nr]);
    strcat(HTML_String, " baud </td>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type=\"checkbox\" onclick=\"sendVal('/?debugmode='+this.checked)\" name=\"debugmode\" id = \"Part\" value = \"1\" ");
    if (Set.debugmode == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode on</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td></tr>");
/*
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" onclick=\"sendVal('/?debugmUBX='+this.checked)\" name=\"debugmUBX\" id = \"Part\" value = \"1\" ");
    if (Set.debugmodeUBX == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode UBX on (good to check, if data from UBlox comes to ESP)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" onclick=\"sendVal('/?debugmHead='+this.checked)\" name=\"debugmHead\" id = \"Part\" value = \"1\" ");
    if (Set.debugmodeHeading == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode Heading on (see Antenna distance real and from GPS ->indicator of GPS signal quality)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" onclick=\"sendVal('/?debugmVirtAnt='+this.checked)\" name=\"debugmVirtAnt\" id = \"Part\" value = \"1\" ");
    if (Set.debugmodeVirtAnt == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode Virtual Antenna on (see, how position is moved by ESP)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" onclick=\"sendVal('/?debugmFiltPos='+this.checked)\" name=\"debugmFiltPos\" id = \"Part\" value = \"1\" ");
    if (Set.debugmodeFilterPos == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode Filter Position on (position is filtered on weak GPS signal)</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" onclick=\"sendVal('/?debugmNtrip='+this.checked)\" name=\"debugmNtrip\" id = \"Part\" value = \"1\" ");
    if (Set.debugmodeNTRIP == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode NTRIP client</label>");
    strcat(HTML_String, "</td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\"><input type=\"checkbox\" onclick=\"sendVal('/?debugmRAW='+this.checked)\" name=\"debugmRAW\" id = \"Part\" value = \"1\" ");
    if (Set.debugmodeRAW == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode RAW data on (sends lots of data as comma separated values)</label>");
    strcat(HTML_String, "</td></tr>");
 */        
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
    strcat(HTML_String, "<h2>WiFi/NTRIP LED light on</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?WiFiLEDon=0')\" name=\"WiFiLEDon\" id=\"JZ\" value=\"0\"");
    if (Set.LEDWiFi_ON_Level == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">LOW</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?WiFiLEDon=1')\" name=\"WiFiLEDon\" id=\"JZ\" value=\"1\"");
    if (Set.LEDWiFi_ON_Level == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">HIGH</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // GPIO PINs selection
/*
    strcat(HTML_String, "<h2>Hardware GPIO selection</h2>");
    strcat(HTML_String, "<br>");
    strcat(HTML_String, "<b>RX1 at ESP32 = TX1 at GPS board</b><br>");

    strcat(HTML_String, "<b>select # of GPIO pin at ESP32</b><br><br>");
    strcat(HTML_String, "<form>");    
    strcat(HTML_String, "<input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button><br>");
    strcat(HTML_String, "<table>");
    set_colgroup(100, 100, 100, 100, 150, 150);
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td><b>RX1</b></td>");
    strcat(HTML_String, "<td><b>TX1</b></td>");
    strcat(HTML_String, "<td><b>RX2</b> </td>");
    strcat(HTML_String, "<td><b>TX2</b></td>");
    strcat(HTML_String, "<td><b>pin for WiFi/NTRIP indication LED</b></td>");
    strcat(HTML_String, "<td><b>pin for WiFi rescan button</b></td>");
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
        if (Set.RX1 == i) { strcat(HTML_String, " CHECKED"); }
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
        if (Set.TX1 == i) { strcat(HTML_String, " CHECKED"); }
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
        if (Set.RX2 == i) { strcat(HTML_String, " CHECKED"); }
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
        if (Set.TX2 == i) { strcat(HTML_String, " CHECKED"); }
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
        if (Set.LEDWiFi_PIN == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?WiFiResc=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"WiFi rescan button\" id=\"GPIOWiFiResc\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (Set.Button_WiFi_rescan_PIN == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "</tr>");
    }
    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form><hr>");
*/

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
        ", 5000);"
        "}"
        "</script>"
        "</body>"
        "</html>";

	WiFi_Server.sendHeader("Connection", "close");
	WiFi_Server.send(200, "text/html", notFound);
	if (Set.debugmode) { Serial.println("redirecting from 404 not found to Webpage root"); }

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

void set_colgroup(int w1, int w2, int w3, int w4, int w5, int w6) {
    strcat(HTML_String, "<colgroup>");
    set_colgroup1(w1);
    set_colgroup1(w2);
    set_colgroup1(w3);
    set_colgroup1(w4);
    set_colgroup1(w5);
    set_colgroup1(w6);
    strcat(HTML_String, "</colgroup>");
}
void set_colgroup(int w1, int w2, int w3, int w4, int w5, int w6, int w7) {
    strcat(HTML_String, "<colgroup>");
    set_colgroup1(w1);
    set_colgroup1(w2);
    set_colgroup1(w3);
    set_colgroup1(w4);
    set_colgroup1(w5);
    set_colgroup1(w6);
    set_colgroup1(w7);
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

