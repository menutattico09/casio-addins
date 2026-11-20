#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RGB(r,g,b) ((((r)>>3)<<11)|(((g)>>2)<<5)|((b)>>3))
#define BG     RGB(244,246,250)
#define INK    RGB(28,34,48)
#define GRAY   RGB(110,118,135)
#define LINEC  RGB(205,210,222)
#define BLUE   RGB(37,84,199)
#define PURPLE RGB(124,58,200)
#define GREEN  RGB(22,150,90)
#define ORANGE RGB(235,120,20)
#define RED    RGB(200,40,50)
#define WHITE  0xFFFF

#define MAXF 11

typedef struct { char l[24]; char v[72]; } Line;

static void titlebar(const char *t, int col)
{
    drect(6, 4, 389, 28, col);
    dprint_opt(197, 16, WHITE, C_NONE, DTEXT_CENTER, DTEXT_MIDDLE, "%s", t);
}

static void softbar(const char *t)
{
    drect(6, 202, 389, 219, LINEC);
    dprint_opt(197, 211, GRAY, C_NONE, DTEXT_CENTER, DTEXT_MIDDLE, "%s", t);
}

#define LNP(a, ...) do { strcpy(ln[nl].l, a); \
    snprintf(ln[nl].v, sizeof ln[nl].v, __VA_ARGS__); nl++; } while(0)
#define LNH(a) do { strcpy(ln[nl].l, a); ln[nl].v[0] = 0; nl++; } while(0)

static void wide(Line *ln, int *n, const char *s)
{
    int len = strlen(s);
    for(int p = 0; p < len || p == 0; p += 52) {
        ln[*n].l[0] = 0;
        strncpy(ln[*n].v, s + p, 52);
        ln[*n].v[52] = 0;
        (*n)++;
    }
}

static void results(const char *title, int col, Line *ln, int n)
{
    int top = 0;
    for(;;) {
        dclear(BG);
        titlebar(title, col);
        drect_border(10, 34, 385, 198, WHITE, 1, LINEC);
        for(int r = 0; r < 9 && top + r < n; r++) {
            Line *L = &ln[top + r];
            int y = 51 + r * 17;
            if(L->v[0] == 0)
                dprint_opt(20, y, col, C_NONE, DTEXT_LEFT, DTEXT_MIDDLE, "%s", L->l);
            else if(L->l[0] == 0)
                dprint_opt(20, y, INK, C_NONE, DTEXT_LEFT, DTEXT_MIDDLE, "%s", L->v);
            else {
                dprint_opt(20, y, GRAY, C_NONE, DTEXT_LEFT, DTEXT_MIDDLE, "%s", L->l);
                dprint_opt(375, y, INK, C_NONE, DTEXT_RIGHT, DTEXT_MIDDLE, "%s", L->v);
            }
        }
        softbar(n > 9 ? "frecce: scorri    EXE: indietro" : "EXE: indietro");
        dupdate();
        int k = getkey().key;
        if(k == KEY_EXE || k == KEY_EXIT) return;
        if(k == KEY_UP && top > 0) top--;
        if(k == KEY_DOWN && top + 9 < n) top++;
    }
}

static void errore(const char *msg)
{
    Line ln[2]; int nl = 0;
    LNH("Errore");
    wide(ln, &nl, msg);
    results("Attenzione", RED, ln, nl);
}

static int form(const char *title, int col, int n, char lab[][20], char val[][14])
{
    static const int dk[10] = {KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
                               KEY_5, KEY_6, KEY_7, KEY_8, KEY_9};
    int cur = 0;
    for(int i = 0; i < n; i++) val[i][0] = 0;
    for(;;) {
        dclear(BG);
        titlebar(title, col);
        int top = cur > 7 ? cur - 7 : 0;
        for(int r = 0; r < 8 && top + r < n; r++) {
            int i = top + r, y = 36 + r * 20, act = (i == cur);
            drect(14, y, 381, y + 17, act ? WHITE : BG);
            drect_border(14, y, 381, y + 17, C_NONE, act ? 2 : 1, act ? col : LINEC);
            dprint_opt(22, y + 9, act ? INK : GRAY, C_NONE, DTEXT_LEFT, DTEXT_MIDDLE, "%s", lab[i]);
            dprint_opt(372, y + 9, INK, C_NONE, DTEXT_RIGHT, DTEXT_MIDDLE, "%s%s", val[i], act ? "_" : "");
        }
        softbar("EXE avanti    DEL cancella    EXIT esci");
        dupdate();
        int k = getkey().key;
        int len = strlen(val[cur]);
        if(k == KEY_EXIT) return 0;
        if(k == KEY_EXE) {
            if(len > 0 && !(len == 1 && val[cur][0] == '-')) {
                if(cur == n - 1) return 1;
                cur++;
            }
        }
        if(k == KEY_DEL && len > 0) val[cur][len - 1] = 0;
        if((k == KEY_NEG || k == KEY_SUB) && len == 0) { val[cur][0] = '-'; val[cur][1] = 0; }
        for(int i = 0; i < 10; i++)
            if(k == dk[i] && len < (val[cur][0] == '-' ? 10 : 9)) {
                val[cur][len] = '0' + i; val[cur][len + 1] = 0;
            }
        if(k == KEY_UP && cur > 0) cur--;
        if(k == KEY_DOWN && cur < n - 1 && len > 0) cur++;
    }
}

static long mcd(long a, long b)
{
    a = labs(a); b = labs(b);
    while(b) { long t = a % b; a = b; b = t; }
    return a;
}

static void app_mcd(void)
{
    static char lab[MAXF][20], val[MAXF][14];
    strcpy(lab[0], "Primo numero (A)");
    strcpy(lab[1], "Secondo numero (B)");
    if(!form("MCD e mcm", BLUE, 2, lab, val)) return;
    long a = strtol(val[0], NULL, 10), b = strtol(val[1], NULL, 10);
    if(a == 0 && b == 0) { errore("Inserisci almeno un numero diverso da zero."); return; }
    long g = mcd(a, b);
    long long m = (long long)labs(a) / g * labs(b);
    Line ln[24]; int nl = 0;
    LNP("MCD", "%ld", g);
    if(m > 2147483647LL) LNP("mcm", "oltre 2 miliardi");
    else LNP("mcm", "%ld", (long)m);
    LNH("Passaggi (algoritmo di Euclide)");
    long x = labs(a), y = labs(b);
    while(y != 0 && nl < 22) {
        LNP("", "%ld = %ld x %ld + %ld", x, x / y, y, x % y);
        long t = x % y; x = y; y = t;
    }
    results("MCD e mcm", BLUE, ln, nl);
}

static void app_frazioni(void)
{
    static char lab[MAXF][20], val[MAXF][14];
    strcpy(lab[0], "Numeratore");
    strcpy(lab[1], "Denominatore");
    if(!form("Semplifica frazione", GREEN, 2, lab, val)) return;
    long n = strtol(val[0], NULL, 10), d = strtol(val[1], NULL, 10);
    if(d == 0) { errore("Il denominatore non puo' essere zero."); return; }
    long g = mcd(n, d);
    if(g == 0) g = 1;
    long n2 = n / g, d2 = d / g;
    if(d2 < 0) { n2 = -n2; d2 = -d2; }
    Line ln[8]; int nl = 0;
    LNP("Frazione data", "%ld / %ld", n, d);
    LNP("MCD", "%ld", g);
    LNP("Semplificata", "%ld / %ld", n2, d2);
    if(g == 1) LNP("Nota", "gia' ridotta ai minimi");
    if(d2 == 1) LNP("Valore intero", "%ld", n2);
    results("Semplifica frazione", GREEN, ln, nl);
}

static void app_hex(void)
{
    static char lab[MAXF][20], val[MAXF][14];
    strcpy(lab[0], "Numero decimale");
    if(!form("Decimale in esadecimale", ORANGE, 1, lab, val)) return;
    long n = strtol(val[0], NULL, 10);
    unsigned long u = n < 0 ? -n : n;
    const char *sg = n < 0 ? "-" : "";
    char bin[40];
    int p = 0;
    if(u == 0) bin[p++] = '0';
    else {
        int started = 0;
        for(int i = 31; i >= 0; i--) {
            int bit = (u >> i) & 1;
            if(bit) started = 1;
            if(started) bin[p++] = '0' + bit;
        }
    }
    bin[p] = 0;
    Line ln[8]; int nl = 0;
    LNP("Decimale", "%ld", n);
    LNP("Esadecimale", "%s0x%lX", sg, u);
    LNP("Ottale", "%s%lo", sg, u);
    LNH("Binario");
    char b2[48];
    snprintf(b2, sizeof b2, "%s%s", sg, bin);
    wide(ln, &nl, b2);
    results("Decimale in esadecimale", ORANGE, ln, nl);
}

static void poly_str(const long long *c, int n, char *out, int cap)
{
    int len = 0, first = 1;
    out[0] = 0;
    for(int i = 0; i <= n; i++) {
        long long v = c[i];
        int p = n - i;
        if(v == 0) continue;
        long a = (long)(v < 0 ? -v : v);
        char t[32], co[16] = "", piece[48];
        if(p == 0) sprintf(t, "%ld", a);
        else {
            if(a != 1) sprintf(co, "%ld", a);
            if(p == 1) sprintf(t, "%sx", co);
            else sprintf(t, "%sx^%d", co, p);
        }
        if(first) sprintf(piece, "%s%s", v < 0 ? "-" : "", t);
        else sprintf(piece, " %c %s", v < 0 ? '-' : '+', t);
        first = 0;
        int pl = strlen(piece);
        if(len + pl < cap - 1) { strcpy(out + len, piece); len += pl; }
    }
    if(first) strcpy(out, "0");
}

static void app_ruffini(void)
{
    static char lab[MAXF][20], val[MAXF][14];
    strcpy(lab[0], "Grado (da 1 a 10)");
    if(!form("Ruffini", PURPLE, 1, lab, val)) return;
    long g = strtol(val[0], NULL, 10);
    if(g < 1 || g > 10) { errore("Il grado deve essere tra 1 e 10."); return; }
    int n = (int)g;
    for(int i = 0; i <= n; i++) sprintf(lab[i], "coeff. di x^%d", n - i);
    if(!form("Ruffini: coefficienti", PURPLE, n + 1, lab, val)) return;

    long long c[12];
    for(int i = 0; i <= n; i++) c[i] = strtol(val[i], NULL, 10);
    if(c[0] == 0) { errore("Il primo coefficiente non puo' essere zero."); return; }

    Line ln[40]; int nl = 0;
    char buf[160], fac[200], tmp[160];
    poly_str(c, n, buf, sizeof buf);
    LNH("Polinomio");
    wide(ln, &nl, buf);

    long roots[10]; int nr = 0;
    while(n > 0 && c[n] == 0) { roots[nr++] = 0; n--; }
    for(long d = 1; n > 0; d++) {
        long long cn = c[n] < 0 ? -c[n] : c[n];
        if(d > cn || d > 1000000) break;
        if(cn % d) continue;
        for(int k = 0; k < 2; k++) {
            long r = k ? -d : d;
            while(n > 0) {
                long long b = c[0];
                for(int i = 1; i <= n; i++) b = b * r + c[i];
                if(b != 0) break;
                for(int i = 1; i < n; i++) c[i] += c[i - 1] * r;
                n--;
                roots[nr++] = r;
            }
        }
    }

    fac[0] = 0;
    if(n == 0 && c[0] != 1) {
        if(c[0] == -1) strcpy(fac, "-");
        else sprintf(fac, "%ld", (long)c[0]);
    }
    for(int i = 0; i < nr && strlen(fac) < 150; i++) {
        char pc[24];
        if(roots[i] == 0) strcpy(pc, "x");
        else if(roots[i] > 0) sprintf(pc, "(x-%ld)", roots[i]);
        else sprintf(pc, "(x+%ld)", -roots[i]);
        strcat(fac, pc);
    }
    if(n > 0) {
        poly_str(c, n, tmp, sizeof tmp);
        if(strlen(fac) + strlen(tmp) + 3 < sizeof fac) {
            strcat(fac, "("); strcat(fac, tmp); strcat(fac, ")");
        }
    }
    LNH("Scomposizione");
    wide(ln, &nl, fac);

    if(nr == 0) LNH("Nessuna radice intera trovata");
    else {
        LNH("Radici intere");
        for(int i = 0; i < nr && nl < 38; ) {
            int m = 1;
            while(i + m < nr && roots[i + m] == roots[i]) m++;
            if(m > 1) LNP("", "x = %ld    (molteplicita' %d)", roots[i], m);
            else LNP("", "x = %ld", roots[i]);
            i += m;
        }
    }
    results("Ruffini: risultato", PURPLE, ln, nl);
}

typedef struct { const char *name, *sub; int col; } Tile;
static const Tile tiles[4] = {
    {"MCD e mcm",    "due numeri",       BLUE},
    {"Ruffini",      "radici e fattori", PURPLE},
    {"Frazioni",     "riduci ai minimi", GREEN},
    {"Esadecimale",  "decimale in hex",  ORANGE},
};

static void draw_menu(int sel)
{
    dclear(BG);
    titlebar("MATEAPP", BLUE);
    for(int i = 0; i < 4; i++) {
        int x = 14 + (i % 2) * 190, y = 38 + (i / 2) * 80, w = 178, h = 72;
        drect(x, y, x + w - 1, y + h - 1, tiles[i].col);
        if(sel == i) drect_border(x - 3, y - 3, x + w + 2, y + h + 2, C_NONE, 3, INK);
        drect(x + 10, y + 10, x + 37, y + 37, WHITE);
        dprint_opt(x + 24, y + 24, tiles[i].col, C_NONE, DTEXT_CENTER, DTEXT_MIDDLE, "%d", i + 1);
        dprint_opt(x + 48, y + 18, WHITE, C_NONE, DTEXT_LEFT, DTEXT_MIDDLE, "%s", tiles[i].name);
        dprint_opt(x + 48, y + 34, WHITE, C_NONE, DTEXT_LEFT, DTEXT_MIDDLE, "%s", tiles[i].sub);
        dprint_opt(x + 10, y + 56, WHITE, C_NONE, DTEXT_LEFT, DTEXT_MIDDLE, "premi EXE");
    }
    softbar("tasti 1-4 o frecce + EXE        EXIT: esci");
    dupdate();
}

static void run(int i)
{
    if(i == 0) app_mcd();
    if(i == 1) app_ruffini();
    if(i == 2) app_frazioni();
    if(i == 3) app_hex();
}

int main(void)
{
    int sel = 0;
    for(;;) {
        draw_menu(sel);
        int k = getkey().key;
        if(k == KEY_EXIT) return 1;
        if(k == KEY_1) { sel = 0; run(0); }
        if(k == KEY_2) { sel = 1; run(1); }
        if(k == KEY_3) { sel = 2; run(2); }
        if(k == KEY_4) { sel = 3; run(3); }
        if(k == KEY_LEFT && sel % 2) sel--;
        if(k == KEY_RIGHT && !(sel % 2)) sel++;
        if(k == KEY_UP && sel >= 2) sel -= 2;
        if(k == KEY_DOWN && sel < 2) sel += 2;
        if(k == KEY_EXE) run(sel);
    }
}
