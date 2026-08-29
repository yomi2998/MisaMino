/*********************************************************
* MisaMino portable graphics front end.
* BGI/EGE-compatible API implemented over SDL2 + SDL2_ttf.
* See docs/phase-b-graphics-audio.md.
*********************************************************/

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#ifndef __cplusplus
#error You must use C++ compiler, or you need filename with '.cpp' suffix
#endif

#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif

#include <SDL.h>
#include <SDL_ttf.h>

typedef unsigned int color_t;

#define EGERGB(r, g, b)         ( ((r)<<16) | ((g)<<8) | (b))
#define EGERGBA(r, g, b, a)     ( ((r)<<16) | ((g)<<8) | (b) | ((a)<<24) )
#define EGEGET_R(c)             ( ((c)>>16) & 0xFF )
#define EGEGET_G(c)             ( ((c)>> 8) & 0xFF )
#define EGEGET_B(c)             ( ((c)) & 0xFF )
#define EGEGET_A(c)             ( ((c)>>24) & 0xFF )

#ifndef TRANSPARENT
#define TRANSPARENT   1
#endif
#ifndef OPAQUE
#define OPAQUE        0
#endif

#define SOLID_LINE    0
#define CENTER_LINE   1
#define DASHED_LINE   2
#define DOTTED_LINE   3
#define NULL_LINE     4

#define INIT_NOBORDER       0x1
#define INIT_CHILD          0x2
#define INIT_TOPMOST        0x4
#define INIT_RENDERMANUAL   0x8
#define INIT_NOFORCEEXIT    0x10
#define INIT_WITHLOGO       0x100
#define INIT_DEFAULT        0x0
#define INIT_ANIMATION      (INIT_DEFAULT | INIT_RENDERMANUAL | INIT_NOFORCEEXIT)

typedef enum key_code_e {
    key_mouse_l     = 0x01,
    key_mouse_r     = 0x02,
    key_mouse_m     = 0x04,
    key_back        = 0x08,
    key_tab         = 0x09,
    key_enter       = 0x0d,
    key_shift       = 0x10,
    key_control     = 0x11,
    key_menu        = 0x12,
    key_pause       = 0x13,
    key_capslock    = 0x14,
    key_esc         = 0x1b,
    key_space       = 0x20,
    key_pageup      = 0x21,
    key_pagedown    = 0x22,
    key_home        = 0x23,
    key_end         = 0x24,
    key_left        = 0x25,
    key_up          = 0x26,
    key_right       = 0x27,
    key_down        = 0x28,
    key_print       = 0x2a,
    key_snapshot    = 0x2c,
    key_insert      = 0x2d,
    key_delete      = 0x2e,
    key_0           = 0x30,
    key_1           = 0x31,
    key_2           = 0x32,
    key_3           = 0x33,
    key_4           = 0x34,
    key_5           = 0x35,
    key_6           = 0x36,
    key_7           = 0x37,
    key_8           = 0x38,
    key_9           = 0x39,
    key_A           = 0x41,
    key_Z           = 0x5a,
    key_win_l       = 0x5b,
    key_win_r       = 0x5c,
    key_sleep       = 0x5f,
    key_num0        = 0x60,
    key_num1        = 0x61,
    key_num2        = 0x62,
    key_num3        = 0x63,
    key_num4        = 0x64,
    key_num5        = 0x65,
    key_num6        = 0x66,
    key_num7        = 0x67,
    key_num8        = 0x68,
    key_num9        = 0x69,
    key_f1          = 0x70,
    key_f2          = 0x71,
    key_f3          = 0x72,
    key_f4          = 0x73,
    key_f5          = 0x74,
    key_f6          = 0x75,
    key_f7          = 0x76,
    key_f8          = 0x77,
    key_f9          = 0x78,
    key_f10         = 0x79,
    key_f11         = 0x7a,
    key_f12         = 0x7b,
    key_numlock     = 0x90,
    key_scrolllock  = 0x91,
    key_shift_l     = 0xa0,
    key_shift_r     = 0xa1,
    key_control_l   = 0xa2,
    key_control_r   = 0xa3,
    key_menu_l      = 0xa4,
    key_menu_r      = 0xa5,
    key_semicolon   = 0xba,
    key_plus        = 0xbb,
    key_comma       = 0xbc,
    key_minus       = 0xbd,
    key_period      = 0xbe,
    key_slash       = 0xbf,
    key_tilde       = 0xc0,
    key_lbrace      = 0xdb,
    key_backslash   = 0xdc,
    key_rbrace      = 0xdd,
    key_quote       = 0xde,
    key_ime_process = 0xe5,
} key_code_e;

typedef enum key_msg_e {
    key_msg_null    = 0,
    key_msg_down    = 1,
    key_msg_up      = 2,
    key_msg_char    = 4,
} key_msg_e;

typedef enum key_flag_e {
    key_flag_shift  = 0x100,
    key_flag_ctrl   = 0x200,
} key_flag_e;

typedef struct key_msg {
    int             key;
    key_msg_e       msg;
    unsigned int    flags;
} key_msg;

struct IMAGE;
typedef IMAGE* PIMAGE;

void    initgraph(int Width, int Height, int Flag = 0);
void    closegraph();
bool    is_run();
void    setcaption(const char* caption);
void    setinitmode(int mode = 0, int x = 0, int y = 0);
int     getwidth();
int     getheight();
float   getfps();

void    cleardevice(PIMAGE pimg = NULL);
void    setcolor(color_t color, PIMAGE pimg = NULL);
void    setfillcolor(color_t color, PIMAGE pimg = NULL);
void    setbkcolor(color_t color, PIMAGE pimg = NULL);
void    setbkmode(int iBkMode, PIMAGE pimg = NULL);

void    line(int x1, int y1, int x2, int y2, PIMAGE pimg = NULL);
void    line_f(float x1, float y1, float x2, float y2, PIMAGE pimg = NULL);
void    rectangle(int left, int top, int right, int bottom, PIMAGE pimg = NULL);
void    bar(int left, int top, int right, int bottom, PIMAGE pimg = NULL);
void    setlinestyle(int linestyle, unsigned short upattern = 0, int thickness = 1, PIMAGE pimg = NULL);
void    setlinewidth(int thickness, PIMAGE pimg = NULL);

void    outtextxy(int x, int y, const char* textstring, PIMAGE pimg = NULL);
void    xyprintf(int x, int y, const char* fmt, ...);
void    rectprintf(int x, int y, int w, int h, const char* fmt, ...);
int     textwidth(const char* textstring, PIMAGE pimg = NULL);
int     textheight(const char* textstring, PIMAGE pimg = NULL);
void    setfont(int nHeight, int nWidth, const char* lpszFace, PIMAGE pimg = NULL);
void    setfont(int nHeight, int nWidth, const char* lpszFace, int nEscapement, int nOrientation,
                int nWeight, int bItalic, int bUnderline, int bStrikeOut, PIMAGE pimg = NULL);

PIMAGE  newimage();
PIMAGE  newimage(int width, int height);
void    delimage(PIMAGE pImg);
void    resize(PIMAGE pDstImg, int width, int height);
void    putimage(int dstX, int dstY, const PIMAGE pSrcImg, unsigned int dwRop = 0);
void    putimage(int dstX, int dstY, int dstWidth, int dstHeight, const PIMAGE pSrcImg, int srcX, int srcY, unsigned int dwRop = 0);

color_t hsv2rgb(float H, float S, float V);
void    rgb2hsv(color_t rgb, float *H, float *S, float *V);

int     kbmsg();
key_msg getkey();

void    delay_ms(long ms);
void    delay_fps(int fps);

#ifdef _MSC_VER
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif

#endif
