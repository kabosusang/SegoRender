#pragma once

//From SDL_scancode.h
enum KeyScand
{
    SG_KEY_UNKNOWN = 0,

    /**
     *  \name Usage page 0x07
     *
     *  These values are from usage page 0x07 (USB keyboard page).
     */
    /* @{ */

    SG_KEY_A = 4,
    SG_KEY_B = 5,
    SG_KEY_C = 6,
    SG_KEY_D = 7,
    SG_KEY_E = 8,
    SG_KEY_F = 9,
    SG_KEY_G = 10,
    SG_KEY_H = 11,
    SG_KEY_I = 12,
    SG_KEY_J = 13,
    SG_KEY_K = 14,
    SG_KEY_L = 15,
    SG_KEY_M = 16,
    SG_KEY_N = 17,
    SG_KEY_O = 18,
    SG_KEY_P = 19,
    SG_KEY_Q = 20,
    SG_KEY_R = 21,
    SG_KEY_S = 22,
    SG_KEY_T = 23,
    SG_KEY_U = 24,
    SG_KEY_V = 25,
    SG_KEY_W = 26,
    SG_KEY_X = 27,
    SG_KEY_Y = 28,
    SG_KEY_Z = 29,

    SG_KEY_1 = 30,
    SG_KEY_2 = 31,
    SG_KEY_3 = 32,
    SG_KEY_4 = 33,
    SG_KEY_5 = 34,
    SG_KEY_6 = 35,
    SG_KEY_7 = 36,
    SG_KEY_8 = 37,
    SG_KEY_9 = 38,
    SG_KEY_0 = 39,

    SG_KEY_RETURN = 40,
    SG_KEY_ESCAPE = 41,
    SG_KEY_BACKSPACE = 42,
    SG_KEY_TAB = 43,
    SG_KEY_SPACE = 44,

    SG_KEY_MINUS = 45,
    SG_KEY_EQUALS = 46,
    SG_KEY_LEFTBRACKET = 47,
    SG_KEY_RIGHTBRACKET = 48,
    SG_KEY_BACKSLASH = 49, /**< Located at the lower left of the return
                                  *   key on ISO keyboards and at the right end
                                  *   of the QWERTY row on ANSI keyboards.
                                  *   Produces REVERSE SOLIDUS (backslash) and
                                  *   VERTICAL LINE in a US layout, REVERSE
                                  *   SOLIDUS and VERTICAL LINE in a UK Mac
                                  *   layout, NUMBER SIGN and TILDE in a UK
                                  *   Windows layout, DOLLAR SIGN and POUND SIGN
                                  *   in a Swiss German layout, NUMBER SIGN and
                                  *   APOSTROPHE in a German layout, GRAVE
                                  *   ACCENT and POUND SIGN in a French Mac
                                  *   layout, and ASTERISK and MICRO SIGN in a
                                  *   French Windows layout.
                                  */
    SG_KEY_NONUSHASH = 50, /**< ISO USB keyboards actually use this code
                                  *   instead of 49 for the same key, but all
                                  *   OSes I've seen treat the two codes
                                  *   identically. So, as an implementor, unless
                                  *   your keyboard generates both of those
                                  *   codes and your OS treats them differently,
                                  *   you should generate SG_KEY_BACKSLASH
                                  *   instead of this code. As a user, you
                                  *   should not rely on this code because SDL
                                  *   will never generate it with most (all?)
                                  *   keyboards.
                                  */
    SG_KEY_SEMICOLON = 51,
    SG_KEY_APOSTROPHE = 52,
    SG_KEY_GRAVE = 53, /**< Located in the top left corner (on both ANSI
                              *   and ISO keyboards). Produces GRAVE ACCENT and
                              *   TILDE in a US Windows layout and in US and UK
                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                              *   and NOT SIGN in a UK Windows layout, SECTION
                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                              *   layouts on ISO keyboards, SECTION SIGN and
                              *   DEGREE SIGN in a Swiss German layout (Mac:
                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                              *   DEGREE SIGN in a German layout (Mac: only on
                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                              *   French Windows layout, COMMERCIAL AT and
                              *   NUMBER SIGN in a French Mac layout on ISO
                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                              *   SIGN in a Swiss German, German, or French Mac
                              *   layout on ANSI keyboards.
                              */
    SG_KEY_COMMA = 54,
    SG_KEY_PERIOD = 55,
    SG_KEY_SLASH = 56,

    SG_KEY_CAPSLOCK = 57,

    SG_KEY_F1 = 58,
    SG_KEY_F2 = 59,
    SG_KEY_F3 = 60,
    SG_KEY_F4 = 61,
    SG_KEY_F5 = 62,
    SG_KEY_F6 = 63,
    SG_KEY_F7 = 64,
    SG_KEY_F8 = 65,
    SG_KEY_F9 = 66,
    SG_KEY_F10 = 67,
    SG_KEY_F11 = 68,
    SG_KEY_F12 = 69,

    SG_KEY_PRINTSCREEN = 70,
    SG_KEY_SCROLLLOCK = 71,
    SG_KEY_PAUSE = 72,
    SG_KEY_INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
    SG_KEY_HOME = 74,
    SG_KEY_PAGEUP = 75,
    SG_KEY_DELETE = 76,
    SG_KEY_END = 77,
    SG_KEY_PAGEDOWN = 78,
    SG_KEY_RIGHT = 79,
    SG_KEY_LEFT = 80,
    SG_KEY_DOWN = 81,
    SG_KEY_UP = 82,

    SG_KEY_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                     */
    SG_KEY_KP_DIVIDE = 84,
    SG_KEY_KP_MULTIPLY = 85,
    SG_KEY_KP_MINUS = 86,
    SG_KEY_KP_PLUS = 87,
    SG_KEY_KP_ENTER = 88,
    SG_KEY_KP_1 = 89,
    SG_KEY_KP_2 = 90,
    SG_KEY_KP_3 = 91,
    SG_KEY_KP_4 = 92,
    SG_KEY_KP_5 = 93,
    SG_KEY_KP_6 = 94,
    SG_KEY_KP_7 = 95,
    SG_KEY_KP_8 = 96,
    SG_KEY_KP_9 = 97,
    SG_KEY_KP_0 = 98,
    SG_KEY_KP_PERIOD = 99,

    SG_KEY_NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                        *   keyboards have over ANSI ones,
                                        *   located between left shift and Y.
                                        *   Produces GRAVE ACCENT and TILDE in a
                                        *   US or UK Mac layout, REVERSE SOLIDUS
                                        *   (backslash) and VERTICAL LINE in a
                                        *   US or UK Windows layout, and
                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
                                        *   in a Swiss German, German, or French
                                        *   layout. */
    SG_KEY_APPLICATION = 101, /**< windows contextual menu, compose */
    SG_KEY_POWER = 102, /**< The USB document says this is a status flag,
                               *   not a physical key - but some Mac keyboards
                               *   do have a power key. */
    SG_KEY_KP_EQUALS = 103,
    SG_KEY_F13 = 104,
    SG_KEY_F14 = 105,
    SG_KEY_F15 = 106,
    SG_KEY_F16 = 107,
    SG_KEY_F17 = 108,
    SG_KEY_F18 = 109,
    SG_KEY_F19 = 110,
    SG_KEY_F20 = 111,
    SG_KEY_F21 = 112,
    SG_KEY_F22 = 113,
    SG_KEY_F23 = 114,
    SG_KEY_F24 = 115,
    SG_KEY_EXECUTE = 116,
    SG_KEY_HELP = 117,
    SG_KEY_MENU = 118,
    SG_KEY_SELECT = 119,
    SG_KEY_STOP = 120,
    SG_KEY_AGAIN = 121,   /**< redo */
    SG_KEY_UNDO = 122,
    SG_KEY_CUT = 123,
    SG_KEY_COPY = 124,
    SG_KEY_PASTE = 125,
    SG_KEY_FIND = 126,
    SG_KEY_MUTE = 127,
    SG_KEY_VOLUMEUP = 128,
    SG_KEY_VOLUMEDOWN = 129,
/* not sure whether there's a reason to enable these */
/*     SG_KEY_LOCKINGCAPSLOCK = 130,  */
/*     SG_KEY_LOCKINGNUMLOCK = 131, */
/*     SG_KEY_LOCKINGSCROLLLOCK = 132, */
    SG_KEY_KP_COMMA = 133,
    SG_KEY_KP_EQUALSAS400 = 134,

    SG_KEY_INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
                                            footnotes in USB doc */
    SG_KEY_INTERNATIONAL2 = 136,
    SG_KEY_INTERNATIONAL3 = 137, /**< Yen */
    SG_KEY_INTERNATIONAL4 = 138,
    SG_KEY_INTERNATIONAL5 = 139,
    SG_KEY_INTERNATIONAL6 = 140,
    SG_KEY_INTERNATIONAL7 = 141,
    SG_KEY_INTERNATIONAL8 = 142,
    SG_KEY_INTERNATIONAL9 = 143,
    SG_KEY_LANG1 = 144, /**< Hangul/English toggle */
    SG_KEY_LANG2 = 145, /**< Hanja conversion */
    SG_KEY_LANG3 = 146, /**< Katakana */
    SG_KEY_LANG4 = 147, /**< Hiragana */
    SG_KEY_LANG5 = 148, /**< Zenkaku/Hankaku */
    SG_KEY_LANG6 = 149, /**< reserved */
    SG_KEY_LANG7 = 150, /**< reserved */
    SG_KEY_LANG8 = 151, /**< reserved */
    SG_KEY_LANG9 = 152, /**< reserved */

    SG_KEY_ALTERASE = 153, /**< Erase-Eaze */
    SG_KEY_SYSREQ = 154,
    SG_KEY_CANCEL = 155,
    SG_KEY_CLEAR = 156,
    SG_KEY_PRIOR = 157,
    SG_KEY_RETURN2 = 158,
    SG_KEY_SEPARATOR = 159,
    SG_KEY_OUT = 160,
    SG_KEY_OPER = 161,
    SG_KEY_CLEARAGAIN = 162,
    SG_KEY_CRSEL = 163,
    SG_KEY_EXSEL = 164,

    SG_KEY_KP_00 = 176,
    SG_KEY_KP_000 = 177,
    SG_KEY_THOUSANDSSEPARATOR = 178,
    SG_KEY_DECIMALSEPARATOR = 179,
    SG_KEY_CURRENCYUNIT = 180,
    SG_KEY_CURRENCYSUBUNIT = 181,
    SG_KEY_KP_LEFTPAREN = 182,
    SG_KEY_KP_RIGHTPAREN = 183,
    SG_KEY_KP_LEFTBRACE = 184,
    SG_KEY_KP_RIGHTBRACE = 185,
    SG_KEY_KP_TAB = 186,
    SG_KEY_KP_BACKSPACE = 187,
    SG_KEY_KP_A = 188,
    SG_KEY_KP_B = 189,
    SG_KEY_KP_C = 190,
    SG_KEY_KP_D = 191,
    SG_KEY_KP_E = 192,
    SG_KEY_KP_F = 193,
    SG_KEY_KP_XOR = 194,
    SG_KEY_KP_POWER = 195,
    SG_KEY_KP_PERCENT = 196,
    SG_KEY_KP_LESS = 197,
    SG_KEY_KP_GREATER = 198,
    SG_KEY_KP_AMPERSAND = 199,
    SG_KEY_KP_DBLAMPERSAND = 200,
    SG_KEY_KP_VERTICALBAR = 201,
    SG_KEY_KP_DBLVERTICALBAR = 202,
    SG_KEY_KP_COLON = 203,
    SG_KEY_KP_HASH = 204,
    SG_KEY_KP_SPACE = 205,
    SG_KEY_KP_AT = 206,
    SG_KEY_KP_EXCLAM = 207,
    SG_KEY_KP_MEMSTORE = 208,
    SG_KEY_KP_MEMRECALL = 209,
    SG_KEY_KP_MEMCLEAR = 210,
    SG_KEY_KP_MEMADD = 211,
    SG_KEY_KP_MEMSUBTRACT = 212,
    SG_KEY_KP_MEMMULTIPLY = 213,
    SG_KEY_KP_MEMDIVIDE = 214,
    SG_KEY_KP_PLUSMINUS = 215,
    SG_KEY_KP_CLEAR = 216,
    SG_KEY_KP_CLEARENTRY = 217,
    SG_KEY_KP_BINARY = 218,
    SG_KEY_KP_OCTAL = 219,
    SG_KEY_KP_DECIMAL = 220,
    SG_KEY_KP_HEXADECIMAL = 221,

    SG_KEY_LCTRL = 224,
    SG_KEY_LSHIFT = 225,
    SG_KEY_LALT = 226, /**< alt, option */
    SG_KEY_LGUI = 227, /**< windows, command (apple), meta */
    SG_KEY_RCTRL = 228,
    SG_KEY_RSHIFT = 229,
    SG_KEY_RALT = 230, /**< alt gr, option */
    SG_KEY_RGUI = 231, /**< windows, command (apple), meta */

    SG_KEY_MODE = 257,    /**< I'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special KMOD_MODE for it I'm adding it here
                                 */

    /* @} *//* Usage page 0x07 */

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    SG_KEY_AUDIONEXT = 258,
    SG_KEY_AUDIOPREV = 259,
    SG_KEY_AUDIOSTOP = 260,
    SG_KEY_AUDIOPLAY = 261,
    SG_KEY_AUDIOMUTE = 262,
    SG_KEY_MEDIASELECT = 263,
    SG_KEY_WWW = 264,
    SG_KEY_MAIL = 265,
    SG_KEY_CALCULATOR = 266,
    SG_KEY_COMPUTER = 267,
    SG_KEY_AC_SEARCH = 268,
    SG_KEY_AC_HOME = 269,
    SG_KEY_AC_BACK = 270,
    SG_KEY_AC_FORWARD = 271,
    SG_KEY_AC_STOP = 272,
    SG_KEY_AC_REFRESH = 273,
    SG_KEY_AC_BOOKMARKS = 274,

    /* @} *//* Usage page 0x0C */

    /**
     *  \name Walther keys
     *
     *  These are values that Christian Walther added (for mac keyboard?).
     */
    /* @{ */

    SG_KEY_BRIGHTNESSDOWN = 275,
    SG_KEY_BRIGHTNESSUP = 276,
    SG_KEY_DISPLAYSWITCH = 277, /**< display mirroring/dual display
                                           switch, video mode switch */
    SG_KEY_KBDILLUMTOGGLE = 278,
    SG_KEY_KBDILLUMDOWN = 279,
    SG_KEY_KBDILLUMUP = 280,
    SG_KEY_EJECT = 281,
    SG_KEY_SLEEP = 282,

    SG_KEY_APP1 = 283,
    SG_KEY_APP2 = 284,

    /* @} *//* Walther keys */

    /**
     *  \name Usage page 0x0C (additional media keys)
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    SG_KEY_AUDIOREWIND = 285,
    SG_KEY_AUDIOFASTFORWARD = 286,

    /* @} *//* Usage page 0x0C (additional media keys) */

    /* Add any other keys here. */

    SDL_NUM_SCANCODES = 512 /**< not a key, just marks the number of scancodes
                                 for array bounds */
};