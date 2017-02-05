(note instructions written for Windows, but may equally well be used for other O/S)


`1.`  Download and install Arduino IDE 1.6.x from https://www.arduino.cc/en/Main/Software

`2.`  Install ESP8266 Arduino support https://github.com/esp8266/Arduino#installing-with-boards-manager

`3.`  Install the following libraries:

`3.1:`  WifiManager: https://github.com/tzapu/WiFiManager#install-through-library-manager

`3.2:`  arduinoWebSockets: https://github.com/Links2004/arduinoWebSockets

`4.` Replace the provided WifiManager.h file from the WifiManager library, with the new template from https://github.com/openhardwarecoza/LaserWeb3/blob/master/ESP8266%20Wifi%20Bridge/WiFiManager.h (For Emblaser/Darklylabs theme) 

`5.` Configure your ESP8266 for sketch upload (GIPOs pulled up and down accordingly, USB to serial connected, reset and ready for upload)

`6.`  Upload the Sketch https://github.com/openhardwarecoza/LaserWeb3/blob/master/ESP8266%20Wifi%20Bridge/websocketserver.ino/websocketserver.ino.ino

`7.`  Connect the TX of the ESP8266 to RX of the Smoothie, and RX of the ESP8266 to TX of the Smoothie. Power up the ESP and Smoothie

`8.`  Connect to the Emblaser2 AP to connect the ESP to your local Wifi, then switch back to your local wifi (Animation below shows the details)

`9.`  Update LaserWeb to the version released on 12 October 2016 or newer

`10.` In LaserWeb, set it to ESP8266, and connect to the IP of the ESP (Note, I will add an IP scanner soon. For now, check on your DHCP server which IP was dished out)

![Setting Up Wifi](wifibridge.gif)


NOTE: To reset the Wifi settings you can:
- Connect GPIO0 of the ESP8266 to GND (for 1-2s) or 
- If WiFi is still connected: Send `resetWiFi` via websocket
