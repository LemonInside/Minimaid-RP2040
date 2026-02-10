#include <string.h>
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

/* * ======================================================================
 * GPIO PIN ASSIGNMENTS
 * ======================================================================
 * Note: Shared signals are mapped to GPIO 25 (Internal LED) to save pins.
 */

// 17 INPUT BUTTONS (Connect Pin to GND)
#define BTN_P1_START     0
#define BTN_P1_SEL_L     1
#define BTN_P1_SEL_R     2
#define BTN_P1_UP        3
#define BTN_P1_DOWN      4
#define BTN_P1_LEFT      5
#define BTN_P1_RIGHT     6

#define BTN_P2_START     7
#define BTN_P2_SEL_L     8
#define BTN_P2_SEL_R     9
#define BTN_P2_UP        10
#define BTN_P2_DOWN      11
#define BTN_P2_LEFT      12
#define BTN_P2_RIGHT     13

#define BTN_COIN         14
#define BTN_TEST         15
#define BTN_SERVICE      16

// 15 OUTPUT LIGHTS (LEDs/Optocouplers)
#define L_P1_START       17
#define L_P1_UP          18
#define L_P1_DOWN        19
#define L_P1_LEFT        20
#define L_P1_RIGHT       21

#define L_P2_START       22
#define L_P2_UP          25 // Shared/Stub
#define L_P2_DOWN        25 // Shared/Stub
#define L_P2_LEFT        25 // Shared/Stub
#define L_P2_RIGHT       25 // Shared/Stub

#define L_MARQUEE_TL     26
#define L_MARQUEE_TR     27
#define L_MARQUEE_BL     28
#define L_MARQUEE_BR     25 // Shared/Stub
#define L_NEON           25 // Shared/Stub

// ======================================================================
// Minimaid switches
// ======================================================================
bool keyboard_enabled = true;

// ======================================================================
// INTERFACE 0 (EP 0x81): FAST KEYBOARD BITMASK
// ======================================================================

void hid_task_interface_0() {
    //if (!tud_hid_n_ready(0)) return;
    if (!keyboard_enabled || !tud_hid_n_ready(0)) return;

    // Report Format: [0] Modifiers, [1] Reserved, [2-5] 32-bit Mask
    uint8_t report[6] = {0};
    uint32_t mask = 0;

    // Based on your Header "Endpoint 1" Logic
    // Byte 2 (Mask bits 0-7)
    if (!gpio_get(BTN_SERVICE))   mask |= (1 << 0);
    if (!gpio_get(BTN_TEST))      mask |= (1 << 1);
    if (!gpio_get(BTN_COIN))      mask |= (1 << 3);

    // Byte 3 (Mask bits 8-15)
    if (!gpio_get(BTN_P1_START))  mask |= (1 << 8);
    if (!gpio_get(BTN_P2_START))  mask |= (1 << 9);
    if (!gpio_get(BTN_P1_UP))     mask |= (1 << 10);
    if (!gpio_get(BTN_P2_UP))     mask |= (1 << 11);
    if (!gpio_get(BTN_P1_DOWN))   mask |= (1 << 12);
    if (!gpio_get(BTN_P2_DOWN))   mask |= (1 << 13);
    if (!gpio_get(BTN_P1_LEFT))   mask |= (1 << 14);
    if (!gpio_get(BTN_P2_LEFT))   mask |= (1 << 15);

    // Byte 4 (Mask bits 16-23)
    if (!gpio_get(BTN_P1_RIGHT))  mask |= (1 << 16);
    if (!gpio_get(BTN_P2_RIGHT))  mask |= (1 << 17);
    if (!gpio_get(BTN_P1_SEL_L))  mask |= (1 << 20); // Mapping Menu L
    if (!gpio_get(BTN_P2_SEL_L))  mask |= (1 << 21);
    if (!gpio_get(BTN_P1_SEL_R))  mask |= (1 << 22); // Mapping Menu R
    if (!gpio_get(BTN_P2_SEL_R))  mask |= (1 << 23);

    memcpy(&report[2], &mask, 4);
    tud_hid_n_report(0, 0, report, sizeof(report));
}

// ======================================================================
// INTERFACE 1 (EP 0x83): LEGACY IO & LIGHTS
// ======================================================================

// 1. OUTGOING: Reporting inputs to the Bemanitools-style interface
// Only send if the endpoint is ready to accept data

void hid_task_interface_1() {
    if (!tud_hid_n_ready(1)) return;

    // Standard Minimaid Input Report (8 bytes)
    // Index:     0     1     2     3     4     5     6     7
    // Purpose: Magic [Cmd] [Pads] [Pad2] Magic [Extra][...] [...]
    uint8_t mm_report[8] = {0xCE, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x00, 0x00};

    // --- BYTE 1: SYSTEM & STARTS ---
    if (!gpio_get(PIN_IN_P1_START))  mm_report[1] |= 0x01;
    if (!gpio_get(PIN_IN_P2_START))  mm_report[1] |= 0x02;
    if (!gpio_get(PIN_IN_SERVICE))   mm_report[1] |= 0x10; 
    if (!gpio_get(PIN_IN_TEST))      mm_report[1] |= 0x20; 

    // --- BYTE 2: PLAYER 1 PAD & COIN ---
    if (!gpio_get(PIN_IN_P1_UP))     mm_report[2] |= 0x01;
    if (!gpio_get(PIN_IN_P1_DOWN))   mm_report[2] |= 0x02;
    if (!gpio_get(PIN_IN_P1_LEFT))   mm_report[2] |= 0x10;
    if (!gpio_get(PIN_IN_P1_RIGHT))  mm_report[2] |= 0x40;
    if (!gpio_get(PIN_IN_COIN))      mm_report[2] |= 0x08;

    // --- BYTE 3: PLAYER 2 PAD ---
    if (!gpio_get(PIN_IN_P2_UP))     mm_report[3] |= 0x01;
    if (!gpio_get(PIN_IN_P2_DOWN))   mm_report[3] |= 0x02;
    if (!gpio_get(PIN_IN_P2_LEFT))   mm_report[3] |= 0x10;
    if (!gpio_get(PIN_IN_P2_RIGHT))  mm_report[3] |= 0x40;

    // --- BYTE 5: SELECTION / MENU BUTTONS (Solo Buttons) ---
    // These are often mapped to the 'Extra' byte in Bemanitools
    if (!gpio_get(PIN_IN_P1_SEL_L))  mm_report[5] |= 0x01;
    if (!gpio_get(PIN_IN_P1_SEL_R))  mm_report[5] |= 0x02;
    if (!gpio_get(PIN_IN_P2_SEL_L))  mm_report[5] |= 0x04;
    if (!gpio_get(PIN_IN_P2_SEL_R))  mm_report[5] |= 0x08;

    // Send the report to Interface 1 (Endpoint 0x83)
    tud_hid_n_report(1, 0, mm_report, sizeof(mm_report));
}

// 2. INCOMING: Set Lights from Host
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    if (itf == 1 && report_type == HID_REPORT_TYPE_OUTPUT) {

        // KEYBOARD TOGGLE
        keyboard_enabled = (buffer[6] ? true : false);
        
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
        gpio_init(outs[i]);
        gpio_set_dir(outs[i], GPIO_OUT);
        gpio_put(outs[i], 0);
    }

    // 17 Input Pins
    const uint ins[] = {BTN_P1_START, BTN_P1_SEL_L, BTN_P1_SEL_R, BTN_P1_UP, BTN_P1_DOWN, BTN_P1_LEFT, BTN_P1_RIGHT,
                        BTN_P2_START, BTN_P2_SEL_L, BTN_P2_SEL_R, BTN_P2_UP, BTN_P2_DOWN, BTN_P2_LEFT, BTN_P2_RIGHT,
                        BTN_COIN, BTN_TEST, BTN_SERVICE};
    for(int i=0; i < 17; i++) {
        gpio_init(ins[i]);
        gpio_set_dir(ins[i], GPIO_IN);
        gpio_pull_up(ins[i]);
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