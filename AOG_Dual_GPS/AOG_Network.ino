#if useWiFi
//---------------------------------------------------------------------
// start WiFi in Workstation mode = log to existing WiFi

void WiFi_Start_STA() {
  unsigned long timeout;

  WiFi.mode(WIFI_STA);   //  Workstation
  
  /*if (!WiFi.config(GPSSet.myIP, GPSSet.gwIP, GPSSet.mask, GPSSet.myDNS))
    {Serial.println("STA Failed to configure\n"); }
    else { Serial.println("trying to connect to WiFi"); }
  */
  WiFi.begin(GPSSet.ssid, GPSSet.password);
  timeout = millis() + (GPSSet.timeoutRouter * 1000);
  Serial.print("try to connect to WiFi: "); Serial.println(GPSSet.ssid);
  while (WiFi.status() != WL_CONNECTED && millis() < timeout) {
    delay(50);
    Serial.print(".");
    //WIFI LED blink in double time while connecting
    if (!LED_WIFI_ON) {
        if (millis() > (LED_WIFI_time + (LED_WIFI_pause >> 2))) 
          {
           LED_WIFI_time = millis();
           LED_WIFI_ON = true;
           digitalWrite(GPSSet.LED_PIN_WIFI, !GPSSet.WIFI_LED_ON);
          }
    }
    if (LED_WIFI_ON) {
      if (millis() > (LED_WIFI_time + (LED_WIFI_pulse >> 2))) {
        LED_WIFI_time = millis();
        LED_WIFI_ON = false;
        digitalWrite(GPSSet.LED_PIN_WIFI, GPSSet.WIFI_LED_ON);
      }
    }
  }  //connected or timeout  
  
  Serial.println(""); //NL
  if (WiFi.status() == WL_CONNECTED)
  {
      delay(200);
      Serial.print("Connected IP - Address : ");
      IPAddress myIP = WiFi.localIP();
      Serial.println(myIP);
      myIP[3] = myIPEnding; //set ESP32 IP to x.x.x.myIP_ending
      Serial.print("changing IP to: ");
      Serial.println(myIP);
      IPAddress gwIP = WiFi.gatewayIP();
      if (!WiFi.config(myIP, gwIP, GPSSet.mask, GPSSet.myDNS))
      {
          Serial.println("STA Failed to configure");
      }
      delay(300);
      Serial.print("WiFi Client successfully connected to : ");
      Serial.println(GPSSet.ssid);
      Serial.print("Connected IP - Address : ");
      myIP = WiFi.localIP();
      Serial.println(myIP);
      IPToAOG = myIP;
      IPToAOG[3] = 255;//set IP to x.x.x.255 according to actual network
      LED_WIFI_ON = true;
      digitalWrite(GPSSet.LED_PIN_WIFI, GPSSet.WIFI_LED_ON);
      my_WiFi_Mode = WIFI_STA;
  }
  else
  {
      WiFi.mode(WIFI_OFF);
      Serial.println("WLAN-Client-Connection failed");
      Serial.println();
      LED_WIFI_ON = false;
      digitalWrite(GPSSet.LED_PIN_WIFI, !GPSSet.WIFI_LED_ON);
  }
  delay(20);
}


//---------------------------------------------------------------------
// start WiFi Access Point = only if no existing WiFi

void WiFi_Start_AP() {
  WiFi.mode(WIFI_AP);   // Accesspoint
  WiFi.softAP(GPSSet.ssid_ap, GPSSet.password_ap);
  while (!SYSTEM_EVENT_AP_START) // wait until AP has started
   {
    delay(100);
    Serial.print(".");
   }   
  delay(100);//right IP adress only with this delay 
  WiFi.softAPConfig(GPSSet.gwIP, GPSSet.gwIP, GPSSet.mask);  // set fix IP for AP  

  IPAddress getmyIP = WiFi.softAPIP();
  delay(300);
  //server.begin();
  //delay(300);

  //AP_time = millis();
  Serial.print("Accesspoint started - Name : ");
  Serial.println(GPSSet.ssid_ap);
  Serial.print( " IP address: ");
  Serial.println(getmyIP);
  LED_WIFI_ON = true;
  digitalWrite(GPSSet.LED_PIN_WIFI, GPSSet.WIFI_LED_ON);
  my_WiFi_Mode = WIFI_AP;
}

//-------------------------------------------------------------------------------------------------

void doUDPNtrip() {
	udpNtrip.onPacket([](AsyncUDPPacket packet)
		{
			if (debugmode) { Serial.println("got NTRIP data"); }
			for (int i = 0; i < packet.length(); i++)
			{
				Serial1.write(packet.data()[i]);
			}
			NtripDataTime = millis();
		});  // end of onPacket call
	if ((NtripDataTime + 30000) < millis())
	{
		NtripDataTime = millis();
		Serial.println("no NTRIP from AOG for more than 30s");
	}

}

#endif