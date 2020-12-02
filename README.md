_Language versions:_\
[![EN](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/flags/lang-US.png)](https://github.com/pskowronek/Lightning-camera-trigger) 
[![PL](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/flags/lang-PL.png)](https://translate.googleusercontent.com/translate_c?sl=en&tl=pl&u=https://github.com/pskowronek/Lightning-camera-trigger)
[![DE](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/flags/lang-DE.png)](https://translate.googleusercontent.com/translate_c?sl=en&tl=de&u=https://github.com/pskowronek/Lightning-camera-trigger)
[![FR](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/flags/lang-FR.png)](https://translate.googleusercontent.com/translate_c?sl=en&tl=fr&u=https://github.com/pskowronek/Lightning-camera-trigger)
[![ES](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/flags/lang-ES.png)](https://translate.googleusercontent.com/translate_c?sl=en&tl=es&u=https://github.com/pskowronek/Lightning-camera-trigger)

# Lightning camera trigger

This project is to automagically trigger the camera as soon as lightning occurs, as quick as possible.
Internet says the average duration of lightning  visibility in the sky lasts for ~100-200ms, so it is enough to sense the lighting and trigger the camera.
There are two well known ways to sense the lightning, one is to trigger the camera when light suddenly changes, the other one is to sense for specific radio waves.
This project uses the latter one by employing the lightning sensor based on AS3935 chip and MA5532-AE antenna - here a module SEN0290 by DFRobot Gravity
has been used, but there are other brands that produce similar modules (can be found on Ali, eBay etc - haven't tested them out, though).
Since I own Canon DSLR all the camera wiring has been done for this particular manufacturer.

DISCLAIMER: The module and the program haven't been tested in the field yet, now it is winter time and no lightnings. It's been tested with piezo lighters
and other noisy devices, though. The module & software are able to properly notify about noisy environment etc, the question is how good it will be
catching lightnings!

Awaiting spring - keep fingers crossed!

## Functionality

The main functionality is to trigger the shutter when lightning occurs - this is being done by shortening wires of Camera's remote trigger cable.
To shorten the wires, the optocoupler is being used as it isolates the lightning detector from the camera itself (very safe solution).

The assembled project uses direct power input via Arduino mini USB. One can use Power Bank to power it up, the simpler the power bank the better - it must 
not have low-current shut off), the power consumption is relatively low and on small power bank it should run more than 12hrs (the power usage could be
lower by removing LEDs, turning off OLED, bypassing/removing voltage regulator on Nano and connecting 3.3V directly to Vin etc).
To display status OLED screen is being used - to avoid burn-in, a simple counter measures have been employed by moving randomly the displayed text by 0-2 pixels around.

Program provides support for two tact-switch buttons:
- Test button - to test the device. It simulates lighting interruption and engages the camera trigger
(good for testing the whole thing when assembling, and on the field to check whether everything is connected as it should be). 
- In/Out button (that must be pressed first and while it is being pressed, the Test button pressed) - it changes the internal configuration
of the sensor between indoor and outdoor use (a proper information on screen is displayed which mode is being currently set).


The screen displays the following information:
- Sensing in progress
- Disruptors in the vicinity 
- Noisy environment
- Lightning occurance along with distance and energy (0-100%)
- Number of lightnings sensed in the session
- Current mode of operation (indoors/outdoors)
- Whether Lightning Sensor has been properly initialized, states the error otherwise


## Hardware

List of parts you will need:
- Arduino Nano or similar
- Lightning Sensor with AS3935 (for example SEN0290 by DFRobot Gravity)
- OLED display SSD1306 (128x32 or similar, resolution can be reconfigured in the code if it sill uses SSD1306 library)
- Optocoupler 4N24 or 4N26, or similar
- a resistor (~460ohm)
- 2x tact switch buttons
- 3.5mm stereo socket (3 pole)
- 3.5mm->2.5mm stereo (3 pole) cable (for Canon - for others manufactures refer to this [site](http://www.doc-diy.net/photo/remote_pinout/))
- breadboard or proto board, connectors/wires etc

## Software

List of software/libraries you will need:
- [Arduino](https://www.arduino.cc/en/software)
- [Lightning Sensor library](https://github.com/DFRobot/DFRobot_AS3935)
- SSD1306 library (install it thru Arduino as described [here](https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/))

## Schematics (wiring)

The schematic of wiring is quite simple, take a look here:
![Schematics](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/other/schematics.jpg)

## Screenshots / Photos

### Screenshots
![Screenshots](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/screenshots/lightning-demo.png)


### Photos
[![Assembled](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/assembled/05.jpg)](https://pskowronek.github.io/Lightning-camera-trigger/www/assembled/index.html "Photos of assembled Lightning camera trigger")

More photos of the assembled Lightning Camera Trigger enclosed in a paper-clip box are [here](https://pskowronek.github.io/Lightning-camera-trigger/www/assembled/index.html "Photos of assembled Lightning camera trigger").

## TODO & Gotchas

- test it out during stormy seasons
- the program may require tweaks
  - proper calibration procedure (now it runs on default settings)
  
- the lightning sensor module
  - must not be too close to Arduino or OLED
  - far away from connectors/cabales
  - must not have metalic surface underneath
  - requires stable and nicely filtered power supply, otherwise false alarms may occur, especially if arduino/oled is close by

## License

The code is licensed under Apache Licence 2.0, pictures under Creative Commons BY-NC.

## Authors

- [Piotr Skowronek](https://github.com/pskowronek)
