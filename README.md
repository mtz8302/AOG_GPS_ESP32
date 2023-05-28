# AOG_GPS_ESP32 PANDA
# only Single GPS supported!!
Bridge between GPS receiver and AgOpenGPS: transmit GPS Signal via WiFi, Ethernet or USB.

for ESP32

you can use WiFi connection, cable Ethernet or USB

sends $PANDA or $GGA, $VTG and IMU PGN to AgOpenGPS
 
you can change the settings via webinterface: x.x.x.79
when access to your network fails, an accesspoint is created. Webinterface: 192.168.137.1


Config via UCenter:
First set in PRT tab UART1 speed to 115200. Uncheck all "enable" in GNSS tab exept of GPS, Galileo, Glonass (only 3!). Then set Nav Rate to 100ms (10Hz).
In the messages I deactivated everything expect of (NMEA) GGA and VTG on UART1.

Don’t forget to save the config!

Wireing the receiver boards:
Connect RX1 and TX1 from both boards to ESP32 GPIOs (not #6-12) (IOref to 3.3V!!!)

Also see UBlox PDFs

# !!When using a new Version, set EEPROM_clear = true; (line 115) - flash - boot reset to EEPROM_clear = false; and flash again to reset settings!!
