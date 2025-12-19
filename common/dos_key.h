/* Key codes for MS-DOS.  Can be used in the OS/2 console port,  VT (DOS
builds) and the DOS and DOSVGA ports.        */

/************************************************************************
 *    Table for key code translation of function keys in keypad mode    *
 *    These values are for strict IBM keyboard compatibles only         *
 ************************************************************************/

#ifdef PDC_WIDE
   typedef int32_t key_t;
#else
   typedef int key_t;
#endif

static key_t key_table[] =
{
    -1,             ALT_ESC,        -1,             0,            /* 0-3 */
    -1,             -1,             -1,             -1,           /* 4-7 */
    -1,             -1,             -1,             -1,          /* 8-11 */
    -1,             -1,             ALT_BKSP,       KEY_BTAB,      /* 12 */
    ALT_Q,          ALT_W,          ALT_E,          ALT_R,         /* 16 */
    ALT_T,          ALT_Y,          ALT_U,          ALT_I,         /* 20 */
    ALT_O,          ALT_P,          ALT_LBRACKET,   ALT_RBRACKET,  /* 24 */
    ALT_ENTER,      -1,             ALT_A,          ALT_S,         /* 28 */
    ALT_D,          ALT_F,          ALT_G,          ALT_H,         /* 32 */
    ALT_J,          ALT_K,          ALT_L,          ALT_SEMICOLON, /* 36 */
    ALT_FQUOTE,     ALT_BQUOTE,     -1,             ALT_BSLASH,    /* 40 */
    ALT_Z,          ALT_X,          ALT_C,          ALT_V,         /* 44 */
    ALT_B,          ALT_N,          ALT_M,          ALT_COMMA,     /* 48 */
    ALT_STOP,       ALT_FSLASH,     -1,             ALT_PADSTAR,   /* 52 */
    -1,             -1,             -1,             KEY_F(1),      /* 56 */
    KEY_F(2),       KEY_F(3),       KEY_F(4),       KEY_F(5),      /* 60 */
    KEY_F(6),       KEY_F(7),       KEY_F(8),       KEY_F(9),      /* 64 */
    KEY_F(10),      -1,             -1,             KEY_HOME,      /* 68 */
    KEY_UP,         KEY_PPAGE,      ALT_PADMINUS,   KEY_LEFT,      /* 72 */
    KEY_B2,         KEY_RIGHT,      ALT_PADPLUS,    KEY_END,       /* 76 */
    KEY_DOWN,       KEY_NPAGE,      KEY_IC,         KEY_DC,        /* 80 */
    KEY_F(13),      KEY_F(14),      KEY_F(15),      KEY_F(16),     /* 84 */
    KEY_F(17),      KEY_F(18),      KEY_F(19),      KEY_F(20),     /* 88 */
    KEY_F(21),      KEY_F(22),      KEY_F(25),      KEY_F(26),     /* 92 */
    KEY_F(27),      KEY_F(28),      KEY_F(29),      KEY_F(30),     /* 96 */
    KEY_F(31),      KEY_F(32),      KEY_F(33),      KEY_F(34),     /* 100 */
    KEY_F(37),      KEY_F(38),      KEY_F(39),      KEY_F(40),     /* 104 */
    KEY_F(41),      KEY_F(42),      KEY_F(43),      KEY_F(44),     /* 108 */
    KEY_F(45),      KEY_F(46),      -1,             CTL_LEFT,      /* 112 */
    CTL_RIGHT,      CTL_END,        CTL_PGDN,       CTL_HOME,      /* 116 */
    ALT_1,          ALT_2,          ALT_3,          ALT_4,         /* 120 */
    ALT_5,          ALT_6,          ALT_7,          ALT_8,         /* 124 */
    ALT_9,          ALT_0,          ALT_MINUS,      ALT_EQUAL,     /* 128 */
    CTL_PGUP,       KEY_F(11),      KEY_F(12),      KEY_F(23),     /* 132 */
    KEY_F(24),      KEY_F(35),      KEY_F(36),      KEY_F(47),     /* 136 */
    KEY_F(48),      CTL_UP,         CTL_PADMINUS,   CTL_PADCENTER, /* 140 */
    CTL_PADPLUS,    CTL_DOWN,       CTL_INS,        CTL_DEL,       /* 144 */
    CTL_TAB,        CTL_PADSLASH,   CTL_PADSTAR,    ALT_HOME,      /* 148 */
    ALT_UP,         ALT_PGUP,       -1,             ALT_LEFT,      /* 152 */
    -1,             ALT_RIGHT,      -1,             ALT_END,       /* 156 */
    ALT_DOWN,       ALT_PGDN,       ALT_INS,        ALT_DEL,       /* 160 */
    ALT_PADSLASH,   ALT_TAB,        ALT_PADENTER,   -1             /* 164 */
};
