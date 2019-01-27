#include QMK_KEYBOARD_H
#include "app_ble_func.h"
#include "bootloader.h"
#include "keymap_jp.h"

#ifdef PROTOCOL_LUFA
#include "lufa.h"
#include "split_util.h"
#endif

#ifdef SSD1306OLED
#include "LUFA/Drivers/Peripheral/TWI.h"
#include "ssd1306.h"
#endif

#define OSM_SFT OSM(MOD_LSFT)
#define NUMERIC TG(_NUMERIC)

#define IS_RAISE_ON  (IS_LAYER_ON(_RAISE_US)  || IS_LAYER_ON(_RAISE_JIS))
#define IS_RAISE_OFF (IS_LAYER_OFF(_RAISE_US) && IS_LAYER_OFF(_RAISE_JIS))
#define IS_LOWER_ON    IS_LAYER_ON(_LOWER)
#define IS_LOWER_OFF   IS_LAYER_OFF(_LOWER)
#define IS_EXTRA_ON    IS_LAYER_ON(_EXTRA)
#define IS_EXTRA_OFF   IS_LAYER_OFF(_EXTRA)
#define IS_NUMERIC_ON  IS_LAYER_ON(_NUMERIC)
#define IS_NUMERIC_OFF IS_LAYER_OFF(_NUMERIC)

#define MODS (get_mods() | get_weak_mods() | get_oneshot_mods())
#define HAS_LSFT (MODS & MOD_BIT(KC_LSFT))
#define HAS_LCTL (MODS & MOD_BIT(KC_LCTL))
#define HAS_LGUI (MODS & MOD_BIT(KC_LGUI))
#define HAS_LALT (MODS & MOD_BIT(KC_LALT))
#define HAS_RSFT (MODS & MOD_BIT(KC_RSFT))
#define HAS_RCTL (MODS & MOD_BIT(KC_RCTL))
#define HAS_RGUI (MODS & MOD_BIT(KC_RGUI))
#define HAS_RALT (MODS & MOD_BIT(KC_RALT))
#define HAS_SFT (MODS & (MOD_BIT(KC_LSFT) | MOD_BIT(KC_RSFT)))
#define HAS_CTL (MODS & (MOD_BIT(KC_LCTL) | MOD_BIT(KC_RCTL)))
#define HAS_GUI (MODS & (MOD_BIT(KC_LGUI) | MOD_BIT(KC_RGUI)))
#define HAS_ALT (MODS & (MOD_BIT(KC_LALT) | MOD_BIT(KC_RALT)))

#define IS_NUMLOCK_ENABLE     (host_keyboard_leds() & (1 << USB_LED_NUM_LOCK))
#define IS_CAPSLOCK_ENABLE    (host_keyboard_leds() & (1 << USB_LED_CAPS_LOCK))
#define IS_SCROLL_LOCK_ENABLE (host_keyboard_leds() & (1 << USB_LED_SCROLL_LOCK))

extern keymap_config_t keymap_config;
extern uint8_t is_master;

bool is_oled_on = false;
bool is_orig_layout_enabled = true;
bool is_rmods_enabled = false;
bool is_zhtg_replaced = false;
bool is_capslock_swapped = false;
bool is_mac = false;
bool is_jis = false;

bool is_oled_extra_on;
bool is_lower_held;
bool is_lower_pressed;
bool is_reset_pressed;

enum layers {
    _QWERTY,
    _ORIGINAL,
    _MOD_SC,
    _MOD_MAC,
    _MOD_SCM,
    _RAISE_US,
    _RAISE_JIS,
    _LOWER,
    _EXTRA,
    _RMODS,
    _NUMERIC,
    _BLE
};

enum additional_keycodes {
    RAISE = SAFE_RANGE,
    LOWER,
    BLE,
    HLD_LOW,
    LAYO_TG,
    RMOD_TG,
    ZHTG_TG,
    JIS_TG,
    SWP_CAP,
    OS_TG,
    OLED_TG,
    AD_WO_L, /* Start advertising without whitelist  */
    /* BLE_DIS, #<{(| Disable BLE HID sending              |)}># */
    BLE_EN,  /* Enable BLE HID sending               */
    /* USB_DIS, #<{(| Disable USB HID sending              |)}># */
    USB_EN,  /* Enable USB HID sending               */
    DELBNDS, /* Delete all bonding                   */
    ADV_ID0, /* Start advertising to PeerID 0        */
    ADV_ID1, /* Start advertising to PeerID 1        */
    ADV_ID2, /* Start advertising to PeerID 2        */
    ADV_ID3, /* Start advertising to PeerID 3        */
    ADV_ID4, /* Start advertising to PeerID 4        */
    BATT_LV, /* Display battery level in milli volts */
    DEL_ID0, /* Delete bonding of PeerID 0           */
    DEL_ID1, /* Delete bonding of PeerID 1           */
    DEL_ID2, /* Delete bonding of PeerID 2           */
    DEL_ID3, /* Delete bonding of PeerID 3           */
    DEL_ID4, /* Delete bonding of PeerID 4           */
    ENT_DFU, /* Start bootloader                     */
    ENT_SLP  /* Deep sleep mode                      */
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Qwerty
     * ,-----------------------------------------.             ,-----------------------------------------.
     * |   (  |   Q  |   W  |   E  |   R  |   T  |             |   Y  |   U  |   I  |   O  |   P  |   =  |
     * |------+------+------+------+------+------|             |------+------+------+------+------+------|
     * | Ctrl |   A  |   S  |   D  |   F  |   G  |             |   H  |   J  |   K  |   L  |   ;  |Raise |
     * |------+------+------+------+------+------|             |------+------+------+------+------+------|
     * |Shift |   Z  |   X  |   C  |   V  |   B  |             |   N  |   M  |   ,  |   .  |   /  |   -  |
     * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
     * | Caps | NUM  | GUI  | Alt  |Lower |Space |OLEDTG|OLEDTG| Bksp |Enter | Left | Down |  Up  |Right |
     * `-------------------------------------------------------------------------------------------------'
     */

    [_QWERTY] = LAYOUT( \
        KC_LPRN, KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_EQL,  \
        KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                      KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, RAISE,   \
        OSM_SFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                      KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_MINS, \
        KC_CAPS, NUMERIC, KC_LGUI, KC_LALT, LOWER,   KC_SPC,  XXXXXXX, XXXXXXX, KC_BSPC, KC_ENT,  KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT  \
    ),

    /* Original
     * ,-----------------------------------------.             ,-----------------------------------------.
     * |      |   Y  |   W  |   E  |   R  |   F  |             |   Q  |   U  |   I  |   O  |   P  |      |
     * |------+------+------+------+------+------|             |------+------+------+------+------+------|
     * |      |   A  |   S  |   D  |   T  |   G  |             |   H  |   J  |   K  |   L  |   ;  |      |
     * |------+------+------+------+------+------|             |------+------+------+------+------+------|
     * |      |   Z  |   X  |   V  |   C  |   B  |             |   N  |   M  |   ,  |   .  |   /  |      |
     * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
     * |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
     * `-------------------------------------------------------------------------------------------------'
     */

    [_ORIGINAL] = LAYOUT( \
        _______, KC_Y,    KC_W,    KC_E,    KC_R,    KC_F,                      KC_Q,    KC_U,    KC_I,    KC_O,    KC_P,    _______, \
        _______, KC_A,    KC_S,    KC_D,    KC_T,    KC_G,                      KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, _______, \
        _______, KC_Z,    KC_X,    KC_V,    KC_C,    KC_B,                      KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______  \
    ),

    /*          SwapCaps  Mac   SwapCaps & Mac
     * Ctrl ->    Caps    GUI        GUI
     * Caps ->    Ctrl               Ctrl
     * GUI  ->            Alt        Alt
     * Alt  ->            Ctrl       Caps
     */

    [_MOD_SC] = LAYOUT( \
        _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
        KC_CAPS, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
        KC_LCTL, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______  \
    ),

    [_MOD_MAC] = LAYOUT( \
        _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
        KC_LGUI, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
        _______, _______, KC_LALT, KC_LCTL, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______  \
    ),

    [_MOD_SCM] = LAYOUT( \
        _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
        KC_LGUI, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
        KC_LCTL, _______, KC_LALT, KC_CAPS, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______  \
    ),

    /* Raise
     * ,-----------------------------------------.             ,-----------------------------------------.
     * | Tab  |   !  |   @  |   #  |   $  |   %  |             |      |   ^  |   &  |   *  | Del  | Esc  |
     * |------+------+------+------+------+------|             |------+------+------+------+------+------|
     * | Ctrl |   "  |   '  |   {  |   [  |   }  |             |   ]  |   |  |   \  |   ~  |   `  |RAISE |
     * |------+------+------+------+------+------|             |------+------+------+------+------+------|
     * |Shift |      |      |      |      |      |             |      |      |      |      |      |      |
     * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
     * |      |      | GUI  | Alt  |LOWER |Space |      |      | Bksp |Enter |      |      |      |      |
     * `-------------------------------------------------------------------------------------------------'
     */

    [_RAISE_US] = LAYOUT( \
        KC_TAB,  KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC,                   XXXXXXX, KC_CIRC, KC_AMPR, KC_ASTR, KC_DEL,  KC_ESC,  \
        _______, KC_DQT,  KC_QUOT, KC_LCBR, KC_LBRC, KC_RCBR,                   KC_RBRC, KC_PIPE, KC_BSLS, KC_TILD, KC_GRV,  _______, \
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
        XXXXXXX, XXXXXXX, _______, _______, _______, _______, _______, _______, _______, _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX  \
    ),

    [_RAISE_JIS] = LAYOUT( \
        KC_TAB,  KC_EXLM, JP_AT,   KC_HASH, KC_DLR,  KC_PERC,                   XXXXXXX, JP_CIRC, JP_AMPR, JP_ASTR, KC_DEL,  KC_ESC,  \
        _______, JP_DQT,  JP_QUOT, JP_LCBR, JP_LBRC, JP_RCBR,                   JP_RBRC, JP_PIPE, JP_BSLS, JP_TILD, JP_GRV,  _______, \
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
        XXXXXXX, XXXXXXX, _______, _______, _______, _______, _______, _______, _______, _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX  \
    ),

    /* Lower
     * ,-----------------------------------------.             ,-----------------------------------------.
     * | Tab  |   1  |   2  |   3  |   4  |   5  |             |   6  |   7  |   8  |   9  |   0  | Esc  |
     * |------+------+------+------+------+------|             |------+------+------+------+------+------|
     * | Ctrl | Home | PgDn | PgUp | End  |      |             | Left | Down |  Up  |Right | Del  |RAISE |
     * |------+------+------+------+------+------|             |------+------+------+------+------+------|
     * |Shift |      |      |      |      |      |             |      |      |   ,  |   .  |HLDLOW|      |
     * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
     * |      |      | GUI  | Alt  |LOWER |Space | BLE  |      | Bksp |Enter |      |      |      |      |
     * `-------------------------------------------------------------------------------------------------'
     */

    [_LOWER] = LAYOUT( \
        KC_TAB,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_ESC,  \
        _______, KC_HOME, KC_PGDN, KC_PGUP, KC_END,  XXXXXXX,                   KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_DEL,  _______, \
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, KC_COMM, KC_DOT,  HLD_LOW, XXXXXXX, \
        XXXXXXX, XXXXXXX, _______, _______, _______, _______, BLE,     _______, _______, _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX  \
    ),

    /* Extra
     * ,-----------------------------------------.             ,-----------------------------------------.
     * |  F1  |  F2  |  F3  |  F4  |  F5  |  F6  |             |  F7  |  F8  |  F9  | F10  | F11  | F12  |
     * |------+------+------+------+------+------|             |------+------+------+------+------+------|
     * | Ctrl | App  |      |      |      |      |             |      |JIS_TG|      |OS_TGL|Insert|RAISE |
     * |------+------+------+------+------+------|             |------+------+------+------+------+------|
     * |Shift |ZHTGTG|      |SWPCAP|LAYOTG|      |             |NumLc |RMODTG| PtSc | ScLc |Pause |      |
     * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
     * |Reset |      | GUI  | Alt  |LOWER |Space |      |      | Bksp |Enter |      |      |      |      |
     * `-------------------------------------------------------------------------------------------------'
     */

    [_EXTRA] = LAYOUT( \
        KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                     KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  \
        _______, KC_APP,  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, JIS_TG,  XXXXXXX, OS_TG,   KC_INS,  _______, \
        _______, ZHTG_TG, XXXXXXX, SWP_CAP, LAYO_TG, XXXXXXX,                   KC_NLCK, RMOD_TG, KC_PSCR, KC_SLCK, KC_PAUS, XXXXXXX, \
        RESET,   XXXXXXX, _______, _______, _______, _______, _______, _______, _______, _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX  \
    ),

    [_RMODS] = LAYOUT( \
        _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_RALT, KC_RGUI, KC_RCTL, KC_RSFT  \
    ),

    /* Numeric
     * ,-----------------------------------------.             ,-----------------------------------------.
     * |      |      |      |  Up  |      |      |             |   7  |   8  |   9  |   -  |   /  | Bksp |
     * |------+------+------+------+------+------|             |-------------+------+------+------+------|
     * |      |      | Left | Down |Right |      |             |   4  |   5  |   6  |   +  |   *  | Del  |
     * |------+------+------+------+------+------|             |------|------+------+------+------+------|
     * |      |      |      |      |      |      |             |   1  |   2  |   3  |Enter |      |      |
     * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
     * |      | NUM  |      |      |      |      |      |Space |      |   0  |   .  |      |      |      |
     * `-------------------------------------------------------------------------------------------------'
     */

    [_NUMERIC] = LAYOUT( \
        XXXXXXX, XXXXXXX, XXXXXXX, KC_UP,   XXXXXXX, XXXXXXX,                   KC_P7,   KC_P8,   KC_P9,   KC_PMNS, KC_PSLS, KC_BSPC, \
        XXXXXXX, XXXXXXX, KC_LEFT, KC_DOWN, KC_RGHT, XXXXXXX,                   KC_P4,   KC_P5,   KC_P6,   KC_PPLS, KC_PAST, KC_DEL,  \
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   KC_P1,   KC_P2,   KC_P3,   KC_PENT, XXXXXXX, XXXXXXX, \
        XXXXXXX, _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_SPC,  XXXXXXX, KC_P0,   KC_PDOT, XXXXXXX, XXXXXXX, XXXXXXX  \
    ),

    /* Bletooth
     * ,-----------------------------------------.             ,-----------------------------------------.
     * |      |      |      |      |      |      |             |      |      |      |      |      |      |
     * |------+------+------+------+------+------|             |-------------+------+------+------+------|
     * |      |      |      |      |      |      |             |      |      |      |      |      |      |
     * |------+------+------+------+------+------|             |------|------+------+------+------+------|
     * |      |      |      |      |      |      |             |      |      |      |      |      |      |
     * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
     * |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
     * `-------------------------------------------------------------------------------------------------'
     */

    [_BLE] = LAYOUT( \
        BLE_EN,  AD_WO_L, XXXXXXX, BATT_LV, XXXXXXX, USB_EN,                    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
        ADV_ID0, ADV_ID1, ADV_ID2, ADV_ID3, ADV_ID4, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
        DELBNDS, XXXXXXX, XXXXXXX, XXXXXXX, _______, XXXXXXX, _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX  \
    )
};

void tap_key(uint16_t kc) {
    register_code(kc);
    unregister_code(kc);
}

void set_orig_layout(void) {
    if (is_orig_layout_enabled) {
        layer_on(_ORIGINAL);
    }
    else {
        layer_off(_ORIGINAL);
    }
}

void set_mod_layer(void) {
    layer_off(_MOD_SC);
    layer_off(_MOD_MAC);
    layer_off(_MOD_SCM);
    if (is_capslock_swapped) {
        if (is_mac) {
            layer_on(_MOD_SCM);
        }
        else {
            layer_on(_MOD_SC);
        }
    }
    else if (is_mac) {
        layer_on(_MOD_MAC);
    }
}

void switch_extra(void) {
    if (IS_RAISE_ON && IS_LOWER_ON) {
        layer_on(_EXTRA);
        is_lower_held = false;
    }
    else {
        layer_off(_EXTRA);
        is_oled_extra_on = false;
    }
}

void initialize(void) {
    is_reset_pressed = false;
    is_oled_extra_on = false;
    is_lower_held    = false;
    is_lower_pressed = false;

    set_single_persistent_default_layer(_QWERTY);
    set_orig_layout();
    set_mod_layer();
    /* set_ble_enabled(false); */
    /* set_usb_enabled(true); */

    #ifdef SSD1306OLED
    TWI_Init(TWI_BIT_PRESCALE_1, TWI_BITLENGTH_FROM_FREQ(1, 800000));
    iota_gfx_init(!has_usb()); // turns on the display
    #endif
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    bool is_left_pressed, is_right_pressed;
    char str[16];
    switch (keycode) {
        case RAISE:
            if (record->event.pressed) {
                layer_on(is_jis ? _RAISE_JIS : _RAISE_US);
            }
            else {
                layer_off(_RAISE_US);
                layer_off(_RAISE_JIS);
            }
            switch_extra();
            return false;
            break;
        case LOWER:
            if (record->event.pressed) {
                layer_on(_LOWER);
            }
            else if (!is_lower_held) {
                layer_off(_LOWER);
            }
            switch_extra();
            is_lower_pressed = record->event.pressed;
            return false;
            break;
        case BLE:
            if (record->event.pressed) {
                layer_on(_BLE);
            }
            else {
                layer_off(_BLE);
            }
            return false;
            break;

        case HLD_LOW:
            if (record->event.pressed && !is_lower_pressed) {
                layer_off(_LOWER);
            }
            else {
                is_lower_held = record->event.pressed;
            }
            return false;
            break;

        case LAYO_TG:
            if (record->event.pressed) {
                is_orig_layout_enabled = !is_orig_layout_enabled;
                set_orig_layout();
                is_oled_extra_on = true;
            }
            return false;
            break;
        case RMOD_TG:
            if (record->event.pressed) {
                is_rmods_enabled = !is_rmods_enabled;
                if (is_rmods_enabled) {
                    layer_on(_RMODS);
                }
                else {
                    layer_off(_RMODS);
                }
                is_oled_extra_on = true;
            }
            return false;
            break;
        case ZHTG_TG:
            if (record->event.pressed) {
                is_zhtg_replaced = !is_zhtg_replaced;
                is_oled_extra_on = true;
            }
            return false;
            break;
        case JIS_TG:
            if (record->event.pressed) {
                layer_off(is_jis ? _RAISE_JIS : _RAISE_US);
                is_jis = !is_jis;
                layer_on(is_jis ? _RAISE_JIS : _RAISE_US);
                is_oled_extra_on = true;
            }
            return false;
            break;
        case SWP_CAP:
            if (record->event.pressed) {
                is_capslock_swapped = !is_capslock_swapped;
                set_mod_layer();
                layer_on(is_jis ? _RAISE_JIS : _RAISE_US);
                layer_on(_LOWER);
                layer_on(_EXTRA);
                is_oled_extra_on = true;
            }
            return false;
            break;
        case OS_TG:
            if (record->event.pressed) {
                is_mac = !is_mac;
                set_mod_layer();
                is_oled_extra_on = true;
            }
            return false;
            break;
        case OLED_TG:
            if (record->event.pressed) {
                is_oled_on = !is_oled_on;
            }
            return false;
            break;

        // "=" for jis
        case KC_EQL:
            if (record->event.pressed && is_jis && IS_RAISE_OFF) {
                if (HAS_SFT) {
                    tap_key(KC_SCLN);
                }
                else {
                    add_weak_mods(MOD_LSFT);
                    tap_key(JP_MINS);
                    del_weak_mods(MOD_LSFT);
                }
                return false;
            }
            return true;
            break;

        // ":" and for jis and Ctrl-; send Ctrl-[
        case KC_SCLN:
            if (record->event.pressed) {
                if (HAS_CTL) {
                    tap_key(KC_LBRC);
                    return false;
                }
                else if (is_jis && HAS_SFT && IS_RAISE_OFF) {
                    is_left_pressed  = HAS_LSFT;
                    is_right_pressed = HAS_RSFT;
                    if (is_left_pressed)  unregister_code(KC_LSFT);
                    if (is_right_pressed) unregister_code(KC_RSFT);

                    tap_key(JP_COLN);

                    if (is_left_pressed)  register_code(KC_LSFT);
                    if (is_right_pressed) register_code(KC_RSFT);
                    return false;
                }
            }
            return true;
            break;

        // "_" for jis and capslock
        case KC_MINS:
            if (record->event.pressed && ((is_jis && HAS_SFT) || IS_CAPSLOCK_ENABLE) && IS_RAISE_OFF) {
                if (IS_CAPSLOCK_ENABLE && HAS_SFT) {
                    is_left_pressed  = HAS_LSFT;
                    is_right_pressed = HAS_RSFT;
                    if (is_left_pressed)  unregister_code(KC_LSFT);
                    if (is_right_pressed) unregister_code(KC_RSFT);

                    tap_key(KC_MINS);

                    if (is_left_pressed)  register_code(KC_LSFT);
                    if (is_right_pressed) register_code(KC_RSFT);
                }
                else {
                    add_weak_mods(MOD_LSFT);
                    tap_key(is_jis ? JP_BSLS : KC_MINS);
                    del_weak_mods(MOD_LSFT);
                }
                return false;
            }
            return true;
            break;

        // replace "(" with  ")"  when pressed Shift
        // replace "(" with "Tab" when pressed Ctrl or Alt
        case KC_LPRN:
            if (record->event.pressed) {
                if (HAS_CTL || HAS_ALT) {
                    tap_key(KC_TAB);
                    return false;
                }
                else if (HAS_SFT) {
                    tap_key(is_jis ? KC_9 : KC_0);
                    return false;
                }
                else if (is_jis) {
                    add_weak_mods(MOD_LSFT);
                    tap_key(KC_8);
                    del_weak_mods(MOD_LSFT);
                    return false;
                }
            }
            return true;
            break;

        // replace "Ctrl-Space" when is_zhtg_replaced is true
        case KC_SPC:
            if (record->event.pressed && is_zhtg_replaced && HAS_CTL
                    && IS_RAISE_OFF && IS_LOWER_OFF && !HAS_SFT && !HAS_GUI && !HAS_ALT) {
                is_left_pressed  = HAS_LCTL;
                is_right_pressed = HAS_RCTL;
                if (is_left_pressed)  unregister_code(KC_LCTL);
                if (is_right_pressed) unregister_code(KC_RCTL);

                if (is_jis) {
                    tap_key(JP_ZHTG);
                }
                else {
                    add_weak_mods(MOD_LALT);
                    tap_key(KC_GRV);
                    del_weak_mods(MOD_LALT);
                }

                if (is_left_pressed)  register_code(KC_LCTL);
                if (is_right_pressed) register_code(KC_RCTL);
                return false;
            }
            return true;
            break;
        case RESET:
            if (record->event.pressed) {
                is_reset_pressed = true;
                /* matrix_scan(); */
                bootloader_jump();
            }
            /* return true; */
            return false;
            break;

        case DELBNDS:
            if (record->event.pressed) {
                delete_bonds();
            }
            return false;
            break;
        case AD_WO_L:
            if (record->event.pressed) {
                restart_advertising_wo_whitelist();
            }
            return false;
            break;
        case USB_EN:
            if (record->event.pressed) {
                set_ble_enabled(false);
                set_usb_enabled(true);
            }
            return false;
            break;
        /*
         * case USB_DIS:
         *     if (record->event.pressed) {
         *         set_usb_enabled(false);
         *     }
         *     return false;
         *     break;
         */
        case BLE_EN:
            if (record->event.pressed) {
                set_usb_enabled(false);
                set_ble_enabled(true);
            }
            return false;
            break;
        /*
         * case BLE_DIS:
         *     if (record->event.pressed) {
         *         set_ble_enabled(false);
         *     }
         *     return false;
         *     break;
         */
        case ADV_ID0:
            if (record->event.pressed) {
                restart_advertising_id(0);
            }
            return false;
            break;
        case ADV_ID1:
            if (record->event.pressed) {
                restart_advertising_id(1);
            }
            return false;
            break;
        case ADV_ID2:
            if (record->event.pressed) {
                restart_advertising_id(2);
            }
            return false;
            break;
        case ADV_ID3:
            if (record->event.pressed) {
                restart_advertising_id(3);
            }
            return false;
            break;
        case ADV_ID4:
            if (record->event.pressed) {
                restart_advertising_id(4);
            }
            return false;
            break;
        case DEL_ID0:
            if (record->event.pressed) {
                delete_bond_id(0);
            }
            return false;
            break;
        case DEL_ID1:
            if (record->event.pressed) {
                delete_bond_id(1);
            }
            return false;
            break;
        case DEL_ID2:
            if (record->event.pressed) {
                delete_bond_id(2);
            }
            return false;
            break;
        case DEL_ID3:
            if (record->event.pressed) {
                delete_bond_id(3);
            }
            return false;
            break;
        case BATT_LV:
            if (record->event.pressed) {
                sprintf(str, "%4dmV", get_vcc());
                send_string(str);
            }
            return false;
            break;
        case ENT_DFU:
            if (record->event.pressed) {
                bootloader_jump();
            }
            return false;
            break;
        case ENT_SLP:
            if (!record->event.pressed) {
                sleep_mode_enter();
            }
            return false;
    }
    return true;
}

//SSD1306 OLED update loop, make sure to add #define SSD1306OLED in config.h
#ifdef SSD1306OLED
void matrix_scan_user(void) {
    iota_gfx_task(); // this is what updates the display continuously
}

void matrix_update(struct CharacterMatrix *dest, const struct CharacterMatrix *source) {
    if (memcmp(dest->display, source->display, sizeof(dest->display))) {
        memcpy(dest->display, source->display, sizeof(dest->display));
        dest->dirty = true;
    }
}

void render_status(struct CharacterMatrix *matrix) {
    /* ,---------------------.
     * |lo    US   [ Layer ] |
     * |go  Qwerty  NumLock  |
     * |RMods EJRep CapsLock |
     * | SwapCaps ScrollLock |
     * `---------------------'
     */
    static char os_logo[][2][3]={{{0x95, 0x96, 0}, {0xB5, 0xB6, 0}}, {{0x97, 0x98, 0}, {0xB7, 0xB8, 0}}};
    char led[24];
    snprintf(led, sizeof(led), "%s   %s   [%s]\n",
            os_logo[is_mac ? 0 : 1][0],
            is_jis        ? "JIS"
                          : " US",
            IS_NUMERIC_ON ? "Numeric"
          : IS_EXTRA_ON   ? " Extra "
          : IS_LOWER_ON   ? " Lower "
          : IS_RAISE_ON   ? " Raise "
                          : "Default"
    );
    matrix_write(matrix, led);

    snprintf(led, sizeof(led), "%s %s %s\n",
            os_logo[is_mac ? 0 : 1][1],
            is_orig_layout_enabled ? "Original"
                                   : " Qwerty ",
            IS_NUMLOCK_ENABLE      ? "NumLock"
                                   : "       "
    );
    matrix_write(matrix, led);

    snprintf(led, sizeof(led), "%s %s %s\n",
            is_rmods_enabled   ? "RMods"
                               : "     ",
            is_zhtg_replaced   ? "EJRep"
                               : "     ",
            IS_CAPSLOCK_ENABLE ? "CapsLock"
                               : "        "
    );
    matrix_write(matrix, led);

    snprintf(led, sizeof(led), " %s %s",
            is_capslock_swapped   ? "SwapCaps"
                                  : "        ",
            IS_SCROLL_LOCK_ENABLE ? "ScrollLock"
                                  : "          "
    );
    matrix_write(matrix, led);
}

#define EMPTY     {{   0,    0,    0,    0,    0, 0}, {   0,    0,    0,    0,    0, 0}, \
                   {   0,    0,    0,    0,    0, 0}, {   0,    0,    0,    0,    0, 0}}
#define CHAR_EXLM {{0xB0,    0,    0,    0,    0, 0}, {0xB0,    0,    0,    0,    0, 0}, \
                   {0x20,    0,    0,    0,    0, 0}, {0xB0,    0,    0,    0,    0, 0}}
#define CHAR_C    {{0x20, 0xB0, 0xB0, 0xB0,    0, 0}, {0xB0, 0x20, 0x20, 0x20,    0, 0}, \
                   {0xB0, 0x20, 0x20, 0x20,    0, 0}, {0x20, 0xB0, 0xB0, 0xB0,    0, 0}}
#define CHAR_D    {{0xB0, 0xB0, 0xB0, 0x20,    0, 0}, {0xB0, 0x20, 0x20, 0xB0,    0, 0}, \
                   {0xB0, 0x20, 0x20, 0xB0,    0, 0}, {0xB0, 0xB0, 0xB0, 0x20,    0, 0}}
#define CHAR_F    {{0xB0, 0xB0, 0xB0, 0xB0,    0, 0}, {0xB0, 0x20, 0x20, 0x20,    0, 0}, \
                   {0xB0, 0xB0, 0xB0, 0x20,    0, 0}, {0xB0, 0x20, 0x20, 0x20,    0, 0}}
#define CHAR_K    {{0xB0, 0x20, 0x20,    0,    0, 0}, {0xB0, 0x20, 0xB0,    0,    0, 0}, \
                   {0xB0, 0xB0, 0x20,    0,    0, 0}, {0xB0, 0x20, 0xB0,    0,    0, 0}}
#define CHAR_L    {{0xB0, 0x20, 0x20, 0x20,    0, 0}, {0xB0, 0x20, 0x20, 0x20,    0, 0}, \
                   {0xB0, 0x20, 0x20, 0x20,    0, 0}, {0xB0, 0xB0, 0xB0, 0xB0,    0, 0}}
#define CHAR_M    {{0xB0, 0x20, 0x20, 0x20, 0xB0, 0}, {0xB0, 0xB0, 0x20, 0xB0, 0xB0, 0}, \
                   {0xB0, 0x20, 0xB0, 0x20, 0xB0, 0}, {0xB0, 0x20, 0x20, 0x20, 0xB0, 0}}
#define CHAR_N    {{0xB0, 0x20, 0x20, 0xB0,    0, 0}, {0xB0, 0xB0, 0x20, 0xB0,    0, 0}, \
                   {0xB0, 0x20, 0xB0, 0xB0,    0, 0}, {0xB0, 0x20, 0x20, 0xB0,    0, 0}}
#define CHAR_P    {{0xB0, 0xB0, 0xB0, 0x20,    0, 0}, {0xB0, 0x20, 0x20, 0xB0,    0, 0}, \
                   {0xB0, 0xB0, 0xB0, 0x20,    0, 0}, {0xB0, 0x20, 0x20, 0x20,    0, 0}}
#define CHAR_S    {{0x20, 0xB0, 0xB0, 0xB0,    0, 0}, {0x20, 0xB0, 0x20, 0x20,    0, 0}, \
                   {0x20, 0x20, 0xB0, 0x20,    0, 0}, {0xB0, 0xB0, 0xB0, 0x20,    0, 0}}
#define CHAR_U    {{0xB0, 0x20, 0x20, 0xB0,    0, 0}, {0xB0, 0x20, 0x20, 0xB0,    0, 0}, \
                   {0xB0, 0x20, 0x20, 0xB0,    0, 0}, {0x20, 0xB0, 0xB0, 0x20,    0, 0}}
#define MAX_LEN 5

void render_alart(struct CharacterMatrix *matrix) {
    /* ,---------------------.---------------------.---------------------.---------------------.---------------------.
     * |@@@  @@@@ @  @       |@  @ @  @ @   @      |@ @  @ @  @ @    @   | @@@ @@@  @    @     | @@@  @@@ @    @     |
     * |@  @ @    @  @       |@@ @ @  @ @@ @@      |@ @@ @ @  @ @    @ @ |@    @  @ @    @ @   | @   @    @    @ @   |
     * |@  @ @@@  @  @       |@ @@ @  @ @ @ @      |  @ @@ @  @ @    @@  |@    @@@  @    @@    |  @  @    @    @@    |
     * |@@@  @     @@        |@  @  @@  @   @      |@ @  @  @@  @@@@ @ @ | @@@ @    @@@@ @ @   |@@@   @@@ @@@@ @ @   |
     * `---------------------'---------------------'---------------------'---------------------'---------------------'
     */ 
    static char str_alarts[][MAX_LEN][4][6] = {
        {CHAR_D,    CHAR_F, CHAR_U, EMPTY,  EMPTY},  // DFU Mode
        {CHAR_N,    CHAR_U, CHAR_M, EMPTY,  EMPTY},  // Numeric
        {CHAR_EXLM, CHAR_N, CHAR_U, CHAR_L, CHAR_K}, // !NumLock
        {CHAR_C,    CHAR_P, CHAR_L, CHAR_K, EMPTY},  // CapsLock
        {CHAR_S,    CHAR_C, CHAR_L, CHAR_K, EMPTY}   // ScrollLock
    };

    int index = is_reset_pressed      ? 0
              : IS_NUMERIC_ON         ? IS_NUMLOCK_ENABLE ? 1 : 2
              : IS_SCROLL_LOCK_ENABLE ? 4
              : IS_CAPSLOCK_ENABLE    ? 3
                                      : -1;
    if (index < 0) return;

    for (int i=0; i<4; i++) {
        matrix_write(matrix, "\n");
        for (int j=0; j<MAX_LEN; j++) {
            matrix_write(matrix, str_alarts[index][j][i]);
            if (j != MAX_LEN - 1) {
                matrix_write(matrix, " ");
            }
        }
    }
}

void iota_gfx_task_user(void) {
    struct CharacterMatrix matrix;
    matrix_clear(&matrix);

    if (is_master) {
        if (is_reset_pressed || IS_NUMERIC_ON || (!is_oled_extra_on && (IS_CAPSLOCK_ENABLE || IS_SCROLL_LOCK_ENABLE))) {
            render_alart(&matrix);
        }
        else if (is_oled_on || is_oled_extra_on){
            render_status(&matrix);
        }
    }
    matrix_update(&display, &matrix);
}
#endif
