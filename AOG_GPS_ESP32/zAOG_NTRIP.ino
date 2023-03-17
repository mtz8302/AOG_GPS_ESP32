// Ethernet NTRIP task code, NTRIP data comming from AgIO -----------------------------------------
/*
void Eth_NTRIP_Code(void* pvParameters) {
    Serial.println("started new task on core 0: Ethernet NTRIP");
    for (;;) { // MAIN LOOP
        if (Eth_connect_step == 0) {
            if (Set.NtripClientBy == 1) {
                task_Eth_NTRIP_running = true;
                if (doEthUDPNtrip() == 0) { vTaskDelay(15); }//no data
            }
            else {
                task_Eth_NTRIP_running = false;
                delay(1);
                vTaskDelete(NULL);
                delay(1);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------

int doEthUDPNtrip() {
    unsigned int EthNTRIPPacketLenght = Eth_udpNtrip.parsePacket();
    char Eth_NTRIP_packetBuffer[512];
    if (EthNTRIPPacketLenght) {
       // if (Set.debugmode) { 
            Serial.print("got NTRIP data via Ethernet lenght: "); Serial.println(EthNTRIPPacketLenght);
      //  }
        Eth_udpNtrip.read(Eth_NTRIP_packetBuffer, EthNTRIPPacketLenght);
        //Eth_udpNtrip.flush();
        Serial1.write(Eth_NTRIP_packetBuffer, EthNTRIPPacketLenght);
        NtripDataTime = millis();
    }  // end of Packet
    return EthNTRIPPacketLenght;
}

*/
//-------------------------------------------------------------------------------------------------
//main NTRIP client loop, running in separate task
/*
void NTRIP_Client_Code(void* pvParameters) {
    unsigned long WiFi_Ntrip_lost_time = 0;// now;

    for (;;) { // MAIN LOOP
        if ((Set.NtripClientBy == 2) && (WiFi_connect_step > 3)) {
            task_NTRIP_Client_running = true;
            if ((WiFi.status() == WL_CONNECTED) && (Ntrip_restart < 20)) { do_WiFi_NTRIP(); }
            else {
                if (my_WiFi_Mode == 0) {
                    task_NTRIP_Client_running = false;
                    delay(1);
                    Serial.println("closing NTRIP task: too many restarts");
                    vTaskDelete(NULL);
                }
                if (WiFi_Ntrip_lost_time == 0) { //first time                    
                    WiFi_connect_step = 1;                     
                    if (!task_WiFiConnectRunning) {//start WiFi
                        xTaskCreate(WiFi_handle_connection, "WiFiConnectHandle", 3072, NULL, 1, &taskHandle_WiFi_connect);
                        delay(200);
                    }
                    WiFi_Ntrip_lost_time = millis();
                }
                delay(1);
                now = millis();
                if (now > (WiFi_Ntrip_lost_time + 10000)) {
                    task_NTRIP_Client_running = false;
                    Serial.println("closing NTRIP task: no connection for 10 sek");
                    delay(1);
                    vTaskDelete(NULL);
                }
            }
        }
        else {
            task_NTRIP_Client_running = false;
            delay(1);
            vTaskDelete(NULL);
            Serial.println("closing NTRIP task");
        }
        vTaskDelay(2);//1
    }
}


//-------------------------------------------------------------------------------------------------


void do_WiFi_NTRIP() {
    //lifesign = millis();  //init timers 
    //NTRIP_GGA_send_lastTime = millis();  // 

    if (Ntrip_restart > 0) {
        if (Set.debugmodeNTRIP) { Serial.print("Requesting Sourcetable: "); }
        if (!getSourcetable()) Serial.print("SourceTable request error !!\n");
        if (Set.debugmodeNTRIP) { Serial.println("try starting RTCM stream !!!!"); }
        if (!startStream()) {
            Serial.print("Stream request error\n");
            Ntrip_restart++;
        }
        else {
            if (Set.debugmodeNTRIP) { Serial.println("RTCM stream started to serial1 (* = RTCM-Package,  G = GGA-sent)"); }
            Ntrip_restart = 0;
        }
    }
    if (!getRtcmData()) {
        Serial.print("\nCan not reach hoster, internet connection broken\n");
        vTaskDelay(5000);
        //delay(5000);
        Serial.print("\nTrying to reconnect\n");
        Ntrip_restart++;
    }
}


//-------------------------------------------------------------------------------------------------

bool getSourcetable() {

    if (!connectCaster()) {
        Serial.println("NTRIP Host connection failed");
        Serial.println("Can not connect to NTRIP Hoster");
        Serial.println("Check Network Name and Port");
        //vTaskDelay(2000 / portTICK_PERIOD_MS);
        
        vTaskDelay(2000);
        return false;
    }
    // This will send the request to the server
    WiFi_Ntrip_cl.print(String("GET /") + " HTTP/1.0\r\n" +
        "User-Agent: " + _userAgent + "\r\n" +
        "Accept: " + _accept + "\r\n" +
        "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (!WiFi_Ntrip_cl.available()) {
        if (millis() - timeout > 8000) {//5000
            Serial.print(">>> Client Timeout while requesting Sourcetable !!\n");
            Serial.print("Check Caster Name and Port !!\n");
            WiFi_Ntrip_cl.stop();
            return false;
        }
        vTaskDelay(1);
        //delay(1); //wdt 
    }

    String currentLine = readLine(); //reads to RTCM_strm_Buffer
    if (currentLine.startsWith("SOURCETABLE 200 OK")) {
        if (Set.debugmodeNTRIP) { Serial.println(currentLine); }
        vTaskDelay(5);
       // delay(5);
        for (int as = 0; as < 2; as++) {
            while (WiFi_Ntrip_cl.available()) {
                currentLine = WiFi_Ntrip_cl.readStringUntil('\n');
                if (Set.debugmodeNTRIP) { Serial.println(currentLine); }
                vTaskDelay(1);
                //delay(1);
            }
            vTaskDelay(100); // wait for additional Data
            //delay(100); // wait for additional Data
        }
        if (Set.debugmodeNTRIP) { Serial.print("---------------------------------------------------------------------------------------------\n"); }
        WiFi_Ntrip_cl.stop();
        return true;
    }
    else {
        if (Set.debugmodeNTRIP) { Serial.println(currentLine); }
        return false;
    }
}

//-------------------------------------------------------------------------------------------------

bool startStream() {
    // Reconnect for getting RTCM Stream
    if (!connectCaster()) {  //reconnect for stream
        Serial.print("NTRIP Host connection failed\n");
        Serial.print("Can not connect to NTRIP Hoster\n");
        Serial.print("Check Network Name and Port\n");
        vTaskDelay(2000);
        //delay(2000);
        return false;
    }

    // This will send the request to the server
    String requestMtp = (String("GET /") + Set.NtripMountpoint + " HTTP/1.0\r\n" +
        "User-Agent: " + _userAgent + "\r\n");
    if (strlen(Set.NtripUser) == 0) {
        requestMtp += (String("Accept: ") + _accept + "\r\n");
        requestMtp += String("Connection: close\r\n");
    }
    else {
        requestMtp += String("Authorization: Basic ");
        requestMtp += _base64Authorization;
        requestMtp += String("\r\n");
    }
    requestMtp += String("\r\n");

    WiFi_Ntrip_cl.print(requestMtp);
    
    if (Set.debugmodeNTRIP) { Serial.println("request Mountpoint"); Serial.print(requestMtp); }

    if (Set.NtripSendWhichGGASentence > 0) {
        sendGGA();  
    }

    vTaskDelay(10);
    //delay(10);

    unsigned long timeout = millis();
    while (!WiFi_Ntrip_cl.available()) {
        if (millis() - timeout > 20000) {//15000
            Serial.println(">>> Client Timeout - no response from host");
            WiFi_Ntrip_cl.stop();
            vTaskDelay(2000);
           // delay(2000);
            return false;
        }
        vTaskDelay(10);
        //delay(1); //wdt
    }
    vTaskDelay(5);
    //delay(5);
    String currentLine;
    if (!(currentLine = readLine())) return false; //read answer 
    if (!(currentLine.startsWith("ICY 200 OK"))) {
        Serial.print("Received Error: ");
        Serial.println(currentLine);
        return false;
    }
    cnt = 0; // reset counter
    return true;
}

//-------------------------------------------------------------------------------------------------

bool getRtcmData() {
    long timeout = millis();
    while (WiFi_Ntrip_cl.available() == 0) {
        if (millis() - timeout > 7000) {//5000
            Serial.println(">>> Client Timeout no RTCM Respond from Caster");
            if ((GGAdigit < 50) && (Set.NtripSendWhichGGASentence > 1)) Serial.println("Invalid NMEA String from GPS");
            if (Set.NtripSendWhichGGASentence == 1) Serial.println("Maybe invalid fixed NMEA String");
            if (Set.NtripSendWhichGGASentence == 0) Serial.println("Check if your NTRIP Provider requires your Position");
            return false;
        }
        vTaskDelay(10);//2
        //delay(1);
    }

    // Read all the bytes of the reply from server and print them to Serial
    while (WiFi_Ntrip_cl.available()) {
        char a = WiFi_Ntrip_cl.read();
        Serial1.print(a);//send to F9P
    }
    vTaskDelay(5);//1
    //delay(1);
    //Serial1.println();
    now = millis();
    if (Set.NtripSendWhichGGASentence > 0) {
        if (now - NTRIP_GGA_send_lastTime > (Set.NtripGGASendRate * 1000)) {
            sendGGA();
            NTRIP_GGA_send_lastTime = now;
        }
        if ((Set.debugmode)||(Set.debugmodeNTRIP)) {
            if (now - lifesign > 1000) {
                Serial.print("*"); // Sectic - Data receiving
                if (cnt++ >= 59) {
                    Serial.println(); //NL
                    cnt = 0;
                }
                lifesign = now;
            }
        }
        NtripDataTime = millis();  //LED WiFi Status timer  
    }
    return true;
}

//-------------------------------------------------------------------------------------------------

bool connectCaster(){

  setAuthorization(Set.NtripUser, Set.NtripPassword); // Create Auth-Code
 
  return WiFi_Ntrip_cl.connect(Set.NtripHost, Set.NtripPort);
}

//-------------------------------------------------------------------------------------------------

char* readLine(){
  int i=0;
  // Read a line of the reply from server and print them to Serial
  //Serial.println("start Line read: ");  
  while(WiFi_Ntrip_cl.available()) {
    RTCM_strm_Buffer[i] = WiFi_Ntrip_cl.read();
    //Serial.print(RTCM_strm_Buffer[i]);
    if( RTCM_strm_Buffer[i] == '\n' || i>=511) break;
    i++;            
  }
  RTCM_strm_Buffer[i]= '\0';  //terminate string
  return (char*)RTCM_strm_Buffer;
  
}

//-------------------------------------------------------------------------------------------------

void sendGGA() {
    if ((Set.debugmode)||(Set.debugmodeNTRIP)) { Serial.print("G"); }
    // This will send the Position to the server, required by VRS - Virtual Reference Station - Systems
    if (Set.NtripSendWhichGGASentence == 1) {
        WiFi_Ntrip_cl.print(Set.NtripFixGGASentence);
        WiFi_Ntrip_cl.print("\r\n");
    }
    if (Set.NtripSendWhichGGASentence > 1) {
        if (Set.sendGGA == 0) { buildGGA(); }
        char GGA_chr[80];
      
        for (byte n = 0; n < GGAdigit; n++) {
            GGA_chr[n] = char(GGABuffer[n]);
        }
        WiFi_Ntrip_cl.print(GGA_chr);
        WiFi_Ntrip_cl.print("\r\n");
    }
    NTRIP_GGA_send_lastTime = millis();
}
    
//-------------------------------------------------------------------------------------------------

void setAuthorization(const char* user, const char* password)
{
       if(user && password) {
           String auth = user;
           auth += ":";
           auth += password;
           _base64Authorization = base64::encode(auth);
       }
}

*/