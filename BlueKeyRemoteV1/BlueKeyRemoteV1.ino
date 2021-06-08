/*
  Blue Key Remote v1
  Copyright, Rob Latour, 2021
  License: MIT

  This program lets a four button remote act as as a bluetooth keyboard

  Libraries:
      https://github.com/T-vK/ESP32-BLE-Keyboard
      https://github.com/a7md0/WakeOnLan

  Directly connected to the ESP32 are two control buttons:
    1. used to pair the remote control with a 1527 board
    2. used to open a web interface where you can change the keys to be sent when a button on the remote is pressed

  Hardware:

  1x ESP32
     physcal board:  ESP32 Devkit v1
     board manager:  ESP32 Deve Module
    (recommended) Partition scheme: Huge APP (3MB no OTA/1MB SPIFFS)

  1x 1527 reciever baord

  2x momentary switches

  6x LEDs (4 blue, 1 green, 1 red)

  5x 10k resistors

  2x 1k resitors

  1x 4 button remote control

  .................................................................................................................
  . Wiring (also see: schematic.jpg and/or pcb.jpg)                                                               .
  .                                                                                                               .
  .               Btn   Btn   R1    R2    R3    R4    R5    R6    R7    Blue   Blue   Blue   Blue   Green   Red   .
  . ESP2   1527   1     2     10K   10K   10K   10K   10K   1K    1K    LED1   LED2   LED3   LED4   LED5    LED6  .
  ,               A B   A B   A B   A B   A B   A B   A B   A B   A B   +  -   +  -   +  -   +  -   +  -    +  -  .
  .                                                                                                               .
  . GND    GND                                                             X      X      X      X      X       X  .
  . 3.3    V+     X     X                                                                                         .
  . D15    D0                 X                                                                                   .
  . D2     D1                       X                                                                             .
  . D4     D2                             X                                                                       .
  . D5     D3                                   X                                                                 .
  .        VT                                         X                                                           .
  . D18                                                 X                                                         .
  . D13             X                                                                                             .
  . D12                                                     X                                                     .
  . D27                   X                                                                                       .
  . D26                                                           X                                               .
  .                             X                                       X                                         .
  .                                   X                                        X                                  .
  .                                         X                                         X                           .
  .                                               X                                          X                    .
  .                                                           X                                     X             .
  .                                                                 X                                       X     .
  .................................................................................................................

  PCB: https://oshwlab.com/RobLatour/bluekeyremote

  To clear prior pairing codes in the remote

    1. Press and hold the top two buttons until the light on the remote flashes quickly
    2. While continuing to hold down the top button, release the second button
    3. Press and release the second from the top button three times
    4. The light on the remote will blink quickly to indicate success

  To pair a remote control, please follow these steps:

    However, first please note, that pairing a remote control will:
     a. cause the previously paired remote and any clones of it to be unpaired, and
     b. will reset the button values to their defaults

    Having that noted, to pair a remote:
     1. clear the remote of prior pairing codes (see section above)
     2. press down the 'Pair Remote' button on the control board; the red LED will go on
     3. continue to hold down the button for five seconds (the five seconds is used to prevent inadvertently triggering the pairing process)
     4. after five seconds the red LED will turn off, you should then release the 'Pair Remote' button
        (if the 'Pair Remote' button is released before five seconds the red LED will go off and the pairing process will be cancelled)
     5. after you have released the 'Pair Remote' button the red LED will automatically flash eight times as the 1527 is reprogrammed
     6. there will be a short pause, and at this point the 1527 will be unpaired
     7. the red LED will automatically flash one more time, and then it will flash quickly six times
     8. finally press any button on the remote
     9. on the control board a blue LED (corresponding to the button you pushed) will turn on, as will a blue LED on the ESP32, and the remote will be paired
     10. once you see the blue LEDs turn on, release the button on the remote - you are done; the remote and all its buttons are paired

  To setup the keys you want sent when you press a button on the remote

    1.  If you are using a Windows computer and this program has the value below for WORKING_WITH_WINDOWS10 set to true:
        a. press and release the 'Update buttons' button on the control board
           or
           press and hold the bottom button on the remote for five seconds and then release it

    1. (alternative) If you are not using a Windows computer or the value below for WORKING_WITH_WINDOWS10 set to false:
        a. open up your browser
        b. clear the URL line
        c. ensure your cursor is at the start of the url line
        d. press the 'Update buttons' button connected to the ESP32
           or
           press and hold the bottom button on the remote for five seconds and then release it

     2. A web interface will open up automatically after a few seconds
     3. type the button values that you would like used
     4. click the 'Change' button at tbe bottom of the Webpage when done


  To pair the device with a Windows 10 computer:

     1. Go to your computer's settings
     2. Ensure Bluetooth is turned on
     3. Scan for Bluetooth devices
     4. Connect to the device called "Blue Key Remote"

*/

#include <Arduino.h>
#include "user_settings.h"
#include "user_secrets.h"

// ESP32 pin connections:

#define BUTTON_1_PIN                 15
#define BUTTON_2_PIN                  2
#define BUTTON_3_PIN                  4
#define BUTTON_4_PIN                  5

#define THE_1527_PROGRAMMING_PIN     18

#define BUTTON_UPDATE_BUTTONS_PIN    13
#define LED_UPDATE_BUTTONS_PIN       12

#define BUTTON_PAIR_REMOTE_PIN       27
#define LED_PAIR_REMOTE_PIN          26


// EEPROM stuff

#include <EEPROM.h>
#define EEPROM_SIZE       512
#define EEPROM_ADDRESS      0

// Bluetooth stuff

#include <BleKeyboard.h>
BleKeyboard bleKeyboard(BLUETOOTH_DEVICE_NAME, "", 100);

// Wifi Stuff

#include <WiFi.h>
#include <WebServer.h>

const char * wifi_name = SECRET_WIFI_SSID;
const char * wifi_pass = SECRET_WIFI_PASSWORD;

String IP_Address_of_this_client = "";

// Webserver stuff

WebServer server(80);


// Wake On LAN stuff

#include <WakeOnLan.h>
WiFiUDP UDP;
WakeOnLan WOL(UDP);

bool WOL_Request = false;


// Misc stuff
bool The_Web_Interface_Is_Open = false;

#define BUTTON_DOWN                   1

String Button_1_text = "";
String Button_2_text = "";
String Button_3_text = "";
String Button_4_text = "";

const String Extend_Key_Codes[] = {"{LEFT_CTRL}",
                                   "{LEFT_SHIFT}",
                                   "{LEFT_ALT}",
                                   "{LEFT_GUI}",
                                   "{RIGHT_CTRL}",
                                   "{RIGHT_SHIFT}",
                                   "{RIGHT_ALT}",
                                   "{RIGHT_GUI}",
                                   "{UP_ARROW}",
                                   "{DOWN_ARROW}",
                                   "{LEFT_ARROW}",
                                   "{RIGHT_ARROW}",
                                   "{BACKSPACE}",
                                   "{TAB}",
                                   "{RETURN}",
                                   "{ESC}",
                                   "{INSERT}",
                                   "{DELETE}",
                                   "{PAGE_UP}",
                                   "{PAGE_DOWN}",
                                   "{HOME}",
                                   "{END}",
                                   "{CAPS_LOCK}",
                                   "{F1}",
                                   "{F2}",
                                   "{F3}",
                                   "{F4}",
                                   "{F5}",
                                   "{F6}",
                                   "{F7}",
                                   "{F8}",
                                   "{F9}",
                                   "{F10}",
                                   "{F11}",
                                   "{F12}",
                                   "{F13}",
                                   "{F14}",
                                   "{F15}",
                                   "{F16}",
                                   "{F17}",
                                   "{F18}",
                                   "{F19}",
                                   "{F20}",
                                   "{F21}",
                                   "{F22}",
                                   "{F23}",
                                   "{F24}",
                                   "{RELEASE}",
                                   "{RELEASE_ALL}",
                                   "{KEY_MEDIA_NEXT_TRACK}",
                                   "{KEY_MEDIA_PREVIOUS_TRACK}",
                                   "{KEY_MEDIA_STOP}",
                                   "{KEY_MEDIA_PLAY_PAUSE}",
                                   "{KEY_MEDIA_MUTE}",
                                   "{KEY_MEDIA_VOLUME_UP}",
                                   "{KEY_MEDIA_VOLUME_DOWN}",
                                   "{KEY_MEDIA_WWW_HOME}",
                                   "{KEY_MEDIA_LOCAL_MACHINE_BROWSER}",
                                   "{KEY_MEDIA_CALCULATOR}",
                                   "{KEY_MEDIA_WWW_BOOKMARKS}",
                                   "{KEY_MEDIA_WWW_SEARCH}",
                                   "{KEY_MEDIA_WWW_STOP}",
                                   "{KEY_MEDIA_WWW_BACK}",
                                   "{KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION}",
                                   "{KEY_MEDIA_EMAIL_READER}",
                                   "{!1}",                  // 1 second delay
                                   "{!2}",                  // 2 second delay
                                   "{!3}",                  // 3 second delay
                                   "{!4}",                  // 4 second delay
                                   "{!5}",                  // 5 second delay
                                   "{!6}",                  // 6 second delay
                                   "{!7}",                  // 7 second delay
                                   "{!8}",                  // 8 second delay
                                   "{!9}",                  // 9 second delay
                                   "{WOL}",                 // Wakeup On Lan; format of coammand is: {WOL} AA:BB:CC:DD:EE:FF
                                   "{LOCK}",                // Windows 10 lock command
                                   "{SLEEP}",               // Windows 10 sleep command
                                   "{SIGNOUT}",             // Windows 10 signout command
                                   "{RESTART}",             // Windows 10 restart command
                                   "{SHUTDOWN}",            // Windows 10 shutdown command
                                   "{MACRO_1}",             // used for button 1 macro
                                   "{MACRO_2}",             // used for button 2 macro
                                   "{MACRO_3}",             // used for button 3 macro
                                   "{MACRO_4}",             // used for button 4 macro
                                   "{ENTER}",               // alias for Return
                                   "{WINDOWS}"              // alias for Left_GUI (Windows Key)
                                  };

int Number_Of_Extended_Key_Codes;  // automatically calculated in setup()


//******************************************************************************************************************************************************************

void SetupWOL() {

  WOL.setRepeat(3, 100); // Optional, repeat the packet three times with 100ms between. WARNING delay() is used between send packet function.
  WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask()); // Optional  => To calculate the broadcast address, otherwise 255.255.255.255 is used (which is denied in some networks).
  Serial.println("WOL has been setup");
  Serial.println("");

}

void SendMagicPacket(String MAC_Address) {

  // Flash update buttons LED three times to indicate start of WOL process
  FlashLED(LED_UPDATE_BUTTONS_PIN, 3, 250);

  Serial.println("");

  if (SetupWiFi()) {

    MAC_Address.toUpperCase();

    Serial.print("Sending WOL to ");
    Serial.println(MAC_Address);
    Serial.println("");

    WOL.sendMagicPacket(MAC_Address); // Send Wake On Lan packet with the above MAC address. Default to port 9.

    WiFi.disconnect(true);
    Serial.println("WiFi disconnected");
    Serial.println("");

    // Flash update buttons LED three times to indicate successful end of WOL process
    FlashLED(LED_UPDATE_BUTTONS_PIN, 3, 250);

  };

}

//******************************************************************************************************************************************************************

bool SetupWiFi() {

  bool Return_value;  // returns true if a connection is made

  int  Number_of_seconds_to_try_to_connect = 3;
  int  counter;
  bool Not_yet_connected = true;

  while ( (Not_yet_connected) && (Number_of_seconds_to_try_to_connect <= 10) ) {

    Serial.print("Attempting to connect to WiFi");

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_name, wifi_pass);
    delay(1000);

    counter = 0;

    while ( ( WiFi.status() != WL_CONNECTED ) && (counter < Number_of_seconds_to_try_to_connect) )
    {

      Serial.print(".");
      counter++;
      delay (1000);
    }

    Serial.println("");

    if ( WiFi.status() == WL_CONNECTED )
    {
      Not_yet_connected = false;
    }
    else
    {

      // at this point the system has not yet connected
      // so increase the number of seconds by three, reset things (which will take two seconds), and try to connect again
      // however give up if the number of seconds to try and connect goes beyond 10
      // total try time will have been ( 3 + 2 ) + ( 6 + 2 ) + ( 9 + 2 ) = with 24 seconds having been spent in total

      Number_of_seconds_to_try_to_connect = Number_of_seconds_to_try_to_connect + 3;

      if (Number_of_seconds_to_try_to_connect <= 10) {

        WiFi.disconnect(true);
        delay(1000);
        WiFi.mode(WIFI_STA);
        delay(1000);

      }

    }

  };

  if (Not_yet_connected) {

    Serial.println("Could not connect to WiFi");

    Return_value = false;

  } else {

    IP_Address_of_this_client = WiFi.localIP().toString();

    Serial.print("Connected to ");
    Serial.print(wifi_name);
    Serial.print(" at IP address ");
    Serial.println(IP_Address_of_this_client);
    Serial.println("");

    Return_value = true;

  }

  return Return_value;

};

//******************************************************************************************************************************************************************

void UpdateLED(int LEDPin, bool TurnOn) {

  if (TurnOn)
    digitalWrite(LEDPin, HIGH);
  else
    digitalWrite(LEDPin, LOW);

}

void FlashLED(int LEDPin, int Times, int BlinkTime) {

  // ensure the LED is off
  UpdateLED(LEDPin, false);

  int HalfBlinkTime = BlinkTime / 2;

  // flash LED
  for (int i = 1; i <= Times; i++) {

    UpdateLED(LEDPin, true);

    delay(HalfBlinkTime);

    UpdateLED(LEDPin, false);

    delay(HalfBlinkTime);

  };

}

//******************************************************************************************************************************************************************

void VirtuallyPressAndReleaseThe1527ProgrammingButton(int NumberOfTimes) {

  for (int i = 1; i <= NumberOfTimes; i++) {

    digitalWrite(THE_1527_PROGRAMMING_PIN, HIGH); // virtually press the button down

    FlashLED(LED_PAIR_REMOTE_PIN, 1, 1000);      // wait 1 second, and flash the LED in that time

    digitalWrite(THE_1527_PROGRAMMING_PIN, LOW);  // virtually release the button

  }

}

void SendResetSequenceTo1527Board() {

  VirtuallyPressAndReleaseThe1527ProgrammingButton(8);  // virtually press the programming button eight times to clear all prior codes
  delay(5000);                                          // wait five seconds
  VirtuallyPressAndReleaseThe1527ProgrammingButton(1);  // virtually press the programming button one time to enable programming of the remote

}

//******************************************************************************************************************************************************************


String CorrectCaseOnExtendedKeyCodes(String OriginalText) {

  // This routine will return the original text the same as it was before except:
  //    Extended Key Code values will be capitalized, and
  //    spaces between Extended Key Code words, such as the space between "PAGE UP" will be converted to underscores.

  // To do this a Mask will be the create which is the same as the Original text except when it is done being created:
  //   the associated characters where extended keys are used will be replaced by char(255)
  //   spaces will be converted to underscores (explained below)
  //   everything else will be capitalized

  String MasterMask = OriginalText;
  MasterMask.toUpperCase();      // this allows the user to use mixed case instead of all upper case in an extended key, for example: {Return} instead of {RETURN}
  MasterMask.replace(" ", "_");  // this allows the user to use a space instead of an underscore as an extended key word seperator, for example: {up arrow} instead of {up_arrow}

  // the Little Mask is just a string of 50 char(255)s
  // it is used when replaceing extended keys in the Master Mask
  String LittleMask = "";
  for (int i = 1; i <= 50; i++)
    LittleMask +=  String(char(255));

  String WorkingMask;
  for (int i = 0; i <= Number_Of_Extended_Key_Codes - 1; i++) {

    // create a working mask the same size as the extended key
    WorkingMask = LittleMask;
    WorkingMask.remove(Extend_Key_Codes[i].length());

    // where a extended key is used, replace it by a similar sized mask (i.e. with all char(255)s)
    MasterMask.replace(Extend_Key_Codes[i], WorkingMask);

  };

  // Serial.print("Original    : "); Serial.println(OriginalText);
  // Serial.print("Master Mask : "); Serial.println(MasterMask);

  String Result = "";
  String ws;
  for (int i = 0; i <= OriginalText.length() - 1; i++) {

    ws = String(OriginalText.charAt(i));

    if ((int)MasterMask.charAt(i) == 255)
      ws.toUpperCase();

    Result.concat(ws);

  };

  // Serial.print("Result  : "); Serial.println(Result);

  return Result;

}

//******************************************************************************************************************************************************************

String ConvertStringReducingExtendedKeyCodes(String input) {

  // This subroutine takes a string of text and converts any Extended Key Code values in it to a two byte code for more effective use of storage

  // More specifically, this subroutine replaces each Extended Key Code in the input string with a unique two byte value which will be used when storing it in perminate memory.
  // This is needed as the updated input string will be stored in perminate memory, and perminate memory is limited to only 512 bytes.
  // In other words, rather than storing the literal "{PAGE_UP}" in perminate memory, a unique two byte code is used in its place.
  // Of the two bytes:
  //    the first byte is set to char(254), which designate it and the following byte as ane extended key code in storage.
  //    the second byte is a number unique to each key code; numbering begins at 100.
  //    ( the numbering begins at 100 as various byte values through to 92 are already needed for storing one byte escape codes,
  //      An example of an escape code is the charage return character, "/n", which is stored as char(10). )
  //
  // Escape codes are explained in more detail in the subroutine ConvertStringReducingEscapeCodes below
  //
  // Regardless, for now, just know that this subroutine reduces the size of the input string by replacing Extended Key Code values with unique two byte codes
  //

  input = CorrectCaseOnExtendedKeyCodes(input);

  input = SimplifyMacrosAndWindow10Commands(input);

  if (input.indexOf("{") >= 0)  // if there are no "{"s in the input string, then there are no Extended Key Codes in it and no further processing is needed
  {

    int BaseValue = 100;
    for (int i = 0; i <= Number_Of_Extended_Key_Codes - 1; i++)
      input.replace(Extend_Key_Codes[i], String(char(254)) + String(char(BaseValue + i)));

  };

  return input;

}

String ConvertStringExpandingExtendedKeyCodes(String input) {

  // used to revert a two byte code into its its expanded Extended Key Code Value
  // char(254) in the first byte indicates an extended key is being used, the second byte denotes which one in particular
  // see ConvertStringReducingExtendedKeyCodes for more information

  String Return_value = "";

  int len = input.length() + 1;

  char wc[len];
  input.toCharArray(wc, len);

  char flag = char(254);

  int BaseValue = 100;

  for (int i = 0; i < len - 1; i++)
    if (int(wc[i]) == flag)
      Return_value.concat(Extend_Key_Codes[ int( wc[++i] ) - BaseValue ] );
    else
      Return_value.concat(String(wc[i]));

  return Return_value;

}

String ConvertStringReducingEscapeCodes(String input) {

  // used to convert these larger two character string codes into a one byte charactor code;

  /*

    \a – bell/alert         7
    \b – backspace          8
    \t – horizontal tab     9
    \n – newline           10
    \v - vertical tab      11
    \f - form feed         12
    \r – carriage return   13
    \s – space             32
    \\ – single backslash  92

  */

  String Return_value = "";

  int len = input.length() + 1;

  char wc[len];
  input.toCharArray(wc, len);

  for (int i = 0; i < len - 1; i++) {

    if (int(wc[i]) == 92 ) {

      // don't store the slash, instead advance to the next character and store it's equivalent ascii code
      i++;

      int test = int(wc[i]);

      switch (test) {
        case 92:
          Return_value.concat(String("\\"));
          break;
        case 97:
          Return_value.concat(String("\a"));
          break;
        case 98:
          Return_value.concat(String("\b"));
          break;
        case 102:
          Return_value.concat(String("\f"));
          break;
        case 110:
          Return_value.concat(String("\n"));
          break;
        case 114:
          Return_value.concat(String("\r"));
          break;
        case 116:
          Return_value.concat(String("\t"));
          break;
        case 118:
          Return_value.concat(String("\v"));
          break;
        default:
          Return_value.concat("~??~");
          break;

      }

    } else

      Return_value.concat(String(wc[i]));

  };

  return Return_value;

};

String ConvertStringExpandingEscapeCodes(String input) {

  // used to convert one byte escape charcters to their two character string equivalents

  input.replace("\a", "\\a");
  input.replace("\b", "\\b");
  input.replace("\t", "\\t");
  input.replace("\v", "\\v");
  input.replace("\f", "\\f");
  input.replace("\\", "\\\\");
  input.replace("\n", "\\n");
  input.replace("\r", "\\r");

  return input;

};

String FullyReducedForStorage(String input) {

  input = ConvertStringReducingEscapeCodes(input);
  input = ConvertStringReducingExtendedKeyCodes(input);
  return input;

};

String FullyExpandedFromStorage(String input) {

  input = ConvertStringExpandingExtendedKeyCodes(input);
  input = ConvertStringExpandingEscapeCodes(input);
  return input;

};


//*****************************************************************************************************************************************************************

bool DataInEEPROMAppearsOK() {

  // load the current key string from memory and count the number of seperators denoted by char(255)
  // If there are three seperators the data in the EEPROM may be assumed to be ok

  char Seperator = char(255);

  char wc[EEPROM_SIZE];
  EEPROM.get(EEPROM_ADDRESS, wc);

  int len = String(wc).length();

  int count = 0;

  for (int i = 0; i <= len; i++) {
    if ( wc[i] == Seperator)
      count++;
  };

  return (count == 3);

};

boolean UpdateEEPROM() {

  bool Return_value;

  const String Seperator = String(char(255));

  String EEPRROM_String = Button_1_text + Seperator + Button_2_text + Seperator + Button_3_text + Seperator + Button_4_text;

  EEPRROM_String = FullyReducedForStorage(EEPRROM_String);

  int len = EEPRROM_String.length();

  if (len <= EEPROM_SIZE) {

    char wc[len + 1];
    EEPRROM_String.toCharArray(wc, len + 1);

    int data;
    for (int i = EEPROM_ADDRESS; i < len; i++) {
      data = wc[i];
      if (data != EEPROM.read(i)) {
        EEPROM.write(i, data);
      };
    };

    // clear the rest of the EEPROM memory
    for (int i = len; i <= EEPROM_SIZE; i++)
      if (EEPROM.read(i) != 0)
        EEPROM.write(i, 0);

    EEPROM.commit();

    Return_value = DataInEEPROMAppearsOK();

  }
  else
    Return_value = false;

  return Return_value;

}

String GetASpecificButtonValue(String data, int index) {

  String Return_value = "";

  const char Seperator = char(255);

  char wc;
  int i = 0;
  int CurrentIndex = 1;
  int len = data.length();

  do {
    wc = data.charAt(i++);
    if (wc == Seperator)
      CurrentIndex++;
    else if (CurrentIndex == index)
      if (wc > 0)
        Return_value.concat(String(wc));
  } while ( (CurrentIndex <= index) && ( i <= len) );

  return Return_value;

}

void LoadFromEEPROM() {

  char wc[EEPROM_SIZE];
  EEPROM.get(0, wc);
  String AllButtonValues = String(wc);

  Button_1_text = GetASpecificButtonValue(AllButtonValues, 1);
  Button_2_text = GetASpecificButtonValue(AllButtonValues, 2);
  Button_3_text = GetASpecificButtonValue(AllButtonValues, 3);
  Button_4_text = GetASpecificButtonValue(AllButtonValues, 4);

  /*
    Serial.println("Stored button values:");
    Serial.print(BUTTON_1_LABEL); Serial.print(": "); Serial.println(Button_1_text);
    Serial.print(BUTTON_2_LABEL); Serial.print(": "); Serial.println(Button_2_text);
    Serial.print(BUTTON_3_LABEL); Serial.print(": "); Serial.println(Button_3_text);
    Serial.print(BUTTON_4_LABEL); Serial.print(": "); Serial.println(Button_4_text);
  */

  Button_1_text = FullyExpandedFromStorage(Button_1_text);
  Button_2_text = FullyExpandedFromStorage(Button_2_text);
  Button_3_text = FullyExpandedFromStorage(Button_3_text);
  Button_4_text = FullyExpandedFromStorage(Button_4_text);

  Serial.println("Expanded button values:");
  Serial.print(BUTTON_1_LABEL); Serial.print(": ");  Serial.println(Button_1_text);
  Serial.print(BUTTON_2_LABEL); Serial.print(": "); Serial.println(Button_2_text);
  Serial.print(BUTTON_3_LABEL); Serial.print(": "); Serial.println(Button_3_text);
  Serial.print(BUTTON_4_LABEL); Serial.print(": "); Serial.println(Button_4_text);
  Serial.println("");

}

void SetupEEPROM(bool ResetRequired) {

  EEPROM.begin(EEPROM_SIZE);

  if ( ResetRequired || !DataInEEPROMAppearsOK() ) {
    Button_1_text = BUTTON_1_LABEL;
    Button_2_text = BUTTON_2_LABEL;
    Button_3_text = BUTTON_3_LABEL;
    Button_4_text = BUTTON_4_LABEL;
    UpdateEEPROM();
  };

  LoadFromEEPROM();

}

//******************************************************************************************************************************************************************

String SimplifyMacrosAndWindow10Commands(String input) {

  input.replace(MACRO_1, "{MACRO_1}");
  input.replace(MACRO_2, "{MACRO_2}");
  input.replace(MACRO_3, "{MACRO_3}");
  input.replace(MACRO_4, "{MACRO_4}");

  if (WORKING_WITH_WINDOWS10) {
    input.replace(W10_LOCK_MACRO, "{LOCK}");
    input.replace(W10_SLEEP_MACRO, "{SLEEP}");
    input.replace(W10_SIGNOUT_MACRO, "{SIGNOUT}");
    input.replace(W10_RESTART_MACRO, "{RESTART}");
    input.replace(W10_SHUTDOWN_MACRO, "{SHUTDOWN}");
  };

  return input;

}

String ConvertForWebDisplay(String input) {

  input.replace(">", "&gt;");
  input.replace("<", "&lt;");
  input.replace("&", "&amp;");
  input.replace(String(char(34)), "&quot;");
  input.replace(String(char(162)), "&cent;");
  input.replace(String(char(163)), "&pound;");
  input.replace(String(char(165)), "&yen;");
  input.replace(String(char(169)), "&copy;");
  input.replace(String(char(174)), "&reg;");

  input = SimplifyMacrosAndWindow10Commands(input);

  return input;

};

String UpdateHomePageWithCurrentButtonValues() {

  const String index_html_string = R"rawliteral("
<!DOCTYPE html>
<html lang="en">
 <head>
  <title>Blue Key Remote</title>
  <style>
  input[type=text] {
   width: 100%;
   padding: 12px 20px;
   margin: 8px 0;
   box-sizing: border-box;
   border: 2px solid blue;
   border-radius: 4px;
   }
  </style>
 </head>
 <body>

  <h3>Blue Key Remote button setup</h3>
  
  To update one or more values, overtype them and click 'Update'.<br><br>
  
  <form name="DataForm" action="/get">
  
   <label>btn*1</label>
   <input type="text" name="IN1" placeholder="value*1" autofocus><br><br>
   
   <label>btn*2</label>
   <input type="text" name="IN2" placeholder="value*2"><br><br>
   
   <label>btn*3</label>
   <input type="text" name="IN3" placeholder="value*3"><br><br>
   
   <label>btn*4</label>
   <input type="text" name="IN4" placeholder="value*4"><br><br>
   
   <input name="Update" type="submit" value="Update">
   &nbsp;&nbsp;
   <input name="Cancel" type="submit" value="Cancel">
   
  </form>

 </body>

</html>
")rawliteral";

  String Return_value = index_html_string;
  Return_value.remove(0, 2);                                           // trim opening quote and cr from raw literal
  Return_value.remove(Return_value.length() - 2);                       // trim closing quote from raw literal

  Return_value.replace("btn*1", BUTTON_1_LABEL);
  Return_value.replace("btn*2", BUTTON_2_LABEL);
  Return_value.replace("btn*3", BUTTON_3_LABEL);
  Return_value.replace("btn*4", BUTTON_4_LABEL);

  Return_value.replace("value*1", ConvertForWebDisplay(Button_1_text));
  Return_value.replace("value*2", ConvertForWebDisplay(Button_2_text));
  Return_value.replace("value*3", ConvertForWebDisplay(Button_3_text));
  Return_value.replace("value*4", ConvertForWebDisplay(Button_4_text));

  return Return_value;

}

//******************************************************************************************************************************************************************

void CreateAResponseWebpage(int ResponseCode, String Text) {

  String Template = "<!DOCTYPE html><html><head><title>Blue Key Remote</title></head><body>$</body></html>";
  Template.replace("$", Text);
  server.send(ResponseCode, "text/html", Template);

};

//******************************************************************************************************************************************************************

void handleDataIn() {

  /*
    String message = "";
    Serial.println("Data received: ");
    for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + " " + server.arg(i).length() + ": " + server.arg(i) + "\n";
    };
    Serial.println(message);
  */

  bool Canceled = false;

  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "Cancel") {
      Canceled = true;
      break;
    }
  }

  if (Canceled) {

    CreateAResponseWebpage(200, "No updates made. Have a nice day!");

  } else
  {

    int Webpage_updates = 0;

    String Button_1_new;
    String Button_2_new;
    String Button_3_new;
    String Button_4_new;

    if (server.arg("IN1").length() == 0)
      Button_1_new = Button_1_text;
    else {
      Button_1_new = server.arg("IN1");
      if (Button_1_new != Button_1_text)
        Webpage_updates++;
    };

    if (server.arg("IN2").length() == 0)
      Button_2_new = Button_2_text;
    else {
      Button_2_new = server.arg("IN2");
      if (Button_2_new != Button_2_text)
        Webpage_updates++;
    };

    if (server.arg("IN3").length() == 0)
      Button_3_new = Button_3_text;
    else {
      Button_3_new = server.arg("IN3");
      if (Button_3_new != Button_3_text)
        Webpage_updates++;
    };

    if (server.arg("IN4").length() == 0)
      Button_4_new = Button_4_text;
    else {
      Button_4_new = server.arg("IN4");
      if (Button_4_new != Button_4_text)
        Webpage_updates++;
    };

    int TotalLength = FullyReducedForStorage(Button_1_new + Button_2_new + Button_3_new + Button_4_new).length();

    if (TotalLength > 509)  // max space in persistant memory is 512 bytes; 3 bytes are needed to seperator the four fields to be stored; leaving 509 bytes for data

      CreateAResponseWebpage(200, "No updates made.<br>This as the total key count exceeds the maximum of 509.<br>Please note extended keys, such as {RETURN}, count as two keys in the total key count.");

    else {

      Button_1_text = Button_1_new;
      Button_2_text = Button_2_new;
      Button_3_text = Button_3_new;
      Button_4_text = Button_4_new;

      bool success = true;

      if ( Webpage_updates > 0 )
        success = UpdateEEPROM();

      if (success) {

        if ( Webpage_updates == 0 )
          CreateAResponseWebpage(200, "No updates made, have a nice day!");
        else if (Webpage_updates == 1 )
          CreateAResponseWebpage(200, "Update made. Please wait about 10 seconds for it to take effect. Have a nice day!");
        else
          CreateAResponseWebpage(200, "Updates made. Please wait about 10 seconds for them to take effect. Have a nice day!");

      } else {

        CreateAResponseWebpage(200, "Keys were changed but could not be saved to permanent memory");

      };

    };

  };

  delay(1000);
  The_Web_Interface_Is_Open = false;

}

void HandleNotFound() {

  String message = "Page not found\n\n";
  message += "URI : ";
  message += server.uri();
  message += "\nMethod : ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments : ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + " : " + server.arg(i) + "\n";
  }

  CreateAResponseWebpage(404, message);

}

//*****************************************************************************************************************************************************************

void LoadWebpage() {

  The_Web_Interface_Is_Open = true;

  String HomePage = UpdateHomePageWithCurrentButtonValues();

  server.on("/", HTTP_GET, [HomePage]() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", HomePage.c_str());
  });

  server.on("/get", handleDataIn);

  server.onNotFound(HandleNotFound);

  server.begin();

};

//*****************************************************************************************************************************************************************

bool UpdateButtonsButtonPressed() {

  bool Return_value = false;

  if (digitalRead(BUTTON_UPDATE_BUTTONS_PIN) == BUTTON_DOWN) {

    // ensure button is down for 100 milliseconds to prevent against ghost signals triggering this routine
    delay(100);

    if (digitalRead(BUTTON_UPDATE_BUTTONS_PIN) == BUTTON_DOWN) {

      // light LED so user knows to let go the button
      UpdateLED(LED_UPDATE_BUTTONS_PIN, true);

      // hold here until button is released
      while (digitalRead(BUTTON_UPDATE_BUTTONS_PIN) == BUTTON_DOWN) {
        delay(10);
      };

      Return_value = true;

    }

  }

  return Return_value;

}


//******************************************************************************************************************************************************************

bool TypeExtendedKey(String Extended_Key) {

  // check for Wake On LAN extended key
  // if found set the WOL_Request flag and return without delay

  if (Extended_Key == "{WOL}") {
    WOL_Request = true;
    return true;
  };

  // check for delay extended key
  // if found delay as appropriate and return

  int wait = 0;
  if (Extended_Key == "{!1}") wait = 1000;
  else if (Extended_Key == "{!2}") wait = 2000;
  else if (Extended_Key == "{!3}") wait = 3000;
  else if (Extended_Key == "{!4}") wait = 4000;
  else if (Extended_Key == "{!5}") wait = 5000;
  else if (Extended_Key == "{!6}") wait = 6000;
  else if (Extended_Key == "{!7}") wait = 7000;
  else if (Extended_Key == "{!8}") wait = 8000;
  else if (Extended_Key == "{!9}") wait = 9000;

  if (wait > 0) {
    delay(wait);
    return true;
  };

  // check for the release all
  // if found, issue release all command, delay and return

  if (Extended_Key == "{RELEASE_ALL}") {
    bleKeyboard.releaseAll();
    delay(MILLISECONDS_DELAY_BETWEEN_KEYSTROKES);
    return true;
  };

  // look for specific extended keys
  // if found, issue them, wait for the normal delay between keystrokes, and return

  bool KeyFound;
  uint8_t SendKey;

  // the following keys use 'press' to send

  KeyFound = true;
  if (Extended_Key == "{LEFT_CTRL}") SendKey = KEY_LEFT_CTRL;
  else if (Extended_Key == "{LEFT_SHIFT}") SendKey = KEY_LEFT_SHIFT;
  else if (Extended_Key == "{LEFT_ALT}") SendKey = KEY_LEFT_ALT;
  else if (Extended_Key == "{LEFT_GUI}") SendKey = KEY_LEFT_GUI;
  else if (Extended_Key == "{RIGHT_CTRL}") SendKey = KEY_RIGHT_CTRL;
  else if (Extended_Key == "{RIGHT_SHIFT}") SendKey = KEY_RIGHT_SHIFT;
  else if (Extended_Key == "{RIGHT_ALT}") SendKey = KEY_RIGHT_ALT;
  else if (Extended_Key == "{RIGHT_GUI}") SendKey = KEY_RIGHT_GUI;
  else if (Extended_Key == "{WINDOWS}") SendKey =  KEY_LEFT_GUI;
  else KeyFound = false;

  if (KeyFound) {
    if (bleKeyboard.isConnected()) {
      bleKeyboard.press(SendKey);
      delay(MILLISECONDS_DELAY_BETWEEN_KEYSTROKES);
    }
    return true;
  };

  // the following keys use 'write' to send

  KeyFound = true;
  if (Extended_Key == "{UP_ARROW}") SendKey = KEY_UP_ARROW;
  else if (Extended_Key == "{DOWN_ARROW}") SendKey = KEY_DOWN_ARROW;
  else if (Extended_Key == "{LEFT_ARROW}") SendKey = KEY_LEFT_ARROW;
  else if (Extended_Key == "{RIGHT_ARROW}") SendKey = KEY_RIGHT_ARROW;
  else if (Extended_Key == "{BACKSPACE}") SendKey = KEY_BACKSPACE;
  else if (Extended_Key == "{TAB}") SendKey = KEY_TAB;
  else if (Extended_Key == "{RETURN}") SendKey = KEY_RETURN;
  else if (Extended_Key == "{ENTER}") SendKey = KEY_RETURN;
  else if (Extended_Key == "{ESC}") SendKey = KEY_ESC;
  else if (Extended_Key == "{INSERT}") SendKey = KEY_INSERT;
  else if (Extended_Key == "{DELETE}") SendKey = KEY_DELETE;
  else if (Extended_Key == "{PAGE_UP}") SendKey = KEY_PAGE_UP;
  else if (Extended_Key == "{PAGE_DOWN}") SendKey = KEY_PAGE_DOWN;
  else if (Extended_Key == "{HOME}") SendKey = KEY_HOME;
  else if (Extended_Key == "{END}") SendKey = KEY_END;
  else if (Extended_Key == "{CAPS_LOCK}") SendKey = KEY_CAPS_LOCK;
  else if (Extended_Key == "{F1}") SendKey = KEY_F1;
  else if (Extended_Key == "{F2}") SendKey = KEY_F2;
  else if (Extended_Key == "{F3}") SendKey = KEY_F3;
  else if (Extended_Key == "{F4}") SendKey = KEY_F4;
  else if (Extended_Key == "{F5}") SendKey = KEY_F5;
  else if (Extended_Key == "{F6}") SendKey = KEY_F6;
  else if (Extended_Key == "{F7}") SendKey = KEY_F7;
  else if (Extended_Key == "{F8}") SendKey = KEY_F8;
  else if (Extended_Key == "{F9}") SendKey = KEY_F9;
  else if (Extended_Key == "{F10}") SendKey = KEY_F10;
  else if (Extended_Key == "{F11}") SendKey = KEY_F11;
  else if (Extended_Key == "{F12}") SendKey = KEY_F12;
  else if (Extended_Key == "{F13}") SendKey = KEY_F13;
  else if (Extended_Key == "{F14}") SendKey = KEY_F14;
  else if (Extended_Key == "{F15}") SendKey = KEY_F15;
  else if (Extended_Key == "{F16}") SendKey = KEY_F16;
  else if (Extended_Key == "{F17}") SendKey = KEY_F17;
  else if (Extended_Key == "{F18}") SendKey = KEY_F18;
  else if (Extended_Key == "{F19}") SendKey = KEY_F19;
  else if (Extended_Key == "{F20}") SendKey = KEY_F20;
  else if (Extended_Key == "{F21}") SendKey = KEY_F21;
  else if (Extended_Key == "{F22}") SendKey = KEY_F22;
  else if (Extended_Key == "{F23}") SendKey = KEY_F23;
  else if (Extended_Key == "{F24}") SendKey = KEY_F24;

  else KeyFound = false;

  if (KeyFound) {
    if (bleKeyboard.isConnected()) {
      bleKeyboard.write(SendKey);
      delay(MILLISECONDS_DELAY_BETWEEN_KEYSTROKES);
    }
    return true;
  };

  uint8_t SendMediaKey[2];

  KeyFound = true;
  if (Extended_Key == "{KEY_MEDIA_NEXT_TRACK}") memcpy(SendMediaKey, KEY_MEDIA_NEXT_TRACK, 2);
  else if (Extended_Key == "{KEY_MEDIA_PREVIOUS_TRACK}") memcpy(SendMediaKey, KEY_MEDIA_PREVIOUS_TRACK, 2);
  else if (Extended_Key == "{KEY_MEDIA_STOP}") memcpy(SendMediaKey, KEY_MEDIA_STOP, 2);
  else if (Extended_Key == "{KEY_MEDIA_PLAY_PAUSE}") memcpy(SendMediaKey, KEY_MEDIA_PLAY_PAUSE, 2);
  else if (Extended_Key == "{KEY_MEDIA_MUTE}") memcpy(SendMediaKey, KEY_MEDIA_MUTE, 2);
  else if (Extended_Key == "{KEY_MEDIA_VOLUME_UP}") memcpy(SendMediaKey, KEY_MEDIA_VOLUME_UP, 2);
  else if (Extended_Key == "{KEY_MEDIA_VOLUME_DOWN}") memcpy(SendMediaKey, KEY_MEDIA_VOLUME_DOWN, 2);
  else if (Extended_Key == "{KEY_MEDIA_WWW_HOME}") memcpy(SendMediaKey, KEY_MEDIA_WWW_HOME, 2);
  else if (Extended_Key == "{KEY_MEDIA_LOCAL_MACHINE_BROWSER}") memcpy(SendMediaKey, KEY_MEDIA_LOCAL_MACHINE_BROWSER, 2);
  else if (Extended_Key == "{KEY_MEDIA_CALCULATOR}") memcpy(SendMediaKey, KEY_MEDIA_CALCULATOR, 2);
  else if (Extended_Key == "{KEY_MEDIA_WWW_BOOKMARKS}") memcpy(SendMediaKey, KEY_MEDIA_WWW_BOOKMARKS, 2);
  else if (Extended_Key == "{KEY_MEDIA_WWW_SEARCH}") memcpy(SendMediaKey, KEY_MEDIA_WWW_SEARCH, 2);
  else if (Extended_Key == "{KEY_MEDIA_WWW_STOP}") memcpy(SendMediaKey, KEY_MEDIA_WWW_STOP, 2);
  else if (Extended_Key == "{KEY_MEDIA_WWW_BACK}") memcpy(SendMediaKey, KEY_MEDIA_WWW_BACK, 2);
  else if (Extended_Key == "{KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION}") memcpy(SendMediaKey, KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION, 2);
  else if (Extended_Key == "{KEY_MEDIA_EMAIL_READER}") memcpy(SendMediaKey, KEY_MEDIA_EMAIL_READER, 2);
  else KeyFound = false;

  if (KeyFound) {
    if (bleKeyboard.isConnected()) {
      bleKeyboard.write(SendMediaKey);
      delay(MILLISECONDS_DELAY_BETWEEN_KEYSTROKES);
      delay(250); // add extra delay for media keys
    };
    return true;
  }

  // extended key was not found
  return false;

}

//******************************************************************************************************************************************************************

void TypeText(String text) {

  //
  // When the web interface is displayed, the escape codes must be included in the keys that are sent.
  // This allows the user to press a button when the web interface is open, and have the entire key sequence,
  // including escape codes, typed into an Webpage input box.
  // Including the escape codes, or not, is controlled by the flag The_Web_Interface_Is_Open
  //
  // note: each time before a key code is sent via bluetooth the program checks to see if bluetooth is connected
  //       if bluetooth is not connected the key is not sent but no error is thrown, rather the routine proceeds as if it were sent
  //       this is by design

  if (The_Web_Interface_Is_Open) {

    text.replace(MACRO_1, "{MACRO_1}");
    text.replace(MACRO_2, "{MACRO_2}");
    text.replace(MACRO_3, "{MACRO_3}");
    text.replace(MACRO_4, "{MACRO_4}");

    if (WORKING_WITH_WINDOWS10) {
      text.replace(W10_LOCK_MACRO, "{LOCK}");
      text.replace(W10_SLEEP_MACRO, "{SLEEP}");
      text.replace( W10_SIGNOUT_MACRO, "{SIGNOUT}");
      text.replace( W10_RESTART_MACRO, "{RESTART}");
      text.replace(W10_SHUTDOWN_MACRO, "{SHUTDOWN}");
    };

  } else {

    text.replace("{MACRO_1}", MACRO_1);
    text.replace("{MACRO_2}", MACRO_2);
    text.replace("{MACRO_3}", MACRO_3);
    text.replace("{MACRO_4}", MACRO_4);

    // Send escape codes to be bluetooth keyed, for example send a carrage return rather than " / n"

    text = ConvertStringReducingEscapeCodes(text);

    // update all the extended keys in the text to be bluetooth keyed
    // this allows the user, for example, to type {return} rather then {RETURN}, or {Page Up} rather than {PAGE_UP}

    text = CorrectCaseOnExtendedKeyCodes(text);

    // make substitutions for specific extended keys as required

    if (WORKING_WITH_WINDOWS10) {
      text.replace("{LOCK}", W10_LOCK_MACRO);
      text.replace("{SLEEP}", W10_SLEEP_MACRO);
      text.replace("{SIGNOUT}", W10_SIGNOUT_MACRO);
      text.replace("{RESTART}", W10_RESTART_MACRO);
      text.replace("{SHUTDOWN}", W10_SHUTDOWN_MACRO);
    };

  };

  // iterate through the text to be bluetooth keyed
  // extended keys are seperated out and processed seperately
  // all other text is more simply sent out to be bluetooth keyed

  int len = text.length();

  bool An_Extended_Key_Was_Sent;

  if (bleKeyboard.isConnected()) {
    bleKeyboard.releaseAll();
    delay(MILLISECONDS_DELAY_BETWEEN_KEYSTROKES);
  }

  WOL_Request = false;

  for (int i = 0; i <= len - 1; i++) {

    An_Extended_Key_Was_Sent = false;

    // Identify and send an extended key if needed; an extended key starts with "{" and ends with "}"; an example is "{RETRUN}"

    // more detail:

    // the first part of the code below runs only when the Web Interface is closed
    // this is because if an extended key, such as "{RETURN}" is found then we want to send the extended key if the Web Interface is closed, but the characters "{RETURN}" if it is open

    // Also, just because there is a "{" and a "}" it doesn't mean that what is in between is a valid extended key, so that needs to be tested to ensure we know if we are dealing with an extended key or not

    // Finally, while "(WOL}" (Wake on LAN) is a special extended key which is processed seperately below

    if (!The_Web_Interface_Is_Open) {

      if ( String(text.charAt(i)) == "{" ) {

        String Extended_Key = text;
        Extended_Key.remove(0, i);

        int x = Extended_Key.indexOf("}");

        // not all text starting with a "{" will be and extended key, for example "this is me, looking at you {o o} "
        if (x >= 0)  {

          // seperate out the current extended key

          if (x != Extended_Key.length())
            Extended_Key.remove(x + 1);

          // here is where the extend key is sent to be Bluetooth keyed
          if (TypeExtendedKey(Extended_Key)) {

            // note that an extended key was sent
            An_Extended_Key_Was_Sent = true;

            // advance to the next character beyond the end of the extended key so that processing can continue from there
            i += Extended_Key.length() - 1;

          };

        };

      };

    };

    if (!An_Extended_Key_Was_Sent) {
      if (bleKeyboard.isConnected()) {
        bleKeyboard.write(text.charAt(i));
        delay(MILLISECONDS_DELAY_BETWEEN_KEYSTROKES);
      }
    };

    if (WOL_Request) {

      // build the MAC Address of the device to be woken; should be in the format AA:BB:CC:DD:EE:FF

      const String Filter = ": 0123456789ABCDEFabcdef";
      String MAC_Address = "";

      for (int j = i; j <= len - 1; j++) {
        if ( Filter.indexOf( String(text.charAt(j)) ) > -1)
          MAC_Address.concat(String(text.charAt(j)) );
      }

      if (MAC_Address.length() == 17)
        SendMagicPacket(MAC_Address);

      i = len; // forces end of processing for this command

    };

  };

  if (bleKeyboard.isConnected()) {
    bleKeyboard.releaseAll();
    delay(MILLISECONDS_DELAY_BETWEEN_KEYSTROKES);
  }

}

//******************************************************************************************************************************************************************

void HandleRemoteButtonPressed() {

  // Handles a button was pressed on the remote
  // (sends keys to host via bluetooth)

  bool Button_Was_Pressed = false;

  if (digitalRead(BUTTON_1_PIN) == BUTTON_DOWN) {

    Button_Was_Pressed = true;
    Serial.print("Button 1 pressed, sending: ");
    Serial.println(Button_1_text);
    TypeText(Button_1_text);

  };

  if (digitalRead(BUTTON_2_PIN) == BUTTON_DOWN) {
    Button_Was_Pressed = true;
    Serial.print("Button 2 pressed, sending: ");
    Serial.println(Button_2_text);
    TypeText(Button_2_text);
  };

  if (digitalRead(BUTTON_3_PIN) == BUTTON_DOWN) {
    Button_Was_Pressed = true;
    Serial.print("Button 3 pressed, sending: ");
    Serial.println(Button_3_text);
    TypeText(Button_3_text);
  };

  if (digitalRead(BUTTON_4_PIN) == BUTTON_DOWN) {

    long Button_Press_Down_Time = millis();
    do
      delay(10);
    while ( (digitalRead(BUTTON_4_PIN) == BUTTON_DOWN) && (( millis() - Button_Press_Down_Time) <= HOLD_REMOTE_BUTTON_4_FOR_THIS_MANY_MILLISECONDS_TO_OPEN_WEB_BROWSWER) );

    if ( ( millis() - Button_Press_Down_Time ) >= HOLD_REMOTE_BUTTON_4_FOR_THIS_MANY_MILLISECONDS_TO_OPEN_WEB_BROWSWER ) {

      // light LED so user knows to let go the button
      UpdateLED(LED_UPDATE_BUTTONS_PIN, true);

      if (The_Web_Interface_Is_Open)
        // here the user has held the button down for HOLD_REMOTE_BUTTON_4_FOR_THIS_MANY_MILLISECONDS_TO_OPEN_WEB_BROWSWER seconds when the Web Iterface is running; likley as they closed the browser without clicking 'Update' or 'Cancel'
        // this ends the current web update session and causes the ESP32 to restart (not a bug, this is by design)
        The_Web_Interface_Is_Open = false;
      else
        StartWebpageInterface();

    }
    else
    {
      Button_Was_Pressed = true;
      Serial.print("Button 4 pressed, sending: ");
      Serial.println(Button_4_text);
      TypeText(Button_4_text);
    }

  };

  if (Button_Was_Pressed)
    // wait until the button is released
    do
      delay(10);
    while ( (digitalRead(BUTTON_1_PIN) == BUTTON_DOWN) || (digitalRead(BUTTON_2_PIN) == BUTTON_DOWN) || (digitalRead(BUTTON_3_PIN) == BUTTON_DOWN) || (digitalRead(BUTTON_4_PIN) == BUTTON_DOWN) );


}

//******************************************************************************************************************************************************************

void StartWebpageInterface() {

  if (SetupWiFi()) {

    // fast flash the LED six times to indicate a successful WiFi connection
    FlashLED(LED_UPDATE_BUTTONS_PIN, 6, 250);

    // open the server address in a browser

    String Open_server_command = "";

    if (WORKING_WITH_WINDOWS10)
      Open_server_command.concat("{WINDOWS}{RELEASE_ALL}{!1}");

    Open_server_command.concat("http://");
    Open_server_command.concat(IP_Address_of_this_client);
    Open_server_command.concat("{!1}{Enter}");

    TypeText(Open_server_command);

    // turn on the LED to indicate the program is awaiting a web page update
    UpdateLED(LED_UPDATE_BUTTONS_PIN, true);

    LoadWebpage();

    // allows the user to press a button on the remote to have its current value keyed into any field on the web interface
    The_Web_Interface_Is_Open = true;

    while (The_Web_Interface_Is_Open) {

      // following the appropriate processing when the user clicks either 'Update' or 'Cancel' on the Webpage
      // the value for The_Web_Interface_Is_Open will be set to false causing this loop to exit, the wifi will be disconnected, and the ESP32 restarted

      HandleRemoteButtonPressed();         // allows the user to click a remote button while the web interface is open so the keys can be automatically typed into any field on the web interface

      if (UpdateButtonsButtonPressed())     // if the user presses the Update Buttons button again, then exit early
        The_Web_Interface_Is_Open = false;

      server.handleClient();

      delay(2);   // allow the cpu to switch to other tasks

    };

    WiFi.disconnect(true);

    // fast flash led six times to indicate we are done here
    FlashLED(LED_UPDATE_BUTTONS_PIN, 6, 250);

    // Restart required or Webpage Interface will not present correctly next time
    delay(1000);
    ESP.restart();

  }
  else {

    FlashLED(LED_UPDATE_BUTTONS_PIN, 4, 1000);   // slow flash led three times indicate failure

  };

}

//******************************************************************************************************************************************************************

void HandleUpdateButtonsButtonPressed() {

  if ( UpdateButtonsButtonPressed() )
    StartWebpageInterface();
};

//******************************************************************************************************************************************************************

void HandlePairRemoteButtonPressed() {

  // Handles the 'Update remote' button was pressed on the control board

  // starts with the user pressing the 'Update remote' button

  if (digitalRead(BUTTON_PAIR_REMOTE_PIN) == BUTTON_DOWN) {

    UpdateLED(LED_PAIR_REMOTE_PIN, true);                           // turn on LED to confirm button is down

    // wait until either the user releases the pair button or 5 seconds has passed
    long ButtonDownTime = millis();
    do
      delay(10);
    while  ( (digitalRead(BUTTON_PAIR_REMOTE_PIN) == BUTTON_DOWN) && ( (millis() - ButtonDownTime) <= 5000 ) );

    UpdateLED(LED_PAIR_REMOTE_PIN, false);                           // turn on LED off confirm the button may be released

    if ( (millis() - ButtonDownTime) >= HOLD_PAIRING_BUTTON_FOR_THIS_MANY_MILLISECONDS_BEFORE_PAIRING )  {

      // wait for the user to release the button
      do
        delay(10);
      while (digitalRead(BUTTON_PAIR_REMOTE_PIN) == BUTTON_DOWN);

      randomSeed(analogRead(0));                                      // for securty purposes, reset button (preventing someone from reseting the button and then having access to the button values
      delay(random(1001));                                            // do this reset sometime within the next second, is also done for security - to avoid hardware hacking
      SetupEEPROM(true);

      SendResetSequenceTo1527Board();

      delay(500);                                                   // take a short break and then
      FlashLED(LED_PAIR_REMOTE_PIN, 6, 250);                         // fast the led six times to indicate success

    } else {

      Serial.print("Pair button was not down for ");
      Serial.println(HOLD_PAIRING_BUTTON_FOR_THIS_MANY_MILLISECONDS_BEFORE_PAIRING);
      Serial.println(" seconds");

    }

  };

}

//******************************************************************************************************************************************************************

void SetupSerial() {

  Serial.begin(115200);
  Serial.println("Blue Key Remote v1 starting");
  Serial.println("Copyright, Rob Latour, 2021");
  Serial.println("");

}

//******************************************************************************************************************************************************************

void SetupESP32Pins() {

  pinMode(BUTTON_1_PIN, INPUT_PULLDOWN);
  pinMode(BUTTON_2_PIN, INPUT_PULLDOWN);
  pinMode(BUTTON_3_PIN, INPUT_PULLDOWN);
  pinMode(BUTTON_4_PIN, INPUT_PULLDOWN);

  pinMode(BUTTON_UPDATE_BUTTONS_PIN, INPUT_PULLDOWN);
  pinMode(LED_UPDATE_BUTTONS_PIN, OUTPUT);
  digitalWrite(LED_UPDATE_BUTTONS_PIN, LOW);

  pinMode(BUTTON_PAIR_REMOTE_PIN, INPUT_PULLDOWN);
  pinMode(LED_PAIR_REMOTE_PIN, OUTPUT);
  digitalWrite(LED_PAIR_REMOTE_PIN, LOW);

  pinMode(THE_1527_PROGRAMMING_PIN, OUTPUT);
  digitalWrite(THE_1527_PROGRAMMING_PIN, LOW);
}

//******************************************************************************************************************************************************************

void setup() {

  SetupSerial();

  SetupEEPROM(false);

  SetupWOL();

  SetupESP32Pins();

  Number_Of_Extended_Key_Codes = sizeof(Extend_Key_Codes) / sizeof(Extend_Key_Codes[0]);

  // Serial.print("Number of extended key codes: "); Serial.println(Number_Of_Extended_Key_Codes);

  // start bluetooth
  bleKeyboard.begin();

}

//******************************************************************************************************************************************************************

void loop() {

  HandlePairRemoteButtonPressed();

  HandleUpdateButtonsButtonPressed();

  HandleRemoteButtonPressed();

}
