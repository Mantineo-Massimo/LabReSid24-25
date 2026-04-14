/* ex1_symbols.c: Analisi della Tabella dei Simboli */

/* VARIABILI GLOBALI */
int a;               /* Simbolo 'B' (BSS): globale non inizializzata */
int a_init = 10;     /* Simbolo 'D' (DATA): globale inizializzata */

/* VARIABILI STATICHE GLOBALI (Visibilita' limitata al file) */
static int b = 10;   /* Simbolo 'd' (data locale): statica inizializzata */
static int b_bss;    /* Simbolo 'b' (bss locale): statica non inizializzata */

void funzione() {
    static int c = 5; /* Simbolo 'd' (data locale): statica locale inizializzata */
    int d = 20;       /* Variabile sullo Stack (NON appare in nm) */
    c++;
    d++;
}

int main() {
    funzione();
    return 0;
}
