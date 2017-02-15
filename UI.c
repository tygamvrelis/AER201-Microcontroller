#include <xc.h>;

#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "I2C.h"
#include "macros.h"
#include "UI.h"
#include "RTC.h"
#include "main.h"
#include "sort.h"

/* This file contains all the UI functionality outside the main loop
 * To add a new state:
 1. Define the state in the STATE TABLE (give it a numerical code and description)
 2. Create a new state change handler
 3. Add a case for the new state in the switch statement in updateMenu
 */

// <editor-fold defaultstate="collapsed" desc="VARIABLE DEFINITIONS">
char input;
const char keys[] = "123A456B789C*0#D"; // possible inputs from keypad
int up; int down; int enter; int back; // correspond to certain inputs to make logic more transparent
int cur_state;
int UIenabled = 0; // 1 if enabled, 0 if disabled. This changes whether interrupts will call updateMenu()
int firstboot; // flag to tell input handler to handle events for the first boot
int logstate = 0; // flag to tell input handler if the UI is in a log (for pause/back purposes)
int log; // index for log of interest
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="STATE TABLE">
/* Define a menu to be a collection of text displayed (excludes cursor)
 * The state codes below use the following convention:
 *      For 2-digit codes: (main navigation menu state)
 *          1st digit = 1 => 1st and 2nd options displayed
 *          1st digit = 2 => 2nd and 3rd options displayed
 *      For 4-digit codes: (logs menu)
 *          3rd digit = 1 => 1st and 2nd options displayed
 *          3rd digit = 2 => 2nd and 3rd options displayed
 *          ...
 *          So on.
 *      For all menu state codes, the last digit specifies the menu option that
 *      the cursor is on. Each cursor location requires a unique state. The
 *      alternative is to write a routine that erases and rewrites the cursor.
 * 
 * 0 - start menu
 * 11 - 1st menu: first and second menu options with cursor on 1st option
 * 12 - 1st menu: first and second menu options with cursor on 2nd option
 * 22 - 2nd menu: second and third menu options with cursor on 2nd option
 * 23 - 2nd menu: second and third menu options with cursor on 3rd option
 * 1011 - 1st logs menu, cursor on 1st option
 * 1012 - 1st logs menu, cursor on 2nd option
 * 1022 - 2nd logs menu, cursor on 2nd option
 * 1023 - 2nd logs menu, cursor on 3rd option
 * 1033 - 3rd logs menu, cursor on 3rd option
 * 1034 - 3rd logs menu, cursor on 4th option
 */

// </editor-fold>

// Initialization routine for UI
void initUI(void){ 
    ei(); // enable interrupts
    machine_state = UI_state;
    
    loadRTC(); // Load RTC with the date and time from main
            
    change_state_to_menu_start(); // Initialization  state
    firstboot = 1;
}

// Main routine for UI
void UI(void){
    if(machine_state == DoneSorting_state){
        __lcd_clear();
        __lcd_home();
        printf("Sort complete");
        __lcd_newline();
         printf("Time: %02d:%02d:%02d", 0, (total_time % 3600) / 60, (total_time % 3600) % 60);
    }
    else if(cur_state == 0){
        __delay_1s();
        if(cur_state == 0){
            printRTC();
        }
    }
    while(logstate){
        // <editor-fold defaultstate="collapsed" desc="PRINT STATEMENTS FOR LOGS">
        __lcd_clear();
        __lcd_home();
        printf("-- Log %2d here--", log);
        __lcd_newline();
        printf("Pause:< | Back:>");

        __delay_1s();if(!logstate){break;}
        __delay_1s();if(!logstate){break;}

        __lcd_clear();
        __lcd_home();
        printf("Start:");
        __lcd_newline();
        printf("06Feb | 01:08:56");

        __delay_1s();if(!logstate){break;}
        __delay_1s();if(!logstate){break;}

        __lcd_clear();
        __lcd_home();
        printf("Duration:");
        __lcd_newline();
        printf("%d min(s) %d secs", 2, 42);

        __delay_1s();if(!logstate){break;}
        __delay_1s();if(!logstate){break;}

        __lcd_clear();
        __lcd_home();
        printf(" -- Pop  can -- ");
        __lcd_newline();
        printf("No tab: 9|Tab: 1");

        __delay_1s();if(!logstate){break;}
        __delay_1s();if(!logstate){break;}

        __lcd_clear();
        __lcd_home();
        printf(" -- Soup can -- ");
        __lcd_newline();
        printf("No lbl: 1|lbl: 1");

        __delay_1s();if(!logstate){break;}
        __delay_1s();if(!logstate){break;}

        __lcd_clear();
        __lcd_home();
        printf("-- Total cans --");
        __lcd_newline();
        printf("12");

        __delay_1s();if(!logstate){break;}
        __delay_1s();if(!logstate){break;}
        // </editor-fold>
    }
}

// Menu update routine
void updateMenu(void){
    up = 0; down = 0; enter = 0; back = 0; // Reset these
    di(); // disable interrupts while writing to LCD 
    
    if (inputHandler()
            // directly calls menu print functions if a number is pressed on the keypad. Returns 0 in this case
            // Else, sets up navigation flags and returns 1 so this if block is entered
            ){
        switch(cur_state){
            case 0 :
                // Any key press will change state from state 0 to state 11
                change_state_to_menu_11();
                break;

            case 11 :
                if(up){
                    change_state_to_menu_23();
                }
                else if(down){
                    change_state_to_menu_12();
                }
                else if(enter){
                    change_state_to_menu_start();
                }
                break;

            case 12 :
                if(up){
                    change_state_to_menu_11();
                }
                else if(down){
                    change_state_to_menu_23();
                }
                else if(enter){
                    machine_state = Sorting_state;
                }
                break;

            case 22 :
                if(up){
                    change_state_to_menu_11();
                }
                else if(down){
                    change_state_to_menu_23();
                }
                else if(enter){
                     machine_state = Sorting_state;
                }
                break;
                
            case 23 :
                if(up){
                    change_state_to_menu_22();
                }
                else if(down){
                    change_state_to_menu_11();
                }
                else if(enter){
                    change_state_to_logs_11();
                }
                break;
                
            case 1011:
                if(up){
                    change_state_to_logs_34();
                }
                else if(down){
                    change_state_to_logs_12();
                }
                else if(enter){
                    dispLogs(1);
                }
                else if(back){
                    change_state_to_menu_23();
                }
                break;
                
            case 1012:
                if(up){
                    change_state_to_logs_11();
                }
                else if(down){
                    change_state_to_logs_23();
                }
                else if(enter){
                    dispLogs(2);
                }
                else if(back){
                    change_state_to_menu_23();
                }
                break;
                
            case 1022 :
                if(up){
                    change_state_to_logs_11();
                }
                else if(down){
                    change_state_to_logs_23();
                }
                else if(enter){
                    dispLogs(2);
                }
                else if(back){
                    change_state_to_menu_23();
                }
                break;
                
            case 1023 :
                if(up){
                    change_state_to_logs_22();
                }
                else if(down){
                    change_state_to_logs_34();
                }
                else if(enter){
                    dispLogs(3);
                }
                else if(back){
                    change_state_to_menu_23();
                }
                break;
                
            case 1033 :
                if(up){
                    change_state_to_logs_22();
                }
                else if(down){
                    change_state_to_logs_34();
                }
                else if(enter){
                    dispLogs(3);
                }
                else if(back){
                    change_state_to_menu_23();
                }
                break;
                
            case 1034 :
                if(up){
                    change_state_to_logs_33();
                }
                else if(down){
                    change_state_to_logs_11();
                }
                else if(enter){
                    dispLogs(4);
                }
                else if(back){
                    change_state_to_menu_23();
                }
                break;

            default:
                break;
        }
    }
    ei(); // enable keypad interrupts after write
}

int inputHandler(void){
    // Uses current inputs to set next state where possible
    // and the navigation request otherwise
    // return 0 tells updateMenu that things have already been taken care of
    // return 1 tells updateMenu that navigation (up/down/enter/back) remains to be taken care of
    if(firstboot){
        change_state_to_menu_11();
        firstboot = 0;
        return 0;
    }
    if (logstate){
        if(input == 'C' && PORTBbits.RB1 == 1){
            // Pause
            while(PORTBbits.RB1 == 1){
                __delay_ms(10);
            }
            return 0;
        }
        else if (input == 'D'){
            // Back
            logstate = 0;
            switch(cur_state){
                case 1011 :
                    change_state_to_logs_11();
                    break;
                case 1012 :
                    change_state_to_logs_12();
                    break;
                case 1022 :
                    change_state_to_logs_22();
                    break;
                case 1023 :
                    change_state_to_logs_23();
                    break;
                case 1033:
                    change_state_to_logs_33();
                    break;
                case 1034 :
                    change_state_to_logs_34();
                    break;
            }
            return 0;
        }
        else{
            return 0;
        }
    }
    switch(input){
        // Tackles cases where next state can be set right away
        case '1' :
            if(cur_state == 11){
                return 0;
            }
            else{ 
            change_state_to_menu_11();
            }
            return 0;
        case '2' :
            if(cur_state == 22 | cur_state == 12){
                return 0;
            }
            else if(cur_state == 23){
                change_state_to_menu_22();
            }
            else{
                change_state_to_menu_12();
            }
            return 0;
        case '3' :
            if(cur_state == 23){
                return 0;
            }
            else{
                change_state_to_menu_23();
            }
            return 0;
        //Tackle navigation cases
        case 'A' :
            up = 1;
            return 1;
        case 'B' :
            down = 1;
            return 1;
        case 'C' :
            enter = 1;
            return 1;
        case 'D' :
            back = 1;
            return 1;
        default :
            // On any other key press, do nothing
            return 1;
    }
}

void dispLogs(int myLog){
    logstate = 1;
    log = myLog;
}

/* STATE CHANGE HANDLERS */
void change_state_to_menu_start(void){
    cur_state = 0;
    // Start menu
    __lcd_home();
    printRTC();
    __lcd_newline();
    printf("PUSH TO CONTINUE");
}
void change_state_to_menu_11(void){
    cur_state = 11;
    // 1st menu: first and second menu options with cursor on 1st option
    __lcd_home();
    printf("1. DATE/TIME   <");
    __lcd_newline();
    printf("2. SORT         ");
}
void change_state_to_menu_12(void){
    cur_state = 12;
    // 1st menu: first and second menu options with cursor on 2nd option
    __lcd_home();
    printf("1. DATE/TIME    ");
    __lcd_newline();
    printf("2. SORT        <");
}
void change_state_to_menu_22(void){
    cur_state = 22;
    // 2nd menu: second and third menu options with cursor on 2nd option
    __lcd_home();
    printf("2. SORT        <");
    __lcd_newline();
    printf("3. LOGS         ");
}
void change_state_to_menu_23(void){
    cur_state = 23;
    // 2nd menu: second and third menu options with cursor on 3rd option
    __lcd_home();
    printf("2. SORT         ");
    __lcd_newline();
    printf("3. LOGS        <");
}
void change_state_to_logs_11(void){
    cur_state = 1011;
    __lcd_home();
    printf("LOG 1          <");
    __lcd_newline();
    printf("LOG 2           ");
}
void change_state_to_logs_12(void){
    cur_state = 1012;
    __lcd_home();
    printf("LOG 1           ");
    __lcd_newline();
    printf("LOG 2          <");
}
void change_state_to_logs_22(void){
    cur_state = 1022;
    __lcd_home();
    printf("LOG 2          <");
    __lcd_newline();
    printf("LOG 3           ");
}
void change_state_to_logs_23(void){
    cur_state = 1023;
    __lcd_home();
    printf("LOG 2           ");
    __lcd_newline();
    printf("LOG 3          <");
}
void change_state_to_logs_33(void){
    cur_state = 1033;
    __lcd_home();
    printf("LOG 3          <");
    __lcd_newline();
    printf("LOG 4           ");
}
void change_state_to_logs_34(void){
    cur_state = 1034;
    __lcd_home();
    printf("LOG 3           ");
    __lcd_newline();
    printf("LOG 4          <");
}

// Interrupt handler
void interrupt handler(void) {
    //Interrupt handler for key presses: keypressed updates the menu state
    if(INT1IF){
        INT1IF = 0;     //Clear flag bit
        if(machine_state == UI_state) { // "If we're supposed to be in the UI..."
            input = keys[(PORTB & 0xF0) >> 4];
            updateMenu();
        }
    }
    
    //** 1 SECOND TIMER THAT CALLS printSortTimer() **
    if(TMR0IF){
        TMR0IF = 0;
        if(machine_state == Sorting_state){
            if(sortTimerCounter == 31250){//31250 min frequency to print once per second
            sortTimerCounter = 0;
            printSortTimer();
            }
            else{
                sortTimerCounter++;  
            }
            T0CON = 0b11011000; // Initialize timer again!
        }
    }
    
    //** Timer for servo delays **
    /*
    if(TMR1IF){
        TMR1IF = 1;
        if(machine_state = Sorting_state){
            if(PWMTimerCounter == 31){ // 31.25 to be exact...
                PWMTimerCounter = 0;
                switch(servoSwitch){
                    // if it was on last time then turn it off this time
                    
                }
                if(servoSelectFlag == 1){
                    LATCbits.LATC1 = 1;
                }
                if(servoSelectFlag == 2){
                    LATCbits.LATC2 = 1;
                }
            } // supposing that Timer1 works like Timer1
        }
    }*/
}