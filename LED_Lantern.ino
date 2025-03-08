#include <FastLED.h>

#define LED_PIN 9  //what pin the matrix os connected to
#define COLOR_ORDER RGB
#define CHIPSET WS2812B  //the type of leds your using

//uncomment this and FastLED.delay(1000 / FRAMES_PER_SECOND) in void loop to change speed of animation
//#define FRAMES_PER_SECOND 1000
//matrix size
const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 16;
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)

// initlize global fire parameters
int Cooling = 0;
int Sparking = 0; 
int Hue = 0;
int flamebrightness = 0;

// If true, will reverse LED order per column (if necessary)
bool gReverseDirection = false;

// LED array
CRGB leds[NUM_LEDS];

// Fire data storage (now a shared heatmap for all columns)
uint8_t heat[kMatrixWidth][kMatrixHeight];

// Customizable fire palette
CRGBPalette16 gPal;

void setup() {

  Serial.begin(9600);  //Serial for de-bugging
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {
  // Improve randomness
  random16_add_entropy(random());  

  // Adjust flame prameters based on potentiometer values.
  Cooling = map(analogRead(A0), 0, 1023, 120, 40);
  Sparking = map(analogRead(A0), 0, 1023, 200, 100);

  //Cap brightness at 100 to prevent battery overcurrent protection. To allow full brightness change 100 to 255.
  flamebrightness = map(analogRead(A0), 0, 1023, 5, 100); 
  Hue = map(analogRead(A1), 0, 1023, 0, 255);  


  // Update palette based on hue
  updatePalette(Hue);

  // Run fire animation
  Fire2012WithPalette();  

  //update led brightness
  FastLED.setBrightness(flamebrightness);  

  FastLED.show(); //Display flames on matrix

  //FastLED.delay(1000 / FRAMES_PER_SECOND);

}

void Fire2012WithPalette() {

  //** TBH I dont fully understand everything in this funtion.. **//

  // Step 1: Cool down every cell slightly
  for (int y = 0; y < kMatrixWidth; y++) {
    for (int x = 0; x < kMatrixHeight; x++) {
      heat[x][y] = qsub8(heat[x][y], random8(0, ((Cooling * 5) / kMatrixHeight) + 2));
    }
  }

  // Step 2: Move heat upward and blend with neighboring columns
  for (int y = 0; y < kMatrixWidth; y++) {
    for (int x = kMatrixHeight - 1; x > 1; x--) {
      // Get heat from below and blend with left & right columns
      int below = heat[x - 1][y];                                        // Heat from below
      int right = (x == kMatrixWidth - 1) ? below : heat[x + 1][y - 1];  // Heat from right column
      int left = (x == 0) ? below : heat[x - 1][y - 1];                  // Heat from left column
      // New heat calculation: Mix vertical and horizontal heat
      heat[x][y] = (below * 3 + left + right) / 5;
    }
  }

  // Step 3: Randomly ignite new "sparks" at the bottom
  for (int y = 0; y < kMatrixWidth; y++) {
    if (random8() < Sparking) {
      int x = random8(3);  // New sparks in the bottom few rows
      heat[x][y] = qadd8(heat[x][y], random8(200, 255));
    }
  }

  // Step 4: Map heat values to LED colors and update the matrix
  for (int y = 0; y < kMatrixWidth; y++) {
    for (int x = 0; x < kMatrixHeight; x++) {
      uint8_t colorindex = scale8(heat[x][y], 240);
      CRGB color = ColorFromPalette(gPal, colorindex);
      leds[XY(x, y)] = color;
    }
  }
}
// **XY function for Serpentine Layout**
uint16_t XY(uint8_t x, uint8_t y) {
  if (y % 2 == 0) {
    return (y * kMatrixWidth) + x;  // Even rows: left to right
  } else {
    return (y * kMatrixWidth) + (kMatrixWidth - 1 - x);  // Odd rows: right to left
  }
}

// ** Update Fire Color Palette Based on Hue Potentiometer**
void updatePalette(uint8_t baseHue) {
  //**Change the S and V values to change how the flame looks**
  gPal = CRGBPalette16(
    CHSV(baseHue, 0, 0),           // Dark base color
    CHSV(baseHue + 30, 255, 50),   // Mid orange
    CHSV(baseHue + 60, 255, 255),  // Bright yellow
    CHSV(baseHue + 90, 200, 255)   // White-hot flame
  );
}


















