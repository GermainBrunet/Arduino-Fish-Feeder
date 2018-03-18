/***************************************************************************************
    Name    : Arduino Fish Feeder
    Author  : Germain Brunet
    Created : March 18, 2018
    Last Modified: March 18, 2018
    Version : 1.0
    Notes   : This code is for use with an Arduino Uno and LCD/button shield. The
              intent is for anyone to use this program to give them a starting
              program with a fully functional menu with minimal modifications
              required by the user.
    License : This program is free software. You can redistribute it and/or modify
              it under the terms of the GNU General Public License as published by
              the Free Software Foundation, either version 3 of the License, or
              (at your option) any later version.
              This program is distributed in the hope that it will be useful,
              but WITHOUT ANY WARRANTY; without even the implied warranty of
              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
              GNU General Public License for more details.
    Sources:  Original Menu Code by Paul Siewert - LCD Button Shield Menu
              http://ha2eqd.com/arduino/018_MENU_menu/MENU_LM75_AM2302__EEPROM_EQD.html
              The guys at Active components for proposing the OSEPP LCD KEYPAD SHIELD for use with the Arduino UNO
              https://www.active123.com/
              Thingiverse for the 3d printed feeder parts.
              https://www.thingiverse.com/thing:2387904
              EZRobot for the 360 Servo
              https://www.ez-robot.com/Shop/AccessoriesDetails.aspx?prevCat=9&productNumber=5
              OSEPP LCD Keypad Shield Reference
              https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)#Pin_Allocation
 ***************************************************************************************/
/*
   This program is designed to get you as close as possible to a finished menu for the standard Arduino Uno LCD/button shield. The only required modifications
   are to add as menu items to the master menu (menuItems array) and then modify/adjust the void functions below for each of those selections.
*/

// You can have up to 10 menu items in the menuItems[] array below without having to change the base programming at all. Name them however you'd like. Beyond 10 items, you will have to add additional "cases" in the switch/case
// section of the operateMainMenu() function below. You will also have to add additional void functions (i.e. menuItem11, menuItem12, etc.) to the program.
String menuItems[] = {"Run Feeder", "Wait Time", "Feed Time", "Feed Now", "Save Settings", "Hi Mom!"};

// Navigation button variables
int readKey;
int savedDistance = 0;

// Menu control variables
int menuPage = 0;
int maxMenuPages = round(((sizeof(menuItems) / sizeof(String)) / 2) + .5);
int cursorPosition = 0;

int wait_time_seconds = 0; // seconds
int wait_time_minutes = 0; // minutes
int wait_time_hours   = 8; // hours
int feed_time = 5; // seconds
int redraw = 0;
int defaultMenuItem1 = 1;
int pos = 0;
int servoPin = 2;
int pulse = 1500;

// Creates 3 custom characters for the menu display
byte downArrow[8] = {
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b10101, // * * *
  0b01110, //  ***
  0b00100  //   *
};

byte upArrow[8] = {
  0b00100, //   *
  0b01110, //  ***
  0b10101, // * * *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100  //   *
};

byte menuCursor[8] = {
  B01000, //  *
  B00100, //   *
  B00010, //    *
  B00001, //     *
  B00010, //    *
  B00100, //   *
  B01000, //  *
  B00000  //
};

#include <Wire.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#include <EEPROM.h>

// Setting the LCD shields pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
Servo myservo;

void setup() {

  // Initializes serial communication
  Serial.begin(9600);

  myservo.attach(2);
  myservo.write(78); // STOP SERVO
  // myservo.writeMicroseconds(1500);

  // Initializes and clears the LCD screen
  lcd.begin(16, 2);
  lcd.clear();

  // Creates the byte for the 3 custom characters
  lcd.createChar(0, menuCursor);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);

  // Load EPROM
  int available_saved_value = EEPROMReadInt(1);
  if (available_saved_value == 1252) { 
    wait_time_seconds = EEPROMReadInt(3); // seconds
    wait_time_minutes = EEPROMReadInt(5); // minutes
    wait_time_hours   = EEPROMReadInt(7); // hours
    feed_time         = EEPROMReadInt(9); // seconds
  }
}

void loop() {
  mainMenuDraw();
  drawCursor();
  operateMainMenu();
  // menuItem1();
}

// This function will generate the 2 menu items that can fit on the screen. They will change as you scroll through your menu. Up and down arrows will indicate your current menu position.
void mainMenuDraw() {
  Serial.print(menuPage);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(menuItems[menuPage]);
  lcd.setCursor(1, 1);
  lcd.print(menuItems[menuPage + 1]);
  if (menuPage == 0) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  } else if (menuPage > 0 and menuPage < maxMenuPages) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  } else if (menuPage == maxMenuPages) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
}

// When called, this function will erase the current cursor and redraw it based on the cursorPosition and menuPage variables.
void drawCursor() {
  for (int x = 0; x < 2; x++) {     // Erases current cursor
    lcd.setCursor(0, x);
    lcd.print(" ");
  }

  // The menu is set up to be progressive (menuPage 0 = Item 1 & Item 2, menuPage 1 = Item 2 & Item 3, menuPage 2 = Item 3 & Item 4), so
  // in order to determine where the cursor should be you need to see if you are at an odd or even menu page and an odd or even cursor position.
  if (menuPage % 2 == 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is even and the cursor position is even that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is even and the cursor position is odd that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
  }
  if (menuPage % 2 != 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is odd and the cursor position is even that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is odd and the cursor position is odd that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
  }
}


void operateMainMenu() {

  int activeButton = 0;

  if (defaultMenuItem1 == 1) {
    menuItem1();
    defaultMenuItem1 = 0;
    activeButton = 1;
  }
  
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0: // When button returns as 0 there is no action taken
        break;
      case 1:  // This case will execute if the "forward" button is pressed
        button = 0;
        switch (cursorPosition) { // The case that is selected here is dependent on which menu page you are on and where the cursor is.
          case 0:
            menuItem1();
            break;
          case 1:
            menuItem2();
            break;
          case 2:
            menuItem3();
            break;
          case 3:
            menuItem4();
            break;
          case 4:
            menuItem5();
            break;
          case 5:
            menuItem6();
            break;
          case 6:
            menuItem7();
            break;
          case 7:
            menuItem8();
            break;
          case 8:
            menuItem9();
            break;
          case 9:
            menuItem10();
            break;
        }
        activeButton = 1;
        mainMenuDraw();
        drawCursor();
        break;
      case 2:
        button = 0;
        if (menuPage == 0) {
          cursorPosition = cursorPosition - 1;
          cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        }
        if (menuPage % 2 == 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition - 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));

        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
      case 3:
        button = 0;
        if (menuPage % 2 == 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition + 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
    }
  }
}

// This function is called whenever a button press is evaluated. The LCD shield works by observing a voltage drop across the buttons all hooked up to A0.
int evaluateButton(int x) {
  int result = 0;
  if (x < 50) {
    result = 1; // right
  } else if (x < 195) {
    result = 2; // up
  } else if (x < 380) {
    result = 3; // down
  } else if (x < 790) {
    result = 4; // left
  }
  return result;
}

// If there are common usage instructions on more than 1 of your menu items you can call this function from the sub
// menus to make things a little more simplified. If you don't have common instructions or verbage on multiple menus
// I would just delete this void. You must also delete the drawInstructions()function calls from your sub menu functions.
void drawInstructions() {
  lcd.setCursor(0, 1); // Set cursor to the bottom line
  lcd.print("Use ");
  lcd.write(byte(1)); // Up arrow
  lcd.print("/");
  lcd.write(byte(2)); // Down arrow
  lcd.print(" buttons");
}

void menuItem1() { // Function executes when you select the 2nd item from main menu
  int activeButton = 0;
  int redraw = 1;
  int period = 0;
  int counter = 0;
  int time_to_feed_hours   = wait_time_hours;
  int time_to_feed_minutes = wait_time_minutes;
  int time_to_feed_seconds = wait_time_seconds;

  while (activeButton == 0) {

    if (redraw == 1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Feeding In: ");
        lcd.setCursor(0, 1);
        if (time_to_feed_hours < 10) lcd.print("0");
        lcd.print(time_to_feed_hours);
        lcd.print(":");
        if (time_to_feed_minutes < 10) lcd.print("0");
        lcd.print(time_to_feed_minutes);
        lcd.print(":");
        if (time_to_feed_seconds < 10) lcd.print("0");
        lcd.print(time_to_feed_seconds);

        // lcd.setCursor(0, 1);
        // lcd.print(feed_time);
        // lcd.print("/");
        //lcd.print(feed_time);
        //lcd.print(time_to_feed);
        //lcd.print("s");
        lcd.setCursor(15, 1);
        // lcd.print(counter+1);
        if (period == 1) {
          lcd.print(".");
        } else {
          lcd.print(" ");
        }
        redraw = 0;
    }

    delay(100);
    counter = counter + 1;
    if (counter > 10) {
      counter = 0;
      if (period == 0) {
        period = 1;
      } else {
        period = 0;
      }
      time_to_feed_seconds--;
      if (time_to_feed_seconds == 0 && time_to_feed_minutes == 0 && time_to_feed_hours == 0) {
        // Feed here.
        feedFish();
  
        time_to_feed_hours   = wait_time_hours;
        time_to_feed_minutes = wait_time_minutes;
        time_to_feed_seconds = wait_time_seconds;
      }
      
      if (time_to_feed_seconds < 0) {
        time_to_feed_minutes--;
        time_to_feed_seconds = 59;
      }
      if (time_to_feed_minutes < 0) {
        time_to_feed_hours--;
        time_to_feed_minutes = 59;
      }
      if (time_to_feed_hours < 0) {
        // Feed Fish Here!
        time_to_feed_hours = 0;
      }
      redraw = 1;
    }
    
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

/**
 * WAIT TIME 
 */
void menuItem2() { // Sets the wait time.
  int activeButton = 0;
  int redraw = 1;
  int selection = 0;

  while (activeButton == 0) {

    if (redraw == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Wait Time: ");
      if (selection == 0) {
        lcd.print(" Hour");
      }
      if (selection == 1) {
        lcd.print(" Mins");
      }
      if (selection == 2) {
        lcd.print(" Secs");
      }
      lcd.setCursor(4, 1);
      if (wait_time_hours < 10) lcd.print("0");
      lcd.print(wait_time_hours);
      lcd.print(":");
      if (wait_time_minutes < 10) lcd.print("0");
      lcd.print(wait_time_minutes);
      lcd.print(":");
      if (wait_time_seconds < 10) lcd.print("0");
      lcd.print(wait_time_seconds);
      
      redraw = 0;
    }
    
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 1:  // Right button
        selection++;
        if (selection > 2) selection = 0;
        redraw = 1;
        break;
      case 2:  // Up button
        if (selection == 2) wait_time_seconds++;
        if (wait_time_seconds > 60) {
          wait_time_seconds = 0;
          wait_time_minutes++; 
        }
        if (selection == 1) wait_time_minutes++;
        if (wait_time_minutes > 60) {
          wait_time_minutes = 0;
          wait_time_hours++; 
        }
        if (selection == 0) wait_time_hours++;
        // wait_time_seconds = wait_time_seconds + 100;
        redraw = 1;
        break;
      case 3:  // Down button
        if (selection == 0) wait_time_hours--;
        if (wait_time_hours < 0) wait_time_hours = 0;
        if (selection == 1) wait_time_minutes--;
        if (wait_time_minutes < 0) wait_time_minutes = 0;
        if (selection == 2) wait_time_seconds--;
        if (wait_time_seconds < 0) wait_time_seconds = 0;
        redraw = 1;
        break;
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

/**
 * FEED TIME 
 */
void menuItem3() { // Feed Time
  int activeButton = 0;
  int redraw = 1;

  while (activeButton == 0) {

    if (redraw == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Feed Time");
      lcd.setCursor(0, 1);
      lcd.print(feed_time);
      redraw = 0;
    }

    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 2:  // Up button
        feed_time++;
        redraw = 1;
        break;
      case 3:  // Down button
        feed_time--;
        if (feed_time < 0) feed_time = 0;
        redraw = 1;
        break;
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

/**
 * MANUAL SERVO 
 */
void menuItem4() { // Function to manually activate the servo.
  int activeButton = 0;
  int redraw = 1;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Manual Feeding!");

  feedFish();
  
  while (activeButton == 0) {

    if (redraw == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Manual Feeding!");
      redraw = 0;
    }
    
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 1:  // This case will execute if the "forward" button is pressed
        feedFish();
        redraw = 1;
        //button = 0;
        //activeButton = 1;
        //break;
    }
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

/**
 * SAVE SETTINGS 
 */
void menuItem5() { // Function executes when you select the 5th item from main menu
  int activeButton = 0;


  EEPROMWriteInt(1, 1252);
  EEPROMWriteInt(3, wait_time_seconds);
  EEPROMWriteInt(5, wait_time_minutes);
  EEPROMWriteInt(7, wait_time_hours);
  EEPROMWriteInt(9, feed_time);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Settings Saved!");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem6() { // Function executes when you select the 6th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Love You Mom!");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem7() { // Function executes when you select the 7th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 7");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem8() { // Function executes when you select the 8th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 8");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem9() { // Function executes when you select the 9th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 9");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem10() { // Function executes when you select the 10th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 10");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

/**
 * Send the command to turn the servo and feed the fish.
 */
void feedFish() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Feeding Now!");
  lcd.setCursor(0, 1);
  lcd.print("For ");
  lcd.print(feed_time);
  lcd.print(" secs!");
  // myservo.write(77); // CW SLOW
  myservo.write(76); // CW LESS SLOW
  delay(feed_time * 1000);
  myservo.write(78); // STOP
}

// Ref: http://forum.arduino.cc/index.php?topic=37470.0
void EEPROMWriteInt(int p_address, int p_value)
{
     byte lowByte = ((p_value >> 0) & 0xFF);
     byte highByte = ((p_value >> 8) & 0xFF);

     EEPROM.write(p_address, lowByte);
     EEPROM.write(p_address + 1, highByte);
}

//This function will read a 2 byte integer from the eeprom at the specified address and address + 1
unsigned int EEPROMReadInt(int p_address)
     {
     byte lowByte = EEPROM.read(p_address);
     byte highByte = EEPROM.read(p_address + 1);

     return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}
