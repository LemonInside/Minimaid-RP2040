
#include <string.h>
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

/* * ======================================================================
 * GPIO PIN ASSIGNMENTS
 * ======================================================================
 * Note: Shared signals are mapped to GPIO 25 (Internal LED) to save pins.
 */

// Use 573 to "null" out a button you haven't wired yet
#define UNUSED 573

// 21 INPUT BUTTONS (Connect Pin to GND)
// the null are nonstandard
#define PIN_IN_P1_START    0
#define PIN_IN_P1_UP       1
#define PIN_IN_P1_DOWN     2
#define PIN_IN_P1_LEFT     3
#define PIN_IN_P1_RIGHT    4
#define PIN_IN_P1_SEL_L    5
#define PIN_IN_P1_SEL_R    6
#define PIN_IN_P1_MENU_UP  UNUSED 
#define PIN_IN_P1_MENU_DN  UNUSED 

#define PIN_IN_P2_START    9
#define PIN_IN_P2_UP       10
#define PIN_IN_P2_DOWN     11
#define PIN_IN_P2_LEFT     12
#define PIN_IN_P2_RIGHT    13
#define PIN_IN_P2_SEL_L    14
#define PIN_IN_P2_SEL_R    15
#define PIN_IN_P2_MENU_UP  UNUSED
#define PIN_IN_P2_MENU_DN  UNUSED

#define PIN_IN_COIN        16
#define PIN_IN_TEST        17
#define PIN_IN_SERVICE     18

// 15 OUTPUT LIGHTS (LEDs/Optocouplers)
#define L_P1_START       25
#define L_P1_UP          UNUSED
#define L_P1_DOWN        UNUSED
#define L_P1_LEFT        UNUSED
#define L_P1_RIGHT       UNUSED

#define L_P2_START       UNUSED
#define L_P2_UP          UNUSED 
#define L_P2_DOWN        UNUSED 
#define L_P2_LEFT        UNUSED 
#define L_P2_RIGHT       UNUSED 

#define L_MARQUEE_TL     UNUSED
#define L_MARQUEE_TR     UNUSED
#define L_MARQUEE_BL     UNUSED
#define L_MARQUEE_BR     UNUSED 
#define L_NEON           25 

// ======================================================================
// Minimaid switches
// ======================================================================
bool keyboard_enabled = true;


// Helper function to safely read pins that might be "null"
bool is_pressed(uint8_t pin) {
    if (pin == UNUSED) return false;
    return !gpio_get(pin); // Low = Pressed (Pull-up)
}

// ======================================================================
// INTERFACE 0 (EP 0x81): FAST KEYBOARD BITMASK
// ======================================================================

void hid_task_interface_0() {
    //if (!tud_hid_n_ready(0)) return;
    if (!keyboard_enabled || !tud_hid_n_ready(0)) return;

    // Report Format: [0] Modifiers, [1] Reserved, [2-5] 32-bit Mask
  
    // Based on your mapping: [2]=Service/Test/Coin, [3]=Start/Directions, [4]=Right/Menu, [5]=Nonstandard
    uint8_t report[6] = {0};

    if (is_pressed(PIN_IN_SERVICE))   report[2] |= 0x01;
    if (is_pressed(PIN_IN_TEST))      report[2] |= 0x02;
    if (is_pressed(PIN_IN_COIN))      report[2] |= 0x08;

    if (is_pressed(PIN_IN_P1_START))  report[3] |= 0x01;
    if (is_pressed(PIN_IN_P2_START))  report[3] |= 0x02;
    if (is_pressed(PIN_IN_P1_UP))     report[3] |= 0x04;
    if (is_pressed(PIN_IN_P2_UP))     report[3] |= 0x08;
    if (is_pressed(PIN_IN_P1_DOWN))   report[3] |= 0x10;
    if (is_pressed(PIN_IN_P2_DOWN))   report[3] |= 0x20;
    if (is_pressed(PIN_IN_P1_LEFT))   report[3] |= 0x40;
    if (is_pressed(PIN_IN_P2_LEFT))   report[3] |= 0x80;

    if (is_pressed(PIN_IN_P1_RIGHT))  report[4] |= 0x01;
    if (is_pressed(PIN_IN_P2_RIGHT))  report[4] |= 0x02;
    if (is_pressed(PIN_IN_P1_SEL_L))  report[4] |= 0x10;
    if (is_pressed(PIN_IN_P2_SEL_L))  report[4] |= 0x20;
    if (is_pressed(PIN_IN_P1_SEL_R))  report[4] |= 0x40;
    if (is_pressed(PIN_IN_P2_SEL_R))  report[4] |= 0x80;

//    if (is_pressed(PIN_IN_P1_MENU_UP)) report[5] |= 0x01;
//    if (is_pressed(PIN_IN_P2_MENU_UP)) report[5] |= 0x02;
//    if (is_pressed(PIN_IN_P1_MENU_DN)) report[5] |= 0x04;
//    if (is_pressed(PIN_IN_P2_MENU_DN)) report[5] |= 0x08;

    tud_hid_n_report(0, 0, report, sizeof(report));
    
}

// ======================================================================
// INTERFACE 1 (EP 0x83): LEGACY IO & LIGHTS
// ======================================================================

// 1. OUTGOING: Reporting inputs to the Bemanitools-style interface
// Only send if the endpoint is ready to accept data

void hid_task_interface_1() {
    if (!tud_hid_n_ready(1)) return;

    // The CE and 3B are the "Anchors". Everything else is the bitmask.
    // Index:                0     1     2     3     4     5     6     7
    uint8_t mm_report[8] = {0xCE, 0x00, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x00};

    // Based on your mapping: CE [1]=Sys, [2]=Pads, [3]=Right/Menu, [4]=Nonstandard, 3B

    if (is_pressed(PIN_IN_SERVICE))   mm_report[1] |= 0x01;
    if (is_pressed(PIN_IN_TEST))      mm_report[1] |= 0x02;
    if (is_pressed(PIN_IN_COIN))      mm_report[1] |= 0x08;

    if (is_pressed(PIN_IN_P1_START))  mm_report[2] |= 0x01;
    if (is_pressed(PIN_IN_P2_START))  mm_report[2] |= 0x02;
    if (is_pressed(PIN_IN_P1_UP))     mm_report[2] |= 0x04;
    if (is_pressed(PIN_IN_P2_UP))     mm_report[2] |= 0x08;
    if (is_pressed(PIN_IN_P1_DOWN))   mm_report[2] |= 0x10;
    if (is_pressed(PIN_IN_P2_DOWN))   mm_report[2] |= 0x20;
    if (is_pressed(PIN_IN_P1_LEFT))   mm_report[2] |= 0x40;
    if (is_pressed(PIN_IN_P2_LEFT))   mm_report[2] |= 0x80;

    if (is_pressed(PIN_IN_P1_RIGHT))  mm_report[3] |= 0x01;
    if (is_pressed(PIN_IN_P2_RIGHT))  mm_report[3] |= 0x02;
    if (is_pressed(PIN_IN_P1_SEL_L))  mm_report[3] |= 0x10;
    if (is_pressed(PIN_IN_P2_SEL_L))  mm_report[3] |= 0x20;
    if (is_pressed(PIN_IN_P1_SEL_R))  mm_report[3] |= 0x40;
    if (is_pressed(PIN_IN_P2_SEL_R))  mm_report[3] |= 0x80;

//    if (is_pressed(PIN_IN_P1_MENU_UP)) mm_report[4] |= 0x01;
//    if (is_pressed(PIN_IN_P2_MENU_UP)) mm_report[4] |= 0x02;
//    if (is_pressed(PIN_IN_P1_MENU_DN)) mm_report[4] |= 0x04;
//    if (is_pressed(PIN_IN_P2_MENU_DN)) mm_report[4] |= 0x08;

    // Send the report to Interface 1 (Endpoint 0x83)
    tud_hid_n_report(1, 0, mm_report, sizeof(mm_report));
}

// 2. INCOMING: Set Lights from Host
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    if (itf == 1 && report_type == HID_REPORT_TYPE_OUTPUT) {

        // KEYBOARD TOGGLE
        keyboard_enabled = (buffer[6] ? true : false);
        // Use onboard LED as indicator for keyboard enable or not. 
        //gpio_put(25, (keyboard_enabled)); 
        
        // buffer[1]: Cabinet & Marquees
        gpio_put(L_MARQUEE_TL, (buffer[1] & 0x80)); 
        gpio_put(L_MARQUEE_BL, (buffer[1] & 0x40)); 
        gpio_put(L_MARQUEE_TR, (buffer[1] & 0x20)); 
        gpio_put(L_MARQUEE_BR, (buffer[1] & 0x10)); 
        gpio_put(L_P1_START,   (buffer[1] & 0x04)); 
        gpio_put(L_P2_START,   (buffer[1] & 0x08)); 

        // buffer[2]: Player 1 Pad
        gpio_put(L_P1_UP,      (buffer[2] & 0x01)); 
        gpio_put(L_P1_DOWN,    (buffer[2] & 0x02)); 
        gpio_put(L_P1_LEFT,    (buffer[2] & 0x04)); 
        gpio_put(L_P1_RIGHT,   (buffer[2] & 0x08)); 

        // buffer[3]: Player 2 Pad
        gpio_put(L_P2_UP,      (buffer[3] & 0x01));
        gpio_put(L_P2_DOWN,    (buffer[3] & 0x02));
        gpio_put(L_P2_LEFT,    (buffer[3] & 0x04));
        gpio_put(L_P2_RIGHT,   (buffer[3] & 0x08));

        // buffer[4]: Neon
        gpio_put(L_NEON,       (buffer[4] & 0x01));
    }
}

// ======================================================================
// BOILERPLATE & INITIALIZATION
// ======================================================================

void init_all_gpio() {
    // 15 Output Pins
    const uint outs[] = {L_P1_START, L_P1_UP, L_P1_DOWN, L_P1_LEFT, L_P1_RIGHT,
                         L_P2_START, L_P2_UP, L_P2_DOWN, L_P2_LEFT, L_P2_RIGHT,
                         L_MARQUEE_TL, L_MARQUEE_TR, L_MARQUEE_BL, L_MARQUEE_BR, L_NEON};
    for(int i=0; i < 15; i++) {
        if (outs[i] != UNUSED) {
            gpio_init(outs[i]);
            gpio_set_dir(outs[i], GPIO_OUT);
            gpio_put(outs[i], 0);
        }
    }

    // 21 Input Pins
    const uint ins[] = {PIN_IN_P1_START, PIN_IN_P1_UP, PIN_IN_P1_DOWN, PIN_IN_P1_LEFT, PIN_IN_P1_RIGHT, 
                        PIN_IN_P1_SEL_L, PIN_IN_P1_SEL_R, PIN_IN_P1_MENU_UP, PIN_IN_P1_MENU_DN, PIN_IN_P2_START, 
                        PIN_IN_P2_UP, PIN_IN_P2_DOWN, PIN_IN_P2_LEFT, PIN_IN_P2_RIGHT, PIN_IN_P2_SEL_L, PIN_IN_P2_SEL_R,  
                        PIN_IN_P2_MENU_UP, PIN_IN_P2_MENU_DN, PIN_IN_COIN, PIN_IN_TEST, PIN_IN_SERVICE  };
    for(int i=0; i < 21; i++) {
        if (ins[i] != UNUSED) {
            gpio_init(ins[i]);
            gpio_set_dir(ins[i], GPIO_IN);
            gpio_pull_up(ins[i]);
        }
    }
}

int main() {
    board_init();
    init_all_gpio();
    tusb_init();

    while (1) {
        tud_task();
        hid_task_interface_0(); // FAST KB Inputs
        hid_task_interface_1(); // LEGACY MM Inputs
    }
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    return 0;
}
