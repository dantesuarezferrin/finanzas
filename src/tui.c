#include "tui.h"
#include "database.h"
#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void tui_run(){
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);

	if(has_colors()){
		start_color();
		init_pair(1, COLOR_CYAN, COLOR_BLACK);
		init_pair(2, COLOR_BLACK, COLOR_WHITE);
	}
	
	bool running = true;
	int ch;
	Account *accounts = NULL;
	int account_count = db_get_all_accounts(&accounts);
	int selected = 0;

	while(running){
		clear();

		attron(COLOR_PAIR(1) | A_BOLD);
		mvprintw(1, 2, "=== SISTEMA DE FINANZAS ===");
		attroff(COLOR_PAIR(1) | A_BOLD);

		mvprintw(LINES - 2, 2, "[Flechas] Navegar | [c] Nueva | [i] Ingreso | [e] Egreso | [d] Borrar | [q] Salir");
		if (account_count < 0) {
      mvprintw(4, 4, "Error crítico: No se pudo leer la base de datos.");
  	} else if (account_count == 0) {
        mvprintw(4, 4, "No hay cuentas registradas. Usá la CLI para crear una.");
    } else {
      	for (int i = 0; i < account_count; i++) {
          if (i == selected) attron(COLOR_PAIR(2));
          mvprintw(4 + i, 4, " %-20s $ %10.2f ", accounts[i].name, accounts[i].balance);
          if (i == selected) attroff(COLOR_PAIR(2));
        }
    }
		refresh();

		ch = getch();
		switch (ch) {
			case 'q':
			case 'Q':
				running = false;
				break;
			case 'c':
			case 'C': { 
				char new_name[64] = {0};
				char balance_str[32] = {0};
				char currency_str[8] = {0};

				echo();
				curs_set(1);

				move(LINES - 2, 0); 
				clrtoeol();         
				mvprintw(LINES - 2, 2, "Nombre de la nueva cuenta: ");
				getnstr(new_name, 63); 

				move(LINES - 2, 0);
				clrtoeol();
				mvprintw(LINES - 2, 2, "Saldo inicial: $ ");
				getnstr(balance_str, 31);

				move(LINES - 2, 0);
				clrtoeol();
				mvprintw(LINES - 2, 2, "Moneda (Enter para ARS): ");
				getnstr(currency_str, 7);

				noecho();
				curs_set(0);

				if (strlen(new_name) > 0) {
					double new_balance = atof(balance_str);
					
					if (strlen(currency_str) == 0) {
						strcpy(currency_str, "ARS");
					}
					
					int target_position = (account_count > 0) ? (selected + 1) : 0;

					if (db_create_account_at(new_name, new_balance, currency_str, target_position)) {
						free(accounts);
						accounts = NULL;
						account_count = db_get_all_accounts(&accounts);
						
						selected = target_position;
					}
				}
				break;
			}
			case 'i':
			case 'I':
			case 'e':
			case 'E': {
				if (account_count > 0) {
					char amount_str[32] = {0};
					char desc_str[128] = {0};
					TransactionType type = (ch == 'i' || ch == 'I') ? INCOME : EXPENSE;

					echo();
					curs_set(1);

					move(LINES - 2, 0);
					clrtoeol();
					mvprintw(LINES - 2, 2, (type == INCOME) ? "Monto del ingreso: $ " : "Monto del egreso: $ ");
					getnstr(amount_str, 31);

					move(LINES - 2, 0);
					clrtoeol();
					mvprintw(LINES - 2, 2, "Descripción: ");
					getnstr(desc_str, 127);

					noecho();
					curs_set(0);

					double amount = atof(amount_str);
					if (amount > 0) {
						const char *final_desc = (strlen(desc_str) > 0) ? desc_str : ((type == INCOME) ? "Ingreso TUI" : "Egreso TUI");
						if (db_add_transaction(accounts[selected].id, type, amount, final_desc)) {
							free(accounts);
							accounts = NULL;
							account_count = db_get_all_accounts(&accounts);
						}
					}
				}
				break;
			}
			case 'd':
			case 'D':
				if (account_count > 0) {
					if (db_delete_account(accounts[selected].name)) {
						free(accounts);
						accounts = NULL;
						account_count = db_get_all_accounts(&accounts);
						if (selected >= account_count && selected > 0) {
							selected--;
						}
					}
				}
				break;
			case KEY_UP:
				if (selected > 0) {
					selected--;
				}
				break;
			case KEY_DOWN:
				if (selected < account_count - 1) {
					selected++;
				}
				break;
		}
	}

	endwin();
}
