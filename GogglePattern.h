#define ROWS 13
#define COLS 4
#define COLOR_WIPE_INTERVAL 50
#define COLOR_WIPE_FADE_RATE 30
#define THEATRE_CHASE_INTERVAL 100
#define RAINBOW_INTERVAL 10
enum direction { FORWARD, REVERSE };

class GogglePattern : public Adafruit_NeoPixel
{
  private:
    String patterns[5] = { "RAINBOW_CYCLE", "COLOR_WIPE", "THEATER_CHASE", "LOOPY" };
  public:
    // Member Variables:
    direction Direction;

    uint32_t Pixels = 0;
    uint32_t Interval;     // milliseconds between updates
    uint32_t FadeRate;     // when fading, amount to reduce colors per interval
    uint32_t lastUpdate;   // last update of position

    uint32_t Color1, Color2;
    uint32_t R, G, B;
    uint32_t TotalSteps;        // total number of steps in the pattern
    uint32_t Index;
    uint32_t PatternIndex = 0;

    int *Design;

    void (*OnComplete)();       // callback

    // Constructor - calls base-class constructor to initialize strip
    GogglePattern(uint16_t pixels, uint8_t pin, int *design, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
        SetPattern();
        Pixels = pixels;
        Design = design;
    }

        // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            String pattern = patterns[PatternIndex];
            if (pattern == "THEATER_CHASE") {
                TheaterChaseUpdate();
            }
            else if (pattern == "RAINBOW_CYCLE") {
              RainbowCycleUpdate();
            }
            else if (pattern == "COLOR_WIPE") {
              ColorWipeUpdate();
            } else {
              LoopyUpdate();
            }
        }
    }

        // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }

    String GetPattern() {
      return patterns[PatternIndex];
    }

    void SetPattern() {
      String pattern = patterns[PatternIndex];
      if (pattern == "THEATER_CHASE") {
        TheaterChase(Color(255,255,0), Color(0,0,50), THEATRE_CHASE_INTERVAL, FORWARD);
      }
      else if (pattern == "RAINBOW_CYCLE") {
        RainbowCycle(RAINBOW_INTERVAL, FORWARD);
      }
      else if (pattern == "COLOR_WIPE") {
        ColorWipe(255, 255, 0, COLOR_WIPE_INTERVAL, COLOR_WIPE_FADE_RATE, FORWARD);
      } else {
        Loopy(255, 255, 0, COLOR_WIPE_INTERVAL, COLOR_WIPE_FADE_RATE, FORWARD);
      }
    }

    void NextPattern()
    {
      if (PatternIndex == 4) {
        PatternIndex = 0;
      } else {
        PatternIndex++;
      }
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t r, uint32_t g, uint32_t b, uint32_t interval, uint8_t fadeRate, direction dir = FORWARD)
    {
        Interval = interval;
        FadeRate = fadeRate;
        TotalSteps = numPixels();
        R = r;
        G = g;
        B = b;
        Color1 = Color(r,g,b);
        Index = 0;
        Direction = dir;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        if (Direction == REVERSE) {
          if (TotalSteps - Index == 1) {
            setPixelColor(0, Color(0,0,0));
          }
          setPixelColor(TotalSteps - Index, Color(0,0,0));
        } else {
          setPixelColor(Index, Color1);
        }
        show();
        Increment();
    }

    // Initialize for a ColorWipe
    void Loopy(uint32_t r, uint32_t g, uint32_t b, uint32_t interval, uint8_t fadeRate, direction dir = FORWARD)
    {
        Interval = interval;
        FadeRate = fadeRate;
        TotalSteps = numPixels();
        R = r;
        G = g;
        B = b;
        Index = 0;
        Direction = dir;
    }

    // Update the Loopy Pattern
    void LoopyUpdate()
    {
        for(int i=0; i < TotalSteps; i++)
        {
          setPixelColor(i, Color(0,0,0));
        }

        // Set the pixel
        setPixelColor(Index, Color(R,G,B));
        show();
        Increment();
    }

    void RainbowCycle(uint32_t interval, direction dir = FORWARD)
    {
      Interval = interval;
      TotalSteps = 255;
      Index = 0;
      Direction = dir;
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

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint32_t interval, direction dir = FORWARD)
    {
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }

   // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }

    // Common Utility Functions
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

    // Return color, dimmed by 75% (used by scanner)
    uint32_t DimColorRGB(uint32_t r, uint32_t g, uint32_t b, uint32_t amount)
    {
      uint32_t dimRed = r - amount;
      if (dimRed < 0) { r = 0; }
      uint32_t dimGreen = g - amount;
      if (dimGreen < 0) { g = 0; }
      uint32_t dimBlue = b - amount;
      if (dimBlue < 0) { b = 0; }
      
      uint32_t dimColor = Color(dimRed, dimGreen, dimBlue);
      return dimColor;
    }

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }

    // Reverse direction of the pattern
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }
};
