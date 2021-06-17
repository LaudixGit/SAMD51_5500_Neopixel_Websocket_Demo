
// this file has the code to setup and run the webserver.
// it seperated out to make the main code easier to read

#ifndef webserver_h
  #define webserver_h

/*
 * This section for reading the 24AA02E48 to get the MAC addresss
 *          
 */
#include <Wire.h>
#include <Ethernet.h>
#include "index_html.h"

#include <EthernetWebServer.h>
#define ETHERNET_PIN 10
#define WEBSERVER_PORT 80
#define maxSendBufferSize 2048   //The NIC, W5x00, can only send 2050 chars at a time
EthernetWebServer webserver(WEBSERVER_PORT);

#define NULLTERMINATOR '\0'

// I2C address of the 24AA02E48, this chip provides the built-in MAC address
#define MAC_I2C_ADDRESS 0x50

// A temporary MAC Address- until the built-in MAC is read
static uint8_t mac[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 

byte readRegister(byte r) {
  // this is the original function from the example
  unsigned char v;
  Wire.beginTransmission(MAC_I2C_ADDRESS);
  Wire.write(r);  // Register to read
  Wire.endTransmission();

  Wire.requestFrom(MAC_I2C_ADDRESS, 1); // Read a byte
  while(!Wire.available())
  {
    // Wait
  }
  v = Wire.read();
  return v;
} 

void mac_update() {
  // read the registers and update the global variable
  mac[0] = readRegister(0xFA);
  mac[1] = readRegister(0xFB);
  mac[2] = readRegister(0xFC);
  mac[3] = readRegister(0xFD);
  mac[4] = readRegister(0xFE);
  mac[5] = readRegister(0xFF);
}

// in order to display the MAC address, go through the array and add two characters (for each nibble) to the string buffer.
String mac_to_string(unsigned char mac[]){
  String hexstring = "";
  for(int i = 0; i < 6; i++) {
    if(mac[i] < 0x10) {
      hexstring += '0';
    }

    hexstring += String(mac[i], HEX);
  }
  return hexstring;
}

void mac_setup() {
  // these steps originally in the setup() function
    Wire.begin();  // Join i2c bus (I2C address is optional for the master)
    mac_update();  // read MAC
}


/*
 * Here is the code to setup and run the webserver
 */
void handleArray(int code, char* content_type, const char* arHTML) {
  //this sends a large page in small chunks
  //since the html library is limited to a 2K buffer
  int bufferSize = 40;
  char outMsg[bufferSize];
  EthernetClient  client = webserver.client(); //https://github.com/khoih-prog/EthernetWebServer_SSL/blob/main/src/EthernetWebServer_SSL.h
  snprintf(outMsg, bufferSize - 1, "HTTP/1.1 %d", code);
  client.println(outMsg);
  snprintf(outMsg, bufferSize - 1, "Content-Type:  %s", content_type);
  client.println(outMsg);
  client.println(F("Connection: close"));  // the connection will be closed after completion of the response
  client.println();
  int chunkStart = 0;
  char chunk[maxSendBufferSize];   // to hold the part of the array to send
  while (chunkStart < strlen(arHTML)) {
    strncpy ( chunk, &arHTML[chunkStart], maxSendBufferSize );
    chunk[maxSendBufferSize] = NULLTERMINATOR;  // ensure null terminated
    client.print(chunk);
    chunkStart += maxSendBufferSize;
  }
  client.stop();
}

void handleIndex() {
  handleArray(200, "text/html", INDEX_HTML);
}

void webserver_setup() {
  // these steps originally in the setup() function
  mac_setup();  //initialize the MAC address
  
  // start the Ethernet connection and the server:
  Ethernet.init (ETHERNET_PIN);
  Ethernet.begin(mac);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware"));
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println(F("Ethernet cable is not connected."));
  }

  // start the webserver
  webserver.on(F("/"), handleIndex);
  webserver.onNotFound([](){webserver.send(404, F("text/plain"), F("Page not found"));  });
  webserver.begin();

}

#endif      //webserver_h
