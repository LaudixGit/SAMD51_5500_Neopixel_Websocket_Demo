/*
 * A non-SSL webserver to change color patterns of a neopixel LED
 * Allows up to to 3 simultaneous clients.
 * 
 * Note: many alternatives available in the original examples have been deleted for clarity
 * but that leaves THIS file only compatible with 
 *  BOARD_NAME    "SAMD51 ADAFRUIT_FEATHER_M4_EXPRESS" https://www.adafruit.com/product/3857
 *  ETHERNET_TYPE "W5x00 and EthernetLarge Library"    https://www.amazon.com/gp/product/B08KTVD7BR
 *  
 * 20210612 Initial file copied from example "M4-simple-server_socketAndWeb"
 *           Uses the Arduino Ethernet and mWebSockets libraries. https://github.com/skaarj1989/mWebSockets
 *           Webserver provided through https://github.com/khoih-prog/EthernetWebServer
 *           create handleArray to send html in small enough chunks
 *           split into seperate files for ino, webserver, and websocket.
 *           add neopixel function (ESP8266_WifiLEDs_AdvancedWebServer, M4-builtin_neopixel_test)
 *           Addneopixel controlls to INDEX page
 * 20210613 convert (to JSON) communications to server
 * 20210614 send neopixel config to clients as Json
 * 20210615 Update to v 2.3.3 of https://jscolor.com/
 *          add neopixelLED.update
 * 20210617 Convert neopixelLED.direction to boolean (instead of enum)
 *          add neopixel.RepeatPattern 
 *          repair the reverse functionality
 *          settings sent when client connects
 * 
 * 
 * Arduino Ethernet library https://www.arduino.cc/en/reference/ethernet
 * The 5500 can only have up to 8 sockets  https://www.wiznet.io/product-item/w5500/
 * overview: https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_server
 */

#include "webserver.h"
#include "websocket.h"
#include "neopixel.h"

//from https://www.seeedstudio.com/blog/2021/04/26/managing-arduino-memory-flash-sram-eeprom/
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}


void setup() {
  Serial.begin(115200);
  long serialInterval = 10000;   // interval to wait
  unsigned long previous = millis();   // hold current 'time'
  while (!Serial) {
    ; // wait for serial port to connect. Needed for built-in USB port only
    if (millis() - previous >= serialInterval) {
      // exceeded the allowed wait-time; give up and don't use the serial communications
      break;
    } else {
      delay(10);
    }
  }
  delay(250);  //it seems that occassionally serial is ready a few moments before it can be written to
  Serial.println();

  Serial.println(F("Initializing ... "));

  webserver_setup();
  Serial.print(F("webserver is at ")); Serial.println(Ethernet.localIP());
  Serial.print(F("MAC Address: ")); Serial.println(mac_to_string(mac));


  socketServer_setup();  //initialize the websocket server
  Serial.print(F("WebSocket Server Running and Ready on IP address: "));
  Serial.print(Ethernet.localIP());
  Serial.print(F(":"));
  Serial.println(SOCKETSERVER_PORT);    // Websockets Server Port

  neopixel_setup();   //initialize NeoPixel

}

void loop() { 
    webserver.handleClient();
    socketServer.listen(); 
    //Serial.print(F("Client Count: ")); Serial.println(socketServer.countClients());
    neopixelLED.Update();
}
