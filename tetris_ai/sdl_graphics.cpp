#define _CRT_SECURE_NO_WARNINGS
#define SDL_MAIN_HANDLED
#include "graphics.h"

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <vector>
#include <map>
#include <cmath>

struct ImgState {
    color_t color     = EGERGB(255, 255, 255);
    color_t fillcolor = EGERGB(0, 0, 0);
    color_t bkcolor   = EGERGB(0, 0, 0);
    int     bkmode    = TRANSPARENT;
    int     linestyle = SOLID_LINE;
};

struct IMAGE {
    int w;
    int h;
    std::vector<unsigned int> px;
    bool dirty;
    SDL_Texture* tex;
    ImgState st;
    IMAGE() : w(0), h(0), dirty(false), tex(NULL) {}
};

struct Graph {
    SDL_Window*   window;
    SDL_Renderer* renderer;
    int W, H;
    bool quit;
    bool inited;
    ImgState st;
    TTF_Font* font;
    int fontHeight;
    std::vector<key_msg> keys;
    unsigned long long frameStart;
    unsigned long long frameCount;
    double fps;
    Graph() : window(NULL), renderer(NULL), W(800), H(500), quit(false), inited(false),
        font(NULL), fontHeight(18), frameStart(0), frameCount(0), fps(60) {}
};

static Graph g;
static IMAGE g_windowImg;

static PIMAGE targetOf(PIMAGE pimg) {
    return pimg ? pimg : &g_windowImg;
}

static void imgPut(IMAGE* img, int x, int y, color_t c) {
    if (x < 0 || y < 0 || x >= img->w || y >= img->h) return;
    img->px[(size_t)y * img->w + x] = 0xFF000000u | (c & 0x00FFFFFFu);
    img->dirty = true;
}

static void imgLine(IMAGE* img, int x1, int y1, int x2, int y2, color_t c, bool dashed) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, i = 0;
    for (;;) {
        if (!dashed || (i >> 1) % 2 == 0) imgPut(img, x1, y1, c);
        ++i;
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

static void imgFill(IMAGE* img, int x, int y, int w, int h, color_t c) {
    for (int j = y; j < y + h; ++j)
        for (int i = x; i < x + w; ++i)
            imgPut(img, i, j, c);
}

static void ensureTexture(IMAGE* img) {
    if (!img->tex && img->w > 0 && img->h > 0 && g.renderer) {
        img->tex = SDL_CreateTexture(g.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, img->w, img->h);
        img->dirty = true;
    }
    if (img->tex && img->dirty) {
        SDL_UpdateTexture(img->tex, NULL, img->px.data(), img->w * 4);
        img->dirty = false;
    }
}

static void winLine(int x1, int y1, int x2, int y2, color_t c, int linestyle) {
    SDL_SetRenderDrawColor(g.renderer, EGEGET_R(c), EGEGET_G(c), EGEGET_B(c), 255);
    if (linestyle != DOTTED_LINE) {
        SDL_RenderDrawLine(g.renderer, x1, y1, x2, y2);
        return;
    }
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int i = 0;
    for (;;) {
        if ((i >> 1) % 2 == 0) SDL_RenderDrawPoint(g.renderer, x1, y1);
        ++i;
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

static std::string exeDir() {
    char* bp = SDL_GetBasePath();
    std::string p = bp ? bp : "";
    if (bp) SDL_free(bp);
    return p;
}

static TTF_Font* loadFontAtHeight(int cellHeight) {
    static std::map<int, TTF_Font*> cache;
    std::map<int, TTF_Font*>::iterator it = cache.find(cellHeight);
    if (it != cache.end()) return it->second;
    std::string base = exeDir();
    const char* rels[] = { "data/font_mono.ttf", "../data/font_mono.ttf", "tetris_ai/data/font_mono.ttf",
        "../tetris_ai/data/font_mono.ttf", "../../tetris_ai/data/font_mono.ttf", "font_mono.ttf" };
    const char* path = NULL;
    static std::string found;
    for (int i = 0; i < 6; ++i) {
        std::string cand = base + rels[i];
        FILE* probe = fopen(cand.c_str(), "rb");
        if (probe) { fclose(probe); found = cand; path = found.c_str(); break; }
    }
    if (!path) { FILE* probe = fopen("data/font_mono.ttf", "rb"); if (probe) { fclose(probe); found = "data/font_mono.ttf"; path = found.c_str(); } }
    TTF_Font* f = NULL;
    if (path) {
        for (int size = cellHeight; size >= 4; --size) {
            f = TTF_OpenFont(path, size);
            if (!f) continue;
            if (TTF_FontHeight(f) <= cellHeight) break;
            TTF_CloseFont(f);
            f = NULL;
        }
        if (!f) f = TTF_OpenFont(path, cellHeight);
    }
    cache[cellHeight] = f;
    return f;
}

static TTF_Font* activeFont() {
    if (!g.font) g.font = loadFontAtHeight(g.fontHeight);
    return g.font;
}

static void drawTextToTarget(IMAGE* img, int x, int y, const std::string& text, const ImgState& st) {
    TTF_Font* font = activeFont();
    if (!font) return;
    if (st.bkmode == OPAQUE) {
        int w = 0, h = 0;
        TTF_SizeUTF8(font, text.c_str(), &w, &h);
        if (img == &g_windowImg) {
            SDL_SetRenderDrawColor(g.renderer, EGEGET_R(st.bkcolor), EGEGET_G(st.bkcolor), EGEGET_B(st.bkcolor), 255);
            SDL_Rect r = { x, y, w, h };
            SDL_RenderFillRect(g.renderer, &r);
        } else {
            imgFill(img, x, y, w, h, st.bkcolor);
        }
    }
    SDL_Color col = { (Uint8)EGEGET_R(st.color), (Uint8)EGEGET_G(st.color), (Uint8)EGEGET_B(st.color), 255 };
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), col);
    if (!surf) return;
    if (img == &g_windowImg) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(g.renderer, surf);
        if (tex) {
            SDL_Rect dst = { x, y, surf->w, surf->h };
            SDL_RenderCopy(g.renderer, tex, NULL, &dst);
            SDL_DestroyTexture(tex);
        }
    } else {
        const unsigned int* srcp = (const unsigned int*)surf->pixels;
        int pitch = surf->pitch / 4;
        for (int j = 0; j < surf->h; ++j) {
            for (int i = 0; i < surf->w; ++i) {
                unsigned int a = srcp[(size_t)j * pitch + i] >> 24;
                if (a) {
                    int px = x + i, py = y + j;
                    if (px >= 0 && py >= 0 && px < img->w && py < img->h) {
                        unsigned int* d = &img->px[(size_t)py * img->w + px];
                        unsigned int bg = *d;
                        unsigned int r = (a * EGEGET_R(st.color) + (255 - a) * EGEGET_R(bg)) / 255;
                        unsigned int gg = (a * EGEGET_G(st.color) + (255 - a) * EGEGET_G(bg)) / 255;
                        unsigned int b = (a * EGEGET_B(st.color) + (255 - a) * EGEGET_B(bg)) / 255;
                        *d = 0xFF000000u | (r << 16) | (gg << 8) | b;
                    }
                }
            }
        }
        img->dirty = true;
    }
    SDL_FreeSurface(surf);
}

static void pumpEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            g.quit = true;
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            int vk = 0;
            SDL_Keycode sym = e.key.keysym.sym;
            if (sym >= SDLK_a && sym <= SDLK_z) vk = (int)sym - 'a' + 'A';
            else if (sym >= SDLK_0 && sym <= SDLK_9) vk = (int)sym;
            else if (sym >= SDLK_F1 && sym <= SDLK_F12) vk = 0x70 + (int)(sym - SDLK_F1);
            else if (sym >= SDLK_KP_1 && sym <= SDLK_KP_9) vk = 0x60 + (int)(sym - SDLK_KP_1) + 1;
            else if (sym == SDLK_KP_0) vk = 0x60;
            else if (sym == SDLK_KP_PERIOD) vk = 0x6E;
            else if (sym == SDLK_KP_DIVIDE) vk = 0x6F;
            else if (sym == SDLK_KP_MULTIPLY) vk = 0x6A;
            else if (sym == SDLK_KP_MINUS) vk = 0x6D;
            else if (sym == SDLK_KP_PLUS) vk = 0x6B;
            else if (sym == SDLK_KP_ENTER) vk = 0x0D;
            else if (sym == SDLK_KP_EQUALS) vk = 0xBB;
            else switch (sym) {
                case SDLK_RETURN: case SDLK_KP_ENTER: vk = 0x0D; break;
                case SDLK_BACKSPACE: vk = 0x08; break;
                case SDLK_TAB: vk = 0x09; break;
                case SDLK_SPACE: vk = 0x20; break;
                case SDLK_ESCAPE: vk = 0x1B; break;
                case SDLK_LEFT: vk = 0x25; break;
                case SDLK_UP: vk = 0x26; break;
                case SDLK_RIGHT: vk = 0x27; break;
                case SDLK_DOWN: vk = 0x28; break;
                case SDLK_INSERT: vk = 0x2D; break;
                case SDLK_DELETE: vk = 0x2E; break;
                case SDLK_HOME: vk = 0x24; break;
                case SDLK_END: vk = 0x23; break;
                case SDLK_PAGEUP: vk = 0x21; break;
                case SDLK_PAGEDOWN: vk = 0x22; break;
                case SDLK_PAUSE: vk = 0x13; break;
                case SDLK_PRINTSCREEN: vk = 0x2C; break;
                case SDLK_CAPSLOCK: vk = 0x14; break;
                case SDLK_NUMLOCKCLEAR: vk = 0x90; break;
                case SDLK_SCROLLLOCK: vk = 0x91; break;
                case SDLK_LSHIFT: vk = 0xA0; break;
                case SDLK_RSHIFT: vk = 0xA1; break;
                case SDLK_LCTRL: vk = 0xA2; break;
                case SDLK_RCTRL: vk = 0xA3; break;
                case SDLK_LALT: vk = 0xA4; break;
                case SDLK_RALT: vk = 0xA5; break;
                case SDLK_LGUI: vk = 0x5B; break;
                case SDLK_RGUI: vk = 0x5C; break;
                case SDLK_QUOTE: vk = 0xDE; break;
                case SDLK_COMMA: vk = 0xBC; break;
                case SDLK_MINUS: vk = 0xBD; break;
                case SDLK_PERIOD: vk = 0xBE; break;
                case SDLK_SLASH: vk = 0xBF; break;
                case SDLK_SEMICOLON: vk = 0xBA; break;
                case SDLK_EQUALS: vk = 0xBB; break;
                case SDLK_LEFTBRACKET: vk = 0xDB; break;
                case SDLK_BACKSLASH: vk = 0xDC; break;
                case SDLK_RIGHTBRACKET: vk = 0xDD; break;
                case SDLK_BACKQUOTE: vk = 0xC0; break;
                default: vk = 0; break;
            }
            if (vk) {
                key_msg k;
                k.key = vk;
                k.msg = (e.type == SDL_KEYDOWN) ? key_msg_down : key_msg_up;
                k.flags = 0;
                if (e.key.keysym.mod & KMOD_SHIFT) k.flags |= key_flag_shift;
                if (e.key.keysym.mod & KMOD_CTRL) k.flags |= key_flag_ctrl;
                g.keys.push_back(k);
            }
            break;
        }
        default:
            break;
        }
    }
}

static void frameTick() {
    unsigned long long now = SDL_GetTicks64();
    ++g.frameCount;
    if (now - g.frameStart >= 500) {
        g.fps = g.frameCount * 1000.0 / (double)(now - g.frameStart);
        g.frameStart = now;
        g.frameCount = 0;
    }
}

static int fmtText(char* buf, size_t cap, const char* fmt, va_list ap) {
#if defined(_MSC_VER)
    return _vsnprintf_s(buf, cap, _TRUNCATE, fmt, ap);
#else
    return vsnprintf(buf, cap, fmt, ap);
#endif
}

void initgraph(int Width, int Height, int Flag) {
    (void)Flag;
    if (g.inited) return;
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    g.W = Width;
    g.H = Height;
    g.window = SDL_CreateWindow("EGE", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                Width, Height, SDL_WINDOW_SHOWN);
    g.renderer = SDL_CreateRenderer(g.window, -1, SDL_RENDERER_SOFTWARE);
    g_windowImg.w = Width;
    g_windowImg.h = Height;
    g_windowImg.px.assign((size_t)Width * Height, 0xFF000000u);
    g.inited = true;
    g.frameStart = SDL_GetTicks64();
    cleardevice(NULL);
}

void closegraph() {
    if (!g.inited) return;
    if (g.renderer) SDL_DestroyRenderer(g.renderer);
    if (g.window) SDL_DestroyWindow(g.window);
    TTF_Quit();
    SDL_Quit();
    g.renderer = NULL;
    g.window = NULL;
    g.inited = false;
}

bool is_run() {
    return !g.quit;
}

void setcaption(const char* caption) {
    if (g.window) SDL_SetWindowTitle(g.window, caption);
}

void setinitmode(int mode, int x, int y) {
    (void)mode; (void)x; (void)y;
}

int getwidth()  { return g.W; }
int getheight() { return g.H; }

float getfps() {
    return (float)g.fps;
}

void cleardevice(PIMAGE pimg) {
    IMAGE* t = targetOf(pimg);
    if (t == &g_windowImg) {
        SDL_SetRenderDrawColor(g.renderer, EGEGET_R(g.st.bkcolor), EGEGET_G(g.st.bkcolor), EGEGET_B(g.st.bkcolor), 255);
        SDL_RenderClear(g.renderer);
    } else {
        imgFill(t, 0, 0, t->w, t->h, t->st.bkcolor);
    }
}

void setcolor(color_t color, PIMAGE pimg) {
    if (pimg) pimg->st.color = color; else g.st.color = color;
}

void setfillcolor(color_t color, PIMAGE pimg) {
    if (pimg) pimg->st.fillcolor = color; else g.st.fillcolor = color;
}

void setbkcolor(color_t color, PIMAGE pimg) {
    if (pimg) { pimg->st.bkcolor = color; imgFill(pimg, 0, 0, pimg->w, pimg->h, color); }
    else {
        g.st.bkcolor = color;
        SDL_SetRenderDrawColor(g.renderer, EGEGET_R(color), EGEGET_G(color), EGEGET_B(color), 255);
        SDL_RenderClear(g.renderer);
    }
}

void setbkmode(int iBkMode, PIMAGE pimg) {
    if (pimg) pimg->st.bkmode = iBkMode; else g.st.bkmode = iBkMode;
}

void line(int x1, int y1, int x2, int y2, PIMAGE pimg) {
    if (pimg) {
        imgLine(pimg, x1, y1, x2, y2, pimg->st.color, pimg->st.linestyle == DOTTED_LINE);
    } else {
        winLine(x1, y1, x2, y2, g.st.color, g.st.linestyle);
    }
}

void line_f(float x1, float y1, float x2, float y2, PIMAGE pimg) {
    line((int)(x1 >= 0 ? x1 + 0.5f : x1 - 0.5f), (int)(y1 >= 0 ? y1 + 0.5f : y1 - 0.5f),
         (int)(x2 >= 0 ? x2 + 0.5f : x2 - 0.5f), (int)(y2 >= 0 ? y2 + 0.5f : y2 - 0.5f), pimg);
}

void rectangle(int left, int top, int right, int bottom, PIMAGE pimg) {
    line(left, top, right, top, pimg);
    line(right, top, right, bottom, pimg);
    line(right, bottom, left, bottom, pimg);
    line(left, bottom, left, top, pimg);
}

void bar(int left, int top, int right, int bottom, PIMAGE pimg) {
    if (pimg) {
        imgFill(pimg, left, top, right - left, bottom - top, pimg->st.fillcolor);
    } else {
        SDL_SetRenderDrawColor(g.renderer, EGEGET_R(g.st.fillcolor), EGEGET_G(g.st.fillcolor), EGEGET_B(g.st.fillcolor), 255);
        SDL_Rect r = { left, top, right - left, bottom - top };
        SDL_RenderFillRect(g.renderer, &r);
    }
}

void setlinestyle(int linestyle, unsigned short upattern, int thickness, PIMAGE pimg) {
    (void)upattern; (void)thickness;
    if (pimg) pimg->st.linestyle = linestyle; else g.st.linestyle = linestyle;
}

void setlinewidth(int thickness, PIMAGE pimg) {
    (void)thickness; (void)pimg;
}

void outtextxy(int x, int y, const char* textstring, PIMAGE pimg) {
    IMAGE* t = targetOf(pimg);
    const ImgState& s = pimg ? pimg->st : g.st;
    drawTextToTarget(t, x, y, textstring, s);
}

void xyprintf(int x, int y, const char* fmt, ...) {
    char buf[2048];
    va_list ap;
    va_start(ap, fmt);
    fmtText(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    outtextxy(x, y, buf, NULL);
}

void rectprintf(int x, int y, int w, int h, const char* fmt, ...) {
    char buf[4096];
    va_list ap;
    va_start(ap, fmt);
    fmtText(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    (void)w; (void)h;
    std::string s = buf;
    size_t pos = 0;
    int lineH = textheight("I", NULL);
    if (lineH <= 0) lineH = g.fontHeight;
    while (pos <= s.size()) {
        size_t nl = s.find('\n', pos);
        std::string line = (nl == std::string::npos) ? s.substr(pos) : s.substr(pos, nl - pos);
        outtextxy(x, y, line.c_str(), NULL);
        if (nl == std::string::npos) break;
        y += lineH;
        pos = nl + 1;
    }
}

int textwidth(const char* textstring, PIMAGE pimg) {
    (void)pimg;
    TTF_Font* font = activeFont();
    int w = 0, h = 0;
    if (font) TTF_SizeUTF8(font, textstring, &w, &h);
    return w;
}

int textheight(const char* textstring, PIMAGE pimg) {
    (void)pimg; (void)textstring;
    TTF_Font* font = activeFont();
    return font ? TTF_FontHeight(font) : g.fontHeight;
}

void setfont(int nHeight, int nWidth, const char* lpszFace, PIMAGE pimg) {
    (void)nWidth; (void)lpszFace; (void)pimg;
    if (nHeight > 0 && nHeight != g.fontHeight) {
        g.fontHeight = nHeight;
        g.font = loadFontAtHeight(nHeight);
    }
}

void setfont(int nHeight, int nWidth, const char* lpszFace, int nEscapement, int nOrientation,
             int nWeight, int bItalic, int bUnderline, int bStrikeOut, PIMAGE pimg) {
    (void)nEscapement; (void)nOrientation; (void)nWeight; (void)bItalic; (void)bUnderline; (void)bStrikeOut;
    setfont(nHeight, nWidth, lpszFace, pimg);
}

PIMAGE newimage() {
    return newimage(1, 1);
}

PIMAGE newimage(int width, int height) {
    IMAGE* img = new IMAGE();
    img->w = width;
    img->h = height;
    img->px.assign((size_t)width * height, 0xFF000000u);
    return img;
}

void delimage(PIMAGE pImg) {
    if (!pImg) return;
    if (pImg->tex) SDL_DestroyTexture(pImg->tex);
    delete pImg;
}

void resize(PIMAGE pDstImg, int width, int height) {
    if (!pDstImg) return;
    pDstImg->w = width;
    pDstImg->h = height;
    pDstImg->px.assign((size_t)width * height, 0xFF000000u);
    if (pDstImg->tex) { SDL_DestroyTexture(pDstImg->tex); pDstImg->tex = NULL; }
    pDstImg->dirty = true;
}

void putimage(int dstX, int dstY, const PIMAGE pSrcImg, unsigned int dwRop) {
    (void)dwRop;
    if (!pSrcImg) return;
    ensureTexture(pSrcImg);
    if (pSrcImg->tex) {
        SDL_Rect dst = { dstX, dstY, pSrcImg->w, pSrcImg->h };
        SDL_RenderCopy(g.renderer, pSrcImg->tex, NULL, &dst);
    }
}

void putimage(int dstX, int dstY, int dstWidth, int dstHeight, const PIMAGE pSrcImg, int srcX, int srcY, unsigned int dwRop) {
    (void)dwRop;
    if (!pSrcImg) return;
    ensureTexture(pSrcImg);
    if (pSrcImg->tex) {
        SDL_Rect src = { srcX, srcY, dstWidth, dstHeight };
        SDL_Rect dst = { dstX, dstY, dstWidth, dstHeight };
        SDL_RenderCopy(g.renderer, pSrcImg->tex, &src, &dst);
    }
}

color_t hsv2rgb(float H, float S, float V) {
    float h = H / 60.0f;
    int i = (int)floorf(h);
    float f = h - (float)i;
    float p = V * (1.0f - S);
    float q = V * (1.0f - S * f);
    float t = V * (1.0f - S * (1.0f - f));
    float r = 0, gg = 0, b = 0;
    switch (((i % 6) + 6) % 6) {
        case 0: r = V; gg = t; b = p; break;
        case 1: r = q; gg = V; b = p; break;
        case 2: r = p; gg = V; b = t; break;
        case 3: r = p; gg = q; b = V; break;
        case 4: r = t; gg = p; b = V; break;
        case 5: r = V; gg = p; b = q; break;
    }
    return EGERGB((int)(r * 255.0f + 0.5f), (int)(gg * 255.0f + 0.5f), (int)(b * 255.0f + 0.5f));
}

void rgb2hsv(color_t rgb, float *H, float *S, float *V) {
    (void)rgb;
    if (H) *H = 0;
    if (S) *S = 0;
    if (V) *V = 0;
}

int kbmsg() {
    pumpEvents();
    return g.keys.empty() ? 0 : 1;
}

key_msg getkey() {
    if (g.keys.empty()) pumpEvents();
    key_msg k;
    if (g.keys.empty()) {
        k.key = 0;
        k.msg = key_msg_up;
        k.flags = 0;
        return k;
    }
    k = g.keys.front();
    g.keys.erase(g.keys.begin());
    return k;
}

void delay_ms(long ms) {
    SDL_RenderPresent(g.renderer);
    pumpEvents();
    if (ms > 0) SDL_Delay((unsigned int)ms);
    frameTick();
}

void delay_fps(int fps) {
    SDL_RenderPresent(g.renderer);
    pumpEvents();
    static unsigned long long lastPresent = 0;
    if (fps <= 0) fps = 60;
    unsigned long long interval = 1000ULL / (unsigned long long)fps;
    unsigned long long now = SDL_GetTicks64();
    if (lastPresent != 0 && now < lastPresent + interval) {
        SDL_Delay((unsigned int)(lastPresent + interval - now));
        now = SDL_GetTicks64();
    }
    lastPresent = now;
    frameTick();
}
