
#define BUTTON_1_LABEL                                                 "Button A"                               // Web Interface button labels
#define BUTTON_2_LABEL                                                 "Button B"
#define BUTTON_3_LABEL                                                 "Button C"
#define BUTTON_4_LABEL                                                 "Button D"

#define MACRO_1                                                        "Button 1 macro"                         // Key values to be used in place of: {MACRO_1}, {MACRO_2}, {MACRO_3}, and {MACRO_4}.
#define MACRO_2                                                        "Button 2 macro"                         // Using these values requires a compile and upload of this sketch to change the values.  This as opposed to more simply using key values entered via the Web interface.
#define MACRO_3                                                        "Button 3 macro"                         // However, using macro assignments in the sketch helps circumvent the EEPROM memory storage limit (of effectively 509 bytes) associated with using individual keys in the Web interface.
#define MACRO_4                                                        "Button 4 macro"                         // For example, entering "Now is the time to come to the aid of the party" in the Web interface uses 47 bytes of EEPROM memory, while
                                                                                                                // setting MACRO_1 to "Hello World" and using {MACRO_1} in the Web interface uses only 2 bytes of EEPROM memory.
                                                                                                                
#define WORKING_WITH_WINDOWS10                                         true                                     // set to true if you are going to be pairning with a Windows PC

                                                                                                                // note, for these commands to work the letters (l, x, s, i, r, and u) must remain in lower case
#define W10_LOCK_MACRO                                                 "{WINDOWS}l{RELEASE_ALL}"                // used in place of {LOCK}
#define W10_SLEEP_MACRO                                                "{WINDOWS}x{RELEASE_ALL}{!1}us"          // used in place of {SLEEP}
#define W10_SIGNOUT_MACRO                                              "{WINDOWS}x{RELEASE_ALL}{!1}ui"          // used in place of {SIGNOUT}
#define W10_RESTART_MACRO                                              "{WINDOWS}x{RELEASE_ALL}{!1}ur"          // used in place of (RESTART}
#define W10_SHUTDOWN_MACRO                                             "{WINDOWS}x{RELEASE_ALL}{!1}uu"          // used in place of (SHUTDOWN}

#define BLUETOOTH_DEVICE_NAME                                          "Blue Key Remote"                        // change this if you like

#define MILLISECONDS_DELAY_BETWEEN_KEYSTROKES                          20                                       // default value of 10 milliseconds   = 1/100 of a second

#define HOLD_PAIRING_BUTTON_FOR_THIS_MANY_MILLISECONDS_BEFORE_PAIRING  5000                                     // default value of 5000 milliseconds = 5 seconds

#define HOLD_REMOTE_BUTTON_4_FOR_THIS_MANY_MILLISECONDS_TO_OPEN_WEB_BROWSWER  5000                              // default value of 5000 milliseconds = 5 seconds
