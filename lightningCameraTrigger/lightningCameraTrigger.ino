/**
 * Trigger the shutter on camera when lightning is detected by using lightning sensor 
 * and OLED to display the current status.
 * 
 * Project is hosted here:
 * https://github.com/pskowronek/Lightning-camera-trigger
 * 
 * Author: Piotr Skowronek, piotr@skowro.net
 * License: Apache License, version 2.0
 * 
 * Hardware:
 * - Arduino Nano or similar
 * - Lightning Sensor SEN0290 by DFRobot Gravity (that employs AS3935 & MA5532-AE antenna).
 * - OLED display SSD1306 (128x32 or larger if OLED_SCREEN_WIDTH/OLED_SCREEN_HEIGHT adjusted)
 * - Optocoupler 4N24 or 4N26, or similar
 * - a resistor (~460ohm)
 * - 3.5mm stereo socket (3 pole)
 * - 3.5mm->2.5mm stereo (3 pole) cable (for Canon - for others manufactures refer to: http://www.doc-diy.net/photo/remote_pinout/)
 * 
 * Tested with:
 * - Canon 750d
 * - Canon 350d
 * 
 * Inspired by:
 * - https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/
 * - https://www.instructables.com/Fun-With-OLED-Display-and-Arduino/
 * - http://www.martyncurrey.com/activating-the-shutter-release/
 * - https://www.doc-diy.net/photo/eos_wired_remote/
 * - https://wiki.dfrobot.com/Gravity%3A%20Lightning%20Sensor%20SKU%3A%20SEN0290
 * - https://www.theengineeringprojects.com/2015/11/reset-arduino-programmatically.html
 * 
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DFRobot_AS3935_I2C.h>   // take it from https://github.com/DFRobot/DFRobot_AS3935
#include <Lib_I2C.h>
#include <EEPROM.h>

// AS3935 this variable will be updated by IRQ when ligthning detected
volatile int8_t eventTriggered = 0;

// AS3935 conf'n 
#define AS3935_IRQ_PIN       2    // The IRQ pin the sensor trigger when it detects something
#define AS3935_CAPACITANCE   96   // antenna tuning capcitance (must be integer multiple of 8, 8 - 120 pf)
#define AS3935_INDOORS       0
#define AS3935_OUTDOORS      1

#define AS3935_DIST_OFF      0
#define AS3935_DIST_ON       1
#define AS3935_DIST          AS3935_DIST_ON   // enable/disable disruptor detection

#define AS3935_I2C_ADDR      AS3935_ADD3      // AS3935 I2C address

void lightningIRQHandler();
void buttonsIRQHandler();

// Buttons - Test button triggers IRQ, then indoor/outdoor button is read
#define TEST_MAGIC_VALUE    66    // A magic number for Test event
#define BUTTONS_IRQ_PIN     3     // The IRQ pin for test button (3 -> D3 on nano)

// Indoor vs outdoor button (configuration of lightning detector) - must be pressed together with Test button
#define INOUT_DOOR_PIN      4     // The pin to check if indoor/outdoor button is pressed (4 -> D4 on nano)
#define INOUT_DOOR_MAGIC_VALUE 67 // A magic number for testing purposes
#define INOUT_DOOR_EEPROM_ADDR 0  // The address in EEPROM where the state is saved 

// A function to reset Arduino
void(* resetFunc) (void) = 0;

DFRobot_AS3935_I2C lightningDetection((uint8_t)AS3935_IRQ_PIN, (uint8_t)AS3935_I2C_ADDR);

// OLED display conf'n
#define OLED_SCREEN_WIDTH   128   // width (in pixels)
#define OLED_SCREEN_HEIGHT  32    // height (in pixels)
#define OLED_RESET_PIN      -1    // reset pin # (or -1 if sharing Arduino reset pin)

#define OLED_UPDATE_INTERVAL 1000  // how often the screen should be updated while sensing (in ms)
#define OLD_BURN_IN_SHIFT   60     // how often to regenerate x,y offsets to reduce OLED burn in (n-times the OLED_UPDATE_INTERVAL)

// Randomized (set in #setup) x,y offsets to reduce OLED burn-in
int8_t oledRandX = 0;
int8_t oledRandY = 0;

Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);

// Counter of bona fide cought lightnings (i.e. shutter triggers)
uint16_t lightningCounter = 0;

// Shutter trigger pins
#define SHUTTER_PIN         12    // PIN for shutter
#define SHUTTER_LED_PIN     13    // PIN for built-in LED (just for debug/confirmation)

#define SHUTTER_TIME        200   // For how long the shutter should be 'pressed' (in ms)

/**
 * Setup.
 */
void setup() {  
  Serial.begin(9600);
  Serial.println(F("The whole sky glows"));
  Serial.println(F("Maybe lightning strikes twice"));

  // first, configure shutter pins (we don't want to trigger the camera by mistake)
  pinMode(SHUTTER_PIN, OUTPUT);
  pinMode(SHUTTER_LED_PIN, OUTPUT);

  // I2C library setup (pullups enable, speed set to 400kHz)
  I2c.begin();
  I2c.pullup(true);
  I2c.setSpeed(1);
  delay(5);

  randomSeed(analogRead(0));  // seed from analog pin 0
  oledBurnInProtection();

   // init of OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 init failed"));
    // let's proceed, it will still work w/o display
  } else {
    Serial.println(F("SSD1306 init succeeded"));
    display.clearDisplay();   // clear before display to supress Adafruit logo
    display.display();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(F("Maybe lightning..."));
    display.println(F(".....strikes twice"));
    display.display();
    delay(2000);
    display.clearDisplay();
    display.display();
  }
 
  // init of AS3935
  if (lightningDetection.defInit() != 0) {
    Serial.println(F("I2C AS3935 init failed"));
    display.println(F("Lightning sensor"));
    display.println(F("failed to init - NOOP."));
    display.display();
    for(;;);  // don't proceed
  }
 
  Serial.println(F("AS3935 init'n looks good"));

  display.println(F("READY! Let's go!"));
  display.display();
  
  lightningDetection.manualCal(AS3935_CAPACITANCE, isOutDoor() ? AS3935_OUTDOORS : AS3935_INDOORS, AS3935_DIST);
  Serial.print(F("Setting AS3935 to "));
  Serial.println(isOutDoor() ? F("outdoor mode") : F("indoor mode"));
  //lightningDetection.printAllRegs();  // debug regs

  pinMode(AS3935_IRQ_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(AS3935_IRQ_PIN), lightningIRQHandler, RISING);
  pinMode(BUTTONS_IRQ_PIN, INPUT_PULLUP);
  pinMode(INOUT_DOOR_PIN, INPUT_PULLUP);  // pin grounded when indoor/outdoor button pressed (read when Test IRQ is generated)
  attachInterrupt(digitalPinToInterrupt(BUTTONS_IRQ_PIN), buttonsIRQHandler, LOW);  // ground when Test button is pressed
}

/**
 * Main loop.
 */
void loop() {
  byte charBeat = 0;
  int oledBurnInCounter = 0;

  unsigned long prevMillis = millis();  
  // Wait for IRQs to trigger and in the meanwhile display heart beat
  while (eventTriggered == 0) {
    unsigned long currentMillis = millis();
    int diffMillis = currentMillis - prevMillis;
    if (diffMillis > OLED_UPDATE_INTERVAL || diffMillis < 0) {
      prevMillis = currentMillis;
      sensing(charBeat);
      charBeat++;
      if (charBeat > 3) {
        charBeat = 0;
      }
      oledBurnInCounter++;
      if (oledBurnInCounter > OLD_BURN_IN_SHIFT) {
        oledBurnInProtection();
        oledBurnInCounter = 0;
      }
    }
  }
  int8_t eventTriggeredCopy = eventTriggered;
  delay(2); // required by lightningDetection.getInterruptSrc(); 

  uint8_t eventType = handleButtonsEvent(eventTriggeredCopy);
  handleSensorEvent(eventType);

  // having long delays above it may happen that sensor may have triggered
  // IRQ during that time and we need to clear its state by calling #getInterruptSrc
  // otherwise another IRQ won't be triggered.
  lightningDetection.getInterruptSrc();
  eventTriggered = 0; // reset the trigger
}

/**
 * Handles events from buttons.
 */
uint8_t handleButtonsEvent(int8_t eventId) {
  uint8_t eventType = -1;
  // TBD tried to use case/switch but it didn't work for unknown reason (to be investigated later)
  if (eventId == INOUT_DOOR_MAGIC_VALUE) {
    Serial.println(F("Button indoor/outdoor pressed!"));
    inVsOutDoorButton();
  } else if (eventId == TEST_MAGIC_VALUE) {
    Serial.println(F("Button Test pressed! Going to initiate lightning simulation!"));
    eventType = TEST_MAGIC_VALUE; // simulate lightning strike
  } else {
    // get interrupt type from sensor
    eventType = lightningDetection.getInterruptSrc();
  }
  return eventType;
}

/**
 * Handles the event from the lightning sensor or buttons.
 */
void handleSensorEvent(uint8_t eventType) {
  boolean lightningTest = eventType == TEST_MAGIC_VALUE;
  if (eventType == 1 || lightningTest) {
    openShutter();
    uint8_t lightningDistKm = TEST_MAGIC_VALUE;
    uint32_t lightningEnergyVal = TEST_MAGIC_VALUE;
    if (!lightningTest) {
      lightningDistKm = lightningDetection.getLightningDistKm();
      lightningEnergyVal = lightningDetection.getStrikeEnergyRaw(); // Energy intensity (0-1000)
      lightningCounter++;
    }
    lightningDetected(lightningDistKm, lightningEnergyVal);
    // let's wait ~200ms to let camera shoot as many photos as she can (if in burst mode, in bulb mode it doesn't really matter)
    // (avg duration of a lightning visibility is ~200ms)
    delay(SHUTTER_TIME);
    releaseShutter();
    flickDisplay(5, 200); // for informational purposes
  } else if (eventType == 2) {
    disruptorDetected();
  } else if (eventType == 3) {
    noiseDetected();
  } else if (eventType < 0) {
    // ignore
  } else {
    unknownDetected();
  } 
}

/**
 * Open shutter.
 */
void openShutter() {
  digitalWrite(SHUTTER_PIN, HIGH);
  digitalWrite(SHUTTER_LED_PIN, HIGH);
}

/**
 * Release shutter.
 */
void releaseShutter() {
  digitalWrite(SHUTTER_PIN, LOW);
  digitalWrite(SHUTTER_LED_PIN, LOW);
}

/**
 * Clear an set screen to default state, optionally showing the lightning counter.
 */
void clearScreenToDefault(boolean showCounter) {  
  display.clearDisplay();
  // display the number of caught lightnings in bottom-right corner
  if (showCounter) {
    static char buf1[10] = {'\0'};
    static char buf2[10] = {'\0'};
    sprintf(buf1, "[%d]", lightningCounter);
    sprintf(buf2, "%10s", buf1);
    display.setCursor(OLED_SCREEN_WIDTH - 62 + oledRandX, OLED_SCREEN_HEIGHT - 8 + oledRandY); // bottom-right
    display.print(buf2);    
  }
  display.setCursor(0 + oledRandX, 0 + oledRandY);
}

/**
 * Generate new random x,y offsets used while printing on OLED to reduce burn-in effect.
 */
void oledBurnInProtection() {
  Serial.println(F("OLED burn-in protection - shifting a bit starting x,y"));
  oledRandX = random(0, 3);
  oledRandY = random(0, 2);
}

/**
 * Display 'sensing' heart beat.
 */
void sensing(byte charBeat) {
  static char charsBeat[4] = { '|', '/', '-', '\\' };
  clearScreenToDefault(true);
  display.print(F("Sensing ["));
  display.print(charsBeat[charBeat]);
  display.println(F("]"));
  display.println();
  display.println();
  display.print(isOutDoor() ? F("[outdoor mode]") : F("[indoor mode]"));
  display.display();
}

/**
 * Flick the display (blink the number of given times every given interval)
 */
void flickDisplay(int times, int interval) {
  for (int i = 0; i < times; i++) {
    display.invertDisplay(true);
    delay(interval);
    display.invertDisplay(false);
    delay(interval);
  }
}

/**
 * Display the lightning detection event.
 */
void lightningDetected(uint8_t lightningDistKm, uint32_t lightningEnergyVal) {
  Serial.println(F("Lightning detected!"));
  clearScreenToDefault(true);
  display.invertDisplay(true);
  display.println(F("Lightning detected!"));
  display.display();
  display.print(F("@ "));
  if (lightningDistKm != 0x3F) {  // 0x3F == out-of-range
    display.print(lightningDistKm);
  } else {
    display.print(F("oo"));
  }
  display.print(F("km of "));
  display.print(lightningEnergyVal / 10); // convert 0-1000 to percentage (val*100/1000 -> val/10)
  display.println(F("% intense"));
  
  display.println(F("Shutter triggered"));
  display.display();
  display.invertDisplay(false);
}

/**
 * Display info that disruptor has been detected.
 */
void disruptorDetected() {
  Serial.println(F("Disruptor discovered!"));
  clearScreenToDefault(true);
  display.println(F("Nasty disruptor!"));
  display.println(F("Electr(on)ic device"));
  display.println(F("is way too close?"));
  display.display(); 
}

/**
 * Display info that environment is too noisy.
 */
void noiseDetected() {
  Serial.println(F("Noise level too high!"));
  clearScreenToDefault(true);
  display.println(F("Noise level too high!"));
  display.display();
}

/**
 * Display info that unknown event has been experienced.
 */
void unknownDetected() {
  Serial.println(F("Unknown lightning event"));
  clearScreenToDefault(true);
  display.println(F("Unknown event"));
  display.display();
}

/**
 * The action for changing indoor vs outdoor mode of lightning sensor.
 */
void inVsOutDoorButton() {
  boolean isOutdoor = not(isOutDoor()); // read and flip the setting
  EEPROM.update(INOUT_DOOR_EEPROM_ADDR, isOutdoor);
  clearScreenToDefault(false);

  // the above should be enough to reconfigure the sensors...
  if (isOutdoor) {
    lightningDetection.setOutdoors();
  } else {
    lightningDetection.setIndoors();
  }
  display.print(F("Set mode to: "));
  display.println(isOutdoor ? F("outdoor") : F("indoor"));

  // ...but let's reboot the whole thing to be sure
  display.println(F("Going to reboot in 5s"));
  display.println(F("to apply changes..."));
  display.display();
  delay(5000);
  clearScreenToDefault(false);
  display.println(F("The system is going"));
  display.println(F("down for reboot NOW!"));
  display.display();
  delay(1000);
  resetFunc();
}

/**
 * Tells whether outdoor mode is set (by reading the state from EEPROM).
 */
boolean isOutDoor() {
  return EEPROM.read(INOUT_DOOR_EEPROM_ADDR);
}

/**
 * The IRQ handler for AS3935 interrupts.
 */
void lightningIRQHandler() {
  eventTriggered = 1;
}

/**
 * The IRQ handler for buttons.
 */
void buttonsIRQHandler() {
  eventTriggered = !digitalRead(INOUT_DOOR_PIN) ? INOUT_DOOR_MAGIC_VALUE : TEST_MAGIC_VALUE;
}
