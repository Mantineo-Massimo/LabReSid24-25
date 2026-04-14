#include <stdio.h>
#include "header.h"

/* Utilizza var_extern (che sara' risolto dal linker cercandolo in file1.o) */
void stampa_var_extern(void) {
    printf("file2: var_extern=%d\n", var_extern);
}
