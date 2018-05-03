
enum Themes { NORMAL, HALLOWEEN, CHRISTMAS };

#define MIN_CHASE_SPEED 60
#define MAX_CHASE_SPEED 30
#define CHASE_SECTION_SIZE 3
#define FADE_INTERVAL_MILIS 50
#define CLAP_INTERVAL_MILIS 50
#define COLOR_INTERVAL 40
#define LAG_OFFSET 2

#define WAVE_FADE_SIZE 20   // Size of the fade trailWaveEnabled

const TProgmemPalette16 themePalette_p PROGMEM;

class GogglePattern
{
  public:
    String Patterns[6] { "RAINBOW_CYCLE", "COLOR_WIPE", "THEATER_CHASE", "LOOPY", "WAVE", "CLAP" };
    
    // Constructor - calls base-class constructor to initialize strip
    GogglePattern(uint16_t pixels, CRGB* ledAlloc)
    {
      TotalSteps = pixels;
      TotalLeds = pixels;
      leds = ledAlloc;
    }

    void Init(Themes theme, int brightness = 255){
      Serial.println("INIT");
      ActivePattern = GetNextPattern("RAINBOW_CYCLE");
      ActiveBrightness = brightness;
      FastLED.setBrightness(ActiveBrightness);
      colored_time = millis();
      InitTheme(theme);
      currentPalette = themePalette;
      lastUpdate = millis() - UpdateInterval - 500;

      CalculateIntervals();
      Update();
    }

    // Update the pattern
    void Update(){
      if (!PatternLocked) { ChangePattern(); }
      if((millis() - lastUpdate) > UpdateInterval){ // time to update
        lastUpdate = millis();
        if (ActivePattern == "RAINBOW_CYCLE") {
          RainbowCycleUpdate();
        } else if (ActivePattern == "COLOR_WIPE") {
          ColorWipeUpdate();
        } else if (ActivePattern == "WAVE") {
          WaveUpdate();
        } else if (ActivePattern == "CLAP") {
          ClapUpdate();
        } else if (ActivePattern == "THEATER_CHASE") {
          TheaterChaseUpdate();
        } else if (ActivePattern == "LOOPY") {
          LoopyUpdate();
        } else {
          // do nothing
        }
      }
    }
    
    // Reverse direction of the pattern
    void Reverse(){
      Serial.println("Reversing");
      GoingForward = !GoingForward;
    }

    void NextPattern() {
      ChangePattern();
    }

    void SetBPM(int tempo) {
      BPM = tempo;
      CalculateIntervals();
    }

    //**************TEST METHODS****************//
    void LockPattern() {
      PatternLocked = true;
    }

    void UnlockPattern() {
      PatternLocked = false;
    }

    void SetRainbow() {
      Serial.println("Set Rainbow");
      ActivePattern = "RAINBOW_CYCLE";
      RainbowCycle();
    }

    void SetColorWipe() {
      Serial.println("Set Colorwipe");
      ActivePattern = "COLOR_WIPE";
      ColorWipe();
    }

    void SetTheaterChase() {
      Serial.println("Set Theater Chase");
      ActivePattern = "THEATER_CHASE";
      TheaterChase();
    }

    void SetLoopy() {
      Serial.println("Set Loopy");
      ActivePattern = "LOOPY";
      Loopy();
    }

    void SetWave() {
      Serial.println("Set Wave");
      ActivePattern = "WAVE";
      Wave();
    }

    void SetClap() {
      Serial.println("Set Clap");
      ActivePattern = "CLAP";
      Clap(50);
    }

   private:
   //***************VARIABLES***************//
    // Settings
    String ActivePattern;
    Themes ActiveTheme;

    CRGB *leds;
    
    int ActiveColor;
    int ActiveBrightness;
    int TotalSteps = 0;        // total number of steps in the pattern
    int TotalLeds = 0;
    int TotalRotations = 1;

    // FastLED settings
    CRGBPalette16 currentPalette;
    CRGBPalette16 themePalette;
    TBlendType    currentBlending;

    // Bool rules
    bool PatternLocked = false;
    bool GoingForward = true;

    // Active Patterns
    bool TheaterChaseEnabled = true;
    bool RainbowCycleEnabled = true;
    bool ColorWipeEnabled = true;
    bool WaveEnabled = true;
    bool ClapEnabled = true;
    bool LoopyEnabled = true;

    // Indices
    uint16_t Index = 0;
    int iColor = 0;     // Index for the current cycle color
    int iRotation = 0; // Index for the current rotation of the complete cycle

    // Pattern Intervals
    // These are overridden when the BPM is changed
    double RainbowInterval = 5;
    double WipeInterval = 50;
    double ChaseInterval = 50;
    double LoopyInterval = 50;
    double WaveInterval = 50;

    // Timing
    int BPM = 128;
    int UpdateInterval = 5;     // milliseconds between updates
    int PatternInterval = 30; // Seconds before changing the pattern
    unsigned long lastUpdate;   // last update of position
    unsigned long this_time = millis();
    unsigned long changed_time = this_time - (PatternInterval * 1000);  // Set to init right away
    unsigned long colored_time = this_time;
    
    int *Design; // double-array for lighting design
 
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
//      Serial.println("Increment: " + (String) Index); 
      if (GoingForward){
        Index += 1;
        if (Index == TotalSteps){
          Index = TotalSteps - 1;
          GogglesComplete(); // call the comlpetion callback
        }
      }
      else { // reverse
        Index -= 1;
        if (Index == 0){
          GogglesComplete(); // call the comlpetion callback
        }
      }
    }

    // set intervals based on BPM
    void CalculateIntervals() {
      RainbowInterval = (((BPM * 60) / TotalLeds) / 4)  - LAG_OFFSET;
      WipeInterval = (((BPM * 60) / TotalLeds) / 4)  - LAG_OFFSET; //(4 beats)
      ChaseInterval = ((BPM * 60) / TotalLeds) - LAG_OFFSET;
      LoopyInterval = (((BPM * 60) / TotalLeds) / 4) - LAG_OFFSET;
      WaveInterval = (((BPM * 60) / TotalLeds) / 16)  - LAG_OFFSET; //(4 beats)
    }

    void GogglesComplete()
    {
      if (iRotation+1 < TotalRotations){
           iRotation++;
           Index = 0;
      } else {
        iRotation = 0;
        Serial.println("GogglesComplete");
        if (ActivePattern == "COLOR_WIPE") {
          Reverse();
          NextColor();
        } else if (ActivePattern == "THEATER_CHASE"
          || ActivePattern == "LOOPY"
          || ActivePattern == "WAVE") {
          NextColor();
          Index = 0;
        } else {
          Index = 0;  // Start Over
        }
      }
    }

    //***************COLOR METHODS***************//
    // Convert separate R,G,B into packed 32-bit RGB color.
    // Packed format is always RGB, regardless of LED strand color order.
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
      return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
    }
    
    // Convert separate R,G,B,W into packed 32-bit WRGB color.
    // Packed format is always WRGB, regardless of LED strand color order.
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
      return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
    }

    void SetPixel(int pixel) {
      leds[pixel] = ColorFromPalette(currentPalette, iColor, ActiveBrightness, currentBlending);
    }

    void SetPixel(int pixel, int brightness) {
      leds[pixel] = ColorFromPalette(currentPalette, iColor, brightness, currentBlending);
    }
    
    void NextColor() {
      iColor += COLOR_INTERVAL;
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

    //***************THEME UTILITY METHODS***************//
    // Set the properties and colors based on the theme
    void InitTheme(Themes theme){
      ActiveTheme = theme;
      if (ActiveTheme == NORMAL){
        PatternInterval = 30;
        themePalette = PartyColors_p;
      }
      
      if (ActiveTheme == HALLOWEEN) {
        PatternInterval = 30;

        const TProgmemPalette16 themePalette_p PROGMEM =
        {
            CRGB::Purple,
            CRGB::Orange,
            CRGB::Red,
            CRGB::White,
            CRGB::Purple,
            CRGB::Green
        };

        themePalette = themePalette_p;

        // deactivate undesired modes
        RainbowCycleEnabled = false;   
      }
      
      if (ActiveTheme == CHRISTMAS) {
        PatternInterval = 30;

        const TProgmemPalette16 themePalette_p PROGMEM =
        {
            CRGB::Red,
            CRGB::Green,
            CRGB::White,
            CRGB::Gold,
            CRGB::Red,
            CRGB::Green
        };

        themePalette = themePalette_p;

        // deactivate undesired modes
        RainbowCycleEnabled = false;
      }
    }

    //***************PATTERN UTILITY METHODS***************//
    void ChangePattern() {
      this_time = millis();
      GoingForward = true;
      if((this_time - changed_time) > (PatternInterval * 1000)) {
        changed_time = millis();
        FastLED.show();
        ActivePattern = GetNextPattern(ActivePattern);
        if (ActivePattern == "RAINBOW_CYCLE") {
          RainbowCycle();
        } else if (ActivePattern == "COLOR_WIPE") {
          ColorWipe();
        } else if (ActivePattern == "WAVE") {
          Wave();
        } else if (ActivePattern == "CLAP") {
          Clap(CLAP_INTERVAL_MILIS);
        } else if (ActivePattern == "THEATER_CHASE") {
          TheaterChase();
        } else if (ActivePattern == "LOOPY") {
          Loopy();
        } else {
          // do nothing
        }
      }
    }

    String GetNextPattern(String pattern) {
      String nextPattern = "RAINBOW_CYCLE";
      if (pattern == "RAINBOW_CYCLE") {
        nextPattern = "COLOR_WIPE";
      } else if (pattern == "COLOR_WIPE") {
        nextPattern =  "WAVE";
      } else if (pattern == "WAVE") {
        nextPattern = "CLAP";
      } else if (pattern == "CLAP") {
        nextPattern = "THEATER_CHASE";
      } else if (pattern == "THEATER_CHASE") {
        nextPattern = "LOOPY";
      }

      if (IsEnabledPattern(nextPattern)){
        return nextPattern;
      } else {
        return GetNextPattern(nextPattern);
      }
    }

    bool IsEnabledPattern(String pattern) {
      if (pattern == "RAINBOW_CYCLE") {
        return RainbowCycleEnabled;
      } else if (pattern == "COLOR_WIPE") {
        return ColorWipeEnabled;
      } else if (pattern == "WAVE") {
        return WaveEnabled;
      } else if (pattern == "CLAP") {
        return ClapEnabled;
      } else if (pattern == "THEATER_CHASE") {
        return TheaterChaseEnabled;
      } else if (pattern == "LOOPY") {
        return LoopyEnabled;
      } else { 
        return true;
      }
    }

    //***************PATTERN METHODS***************//
    void RainbowCycle(){
      Serial.println("Begin RAINBOW_CYCLE");
      currentPalette = RainbowColors_p;
      currentBlending = LINEARBLEND;
      UpdateInterval = RainbowInterval;
      TotalRotations = 1;
      TotalSteps = 255;
      Index = 0;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate(){
      static int rainbowIndex = 1;
      static int rainbowColorInterval = 255 / TotalLeds;
      for(int i=0; i<TotalLeds; i++){
        leds[i] = ColorFromPalette(currentPalette, rainbowIndex, ActiveBrightness, currentBlending);
        rainbowIndex += rainbowColorInterval;
      }
      FastLED.show();
    }
    
    // Initialize for a ColorWipe
    void ColorWipe(){
      Serial.println("Begin COLOR_WIPE");
      currentPalette = themePalette;
      currentBlending = NOBLEND;
      UpdateInterval = WipeInterval;
      TotalRotations = 1;
      TotalSteps = TotalLeds;
      Index = 0;
      iColor = 0;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate(){
      leds[Index] = ColorFromPalette(currentPalette, iColor, ActiveBrightness, currentBlending);
      FastLED.show();
      Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(){
      Serial.println("Begin THEATER_CHASE");
      currentBlending = NOBLEND;
      currentPalette = themePalette;
      ActivePattern = "THEATER_CHASE";
      UpdateInterval = ChaseInterval;
      TotalRotations = 1;
      TotalSteps = TotalLeds;
      Index = 0;
    }

    // Update the Theater Chase Pattern
    void TheaterChaseUpdate(){
      for(int i=0; i<TotalLeds; i++){
        if ((i + Index) % CHASE_SECTION_SIZE == 0){
          leds[i] = ColorFromPalette(currentPalette, iColor, ActiveBrightness, currentBlending);
        }
        else{
          leds[i] = ColorFromPalette(currentPalette, iColor + COLOR_INTERVAL, ActiveBrightness, currentBlending);
        }
      }
      FastLED.show();
      Increment();
    }

    // Initialize for a ColorWipe
    void Loopy()
    {
      Serial.println("Begin LOOPY");
      ActivePattern = "LOOPY";
      UpdateInterval = LoopyInterval;
      TotalRotations = 1;
      TotalSteps = TotalLeds;
      Index = 0;
    }

    // Update the Loopy Pattern
    void LoopyUpdate()
    {
        for(int i=0; i < TotalSteps; i++){
          SetPixel(i, 0);
        }

        // Set the pixel
        SetPixel(Index);
        SetPixel(TotalLeds - Index);
        FastLED.show();
        Increment();
    }

    void Wave()
    {
      Serial.println("Begin WAVE");
      ActivePattern = "WAVE";
      UpdateInterval = WaveInterval;
      TotalRotations = 5;
      TotalSteps = TotalLeds;
      Index = 0;
    }

    // Update the Loopy Pattern
    void WaveUpdate()
    {
        for (int i=0; i < TotalSteps; i++) {
          SetPixel(i, Color(0,0,0));
        }

        SetPixel(Index);
        
        for (int i=0; i < WAVE_FADE_SIZE; i++){
          int point = Index - i;
          if (point < 0) { point = TotalSteps + point; }
          int brightness = 255 * ((float)i/((float)WAVE_FADE_SIZE * 2));
          //Serial.println(percent);
          //int colorDimmed = DimColorPercent(currentPalette[iColor], brightness);
          SetPixel(point, brightness);
        }

        for (int i=0; i < WAVE_FADE_SIZE; i++){
          int point = Index + i;
          if (point > TotalSteps) { point = point - TotalSteps; }
          int brightness = 255 * ((float)i/((float)WAVE_FADE_SIZE * 2));
          //Serial.println(percent);
          //int colorDimmed = DimColorPercent(currentPalette[iColor], brightness);
          SetPixel(point, brightness);
        }

        FastLED.show();
        Increment();
    }

    void Clap(uint16_t interval){
      Serial.println("Begin CLAP");
      ActivePattern = "CLAP";
      UpdateInterval = interval;
      TotalSteps = TotalLeds;
      Index = 0;
    }

    // Update the Theater Chase Pattern
    void ClapUpdate(){
      FastLED.show();
      if (Index >= (TotalSteps / 2)){
        //Serial.println("REVERSE");
        Reverse();
      }

      // Set the ON lights
      for (int i=0; i < CHASE_SECTION_SIZE; i++){
        int point = Index - i;
        if (!GoingForward){
          point = Index + i;
        }
        ClapSet(point);
      }

      // Set the OFF lights
      int point = Index - CHASE_SECTION_SIZE;
      if (!GoingForward){
        point = Index + CHASE_SECTION_SIZE;
      }
      ClapSet(point);
      FastLED.show();
    }

    void ClapSet(int point){
      if (point >= 0){
        if (point <= TotalSteps / 2){
          SetPixel(point);
          int oppositePoint = TotalSteps - point;
          SetPixel(oppositePoint);
        }
        else{
          SetPixel(point);
          int oppositePoint = TotalSteps - point;
          SetPixel(oppositePoint);
        }
      }
    }
};

