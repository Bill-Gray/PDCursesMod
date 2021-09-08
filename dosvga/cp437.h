
static const unsigned short cp437_to_unicode[256] = {
   0,
   0x263a,        /*   1 smiling face              */
   0x263b,        /*   2 smiling face inverted     */
   0x2665,        /*   3 heart                     */
   0x2666,        /*   4 diamond                   */
   0x2663,        /*   5 club                      */
   0x2660,        /*   6 spade                     */
   0x2024,        /*   7 small bullet              */
   0x25d8,        /*   8 inverted bullet           */
   0x25bc,        /*   9 hollow bullet             */
   0x25d9,        /*  10 inverted hollow bullet    */
   0x2642,        /*  11 male/Mars symbol          */
   0x2640,        /*  12 female/Venus symbol       */
   0x266a,        /*  13 eighth note               */
   0x266c,        /*  14 two sixteenth notes       */
   0x263c,        /*  15 splat                     */
   0x25b6,        /*  16 right-pointing triangle   */
   0x25c0,        /*  17 left-pointing triangle    */
   0x2195,        /*  18 double up/down arrow      */
   0x203c,        /*  19 double exclamation !!     */
   0x00b6,        /*  20 pilcrow                   */
   0xa7,          /*  21                           */
   0x2582,        /*  22 lower 1/3 block           */
   0x280d,        /*  23 double up/down arrow      */
   0x2191,        /*  24 up arrow                  */
   0x2193,        /*  25 down arrow                */
   0x2192,        /*  26 right arrow               */
   0x2190,        /*  27 left arrow                */
   0x2319,        /*  28                           */
   0x280c,        /*  29 left & right arrow        */
   0x25b2,        /*  30 up triangle               */
   0x25bc,        /*  31 down triangle             */

   32,    33,    34,    35,    36,    37,    38,    39, /*  !"#$%&'  */
   40,    41,    42,    43,    44,    45,    46,    47, /* ()*+,-./  */
   48,    49,    50,    51,    52,    53,    54,    55, /* 01234567  */
   56,    57,    58,    59,    60,    61,    62,    63, /* 89:;<=>?  */
   64,    65,    66,    67,    68,    69,    70,    71, /* @ABCDEFG  */
   72,    73,    74,    75,    76,    77,    78,    79, /* HIJKLMNO  */
   80,    81,    82,    83,    84,    85,    86,    87, /* PQRSTUVW  */
   88,    89,    90,    91,    92,    93,    94,    95, /* XYZ[\]^_  */
   96,    97,    98,    99,   100,   101,   102,   103, /* `abcdefg  */
  104,   105,   106,   107,   108,   109,   110,   111, /* hijklmno  */
  112,   113,   114,   115,   116,   117,   118,   119, /* pqrstuvw  */
  120,   121,   122,   123,   124,   125,   126,        /* xyz{|}~   */

   0x2302,        /*  127                          */
   0x00c7,        /*  128 C cedilla                */
   0x00fc,        /*  129 u umlaut                 */
   0x00e9,        /*  130 e acute                  */
   0x00e2,        /*  131 a circumflex             */
   0x00e4,        /*  132 a umlaut                 */
   0x00e0,        /*  133 a grave                  */
   0x00e5,        /*  134 a ring                   */
   0x00e7,        /*  135 c cedilla                */
   0x00ea,        /*  136 e circumflex             */
   0x00eb,        /*  137 e umlaut                 */
   0x00e8,        /*  138 e grave                  */
   0x00ef,        /*  139 i umlaut                 */
   0x00ee,        /*  140 i circumflex             */
   0x00ec,        /*  141 i grave                  */
   0x00c4,        /*  142 A umlaut                 */
   0x00c5,        /*  143 A ring                   */

   0x00c9,        /*  144 E acute                  */
   0x00e6,        /*  145 ae ligature/ash          */
   0x00c6,        /*  146 AE ligature/ash          */
   0x00f4,        /*  147 o circumflex             */
   0x00f5,        /*  148 o umlaut                 */
   0x00f2,        /*  149 o grave                  */
   0x00fb,        /*  150 u circumflex             */
   0x00f9,        /*  151 u grave                  */
   0x00ff,        /*  152 y umlaut                 */
   0x00d6,        /*  153 O umlaut                 */
   0x00dc,        /*  154 U umlaut                 */
   0x00a2,        /*  155 cent                     */
   0x00a3,        /*  156 sterling                 */
   0x00a5,        /*  157 yen                      */
   0x20a7,        /*  158 peseta                   */
   0x0192,        /*  159 f with hook              */

   0x00e1,        /*  160 a acute                  */
   0x00ed,        /*  161 i acute                  */
   0x00f3,        /*  162 o acute                  */
   0x00fa,        /*  163 u acute                  */
   0x00f1,        /*  164 n tilde                  */
   0x00d1,        /*  165 N tilde                  */
   0x00aa,        /*  166 a ordinal                */
   0x00ba,        /*  167 o ordinal                */
   0x00bf,        /*  168 inverted question mark   */
   0x2310,        /*  169                          */
   0x00ac,        /*  170                          */
   0x00bd,        /*  171 vulgar 1/2               */
   0x00bc,        /*  172 vulgar 1/4               */
   0x00a1,        /*  173 inverted exclamation     */
   0x00ab,        /*  174 left angle quote mark    */
   0x00bb,        /*  175 right angle quote mark   */

   0x2591,        /*  176 light shade              */
   0x2592,        /*  177 medium shade             */
   0x2593,        /*  178 dark shade               */
   0x2502,        /*  179 vertical line            */
   0x2524,        /*  180 right tee                */
   0x2561,        /*  181 SD right tee             */
   0x2562,        /*  182 DS right tee             */
   0x2556,        /*  183 DS upper right corner    */
   0x2555,        /*  184 SD upper right corner    */
   0x2563,        /*  185 D right tee              */
   0x2551,        /*  186 D vertical line          */
   0x2557,        /*  187 D upper right corner     */
   0x255d,        /*  188 D lower right corner     */
   0x255c,        /*  189 DS lower right corner    */
   0x255b,        /*  190 SD lower right corner    */
   0x2510,        /*  191 upper right corner       */

   0x2514,        /*  192 lower left corner        */
   0x2534,        /*  193 bottom tee               */
   0x252c,        /*  194 top tee                  */
   0x251c,        /*  195 left tee                 */
   0x2500,        /*  196 horizontal line          */
   0x253c,        /*  197 plus                     */
   0x255e,        /*  198 SD left tee              */
   0x255f,        /*  199 DS left tee              */
   0x255a,        /*  200 D lower left corner      */
   0x2554,        /*  201 D upper left corner      */
   0x2569,        /*  202 D bottom tee             */
   0x2566,        /*  203 D top tee                */
   0x2560,        /*  204 D left tee               */
   0x2550,        /*  205 D horizontal line        */
   0x256c,        /*  206 D plus                   */
   0x2567,        /*  207 SD bottom tee            */

   0x2568,        /*  208 DS bottom tee            */
   0x2564,        /*  209 SD top tee               */
   0x2565,        /*  210 DS top tee               */
   0x2559,        /*  211 DS lower left corner     */
   0x2558,        /*  212 SD lower left corner     */
   0x2552,        /*  213 SD upper left corner     */
   0x2553,        /*  214 DS upper left corner     */
   0x256b,        /*  215 DS plus                  */
   0x256a,        /*  216 SD plus                  */
   0x2518,        /*  217 lower right corner       */
   0x250c,        /*  218 upper left corner        */
   0x2588,        /*  219 full block               */
   0x2584,        /*  220 lower half block         */
   0x258c,        /*  221 left half block          */
   0x2590,        /*  222 right half block         */
   0x2580,        /*  223 top half block           */

   0x03b1,        /*  224 alpha                    */
   0x00df,        /*  225 beta                     */
   0x0393,        /*  226 gamma                    */
   0x03c0,        /*  227 pi                       */
   0x03a3,        /*  228 Sigma                    */
   0x03c3,        /*  229 sigma                    */
   0x00b5,        /*  230 mu                       */
   0x03c4,        /*  231 tau                      */
   0x03a6,        /*  232 Phi                      */
   0x0398,        /*  233 theta                    */
   0x03a9,        /*  234 Omega                    */
   0x03b4,        /*  235 delta                    */
   0x221e,        /*  236 infinity                 */
   0x03c6,        /*  237 phi                      */
   0x03b5,        /*  238 epsilon                  */
   0x2229,        /*  239 intersection             */

   0x2261,        /*  240 triple bar               */
   0x00b1,        /*  241 plus/minus               */
   0x2265,        /*  242 greater than/equal to    */
   0x2264,        /*  243 less than/equal to       */
   0x2320,        /*  244 upper half integral sign */
   0x2321,        /*  245 lower half integral sign */
   0x00f7,        /*  246 division sign            */
   0x2248,        /*  247 wavy (approx) equals sign*/
   0x00b0,        /*  248 degree sign              */
   0x2219,        /*  249 large bullet             */
   0x00b7,        /*  250 small bullet             */
   0x221a,        /*  251 square root              */
   0x207f,        /*  252 superscript n            */
   0x00b2,        /*  253 superscript 2            */
   0x25a0,        /*  254 centered square          */
   0x00a0 };      /*  255 non-breaking space       */
