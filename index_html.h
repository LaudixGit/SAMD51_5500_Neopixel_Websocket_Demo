
// this file has the default/root page content.
// it seperated out to make the main code easier to read
// https://www.fileformat.info/info/unicode/char/search.htm
// https://jscolor.com/docs/#doc-configuring-instance

#ifndef index_html_h
  #define index_html_h

static const char PROGMEM INDEX_HTML[] = R"rawliteral(

<!DOCTYPE html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>SAMD51 Websocket Test</title>
    <link rel="icon" href="data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 100 100%22><text y=%22.9em%22 font-size=%2290%22>&#x1f58d;</text></svg>">    

    <script src="https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.3.3/jscolor.min.js"></script>

    <script type="text/javascript">
      var websock;
      
      window.onload = function () {
        startWebsocket();
      }

      function startWebsocket() {
        console.log('Starting Websocket');
        websock = new WebSocket('ws://' + window.location.hostname + ':8080/');
        websock.onopen = function(evt) { 
          console.log('websock open'); 
          websock.send("syn");
          };
        websock.onclose = function(evt) { 
          console.log('websock close');
          document.getElementById("banner").style.color = "red";
          };
        websock.onerror = function(evt) { console.log(evt); };
        websock.onmessage = processMessage;
      }

      function processMessage(evt) {
          try {
            jsonCurrent = JSON.parse(evt.data);
            jsonValid = true;
          } catch (e) {
            //invalid JSON
            jsonValid = false;
          }

        if (jsonValid) {
          //process all known elements
          for (var key in jsonCurrent){
            var value = jsonCurrent[key];
//            console.log(key + ": " + value);

            if (key == "NeoPixel") { processNeoPixel(jsonCurrent);}
            else { console.log( "Unknown JSON object: ", key, "\n", jsonCurrent );}
          }
        } else {
          //event wasn't json maybe it's something else interesting
          var evtData = evt.data;  // variable for convenience
          if (evtData == "ack") {
            //server has acknowledged connection
            document.getElementById("banner").style.color = "green";
          } else {
            // not an event we are looking for
//            console.log('unknown event\n', evt, "\n", evt.data);
          }
        }
          
      }

      function processNeoPixel (jsonDoc) {
        //extract from the received JSON and update page
//        console.log(jsonDoc);
        document.getElementById("pattern").value            = jsonDoc["NeoPixel"]["Pattern"];
        document.getElementById("interval").value           = jsonDoc["NeoPixel"]["Interval"];
        document.getElementById("intervalOutput").innerHTML = jsonDoc["NeoPixel"]["Interval"];
        document.getElementById("steps").value              = jsonDoc["NeoPixel"]["Steps"];
        document.getElementById("stepsOutput").innerHTML    = jsonDoc["NeoPixel"]["Steps"];
        document.getElementById("forward").checked          = jsonDoc["NeoPixel"]["Forward"];
        document.getElementById("repeat").value             = jsonDoc["NeoPixel"]["Repeat"];

        document.querySelector("#color1").jscolor.fromString((jsonDoc["NeoPixel"]["Color1"]));
        document.querySelector("#color2").jscolor.fromString(jsonDoc["NeoPixel"]["Color2"]);
      }

      function neopixelSend() {
        var jsonOut = {"NeoPixel": {}};   //holds the structure to send
        jsonOut["NeoPixel"]["Pattern"]=document.getElementById('pattern').value;
        jsonOut["NeoPixel"]["Interval"]=document.getElementById('interval').value;
        jsonOut["NeoPixel"]["Steps"]=document.getElementById('steps').value;
        jsonOut["NeoPixel"]["Forward"]=document.getElementById('forward').checked;
        jsonOut["NeoPixel"]["Repeat"]=document.getElementById('repeat').value;

        jsonOut["NeoPixel"]["Color1"]=document.querySelector('#color1').jscolor.toHEXString();
        jsonOut["NeoPixel"]["Color2"]=document.querySelector('#color2').jscolor.toHEXString();
        
        websock.send(JSON.stringify(jsonOut));
      }
            
      function disconnectClick(e) {
        console.log('goodbye ', e.id);
        websock.close();
      }

    </script>
  </head>
  <body>
    <h2 id="banner">NEOPIXEL</h2>
    <div style="text-align: right;"> <button id="disconnect" type="button" onclick="disconnectClick(this);" title="end the websocket connection">Disconnect</button> </div>
    <div>
      <button id="submit1" type="button" onclick="neopixelSend();" title="submit changes">&#x1f5ab;</button></br>
      <label for="pattern" title="What pattern to display">Pattern:</label>
      <select id="pattern">
        <option value="OFF">OFF</option>
        <option value="ON">ON</option>
        <option value="RAINBOW_CYCLE">RAINBOW</option>
        <option value="FADE">FADE</option>
      </select></br>
      Color1: <input id="color1" name="color1" data-jscolor="{alphaChannel:false}" ></br>
      Color2: <input id="color2" data-jscolor="{alphaChannel:false}" ></br>
      <label for="interval" Title="Duration (ms) between steps">Interval: </label><input type="range" id="interval" min="0" max="255" oninput="document.getElementById('intervalOutput').innerHTML = this.value;">(<span id="intervalOutput"></span>)</br>
      <label for="steps" Title="Steps of the pattern">Steps: </label><input type="range" id="steps" min="0" max="255" oninput="document.getElementById('stepsOutput').innerHTML = this.value;">(<span id="stepsOutput"></span>)</br>
      <label for="forward" Title="Off = reverse">Forward: </label><input type="checkbox" id="forward"></br>
      <label for="repeat" title="What to do when the pattern finishes">Repeat:</label>
      <select id="repeat">
        <option value="STOP">STOP</option>
        <option value="TOGGLE">TOGGLE</option>
        <option value="REVERSE">REVERSE</option>
        <option value="FORWARD">FORWARD</option>
      </select></br>
      <button id="submit2" type="button" onclick="neopixelSend();" title="submit changes">&#x1f5ab;</button></br>
    </div>
  </body>
<html>

)rawliteral";


#endif      //index_html_h
