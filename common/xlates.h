typedef struct
{
   unsigned short key_code;
   unsigned char modifiers;
   const char *xlation;
} xlate_t;

/* Short versions of the 'modifier' #defines,  to keep the columns
below reasonably short. */

#define SHF             PDC_KEY_MODIFIER_SHIFT
#define CTL             PDC_KEY_MODIFIER_CONTROL
#define ALT             PDC_KEY_MODIFIER_ALT

static const xlate_t xlates[] =  {
             { KEY_END,    0,        "OF"      },
             { KEY_HOME,   0,        "OH"      },
             { KEY_F(1),   0,        "OP"      },
             { KEY_F(2),   0,        "OQ"      },
             { KEY_F(3),   0,        "OR"      },
             { KEY_F(4),   0,        "OS"      },
             { KEY_F(1),   0,        "[11~"    },
             { KEY_F(2),   0,        "[12~"    },
             { KEY_F(3),   0,        "[13~"    },
             { KEY_F(4),   0,        "[14~"    },
             { KEY_F(17),  SHF,      "[15;2~"  },   /* shift-f5 */
             { KEY_F(29),  CTL,      "[15;5~"  },   /* ctrl-f5 */
             { KEY_F(5),   0,        "[15~"    },
             { KEY_F(18),  SHF,      "[17;2~"  },   /* shift-f6 */
             { KEY_F(30),  CTL,      "[17;5~"  },   /* ctrl-f6 */
             { KEY_F(6),   0,        "[17~"    },
             { KEY_F(19),  SHF,      "[18;2~"  },   /* shift-f7 */
             { KEY_F(31),  CTL,      "[18;5~"  },   /* ctrl-f7 */
             { KEY_F(7),   0,        "[18~"    },
             { KEY_F(20),  SHF,      "[19;2~"  },   /* shift-f8 */
             { KEY_F(32),  CTL,      "[19;5~"  },   /* ctrl-f8 */
             { KEY_F(8),   0,        "[19~"    },
             { KEY_SUP,    SHF,      "[1;2A"   },
             { KEY_SDOWN,  SHF,      "[1;2B"   },
             { KEY_SRIGHT, SHF,      "[1;2C"   },
             { KEY_SLEFT,  SHF,      "[1;2D"   },
             { KEY_SEND,   SHF,      "[1;2F"   },   /* shift-end */
             { KEY_SHOME,  SHF,      "[1;2H"   },   /* shift-home */
             { KEY_F(13),  SHF,      "[1;2P"   },   /* shift-f1 */
             { KEY_F(14),  SHF,      "[1;2Q"   },   /* shift-f2 */
             { KEY_F(15),  SHF,      "[1;2R"   },   /* shift-f3 */
             { KEY_F(16),  SHF,      "[1;2S"   },   /* shift-f4 */
             { KEY_F(37),  SHF|CTL,  "[1;6P",  },   /* ctrl-shift-f1 */
             { KEY_F(38),  SHF|CTL,  "[1;6Q",  },   /* ctrl-shift-f2 */
             { KEY_F(39),  SHF|CTL,  "[1;6R",  },   /* ctrl-shift-f3 */
             { KEY_F(40),  SHF|CTL,  "[1;6S",  },   /* ctrl-shift-f4 */
             { KEY_F(41),  SHF|CTL,  "[15;6~"  },   /* ctrl-shift-f5 */
             { KEY_F(42),  SHF|CTL,  "[17;6~"  },   /* ctrl-shift-f6 */
             { KEY_F(43),  SHF|CTL,  "[18;6~"  },   /* ctrl-shift-f7 */
             { KEY_F(44),  SHF|CTL,  "[19;6~"  },   /* ctrl-shift-f8 */
             { KEY_F(45),  SHF|CTL,  "[20;6~"  },   /* ctrl-shift-f9 */
             { KEY_F(46),  SHF|CTL,  "[21;6~"  },   /* ctrl-shift-f10 */
             { KEY_F(47),  SHF|CTL,  "[23;6~"  },   /* ctrl-shift-f11 */
             { KEY_F(48),  SHF|CTL,  "[24;6~"  },   /* ctrl-shift-f12 */
             { KEY_F(58),  ALT,      "[21;3~"  },   /* alt-f10  */
             { ALT_UP,     ALT,      "[1;3A"   },
             { ALT_DOWN,   ALT,      "[1;3B"   },
             { ALT_RIGHT,  ALT,      "[1;3C"   },
             { ALT_LEFT,   ALT,      "[1;3D"   },
             { ALT_PAD5,   ALT,      "[1;3E"   },
             { ALT_END,    ALT,      "[1;3F"   },
             { ALT_HOME,   ALT,      "[1;3H"   },
             { CTL_UP,     CTL,      "[1;5A"   },
             { CTL_DOWN,   CTL,      "[1;5B"   },
             { CTL_RIGHT,  CTL,      "[1;5C"   },
             { CTL_LEFT,   CTL,      "[1;5D"   },
             { CTL_END,    CTL,      "[1;5F"   },
             { CTL_HOME,   CTL,      "[1;5H"   },
             { KEY_F(25),  CTL,      "[1;5P"   },   /* ctrl-f1 */
             { KEY_F(26),  CTL,      "[1;5Q"   },   /* ctrl-f2 */
             { KEY_F(27),  CTL,      "[1;5R"   },   /* ctrl-f3 */
             { KEY_F(28),  CTL,      "[1;5S"   },   /* ctrl-f4 */
             { KEY_HOME,   0,        "[1~"     },
             { KEY_F(21),  SHF,      "[20;2~"  },   /* shift-f9 */
             { KEY_F(33),  CTL,      "[20;5~"  },   /* ctrl-f9 */
             { KEY_F(9),   0,        "[20~"    },
             { KEY_F(22),  SHF,      "[21;2~"  },   /* shift-f10 */
             { KEY_F(34),  CTL,      "[21;5~"  },   /* ctrl-f10 */
             { KEY_F(10),  0,        "[21~"    },
             { KEY_F(23),  SHF,      "[23$"    },   /* shift-f11 on rxvt */
             { KEY_F(23),  SHF,      "[23;2~"  },   /* shift-f11 */
             { KEY_F(35),  CTL,      "[23;5~"  },   /* ctrl-f11 */
             { KEY_F(11),  0,        "[23~"    },
             { KEY_F(24),  SHF,      "[24$"    },   /* shift-f12 on rxvt */
             { KEY_F(24),  SHF,      "[24;2~"  },   /* shift-f12 */
             { KEY_F(36),  CTL,      "[24;5~"  },   /* ctrl-f12 */
             { KEY_F(12),  0,        "[24~"    },
             { KEY_F(15),  SHF,      "[25~"    },   /* shift-f3 on rxvt */
             { KEY_F(16),  SHF,      "[26~"    },   /* shift-f4 on rxvt */
             { KEY_F(17),  SHF,      "[28~"    },   /* shift-f5 on rxvt */
             { KEY_F(18),  SHF,      "[29~"    },   /* shift-f6 on rxvt */
             { KEY_SIC,    SHF,      "[2;2~"   },   /* shift-ins */
             { ALT_INS,    ALT,      "[2;3~"   },
             { CTL_INS,    CTL,      "[2;5~"   },   /* ctrl-ins */
             { KEY_IC,     0,        "[2~"     },
             { KEY_F(19),  SHF,      "[31~"    },   /* shift-f7 on rxvt */
             { KEY_F(20),  SHF,      "[32~"    },   /* shift-f8 on rxvt */
             { KEY_F(21),  SHF,      "[33~"    },   /* shift-f9 on rxvt */
             { KEY_F(22),  SHF,      "[34~"    },   /* shift-f10 on rxvt */
             { KEY_SDC,    SHF,      "[3;2~"   },   /* shift-del */
             { ALT_DEL,    ALT,      "[3;3~"   },
             { CTL_DEL,    CTL,      "[3;5~"   },
             { CTL_DEL,    CTL,      "[3;5~"   },   /* ctrl-del */
             { KEY_DC,     0,        "[3~"     },
             { KEY_END,    0,        "[4~"     },
             { KEY_SPREVIOUS, SHF,   "[5;2~"   },   /* shift-pgup */
             { ALT_PGUP,   ALT,      "[5;3~"   },
             { CTL_PGUP,   CTL,      "[5;5~"   },
             { KEY_PPAGE,  0,        "[5~"     },
             { KEY_SNEXT,  SHF,      "[6;2~"   },   /* shift-pgdn */
             { ALT_PGDN,   ALT,      "[6;3~"   },
             { CTL_PGDN,   CTL,      "[6;5~"   },
             { KEY_NPAGE,  0,        "[6~"     },
             { KEY_HOME,   0,        "[7~"     },    /* rxvt */
             { KEY_END,    0,        "[8~"     },    /* rxvt */
#ifdef __HAIKU__
             { KEY_UP,     0,        "OA"      },
             { KEY_DOWN,   0,        "OB"      },
             { KEY_RIGHT,  0,        "OC"      },
             { KEY_LEFT,   0,        "OD"      },
#else
             { KEY_UP,     0,        "[A"      },
             { KEY_DOWN,   0,        "[B"      },
             { KEY_RIGHT,  0,        "[C"      },
             { KEY_LEFT,   0,        "[D"      },
#endif
             { KEY_B2,     0,        "[E"      },
             { KEY_END,    0,        "[F"      },
             { KEY_HOME,   0,        "[H"      },
             { KEY_BTAB,   SHF,      "[Z"      },    /* Shift-Tab */
             { KEY_F(1),   0,        "[[A"     },    /* Linux console */
             { KEY_F(2),   0,        "[[B"     },
             { KEY_F(3),   0,        "[[C"     },
             { KEY_F(4),   0,        "[[D"     },
             { KEY_F(5),   0,        "[[E"     },
             };
static const size_t n_keycodes = sizeof( xlates) / sizeof( xlates[0]);
