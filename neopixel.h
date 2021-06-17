
// this file has the code to setup and run the webserver.
// it seperated out to make the main code easier to read

// class: https://learn.adafruit.com/multi-tasking-the-arduino-part-3
// derived from: https://gist.github.com/oesterle/69e4ebb0a5591bbf586564ee1dc72b58

#ifndef neopixel_h
  #define neopixel_h
  
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

// Adafruit Feather M4 Express uses pin 8 for its built-in  NeoPixel.
#define NEOPIXEL_PIN 8

// Pattern types supported: (remember to update the ActivePatternName function when assing a new pattern)
enum  pattern { OFF, ON, RAINBOW_CYCLE, COLOR_WIPE, FADE };
// what to do when the pattern completes (remember to update the RepeatPatternName function)
enum  repeatPattern { STOP, TOGGLE, REVERSE, FORWARD};


// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:

    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    bool Forward;            // direction to run the pattern false=reverse
    repeatPattern  Repeat;   // what to do when the pattern complete
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    uint8_t MaxBright=3;  // maxBright is from 0 to 255; low numbers are dimmer.
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case OFF:
                    OffUpdate();
                    break;
                case ON:
                    OnUpdate();
                    break;
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }
  
    // Increment the Index and reset at the end
    void Increment()
    {
        if (Forward)
        {
            Index++;
            if (Index >= TotalSteps)
            {
              //to avoid bouncing back and forth, do not reset the index if in toggle mode
              if (Repeat != TOGGLE) { Index = 1; }
              if (OnComplete != NULL)
              {
                  OnComplete(); // call the comlpetion callback
              }
            } else if (Index <= 0) {
                Index = TotalSteps-1;
            }
        }
        else // Direction == REVERSE
        {
            Index--;
          if (Index <= 1)
            {
              //to avoid bouncing back and forth, do not reset the index if in toggle mode
              if (Repeat != TOGGLE) { Index = TotalSteps-1; }
              if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            } else if (Index >= TotalSteps) {
                Index = 1;
            }
        }
    }
    
    // Flip pattern direction
    void ToggleDirection()
    {
      Forward =!Forward;
    }

    // enforce pattern direction
    void SetDirection(bool dir)
    {
      Forward = dir;
    }

    //return the name of the enum currently active
    String ActivePatternName(){
      switch(ActivePattern)
      {
          case OFF:
              return "OFF";
              break;
          case ON:
              return "ON";
              break;
          case RAINBOW_CYCLE:
              return "RAINBOW_CYCLE";
              break;
          case COLOR_WIPE:
              return "COLOR_WIPE";
              break;
          case FADE:
              return "FADE";
              break;
          default:
              break;
      }
    }
        //return the name of the repeat enum
    String RepeatPatternName(){
      switch(Repeat)
      {
          case STOP:
              return "STOP";
              break;
          case TOGGLE:
              return "TOGGLE";
              break;
          case REVERSE:
              return "REVERSE";
              break;
          case FORWARD:
              return "FORWARD";
              break;
          default:
              break;
      }
    }
    
    //return the enum matching the passed name
    pattern ActivePatternId(const char *ActivePatternName){
      if (!strcmp(ActivePatternName, "OFF")) { return OFF;
      } else if (!strcmp(ActivePatternName, "ON")) { return ON;
      } else if (!strcmp(ActivePatternName, "RAINBOW_CYCLE")) { return RAINBOW_CYCLE;
      } else if (!strcmp(ActivePatternName, "COLOR_WIPE")) { return COLOR_WIPE;
      } else if (!strcmp(ActivePatternName, "FADE")) { return FADE;
      }
    }
    
    //return the enum matching the passed repeat name
    repeatPattern RepeatPatternID(const char *RepeatPatternName){
      if (!strcmp(RepeatPatternName, "STOP")) { return STOP;
      } else if (!strcmp(RepeatPatternName, "TOGGLE")) { return TOGGLE;
      } else if (!strcmp(RepeatPatternName, "REVERSE")) { return REVERSE;
      } else if (!strcmp(RepeatPatternName, "FORWARD")) { return FORWARD;
      }
    }

    //expose the Color function 
    uint32_t xColor(uint8_t rd, uint8_t gr, uint8_t bl) { return Color(rd, gr, bl); }

    //a function to update all the variables
    void Update(const char *ActivePatternName, uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, bool dir, const char *RepeatPatternName){
      ActivePattern = ActivePatternId(ActivePatternName);
      Color1 = color1;
      Color2 = color2;
      Interval = interval;
      TotalSteps = steps;
      Index = 0;
      Forward = dir ;
      Repeat = RepeatPatternID(RepeatPatternName);
    }
    
    // Initialize for turning OFF the LED
    void Off()
    {
        ActivePattern = OFF;
        fill(Color(0, 0, 0)); //set all Pixels to off/black
        show(); //this sets to color above (black)
        show(); //the second turns off the LEDs
    }
    
    // Update the OFF Pattern
    void OffUpdate()
    {
      //ensure that the pattern is applied even if initialize function is skipped over
      // but only write once
      if (Index == 0) {
        fill(Color(0, 0, 0)); //set all Pixels to off/black
        show(); //this sets to color above (black)
        show(); //the second turns off the LEDs
        Index++;  //is now nono-zero
      }
      Increment();
    }

    // Initialize for turning ON the LED
    void On(uint32_t color)
    {
        Color1 = color;
        ActivePattern = ON;
        fill(Color1); //set all Pixels to primary color
        show();
    }
    
    // Update the ON Pattern
    void OnUpdate()
    {
      //ensure that the pattern is applied even if initialize function is skipped over
      // but only write once
      if (Index == 0) {
        fill(Color1); //set all Pixels to primary color
        show();
      }
      Increment();
    }

    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, bool frwrd = true)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Forward - frwrd; 
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, bool frwrd = true)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Forward = frwrd;
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }

    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, bool frwrd = true)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Forward = frwrd;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        fill(Color(red, green, blue));
        show();
        Increment();
    }
   
    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        uint16_t r = 0, g = 0, b = 0;
        
        if(WheelPos < 85) {
          r = 255 - WheelPos * 3;
          b = WheelPos * 3;
        } else if(WheelPos < 170) {
          WheelPos -= 85;
          g = WheelPos * 3;
          b = 255 - WheelPos * 3;
        } else {
          WheelPos -= 170;
          r = WheelPos * 3;
          g = 255 - WheelPos * 3;
        }
      
        r = r * MaxBright / 255;
        g = g * MaxBright / 255;
        b = b * MaxBright / 255;
      
        return Color(r, g, b);
    }

    //return all settings as a JSON string
    String json() {

      char hex_string[20] = "#";
      StaticJsonDocument<255> neopixelsJsonDoc;
      neopixelsJsonDoc["NeoPixel"]["Pattern"]   = ActivePatternName();
      sprintf(hex_string+1, "%06X", Color1); //convert number to hex
      neopixelsJsonDoc["NeoPixel"]["Color1"] = hex_string;
      sprintf(hex_string+1, "%06X", Color2); //convert number to hex
      neopixelsJsonDoc["NeoPixel"]["Color2"] = hex_string;
      neopixelsJsonDoc["NeoPixel"]["Interval"]  = Interval;
      neopixelsJsonDoc["NeoPixel"]["Steps"]     = TotalSteps;
      neopixelsJsonDoc["NeoPixel"]["Forward"] = Forward;
      neopixelsJsonDoc["NeoPixel"]["Repeat"]   = RepeatPatternName();
      String outJson;
      serializeJson(neopixelsJsonDoc, outJson);
//      Serial.print(F("neopixel JSON: ")); Serial.println(outJson);
      return outJson;
    }
};

void neopixelComplete();

NeoPatterns neopixelLED(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800, &neopixelComplete);

void neopixelComplete() {
  //if no changes are declared here, the existing pattern repeats
  switch(neopixelLED.Repeat)
  {
      case STOP:
          neopixelLED.Off();
          break;
      case TOGGLE:
          neopixelLED.ToggleDirection();;
          break;
      case REVERSE:
          neopixelLED.SetDirection(false);
          break;
      case FORWARD:
          neopixelLED.SetDirection(true);
          break;
      default:
          break;
  }
}

void neopixel_setup() {
  neopixelLED.begin();
//  neopixelLED.RainbowCycle(25);
//  neopixelLED.Fade(0x0F0000, 0x00000F, 6, 255);
  neopixelLED.Update(
    "RAINBOW_CYCLE",    //ActivePattern
    0x1F0000,  //Color1
    0x00001F,  //Color2
    255,       //TotalSteps
    16,         //Itnterval
    true,      //Forward
    "TOGGLE"   //Repeat
    );
}


#endif  //neopixel_h
