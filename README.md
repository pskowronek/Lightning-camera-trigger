_Language versions:_\
[![EN](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/flags/lang-US.png)](https://github.com/pskowronek/Lightning-camera-trigger) 
[![PL](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/flags/lang-PL.png)](https://translate.googleusercontent.com/translate_c?sl=en&tl=pl&u=https://github.com/pskowronek/Lightning-camera-trigger)
[![DE](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/flags/lang-DE.png)](https://translate.googleusercontent.com/translate_c?sl=en&tl=de&u=https://github.com/pskowronek/Lightning-camera-trigger)
[![FR](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/flags/lang-FR.png)](https://translate.googleusercontent.com/translate_c?sl=en&tl=fr&u=https://github.com/pskowronek/Lightning-camera-trigger)
[![ES](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/flags/lang-ES.png)](https://translate.googleusercontent.com/translate_c?sl=en&tl=es&u=https://github.com/pskowronek/Lightning-camera-trigger)

# Lightning camera trigger

This project is to automagically trigger the camera as soon as lightning occurs, as quick as possible.
Internet says the average duration of lightnig visibility in the sky lasts for ~100-200ms, so it is enough to sense the lighting and trigger the camera.
There are two well known ways to sense the lightning, one is to trigger the camera when light suddenly changes, the other one is to sense for spacific radio waves.
This project uses the latter one by emplying the lightning sensor based on AS3935 chip and MA5532-AE antenna - here a module SEN0290 by DFRobot Gravity
has been used, but there are other brands that produce similar modules (can be found on Ali, eBay etc - haven't tested them out, though).
Since I own Canon dSLR all the camera wiring has been done for this particular manufacturer.

DISCLAIMER: The module and the program haven't been tested in the field yet, now is winter time and now lightning. It's been tested with piezo lighters
and other noisy devices - the module + software is able to properly notify about noisy environment, the question is how good it will be catching lightnings!
Awaiting spring - keep fingers crossed!

## Functionality

The assembled project has direct power input via Arduino mini USB. One can use Power Bank to power it up, the simpler power bank the better - it must 
not have low-current shut off), the power consumption is relatively low and on small power bank it should run more than 12hrs (the power usage could be
lower removing LEDs, turning off OLED, bypassing/removing voltage stabilizer from Nano etc). To display status OLED screen is being used.
Program provides support for two tact-switch buttons:
- Test button - to test the device. It simulates lighting interruption and engages the camera trigger
(good for testing the whole thing when assembling, and on the field to check whether everything is connected as it should be). 
- In/Out button (that must be pressed first and while it is being pressed, the Test button pressed) - it changes the internal configuration
of the sensor between indoor and outdoor use (a proper information on screen is displayed which one is being currently set).

## Hardware

List of parts you will need:
- Arduino Nano or similiar
- Lightning Sensor with AS3935 (for example SEN0290 by DFRobot Gravity)
- OLED display SSD1306 (128x32 or similar
- Optocoupler 4N24 or 4N26, or similar
- a resistor (~460ohm)
- 2x tact switch buttons
- 3.5mm stereo socket (3 pole)
- 3.5mm->2.5mm stereo (3 pole) cable (for Canon - for others manufactures refer to this [site](http://www.doc-diy.net/photo/remote_pinout/))
- proto board, connectors etc

## Software

List of software/libraries you will need:
- [Arduino](https://www.arduino.cc/en/software)
- [Lightning Sensor library](https://github.com/DFRobot/DFRobot_AS3935)
- SSD1306 library (install it thru Arduino as described [here[(https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/))

## Schematics (wiring)

The schematic of wiring is quite simple, take a look here:
![Schematics](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/other/schematics.jpg)

## Screenshots / Photos

### Screenshots
![Screenshots](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/screenshots/lightning-demo.png)


### Photos
[![Assembled](https://github.com/pskowronek/Lightning-camera-trigger/raw/main/www/assembled/01.JPG)](https://pskowronek.github.io/Lightning-camera-trigger/www/assembled/index.html "Photos of assembled Lightning camera trigger")

More photos of the assembled Lightning Camera Trigger enclosed in a paper-clip box are [here]((https://pskowronek.github.io/Lightning-camera-trigger/www/assembled/index.html "Photos of assembled Lightning camera trigger").

## TODO

- test it out during stormy seasons
- the program may require tweaks
  - proper calibration procedure (now it runs on default settings)

## License

The code is licensed under Apache Licence 2.0, pictures under Creative Commons BY-NC.

## Authors

- [Piotr Skowronek](https://github.com/pskowronek)
