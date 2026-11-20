#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/timer.h>
#include <gint/clock.h>
#include <stdio.h>

#define RGB(r,g,b) ((((r)>>3)<<11)|(((g)>>2)<<5)|((b)>>3))
#define BG     RGB(16,20,32)
#define PANEL  RGB(30,37,56)
#define GHOST  RGB(36,44,64)
#define AMBER  RGB(255,170,20)
#define CYAN   RGB(60,200,230)
#define GRAY   RGB(140,150,170)
#define GREEN  RGB(60,200,110)
#define RED    RGB(230,60,70)
#define WHITE  0xFFFF

static volatile int running = 0, trun = 0, done = 0;
static volatile long ms = 0, tleft = 0, ttot = 0, tk = 0;

static int tick(void)
{
    tk++;
    if(running) ms += 10;
    if(trun) {
        tleft -= 10;
        if(tleft <= 0) { tleft = 0; trun = 0; done = 1; }
    }
    return TIMER_CONTINUE;
}

static void fmt(long v, char *s)
{
    if(v > 5999999) v = 5999999;
    sprintf(s, "%02ld:%02ld.%02ld", v / 60000, (v / 1000) % 60, (v % 1000) / 10);
}

static const unsigned char SEG[10] =
    {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

static void dig(int x, int y, int w, int h, int t, int d, int on)
{
    int m = y + (h - t) / 2;
    int s = SEG[d];
    int c[7];
    for(int i = 0; i < 7; i++) c[i] = ((s >> i) & 1) ? on : GHOST;
    drect(x + t, y,         x + w - t - 1, y + t - 1,     c[0]);
    drect(x + w - t, y + t, x + w - 1,     m - 1,         c[1]);
    drect(x + w - t, m + t, x + w - 1,     y + h - t - 1, c[2]);
    drect(x + t, y + h - t, x + w - t - 1, y + h - 1,     c[3]);
    drect(x, m + t,         x + t - 1,     y + h - t - 1, c[4]);
    drect(x, y + t,         x + t - 1,     m - 1,         c[5]);
    drect(x + t, m,         x + w - t - 1, m + t - 1,     c[6]);
}

static void draw_time(long v, int y, int col)
{
    if(v > 5999999) v = 5999999;
    int mm = v / 60000, ss = (v / 1000) % 60, cc = (v % 1000) / 10;
    int ox = 71;
    dig(ox,       y, 36, 70, 8, mm / 10, col);
    dig(ox + 44,  y, 36, 70, 8, mm % 10, col);
    drect(ox + 84, y + 19, ox + 91, y + 26, col);
    drect(ox + 84, y + 43, ox + 91, y + 50, col);
    dig(ox + 98,  y, 36, 70, 8, ss / 10, col);
    dig(ox + 142, y, 36, 70, 8, ss % 10, col);
    drect(ox + 184, y + 62, ox + 191, y + 69, col);
    dig(ox + 198, y + 26, 24, 44, 5, cc / 10, col);
    dig(ox + 228, y + 26, 24, 44, 5, cc % 10, col);
}

static void tabs(int mode)
{
    drect(6, 4, 196, 28, mode == 0 ? AMBER : PANEL);
    drect(199, 4, 389, 28, mode == 1 ? AMBER : PANEL);
    dprint_opt(101, 16, mode == 0 ? BG : GRAY, C_NONE, DTEXT_CENTER, DTEXT_MIDDLE, "CRONOMETRO");
    dprint_opt(294, 16, mode == 1 ? BG : GRAY, C_NONE, DTEXT_CENTER, DTEXT_MIDDLE, "TIMER");
}

static void softkeys(const char *l[6])
{
    for(int i = 0; i < 6; i++) {
        if(!l[i][0]) continue;
        int x = 6 + i * 64;
        drect(x, 202, x + 62, 219, PANEL);
        dprint_opt(x + 31, 211, WHITE, C_NONE, DTEXT_CENTER, DTEXT_MIDDLE, "%s", l[i]);
    }
}

int main(void)
{
    int t = timer_configure(TIMER_ANY, 10000, GINT_CALL(tick));
    timer_start(t);

    int mode = 0, nl = 0;
    long laps[6] = {0};
    char s[32], s2[32];

    clearevents();
    while(1)
    {
        key_event_t e;
        while((e = pollevent()).type != KEYEV_NONE)
        {
            if(e.type != KEYEV_DOWN) continue;
            int k = e.key;
            if(k == KEY_EXIT) { timer_stop(t); return 1; }
            if(k == KEY_F6) mode ^= 1;

            if(mode == 0) {
                if(k == KEY_F1 || k == KEY_EXE) running ^= 1;
                if(k == KEY_F2 && ms > 0) {
                    for(int i = 5; i > 0; i--) laps[i] = laps[i - 1];
                    laps[0] = ms;
                    nl++;
                }
                if(k == KEY_F3 && !running) { ms = 0; nl = 0; }
            } else {
                if(k == KEY_F1 && !trun) {
                    tleft += 60000; if(tleft > 5999000) tleft = 5999000;
                    ttot = tleft; done = 0;
                }
                if(k == KEY_F2 && !trun) {
                    tleft += 10000; if(tleft > 5999000) tleft = 5999000;
                    ttot = tleft; done = 0;
                }
                if(k == KEY_F3) { trun = 0; tleft = 0; ttot = 0; done = 0; }
                if(k == KEY_F4 || k == KEY_EXE) {
                    if(done) done = 0;
                    else if(tleft > 0) {
                        if(!trun && ttot < tleft) ttot = tleft;
                        trun ^= 1;
                    }
                }
            }
        }

        int flash = done && ((tk / 50) & 1);
        dclear(flash ? RGB(110, 20, 30) : BG);
        tabs(mode);

        if(mode == 0) {
            draw_time(ms, 38, running ? AMBER : WHITE);
            dprint_opt(197, 118, running ? GREEN : GRAY, C_NONE, DTEXT_CENTER, DTEXT_MIDDLE,
                       running ? "IN CORSO" : (ms ? "FERMO" : "PRONTO"));
            drect(10, 128, 385, 198, PANEL);
            if(nl == 0) {
                dprint_opt(197, 163, GRAY, C_NONE, DTEXT_CENTER, DTEXT_MIDDLE,
                           "premi F2 per segnare un parziale");
            }
            int stored = nl < 6 ? nl : 6;
            int show = nl < 5 ? nl : 5;
            for(int i = 0; i < show; i++) {
                long dl = (i + 1 < stored) ? laps[i] - laps[i + 1] : laps[i];
                int y = 136 + i * 13;
                fmt(laps[i], s);
                fmt(dl, s2);
                dprint_opt(20, y, CYAN, C_NONE, DTEXT_LEFT, DTEXT_TOP, "P%d", nl - i);
                dprint_opt(197, y, WHITE, C_NONE, DTEXT_CENTER, DTEXT_TOP, "%s", s);
                dprint_opt(375, y, GRAY, C_NONE, DTEXT_RIGHT, DTEXT_TOP, "+%s", s2);
            }
            const char *l[6] = {running ? "STOP" : "AVVIA", "PARZ.", "AZZERA", "", "", "TIMER"};
            softkeys(l);
        } else {
            int col = (tleft > 0 && tleft < 10000) ? RED : CYAN;
            draw_time(tleft, 38, trun ? col : WHITE);
            drect(14, 124, 375, 138, GHOST);
            if(ttot > 0) {
                int w = (int)(((tleft / 100) * 361) / (ttot / 100));
                if(w > 361) w = 361;
                if(w > 0) drect(14, 124, 14 + w - 1, 138, col);
            }
            if(done)
                dprint_opt(197, 165, WHITE, C_NONE, DTEXT_CENTER, DTEXT_MIDDLE, "TEMPO SCADUTO!  premi EXE");
            else if(tleft == 0)
                dprint_opt(197, 165, GRAY, C_NONE, DTEXT_CENTER, DTEXT_MIDDLE, "imposta il tempo con F1 e F2");
            else
                dprint_opt(197, 165, trun ? GREEN : GRAY, C_NONE, DTEXT_CENTER, DTEXT_MIDDLE,
                           trun ? "IN CORSO" : "IN PAUSA");
            const char *l[6] = {"+1 MIN", "+10 SEC", "AZZERA", trun ? "STOP" : "AVVIA", "", "CRONO"};
            softkeys(l);
        }
        dupdate();
        sleep();
    }
}
