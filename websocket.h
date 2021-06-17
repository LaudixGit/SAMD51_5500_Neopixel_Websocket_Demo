
// this file has the code to setup and run the websocket server.
// it seperated out to make the main code easier to read

#ifndef websocket_h
  #define websocket_h

#include "neopixel.h"  //used to set the neopixel in response to socket messages

#define _DEBUG
#include <WebSocketServer.h>
using namespace net;

#define SOCKETSERVER_PORT 8080
WebSocketServer socketServer(SOCKETSERVER_PORT);

#include <ArduinoJson.h>
#define JSON_DOC_SIZE 512

void neopixels_config_send(){
  //wrap the neopixel class into a json object and send to all attached websockets clients

  //count the connected clients
  // if none connected skip this function
  //Serial.print(F("connected clients: ")); Serial.println(socketServer.countClients());
  if (socketServer.countClients() > 0){ 
    String jsonStr = neopixelLED.json();
    int str_len = jsonStr.length() + 1; 
    char jsonChar[str_len];
    jsonStr.toCharArray(jsonChar, str_len); 
    socketServer.broadcast(WebSocket::DataType::TEXT, jsonChar, str_len-1);
    //Serial.println(jsonChar);
  }
}


bool socketServer_processTextMessage (WebSocket &client, const char *message, uint16_t length) {
  //return true if known message was processed
  // https://wandbox.org/permlink/Dep2QennMdQQ1cqC
  StaticJsonDocument<512> incomingJsonDoc;

  DeserializationError err = deserializeJson(incomingJsonDoc, message);
  switch (err.code()) {
      case DeserializationError::Ok:
//          Serial.println(F("Deserialization succeeded"));
//          Serial.print("set \"StaticJsonDocument<???> settingsJson;\" to :");
//          Serial.println(incomingJsonDoc.memoryUsage());  //Use this function at design time to measure the required capacity for the JsonDocument.
          break;
      case DeserializationError::InvalidInput:
//          Serial.println(F("Invalid json input!"));
          break;
      case DeserializationError::NoMemory:
          Serial.println(F("Not enough memory"));
          break;
      default:
          Serial.println(F("Deserialization failed"));
          break;
  }

  if (incomingJsonDoc.isNull()) {
    // incoming data is NOT json
    char  outMsg[12]="";
    if (strcmp(message, "syn") == 0) {
      //reply with acknowledgement (not required since the websocket protocol uses builtin ping/pong for keep alive
      strcpy(outMsg, "ack");
      client.send(WebSocket::DataType::TEXT, outMsg, strlen(outMsg));
    } else {
      // Echo message
      //client.send("Unexpected message: " + data);
      strcpy(outMsg, "Unexpected message: ");
      strcat(outMsg,message);
      client.send(WebSocket::DataType::TEXT, outMsg, strlen(outMsg));
    }
  } else {
    // incoming data IS json
//    serializeJsonPretty(incomingJsonDoc, Serial);
//    Serial.println("");

//    for (JsonPair kvPair : incomingJsonDoc.as<JsonObject>()) {
//      Serial.println(kvPair.key().c_str());
//    }

    JsonObject incomingJsonObjNested = incomingJsonDoc["NeoPixel"];
//    for (JsonPair kvPair : incomingJsonObjNested) {
//      // https://arduinojson.org/v6/api/jsonobject/begin_end/
//      // https://arduinojson.org/v5/doc/tricks/
//      Serial.print(kvPair.key().c_str());
//      Serial.print(": ");
//      Serial.println(kvPair.value().as<char*>());
//      Serial.println(kvPair.value().as<bool>());
//
    neopixelLED.Update(
      incomingJsonObjNested["Pattern"].as<char*>(),
      strtoul((incomingJsonObjNested["Color1"].as<char*>())+1, NULL, 16),  //increment pointer to 2nd char in array to skip the "#"
      strtoul((incomingJsonObjNested["Color2"].as<char*>())+1, NULL, 16),
      incomingJsonObjNested["Steps"].as<int>(),
      incomingJsonObjNested["Interval"].as<int>(),
      incomingJsonObjNested["Forward"].as<bool>(),
      incomingJsonObjNested["Repeat"].as<char*>()
      );
    neopixels_config_send(); //once all settings are updated, inform the websocket clients
  }
}


void socketServer_onMessage(WebSocket &ws, const WebSocket::DataType &dataType, const char *message, uint16_t length) {
  //process incoming messages
      switch (dataType) {
      case WebSocket::DataType::TEXT:
        if (socketServer_processTextMessage (ws, message, length)) {
        } else {
          // message was NOT processed  
          Serial.print(F("Received unknown message: "));
          Serial.println(message);
        }
        break;
      case WebSocket::DataType::BINARY:
        Serial.println(F("Received binary data"));
        break;
      }
//      ws.send(dataType, message, length);
}

void socketServer_onClose(WebSocket &ws, const WebSocket::CloseCode &code, const char *reason, uint16_t length) {
  Serial.println(F("Disconnected"));
}

// this was in the setup() function
void socketServer_setup( void ){
  socketServer.onConnection([](WebSocket &ws) {
    ws.onMessage(socketServer_onMessage);
    ws.onClose(socketServer_onClose);

    Serial.print(F("New client: "));
    Serial.println(ws.getRemoteIP());

    const char message[]{ "Hello from SolarController server!" };
    ws.send(WebSocket::DataType::TEXT, message, strlen(message));
    neopixels_config_send(); //inform the websocket clients of settings
  });

  // Start websockets server.
  socketServer.begin();
}


#endif      //websocket_h
