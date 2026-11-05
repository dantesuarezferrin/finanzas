#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "cli.h"
#include "database.h"

void show_help() {
    printf("Uso: finanzas [ACCION] [CUENTA] [MONTO]\n\n");
    printf("Acciones rapidas:\n");
    printf("  -c <cuenta> ([monto] || -m [moneda])   Crear una cuenta nueva (monto inicial opcional, por defecto 0. -m le da moneda a la cuenta, no se puede usar junto a monto)\n");
    printf("  -d <cuenta>  				  Elimina una cuenta, esta debe no tener movimientos registrados\n");
    printf("  -r <viejo> <nuevo> 		Renombra una cuenta\n");
    printf("  -i <cuenta> <monto>   Registrar un ingreso en la cuenta\n");
    printf("  -e <cuenta> <monto>   Registrar un egreso en la cuenta\n");
		printf("  -s 										Muestra el estado de todas las cuentas\n");
    printf("  -h                    Mostrar ayuda\n\n");
    printf("Ejemplos:\n");
    printf("  finanzas -c Billetera 1500\n");
    printf("  finanzas -i Billetera 500\n");
}

void ex_cli(int argc, char *argv[]) {
    char *account_name = NULL;
		char *new_account_name = NULL;
    double amount = 0.0;
    int action = 0;
    int c;
		char *currency = "ARS";

    while ((c = getopt(argc, argv, "c:d:i:r:e:m:sh")) != -1) {
        switch (c) {

            case 'c':
                action = 1;
                account_name = optarg;
                if (optind < argc && argv[optind][0] != '-') {
                    amount = atof(argv[optind]);
                    optind++;
                }
                break;

            case 'i':
                action = 2;
                account_name = optarg;
                if (optind < argc && argv[optind][0] != '-') {
                    amount = atof(argv[optind]);
                    optind++;
                } else {
                    printf("Error: Debes especificar un monto para el ingreso.\n");
                    return;
                }
                break;

            case 'e':
                action = 3;
                account_name = optarg;
                if (optind < argc && argv[optind][0] != '-') {
                    amount = atof(argv[optind]);
                    optind++;
                } else {
                    printf("Error: Debes especificar un monto para el egreso.\n");
                    return;
                }
                break;

						case 'm':
								currency = optarg;
								break;

						case 's':
                action = 4;
                break;
						
						case 'd':
								action = 5;
								account_name = optarg;
								break;

						case 'r':
								action = 6;
                account_name = optarg;
                if (optind < argc && argv[optind][0] != '-') {
                    new_account_name = argv[optind];
                    optind++;
                } else {
                    printf("Error: Debes especificar el nuevo nombre para la cuenta.\n");
                    return;
                }
								break;

            case 'h':
                show_help();
                return;

            case '?':
                return;

            default:
                abort();
        }
    }

    Account target_account;

    switch (action) {
        case 1:
            printf("Creando cuenta '%s' con balance $%.2f...\n", account_name, amount);
            if (db_create_account(account_name, amount, currency)) {
                printf("-> Cuenta creada con exito.\n");
            } else {
                printf("-> Error al crear la cuenta (quizas ya existe).\n");
            }
            break;

        case 2:
            if (db_get_account_by_name(account_name, &target_account)) {
                if (db_add_transaction(target_account.id, INCOME, amount, "Ingreso por CLI")) {
                    printf("-> Ingreso de $%.2f registrado en '%s'. Nuevo balance: $%.2f\n", 
                            amount, account_name, target_account.balance + amount);
                } else {
                    printf("-> Error al registrar el ingreso en la BD.\n");
                }
            } else {
                printf("-> Error: La cuenta '%s' no existe.\n", account_name);
            }
            break;

        case 3:
            if (db_get_account_by_name(account_name, &target_account)) {
                if (db_add_transaction(target_account.id, EXPENSE, amount, "Egreso por CLI")) {
                    printf("-> Egreso de $%.2f registrado en '%s'. Nuevo balance: $%.2f\n", 
                            amount, account_name, target_account.balance - amount);
                } else {
                    printf("-> Error al registrar el egreso en la BD.\n");
                }
            } else {
                printf("-> Error: La cuenta '%s' no existe.\n", account_name);
            }
            break;

				case 4:
						Account list[10];
            int total_accounts = db_get_all_accounts(list, 10);
            
            if (total_accounts < 0) {
                printf("Error al obtener las cuentas de la base de datos.\n");
            } else if (total_accounts == 0) {
                printf("No hay cuentas registradas. Usa -c para crear una.\n");
            } else {
                printf("=== ESTADO DE CUENTAS ===\n");

								struct { char curr[8]; double total; } totals[20];
                int totals_count = 0;

                for (int i = 0; i < total_accounts; i++) {
                    printf("%-15s : $ %10.2f %s\n", list[i].name, list[i].balance, list[i].currency);
                    
                    int found = 0;
                    for (int j = 0; j < totals_count; j++) {
                        if (strcmp(totals[j].curr, list[i].currency) == 0) {
                            totals[j].total += list[i].balance;
                            found = 1;
                            break;
                        }
                    }
                    if (!found && totals_count < 20) {
                        strcpy(totals[totals_count].curr, list[i].currency);
                        totals[totals_count].total = list[i].balance;
                        totals_count++;
                    }
                }
                
                printf("----------------------------------\n");
                printf("PATRIMONIO TOTAL:\n");
                for (int j = 0; j < totals_count; j++) {
                    printf("%-15s : $ %10.2f %s\n", "Total", totals[j].total, totals[j].curr);
                }
            }
					break;

				case 5:
            printf("Eliminando cuenta '%s'...\n", account_name);
            if(db_delete_account(account_name)) {
                printf("-> Cuenta eliminada con exito.\n");
            }
            else{
                printf("-> Error al eliminar la cuenta.\n");
            }
					break;

				case 6:
					printf("Renombrando la cuenta...\n");
					if(db_rename_account(account_name, new_account_name)){
						printf("-> Cuenta renombrada con exito.\n");
					}
					break;

        default:
            printf("No se especifico ninguna accion valida.\n");
            show_help();
            break;
    }
}
