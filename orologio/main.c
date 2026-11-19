#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/timer.h>
#include <gint/clock.h>
#include <stdio.h>

static volatile int running = 0, trun = 0;
static volatile long ms = 0, tleft = 0;

static int tick(void)
{
    if(running) ms += 10;
    if(trun) {
        tleft -= 10;
        if(tleft <= 0) { tleft = 0; trun = 0; }
    }
    return TIMER_CONTINUE;
}

static void fmt(long v, char *s)
{
    sprintf(s, "%02ld:%02ld.%02ld", v / 60000, (v / 1000) % 60, (v % 1000) / 10);
}

int main(void)
{
    int t = timer_configure(TIMER_ANY, 10000, GINT_CALL(tick));
    timer_start(t);

    int mode = 0, nlap = 0;
    long laps[5] = {0};
    char s[32];

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
                if(k == KEY_EXE) running ^= 1;
                if(k == KEY_F1 && running) {
                    for(int i = 4; i > 0; i--) laps[i] = laps[i-1];
                    laps[0] = ms;
                    if(nlap < 5) nlap++;
                }
                if(k == KEY_F2 && !running) { ms = 0; nlap = 0; }
            } else {
                if(k == KEY_F1 && !trun) tleft += 60000;
                if(k == KEY_F2 && !trun) tleft += 10000;
                if(k == KEY_F3 && !trun) tleft = 0;
                if(k == KEY_EXE && tleft > 0) trun ^= 1;
            }
        }

        dclear(C_WHITE);
        if(mode == 0) {
            dprint(10, 10, C_BLACK, "CRONOMETRO");
            fmt(ms, s);
            dprint(10, 40, C_BLACK, "%s", s);
            for(int i = 0; i < nlap; i++) {
                fmt(laps[i], s);
                dprint(10, 80 + i * 16, C_BLACK, "P%d  %s", nlap - i, s);
            }
            dprint(10, 205, C_BLACK, "EXE:start/stop F1:parz F2:reset F6:timer");
        } else {
            dprint(10, 10, C_BLACK, "TIMER");
            fmt(tleft, s);
            dprint(10, 40, C_BLACK, "%s", s);
            if(tleft == 0 && !trun) dprint(10, 80, C_BLACK, "(imposta il tempo)");
            dprint(10, 205, C_BLACK, "F1:+1m F2:+10s F3:azzera EXE:via F6:crono");
        }
        dupdate();
        sleep();
    }
}
