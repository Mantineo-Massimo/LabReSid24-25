#include <stdio.h>
#include "header.h"

/* Definizione: alloca spazio in memoria (Sezione DATA) */
int var_extern = 10; 

/* Definizione locale al file (Sezione data locale) */
static int var_static = 100; 

void modifica_var_extern(void) {
    var_extern++;
    var_static += 10;
    printf("file1: var_extern=%d, var_static=%d\n", var_extern, var_static);
}
