#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "cli.h"

int main(int argc, char *argv[]) {
    char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, "Error: No se pudo obtener la variable HOME.\n");
        return 1;
    }

    char db_path[256];
    snprintf(db_path, sizeof(db_path), "%s/.finanzas.sqlite", home);

    if (!db_init(db_path)) {
        fprintf(stderr, "Error critico: No se pudo iniciar la base de datos.\n");
        return 1;
    }

    if (argc > 1) {
        ex_cli(argc, argv);
    } else {
        printf("TUI no implementada todavia. Usa --help para ver opciones CLI.\n");
    }

    db_close();
    return 0;
}
