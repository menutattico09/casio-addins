#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include <stdlib.h>

static int Y;

static void nuova(void) { dclear(C_WHITE); Y = 10; }

#define RIGA(...) do { \
    if(Y > 200) { dupdate(); getkey(); nuova(); } \
    dprint(10, Y, C_BLACK, __VA_ARGS__); Y += 16; \
} while(0)

static void fine(void) { dupdate(); getkey(); }

static long leggi(const char *p, int y)
{
    static const int dk[10] = {KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
                               KEY_5, KEY_6, KEY_7, KEY_8, KEY_9};
    char b[14] = "";
    int n = 0;
    for(;;) {
        drect(0, y, 395, y + 14, C_WHITE);
        dprint(10, y, C_BLACK, "%s %s_", p, b);
        dupdate();
        int k = getkey();
        if(k == KEY_EXE && n > 0) break;
        if(k == KEY_DEL && n > 0) b[--n] = 0;
        if(k == KEY_NEG && n == 0) { b[n++] = '-'; b[n] = 0; }
        for(int i = 0; i < 10; i++)
            if(k == dk[i] && n < 11) { b[n++] = '0' + i; b[n] = 0; }
    }
    return atol(b);
}

static long mcd(long a, long b)
{
    a = labs(a); b = labs(b);
    while(b) { long t = a % b; a = b; b = t; }
    return a;
}

static void app_mcd(void)
{
    nuova();
    long a = leggi("A =", 10);
    long b = leggi("B =", 26);
    long g = mcd(a, b);
    Y = 58;
    if(g == 0) { RIGA("inserisci numeri non nulli"); fine(); return; }
    RIGA("MCD = %ld", g);
    RIGA("mcm = %ld", labs(a / g * b));
    fine();
}

static void app_frazioni(void)
{
    nuova();
    long n = leggi("Numeratore =", 10);
    long d = leggi("Denominatore =", 26);
    Y = 58;
    if(d == 0) { RIGA("denominatore nullo"); fine(); return; }
    long g = mcd(n, d);
    if(g == 0) g = 1;
    n /= g; d /= g;
    if(d < 0) { n = -n; d = -d; }
    RIGA("Semplificata:");
    RIGA("%ld / %ld", n, d);
    fine();
}

static void app_hex(void)
{
    nuova();
    long n = leggi("Decimale =", 10);
    Y = 42;
    if(n < 0) RIGA("HEX = -%lX", -n);
    else RIGA("HEX = %lX", n);
    fine();
}

static void app_ruffini(void)
{
    long c[12];
    nuova();
    long g = leggi("Grado (1-10) =", 10);
    if(g < 1 || g > 10) { Y = 42; RIGA("grado non valido"); fine(); return; }
    int n = (int)g;
    for(int i = 0; i <= n; i++) {
        char p[24];
        sprintf(p, "coeff x^%d =", n - i);
        c[i] = leggi(p, 26 + 16 * i);
    }
    nuova();
    while(n > 0 && c[n] == 0) { RIGA("radice x = 0"); n--; }
    for(long d = 1; n > 0 && d <= labs(c[n]); d++) {
        for(int k = 0; k < 2; k++) {
            long r = k ? -d : d;
            while(n > 0) {
                long b = c[0];
                for(int i = 1; i <= n; i++) b = b * r + c[i];
                if(b != 0) break;
                for(int i = 1; i < n; i++) c[i] += c[i - 1] * r;
                n--;
                RIGA("radice x = %ld", r);
            }
        }
    }
    RIGA("Resto, grado %d:", n);
    for(int i = 0; i <= n; i++) RIGA("%ld", c[i]);
    fine();
}

int main(void)
{
    while(1) {
        dclear(C_WHITE);
        dprint(10, 10, C_BLACK, "MATEAPP");
        dprint(10, 42, C_BLACK, "1  MCD e mcm");
        dprint(10, 58, C_BLACK, "2  Ruffini");
        dprint(10, 74, C_BLACK, "3  Frazioni");
        dprint(10, 90, C_BLACK, "4  Esadecimale");
        dprint(10, 205, C_BLACK, "EXIT: esci");
        dupdate();
        int k = getkey();
        if(k == KEY_EXIT) return 1;
        if(k == KEY_1) app_mcd();
        if(k == KEY_2) app_ruffini();
        if(k == KEY_3) app_frazioni();
        if(k == KEY_4) app_hex();
    }
}
