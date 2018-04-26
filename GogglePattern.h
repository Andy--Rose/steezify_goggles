enum Directions { FORWARD, REVERSE };
enum Themes { NORMAL, HALLOWEEN, CHRISTMAS };
enum Patterns { RAINBOW_CYCLE, COLOR_WIPE, THEATER_CHASE, LOOPY, CIRCLE_FADE, CLAP };

#define MIN_CHASE_SPEED 60
#define MAX_CHASE_SPEED 30
#define CHASE_INTERVAL_MILIS 500
#define WIPE_INTERVAL_MILIS 50
#define RAINBOW_INTERVAL_MILIS 5
#define FADE_INTERVAL_MILIS 50
#define CLAP_INTERVAL_MILIS 50

class GogglePattern : public Adafruit_NeoPixel
{
  public:
    // Constructor - calls base-class constructor to initialize strip
    GogglePattern(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
      OnComplete = callback;
      TotalSteps = pixels;
      TotalLeds = pixels;
    }

    void Init(Themes theme, int brightness = 255){
      Serial.println("INIT");
      ActivePattern = GetNextPattern(RAINBOW_CYCLE);
      ActiveBrightness = brightness;
      setBrightness(ActiveBrightness);
      colored_time = millis();
      InitTheme(theme);
      lastUpdate = millis() - Interval - 500;
      Update();
    }

    // Update the pattern
    void Update(){
      if (!PatternLocked) { ChangePattern(); }
      if (ColorChange || ColorCycle) { ChangeColor(); }
      if((millis() - lastUpdate) > Interval){ // time to update
        lastUpdate = millis();
        if (ActivePattern == RAINBOW_CYCLE) {
          RainbowCycleUpdate();
        } else if (ActivePattern == COLOR_WIPE) {
          ColorWipeUpdate();
        } else if (ActivePattern == CIRCLE_FADE) {
          CircleFadeUpdate();
        } else if (ActivePattern == CLAP) {
          ClapUpdate();
        } else if (ActivePattern == THEATER_CHASE) {
          TheaterChaseUpdate();
        } else if (ActivePattern == LOOPY) {
          LoopyUpdate();
        } else {
          // do nothing
        }
        Increment();
      }
    }
    
    Patterns GetPattern() {
      return ActivePattern;
    }
    
    // Reverse direction of the pattern
    void Reverse(){
      if (Direction == FORWARD){
        Direction = REVERSE;
      }
      else{
        Direction = FORWARD;
      }
    }

    void NextPattern() {
      ChangePattern();
    }

    //**************TEST METHODS****************//
    void LockPattern() {
      PatternLocked = true;
    }

    void TestClap() {
      Serial.println("Testing Clap");
      ActivePattern = CLAP;
      Clap(Color(255, 0, 0), Color(0,255,0), 50, 3);
    }

    void TestRainbow() {
      Serial.println("Testing Rainbow");
      ActivePattern = RAINBOW_CYCLE;
      RainbowCycle();
    }

   private:
   //***************VARIABLES***************//
    // Settings
    Patterns ActivePattern;
    Themes ActiveTheme;
    Directions Direction;
    int ActiveColor;
    int ActiveBrightness;
    int TotalSteps = numPixels();        // total number of steps in the pattern
    int TotalLeds = numPixels();
    uint16_t CircleFadeSize = 6;    // Size of the fade trail
    uint16_t ChaseSectionSize = 3;  // Size of the chase section

    // Bool rules
    bool SpeedChange = false;   // must be overridden by theme
    bool Accelerating = false;  // must be overridden
    bool ColorChange = false;  // must be overridden by theme
    bool ColorCycle = false;
    bool PatternLocked = false;
    bool circleFadeDouble = false;

    // Active Patterns
    bool TheaterChaseEnabled = true;
    bool RainbowCycleEnabled = true;
    bool ColorWipeEnabled = true;
    bool CircleFadeEnabled = true;
    bool ClapEnabled = true;
    bool LoopyEnabled = true;

    // Colors
    uint32_t Color1;
    uint32_t Color1Wheel;
    uint32_t Color2;
    uint32_t Color2Wheel;
    uint32_t WipeColor;
    uint32_t ColorCollection[6] = {
      Color(10,10,10),       // Black
      Color(255,153,0),   // Orange
      Color(255,0,0),     // Red
      Color(255,255,255), // White
      Color(204,0,204),   // Purple
      Color(0,255,0)      // Green
    };

    // Indices
    uint16_t Index = 0;
    int iColor = 0;     // Index for the current cycle color
    int iRainbow = 0;   // Index for iterating the rainbow

    // Timing
    unsigned long Interval;     // milliseconds between updates
    int ColorTimeSeconds = 10; // Seconds before changing the color
    int ColorInterval = 30;     // Amount to change the color at interval
    int PatternInterval = 30; // Seconds before changing the pattern
    unsigned long lastUpdate;   // last update of position
    unsigned long this_time = millis();
    unsigned long changed_time = this_time - (PatternInterval * 1000);  // Set to init right away
    unsigned long colored_time = this_time;
    
    int *Design; // double-array for lighting design
 
    void (*OnComplete)();       // callback
    
    //***************UTILITY METHODS***************//
    // Returns the Red component of a 32-bit color
    uint8_t Red(int color){
      return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(int color){
      return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(int color){
      return color & 0xFF;
    }

    // Increment the Index and reset at the end
    void Increment(){
      if (Direction == FORWARD){
        Index++;
        if (Index > TotalSteps){
          Index = 0;
          if (OnComplete != NULL){
            OnComplete(); // call the comlpetion callback
          }
        }
      }
      else{ // Direction == REVERSE
        --Index;
        if (Index < 0){
          Index = TotalSteps-1;
          if (OnComplete != NULL){
            OnComplete(); // call the comlpetion callback
          }
        }
      }
    }

    //***************COLOR METHODS***************//
    // Return color, dimmed by 75% (used by scanner)
    int DimColor(int color){
      int dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
      return dimColor;
    }

    int DimColorPercent(int color, double brightness){
      if (brightness == 255) { return color; }
      int redPart = FlipColor(brightness*Red(color)/255);
      int greenPart = FlipColor(brightness*Green(color)/255);
      int bluePart = FlipColor(brightness*Blue(color)/255);
      int dimColor = Color(redPart, greenPart, bluePart);
      return dimColor;
    }

    int FlipColor(int color){
      if (color != 0){
        return 255 - color;
      }
      return color;
    }

    // Input a value 0 to 255 to get a color value.
    // The colors are a transition r - g - b - back to r.
    int Wheel(int WheelPos){
      WheelPos = 255 - WheelPos;
      if(WheelPos < 85){
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
      }
      else if(WheelPos < 170){
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
      }
      else{
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
      }
    }

    void ChangeColor() {
      this_time = millis();
      if((this_time - colored_time) > (ColorTimeSeconds * 1000)) {
        colored_time = millis();
        if (ColorChange && ActivePattern != RAINBOW_CYCLE) {
          SetNextColorFromWheel();
        }
        if (ColorCycle) {
          Color1 = Color2;
          Color2 = GetNextColorFromCollection();
        }
      }
    }

    void SetNextColorFromWheel() {
      Color1Wheel += ColorInterval;
      if (Color1Wheel > 255) { Color1Wheel -= 255; }
      Color2Wheel += ColorInterval;
      if (Color2Wheel > 255) { Color2Wheel -= 255; }

      Color1 = Wheel(Color1Wheel);
      Color2 = Wheel(Color2Wheel);
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color) {
      for (int i = 0; i < numPixels(); i++) {
        setPixelColor(i, color);
      }
      show();
    }

    uint32_t GetNextColorFromCollection() {
      int arrSize = sizeof(ColorCollection)/sizeof(ColorCollection[0]);
      iColor++;
      if (iColor == arrSize) {
        iColor = 0;
      }
      return ColorCollection[iColor];
    }

    //***************THEME UTILITY METHODS***************//
    // Set the properties and colors based on the theme
    void InitTheme(Themes theme){
      ActiveTheme = theme;
      if (ActiveTheme == NORMAL){
        ColorChange = true;
        ColorCycle = false;
        ColorTimeSeconds = 10;
        ColorInterval = 30;
        PatternInterval = 30;

        Color1 = Color(0,255,0);
        Color1Wheel = 170;
        Color2 = Color(0,0,255);
        Color2Wheel = 255;
      }
      
      if (ActiveTheme == HALLOWEEN) {
        ColorChange = false;
        ColorCycle = true;
        ColorTimeSeconds = 10;
        PatternInterval = 30;
        
        ColorCollection[0] = Color(104,0,104);  // Purple
        ColorCollection[1] = Color(153,153,0);  // Orange
        ColorCollection[2] = Color(255,0,0);    // Red
        ColorCollection[3] = Color(255,255,255);// White
        ColorCollection[4] = Color(80,0,80);  // Purple
        ColorCollection[5] = Color(0,255,0);    // Green
        Color1 = ColorCollection[0];
        Color2 = ColorCollection[1];

        // deactivate undesired modes
        RainbowCycleEnabled = false;   
      }
      
      if (ActiveTheme == CHRISTMAS) {
        ColorChange = false;
        ColorCycle = true;
        ColorTimeSeconds = 10;
        PatternInterval = 30;
        
        ColorCollection[0] = Color(255,0,0);    // Red
        ColorCollection[1] = Color(0,255,0);    // Green
        ColorCollection[2] = Color(255,255,255);// White
        ColorCollection[3] = Color(153,153,0);  // Gold
        ColorCollection[4] = Color(255,0,0);    // Red
        ColorCollection[5] = Color(0,255,0);    // Green
        
        Color1 = ColorCollection[0];
        Color2 = ColorCollection[1];

        // deactivate undesired modes
        RainbowCycleEnabled = false;
      }
    }

    //***************PATTERN UTILITY METHODS***************//
    void ChangePattern() {
      this_time = millis();
      Direction = FORWARD;
      if((this_time - changed_time) > (PatternInterval * 1000)) {
        changed_time = millis();
        show();
        ActivePattern = GetNextPattern(ActivePattern);
        if (ActivePattern == RAINBOW_CYCLE) {
          RainbowCycle();
        } else if (ActivePattern == COLOR_WIPE) {
          ColorWipe(Color1, Color2, WIPE_INTERVAL_MILIS);
        } else if (ActivePattern == CIRCLE_FADE) {
          CircleFade(Color1, Color2, FADE_INTERVAL_MILIS, 8, true);
        } else if (ActivePattern == CLAP) {
          Clap(Color1, Color2, CLAP_INTERVAL_MILIS, 3);
        } else if (ActivePattern == THEATER_CHASE) {
          TheaterChase(Color1, Color2, CHASE_INTERVAL_MILIS, 3);
        } else if (ActivePattern == LOOPY) {
          Loopy(Color1, WIPE_INTERVAL_MILIS, FORWARD);
        } else {
          // do nothing
        }
      }
    }

    Patterns GetNextPattern(Patterns pattern) {
      Patterns nextPattern = RAINBOW_CYCLE;
      if (pattern == RAINBOW_CYCLE) {
        nextPattern = COLOR_WIPE;
      } else if (pattern == COLOR_WIPE) {
        nextPattern =  CIRCLE_FADE;
      } else if (pattern == CIRCLE_FADE) {
        nextPattern =  CLAP;
      } else if (pattern == CLAP) {
        nextPattern =  THEATER_CHASE;
      } else if (pattern == THEATER_CHASE) {
        nextPattern =  LOOPY;
      } else {
        nextPattern =  RAINBOW_CYCLE;
      }

      if (IsEnabledPattern(nextPattern)){
        return nextPattern;
      } else {
        return GetNextPattern(nextPattern);
      }
    }

    bool IsEnabledPattern(Patterns pattern) {
      if (pattern == RAINBOW_CYCLE) {
        return RainbowCycleEnabled;
      } else if (pattern == COLOR_WIPE) {
        return ColorWipeEnabled;
      } else if (pattern == CIRCLE_FADE) {
        return CircleFadeEnabled;
      } else if (pattern == CLAP) {
        return ClapEnabled;
      } else if (pattern == THEATER_CHASE) {
        return TheaterChaseEnabled;
      } else if (pattern == LOOPY) {
        return LoopyEnabled;
      } else { 
        return true;
      }
    }

    //***************PATTERN METHODS***************//
    void RainbowCycle(){
      Serial.println("Begin RAINBOW_CYCLE");
      Interval = RAINBOW_INTERVAL_MILIS;
      TotalSteps = 255;
      Index = 0;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate(){
      int interval = 255 / TotalLeds;
      for(int i=0; i<TotalLeds; i++){
        setPixelColor(i, Wheel(i * interval));
//        int root = ((i * 256 / TotalLeds) + Index);
//        int rootrem = root & 255;
//        int rcol = Wheel(rootrem);
////        Serial.println("rcol: " + (String)rcol);
//        setPixelColor(i, rcol);
      }
      show();
    }
    
    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color1, uint32_t color2, uint8_t interval, Directions dir = FORWARD){
      Serial.println("Begin COLOR_WIPE");
      ActivePattern = COLOR_WIPE;
      Interval = interval;
      TotalSteps = TotalLeds;
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
      WipeColor = Color1;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate(){
      setPixelColor(Index, WipeColor);
      show();
      if (Index + 1 == TotalSteps){
        if (WipeColor == Color1){
          WipeColor = Color2;
        }
        else{
          WipeColor = Color1;
        }
      }
    }

    // Initialize for a ColorWipe
    void Loopy(uint32_t color1, uint32_t interval, Directions dir = FORWARD)
    {
      Serial.println("Begin LOOPY");
      ActivePattern = LOOPY;
      Interval = interval;
      TotalSteps = TotalLeds;
      Color1 = color1;
      Index = 0;
      Direction = dir;
    }

    // Update the Loopy Pattern
    void LoopyUpdate()
    {
        for(int i=0; i < TotalSteps; i++){
          setPixelColor(i, Color(0,0,0));
        }

        // Set the pixel
        setPixelColor(Index, Color1);
        show();
        Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, uint16_t count, Directions dir = FORWARD){
      Serial.println("Begin THEATER_CHASE");
      ActivePattern = THEATER_CHASE;
      Interval = interval;
      ChaseSectionSize = count;   // ChaseSectionSize here will be the length of Color1
      TotalSteps = TotalLeds;
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
    }

    // Update the Theater Chase Pattern
    void TheaterChaseUpdate(){
      for(int i=0; i< numPixels(); i++){
        if ((i + Index) % ChaseSectionSize == 0){
          setPixelColor(i, Color2);
        }
        else{
          setPixelColor(i, Color1);
        }
      }
      show();
    }

    // Initialize for a Circle Fade
    void CircleFade(uint32_t color1, uint32_t color2, uint16_t interval, uint16_t fadeLength, bool doubletone = false, Directions dir = FORWARD){
      Serial.println("Begin CIRCLE_FADE");
      ActivePattern = CIRCLE_FADE;
      CircleFadeSize = fadeLength;
      Interval = interval;
      circleFadeDouble = doubletone;
      TotalSteps = TotalLeds + 1;
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
    }

    // Update the Theater Chase Pattern
    void CircleFadeUpdate(){
      CircleFadeSet(Index, Color1);
      if (circleFadeDouble){
        int start = (Index + (TotalSteps / 2)) % TotalSteps;
        CircleFadeSet(start, Color2);
      }
      show();
    }

    void CircleFadeSet(int start, uint32_t color){
      for (int i=0; i < CircleFadeSize; i++){
        int point = start - i;
        if (point < 0) { point = TotalSteps + point; }
        //Serial.println(point);
        int brightness = 255 * ((float)i/((float)CircleFadeSize * 2));
        //Serial.println(percent);
        int colorDimmed = DimColorPercent(color, brightness);
        setPixelColor(point, colorDimmed);
      }
      int point = start - CircleFadeSize;
      if (point < 0) { point = TotalSteps + point; }
      setPixelColor(point, 0); 
    }

    void Clap(uint32_t color1, uint32_t color2, uint16_t interval, uint16_t len){
      Serial.println("Begin CLAP");
      ActivePattern = CLAP;
      Interval = interval;
      ChaseSectionSize = len;
      TotalSteps = TotalLeds;
      Color1 = color1;
      Color2 = color2;
      Index = 0;
    }

    // Update the Theater Chase Pattern
    void ClapUpdate(){
      show();
      if (Index >= (TotalSteps / 2)){
        //Serial.println("REVERSE");
        Reverse();
      }

      // Set the ON lights
      for (int i=0; i < ChaseSectionSize; i++){
        int point = Index - i;
        if (Direction == REVERSE){
          point = Index + i;
        }
        ClapSet(point, Color1, Color2);
      }

      // Set the OFF lights
      int point = Index - ChaseSectionSize;
      if (Direction == REVERSE){
        point = Index + ChaseSectionSize;
      }
      ClapSet(point, 0, 0);
      show();
    }

    void ClapSet(int point, int32_t colorOne, int32_t colorTwo){
      if (point >= 0){
        if (point <= TotalSteps / 2){
          setPixelColor(point, colorOne);
          int oppositePoint = TotalSteps - point;
          setPixelColor(oppositePoint, colorTwo);
        }
        else{
          if (colorOne == 0 && colorTwo == 0){
            setPixelColor(point, colorOne);
            int oppositePoint = TotalSteps - point;
            setPixelColor(oppositePoint, colorTwo);
          }
        }
      }
    }
};

