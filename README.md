# Arduino-Fish-Feeder
Software used with an Arduino UNO, OSEPP LCD Keypad Shield, EZRobot 360 servo, to control an auger used to deliver food to fish on a scheduled basis.  The power source is a 6x1.5v AA battery pack.

# Features
- Control time between feedings;
- Control duration of auger rotation time when feeding;
- Automatic feeding;
- Defaults to automatic feeding program when the arduino starts;
- Manual feeding;
- Default settings saved to EEPROM and automatically loaded when arduino starts;

# Instructions
By default, the software starts in the "Feeding In:" mode that displays a count down timer.  To exit this menu selection, and access the main menu use the "left" button.

Note:  The arduino clock is not precice and you will need to adjust your time values in consequence.

In the main menu, you will see the following options.  Using the "up" and "down" option, you can move the arrow to the desired menu setting.  Click on the "right" button to select that menu entry. 
- Run Feeder
- Wait Time
- Feed Time
- Feed Now
- Save Settings
- Hi Mom!

Run Feeder: This menu entry will run the feeding program.  The software will display a countdown timer and wait until the wait time defined in the Wait Time menu option.  When the time is elapsed, it will trigger the auger for the period indicated by the Feed Time.  Once the feeding is completed, the "Wait Time" is reset back to it's default value. 

Wait Time: This menu entry allows you to setup the wait time.  Use the "right" buttons to change between controlling the Hour, Minute and Second.  Use the "up" and "down" button to increase or decrease the selected time interval.  

Feed Time: This menu entry allows you to setup the feed time.  Use the "up" and "down" buttons to increase and decrease the time the auger will rotate during a feeding.  

Feed Now: This menu entry allows you to manually trigger a feeding. Selecting this menu entry will automatically trigger the auger.  Clicking on the "right" button again will trigger the auger again. 

Save Settings:  This menu entry will store your Wait Time and Feed Time values to the Arduino EEPROM.  A restart of the Arduino will automatically load your values.  This is useful in case of a power outage. 

Use the "left" button to exit this menu selection and return to the main menu.

# Hardware
- Arduino UNO
- OSEPP LCD Keypad Shield
- EZRobot 360 Servo
- 6x1.5v AA battery holder.

# Challenges
- Finding an easy arduino menu to extend.  Thank you Paul Siewert for your easy to configure menu.
- Data pins between OSEPP LCD and Arduino DO NOT LINE UP!  (Read the manual)

# Sources
Why program when you can integrate!  This software uses:
- Original Menu Code by Paul Siewert - LCD Button Shield Menu: http://ha2eqd.com/arduino/018_MENU_menu/MENU_LM75_AM2302__EEPROM_EQD.html
- Thank you Active for proposing the OSEPP LCD KEYPAD SHIELD for use with the Arduino UNO: https://www.active123.com/
- Thingiverse for the 3d printed feeder parts that leverages PVC parts: https://www.thingiverse.com/thing:2387904
- EZRobot for the 360 Servo: https://www.ez-robot.com/Shop/AccessoriesDetails.aspx?prevCat=9&productNumber=5
- OSEPP LCD Keypad Shield Reference: https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)#Pin_Allocation



