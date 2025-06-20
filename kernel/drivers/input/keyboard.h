/**
 * @file keyboard.h
 * @brief Keyboard Input Device Driver for FG-OS
 * 
 * This file provides PS/2 keyboard device support including scan code
 * translation, key event generation, and keyboard state management
 * for the device driver framework.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright Copyright (c) 2024 FGCompany Official. All rights reserved.
 */

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <types.h>
#include "../device.h"

/**
 * @brief PS/2 keyboard I/O ports
 */
#define KB_DATA_PORT            0x60    /**< Keyboard data port */
#define KB_STATUS_PORT          0x64    /**< Keyboard status port */
#define KB_COMMAND_PORT         0x64    /**< Keyboard command port */

/**
 * @brief PS/2 controller commands
 */
#define KB_CMD_READ_CONFIG      0x20    /**< Read configuration byte */
#define KB_CMD_WRITE_CONFIG     0x60    /**< Write configuration byte */
#define KB_CMD_DISABLE_PORT2    0xA7    /**< Disable second PS/2 port */
#define KB_CMD_ENABLE_PORT2     0xA8    /**< Enable second PS/2 port */
#define KB_CMD_TEST_PORT2       0xA9    /**< Test second PS/2 port */
#define KB_CMD_TEST_CONTROLLER  0xAA    /**< Test PS/2 controller */
#define KB_CMD_TEST_PORT1       0xAB    /**< Test first PS/2 port */
#define KB_CMD_DISABLE_PORT1    0xAD    /**< Disable first PS/2 port */
#define KB_CMD_ENABLE_PORT1     0xAE    /**< Enable first PS/2 port */

/**
 * @brief PS/2 keyboard commands
 */
#define KB_DEV_CMD_LED          0xED    /**< Set LED state */
#define KB_DEV_CMD_ECHO         0xEE    /**< Echo command */
#define KB_DEV_CMD_SCANCODE     0xF0    /**< Get/set scan code set */
#define KB_DEV_CMD_IDENTIFY     0xF2    /**< Identify keyboard */
#define KB_DEV_CMD_REPEAT       0xF3    /**< Set repeat rate and delay */
#define KB_DEV_CMD_ENABLE       0xF4    /**< Enable scanning */
#define KB_DEV_CMD_DISABLE      0xF5    /**< Disable scanning */
#define KB_DEV_CMD_DEFAULT      0xF6    /**< Set default parameters */
#define KB_DEV_CMD_RESET        0xFF    /**< Reset keyboard */

/**
 * @brief PS/2 status register bits
 */
#define KB_STATUS_OUTPUT_FULL   (1 << 0)   /**< Output buffer full */
#define KB_STATUS_INPUT_FULL    (1 << 1)   /**< Input buffer full */
#define KB_STATUS_SYSTEM_FLAG   (1 << 2)   /**< System flag */
#define KB_STATUS_COMMAND_DATA  (1 << 3)   /**< Command/data flag */
#define KB_STATUS_TIMEOUT       (1 << 6)   /**< Timeout error */
#define KB_STATUS_PARITY        (1 << 7)   /**< Parity error */

/**
 * @brief Keyboard scan codes (Set 1)
 */
#define KB_SCAN_ESCAPE          0x01
#define KB_SCAN_1               0x02
#define KB_SCAN_2               0x03
#define KB_SCAN_3               0x04
#define KB_SCAN_4               0x05
#define KB_SCAN_5               0x06
#define KB_SCAN_6               0x07
#define KB_SCAN_7               0x08
#define KB_SCAN_8               0x09
#define KB_SCAN_9               0x0A
#define KB_SCAN_0               0x0B
#define KB_SCAN_MINUS           0x0C
#define KB_SCAN_EQUALS          0x0D
#define KB_SCAN_BACKSPACE       0x0E
#define KB_SCAN_TAB             0x0F
#define KB_SCAN_Q               0x10
#define KB_SCAN_W               0x11
#define KB_SCAN_E               0x12
#define KB_SCAN_R               0x13
#define KB_SCAN_T               0x14
#define KB_SCAN_Y               0x15
#define KB_SCAN_U               0x16
#define KB_SCAN_I               0x17
#define KB_SCAN_O               0x18
#define KB_SCAN_P               0x19
#define KB_SCAN_LEFT_BRACKET    0x1A
#define KB_SCAN_RIGHT_BRACKET   0x1B
#define KB_SCAN_ENTER           0x1C
#define KB_SCAN_LEFT_CTRL       0x1D
#define KB_SCAN_A               0x1E
#define KB_SCAN_S               0x1F
#define KB_SCAN_D               0x20
#define KB_SCAN_F               0x21
#define KB_SCAN_G               0x22
#define KB_SCAN_H               0x23
#define KB_SCAN_J               0x24
#define KB_SCAN_K               0x25
#define KB_SCAN_L               0x26
#define KB_SCAN_SEMICOLON       0x27
#define KB_SCAN_QUOTE           0x28
#define KB_SCAN_BACKTICK        0x29
#define KB_SCAN_LEFT_SHIFT      0x2A
#define KB_SCAN_BACKSLASH       0x2B
#define KB_SCAN_Z               0x2C
#define KB_SCAN_X               0x2D
#define KB_SCAN_C               0x2E
#define KB_SCAN_V               0x2F
#define KB_SCAN_B               0x30
#define KB_SCAN_N               0x31
#define KB_SCAN_M               0x32
#define KB_SCAN_COMMA           0x33
#define KB_SCAN_PERIOD          0x34
#define KB_SCAN_SLASH           0x35
#define KB_SCAN_RIGHT_SHIFT     0x36
#define KB_SCAN_KEYPAD_STAR     0x37
#define KB_SCAN_LEFT_ALT        0x38
#define KB_SCAN_SPACE           0x39
#define KB_SCAN_CAPS_LOCK       0x3A
#define KB_SCAN_F1              0x3B
#define KB_SCAN_F2              0x3C
#define KB_SCAN_F3              0x3D
#define KB_SCAN_F4              0x3E
#define KB_SCAN_F5              0x3F
#define KB_SCAN_F6              0x40
#define KB_SCAN_F7              0x41
#define KB_SCAN_F8              0x42
#define KB_SCAN_F9              0x43
#define KB_SCAN_F10             0x44
#define KB_SCAN_NUM_LOCK        0x45
#define KB_SCAN_SCROLL_LOCK     0x46

/**
 * @brief Special scan codes
 */
#define KB_SCAN_RELEASED        0x80    /**< Key released flag */
#define KB_SCAN_EXTENDED        0xE0    /**< Extended scan code prefix */

/**
 * @brief Key codes (virtual key codes)
 */
typedef enum {
    KEY_UNKNOWN = 0,
    KEY_ESCAPE,
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
    KEY_MINUS, KEY_EQUALS, KEY_BACKSPACE, KEY_TAB,
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
    KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_ENTER, KEY_LEFT_CTRL,
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L,
    KEY_SEMICOLON, KEY_QUOTE, KEY_BACKTICK, KEY_LEFT_SHIFT, KEY_BACKSLASH,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M,
    KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_RIGHT_SHIFT,
    KEY_KEYPAD_STAR, KEY_LEFT_ALT, KEY_SPACE, KEY_CAPS_LOCK,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
    KEY_NUM_LOCK, KEY_SCROLL_LOCK,
    KEY_KEYPAD_7, KEY_KEYPAD_8, KEY_KEYPAD_9, KEY_KEYPAD_MINUS,
    KEY_KEYPAD_4, KEY_KEYPAD_5, KEY_KEYPAD_6, KEY_KEYPAD_PLUS,
    KEY_KEYPAD_1, KEY_KEYPAD_2, KEY_KEYPAD_3, KEY_KEYPAD_0, KEY_KEYPAD_PERIOD,
    KEY_F11, KEY_F12,
    KEY_RIGHT_CTRL, KEY_RIGHT_ALT,
    KEY_HOME, KEY_UP, KEY_PAGE_UP, KEY_LEFT, KEY_RIGHT, KEY_END, KEY_DOWN, KEY_PAGE_DOWN,
    KEY_INSERT, KEY_DELETE,
    KEY_LEFT_GUI, KEY_RIGHT_GUI, KEY_MENU,
    KEY_MAX
} key_code_t;

/**
 * @brief Key event types
 */
typedef enum {
    KEY_EVENT_PRESSED = 0,      /**< Key pressed */
    KEY_EVENT_RELEASED,         /**< Key released */
    KEY_EVENT_REPEAT,           /**< Key repeat */
    KEY_EVENT_MAX
} key_event_type_t;

/**
 * @brief Modifier key flags
 */
#define KB_MOD_LEFT_SHIFT       (1 << 0)   /**< Left Shift */
#define KB_MOD_RIGHT_SHIFT      (1 << 1)   /**< Right Shift */
#define KB_MOD_LEFT_CTRL        (1 << 2)   /**< Left Ctrl */
#define KB_MOD_RIGHT_CTRL       (1 << 3)   /**< Right Ctrl */
#define KB_MOD_LEFT_ALT         (1 << 4)   /**< Left Alt */
#define KB_MOD_RIGHT_ALT        (1 << 5)   /**< Right Alt */
#define KB_MOD_LEFT_GUI         (1 << 6)   /**< Left GUI (Windows) */
#define KB_MOD_RIGHT_GUI        (1 << 7)   /**< Right GUI (Windows) */
#define KB_MOD_CAPS_LOCK        (1 << 8)   /**< Caps Lock */
#define KB_MOD_NUM_LOCK         (1 << 9)   /**< Num Lock */
#define KB_MOD_SCROLL_LOCK      (1 << 10)  /**< Scroll Lock */

/**
 * @brief Convenience modifier combinations
 */
#define KB_MOD_SHIFT            (KB_MOD_LEFT_SHIFT | KB_MOD_RIGHT_SHIFT)
#define KB_MOD_CTRL             (KB_MOD_LEFT_CTRL | KB_MOD_RIGHT_CTRL)
#define KB_MOD_ALT              (KB_MOD_LEFT_ALT | KB_MOD_RIGHT_ALT)
#define KB_MOD_GUI              (KB_MOD_LEFT_GUI | KB_MOD_RIGHT_GUI)

/**
 * @brief LED flags
 */
#define KB_LED_SCROLL_LOCK      (1 << 0)   /**< Scroll Lock LED */
#define KB_LED_NUM_LOCK         (1 << 1)   /**< Num Lock LED */
#define KB_LED_CAPS_LOCK        (1 << 2)   /**< Caps Lock LED */

/**
 * @brief Key event structure
 */
typedef struct {
    key_event_type_t    type;           /**< Event type */
    key_code_t          key_code;       /**< Virtual key code */
    uint8_t             scan_code;      /**< Hardware scan code */
    uint16_t            modifiers;      /**< Current modifier state */
    char                ascii;          /**< ASCII character (if printable) */
    uint64_t            timestamp;      /**< Event timestamp */
} key_event_t;

/**
 * @brief Keyboard state structure
 */
typedef struct {
    uint16_t            modifiers;      /**< Current modifier state */
    uint8_t             leds;           /**< Current LED state */
    bool                key_states[256]; /**< Key state array (pressed/released) */
    uint32_t            repeat_delay;   /**< Key repeat delay in ms */
    uint32_t            repeat_rate;    /**< Key repeat rate in ms */
    uint64_t            last_key_time;  /**< Last key event timestamp */
    key_code_t          last_key;       /**< Last key pressed */
} keyboard_state_t;

/**
 * @brief Keyboard device structure
 */
typedef struct keyboard_device {
    device_t                device;         /**< Base device structure */
    keyboard_state_t        state;          /**< Keyboard state */
    key_event_t             event_buffer[64]; /**< Event buffer */
    uint32_t                event_head;     /**< Event buffer head */
    uint32_t                event_tail;     /**< Event buffer tail */
    uint32_t                event_count;    /**< Number of events in buffer */
    bool                    extended_mode;  /**< Extended scan code mode */
    struct keyboard_device* next;           /**< Next keyboard device */
} keyboard_device_t;

/**
 * @brief Keyboard statistics
 */
typedef struct {
    uint32_t    devices_found;          /**< Keyboards found */
    uint64_t    keys_pressed;           /**< Total keys pressed */
    uint64_t    keys_released;          /**< Total keys released */
    uint64_t    key_repeats;            /**< Key repeat events */
    uint32_t    scan_code_errors;       /**< Scan code errors */
    uint32_t    buffer_overflows;       /**< Event buffer overflows */
    uint64_t    events_generated;       /**< Total events generated */
    uint64_t    events_consumed;        /**< Total events consumed */
} keyboard_stats_t;

// Function declarations

/**
 * @brief Initialize the keyboard driver
 * 
 * @return 0 on success, negative error code on failure
 */
int keyboard_init(void);

/**
 * @brief Shutdown the keyboard driver
 */
void keyboard_shutdown(void);

/**
 * @brief Detect and initialize PS/2 keyboards
 * 
 * @return Number of keyboards found
 */
uint32_t keyboard_detect_devices(void);

/**
 * @brief Read key event from keyboard
 * 
 * @param device Pointer to keyboard device
 * @param event Pointer to event structure to fill
 * @return 0 on success, negative error code on failure
 */
int keyboard_read_event(keyboard_device_t* device, key_event_t* event);

/**
 * @brief Check if key events are available
 * 
 * @param device Pointer to keyboard device
 * @return Number of available events
 */
uint32_t keyboard_events_available(keyboard_device_t* device);

/**
 * @brief Set keyboard LED state
 * 
 * @param device Pointer to keyboard device
 * @param leds LED flags
 * @return 0 on success, negative error code on failure
 */
int keyboard_set_leds(keyboard_device_t* device, uint8_t leds);

/**
 * @brief Set keyboard repeat rate and delay
 * 
 * @param device Pointer to keyboard device
 * @param delay Repeat delay in milliseconds
 * @param rate Repeat rate in milliseconds
 * @return 0 on success, negative error code on failure
 */
int keyboard_set_repeat(keyboard_device_t* device, uint32_t delay, uint32_t rate);

/**
 * @brief Reset keyboard to default state
 * 
 * @param device Pointer to keyboard device
 * @return 0 on success, negative error code on failure
 */
int keyboard_reset(keyboard_device_t* device);

/**
 * @brief Enable/disable keyboard scanning
 * 
 * @param device Pointer to keyboard device
 * @param enable Enable or disable scanning
 * @return 0 on success, negative error code on failure
 */
int keyboard_set_scanning(keyboard_device_t* device, bool enable);

/**
 * @brief Get current keyboard state
 * 
 * @param device Pointer to keyboard device
 * @return Pointer to keyboard state structure
 */
const keyboard_state_t* keyboard_get_state(keyboard_device_t* device);

/**
 * @brief Check if key is currently pressed
 * 
 * @param device Pointer to keyboard device
 * @param key_code Key code to check
 * @return true if pressed, false otherwise
 */
bool keyboard_is_key_pressed(keyboard_device_t* device, key_code_t key_code);

/**
 * @brief Convert scan code to key code
 * 
 * @param scan_code Hardware scan code
 * @param extended Extended scan code flag
 * @return Virtual key code
 */
key_code_t keyboard_scan_to_key(uint8_t scan_code, bool extended);

/**
 * @brief Convert key code to ASCII character
 * 
 * @param key_code Virtual key code
 * @param modifiers Current modifier state
 * @return ASCII character (0 if not printable)
 */
char keyboard_key_to_ascii(key_code_t key_code, uint16_t modifiers);

/**
 * @brief Get keyboard device by index
 * 
 * @param index Device index
 * @return Pointer to keyboard device, NULL if not found
 */
keyboard_device_t* keyboard_get_device(uint32_t index);

/**
 * @brief Get keyboard statistics
 * 
 * @return Pointer to keyboard statistics structure
 */
const keyboard_stats_t* keyboard_get_stats(void);

/**
 * @brief Reset keyboard statistics
 */
void keyboard_reset_stats(void);

/**
 * @brief Get key event type name
 * 
 * @param type Event type
 * @return Event type name string
 */
const char* keyboard_event_type_to_string(key_event_type_t type);

/**
 * @brief Get key code name
 * 
 * @param key_code Key code
 * @return Key code name string
 */
const char* keyboard_key_code_to_string(key_code_t key_code);

/**
 * @brief Dump keyboard device information
 * 
 * @param device Pointer to keyboard device (NULL for all devices)
 */
void keyboard_dump_device_info(keyboard_device_t* device);

/**
 * @brief Handle keyboard interrupt
 * 
 * @param irq IRQ number
 */
void keyboard_interrupt_handler(uint32_t irq);

/**
 * @brief Process raw scan code
 * 
 * @param device Pointer to keyboard device
 * @param scan_code Raw scan code from hardware
 */
void keyboard_process_scan_code(keyboard_device_t* device, uint8_t scan_code);

/**
 * @brief Add event to keyboard buffer
 * 
 * @param device Pointer to keyboard device
 * @param event Pointer to event to add
 * @return 0 on success, negative error code on failure
 */
int keyboard_add_event(keyboard_device_t* device, const key_event_t* event);

/**
 * @brief Clear keyboard event buffer
 * 
 * @param device Pointer to keyboard device
 */
void keyboard_clear_events(keyboard_device_t* device);

#endif /* __KEYBOARD_H__ */ 