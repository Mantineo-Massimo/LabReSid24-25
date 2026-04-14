#include "header.h"

int main() {
    /* Prima modifica in file1 */
    modifica_var_extern();
    
    /* Stampa in file2 */
    stampa_var_extern();

    /* Seconda modifica in file1 */
    modifica_var_extern();
    
    /* Stampa in file2 (mostra che var_extern e' la stessa) */
    stampa_var_extern();

    return 0;
}
