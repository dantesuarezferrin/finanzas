#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include "database.h"
#include "models.h"

static sqlite3 *db = NULL;


bool db_init(const char *db_path){
	if(sqlite3_open(db_path, &db) != SQLITE_OK){
		fprintf(stderr, "Error al abrir la BD: %s\n", sqlite3_errmsg(db)) ;
		return false;
	}
	sqlite3_exec(db, "PRAGMA foreign_keys=ON", NULL, NULL, NULL);
	const char *sql =
		"CREATE TABLE IF NOT EXISTS accounts ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"name TEXT UNIQUE NOT NULL,"
		"balance REAL NOT NULL DEFAULT 0.0,"
		"currency TEXT NOT NULL DEFAULT 'ARS',"
		"display_order INTEGER NOT NULL DEFAULT 0"
		");"
		"CREATE TABLE IF NOT EXISTS transactions ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"account_id INTEGER NOT NULL,"
		"type INTEGER NOT NULL,"
		"amount REAL NOT NULL,"
		"timestamp INTEGER NOT NULL,"
		"description TEXT,"
		"FOREIGN KEY(account_id) REFERENCES accounts(id)"
		");";
	char *err_msg = NULL;
	if(sqlite3_exec(db, sql, NULL, NULL, &err_msg) != SQLITE_OK){
		fprintf(stderr, "Error SQL al crear tablas: %s\n", err_msg);
		return false;
	}

	return true;
}


void db_close(void){
	if(db){
		sqlite3_close(db);
		db = NULL;
	}
}

bool db_create_account(const char *name, double initial_balance, const char *currency){
	const char *sql = "INSERT INTO accounts (name, balance, currency) VALUES (?, ?, ?);";
	sqlite3_stmt *stmt;
	
	if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparando consulta: %s\n", sqlite3_errmsg(db));
		return false;
	}
	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_double(stmt, 2, initial_balance);
	sqlite3_bind_text(stmt, 3, currency, -1, SQLITE_STATIC);

	bool succes = true;
	if(sqlite3_step(stmt) != SQLITE_DONE){
		fprintf(stderr, "Error al insertar la cuenta: %s\n", sqlite3_errmsg(db));
		succes = false;
	}
	sqlite3_finalize(stmt);
	
	return succes;
}

bool db_create_account_at(const char *name, double initial_balance, const char *currency, int target_pos) {
    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    const char *sql_shift = "UPDATE accounts SET display_order = display_order + 1 WHERE display_order >= ?;";
    sqlite3_stmt *stmt_shift;
    if (sqlite3_prepare_v2(db, sql_shift, -1, &stmt_shift, NULL) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        return false;
    }
    sqlite3_bind_int(stmt_shift, 1, target_pos);
    sqlite3_step(stmt_shift);
    sqlite3_finalize(stmt_shift);

    const char *sql_insert = "INSERT INTO accounts (name, balance, currency, display_order) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt_insert;
    if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, NULL) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        return false;
    }
    sqlite3_bind_text(stmt_insert, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt_insert, 2, initial_balance);
    sqlite3_bind_text(stmt_insert, 3, currency, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt_insert, 4, target_pos);

    if (sqlite3_step(stmt_insert) != SQLITE_DONE) {
        sqlite3_finalize(stmt_insert);
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        return false;
    }
    sqlite3_finalize(stmt_insert);

    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
    return true;
}

bool db_delete_account(const char *name){
	const char *sql = "DELETE FROM accounts WHERE name = ?";
	sqlite3_stmt *stmt;
	if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparando al consulta: %s\n", sqlite3_errmsg(db));
		return false;
	}
	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	bool succes = true;
	if(sqlite3_step(stmt) != SQLITE_DONE){
		fprintf(stderr, "Error al eliminar la cuenta: %s\n", sqlite3_errmsg(db));
		succes = false;
	}
	else{
		if(sqlite3_changes(db) == 0){
			fprintf(stderr, "La cuenta '%s' no existe.\n", name);
			return false;
		}
	}

	sqlite3_finalize(stmt);
	return succes;
}

bool db_rename_account(const char *old_name, const char *new_name){

	const char *sql = "UPDATE accounts SET name = ? WHERE name = ?;";
	sqlite3_stmt *stmt;
	if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparando la consulta: %s\n", sqlite3_errmsg(db));
		return false;
	}

	sqlite3_bind_text(stmt, 1, new_name, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, old_name, -1, SQLITE_STATIC);

	bool succes = true;
	
	if(sqlite3_step(stmt) != SQLITE_DONE){
		fprintf(stderr, "Error al cambiar el nombre de la cuenta '%s': %s.\n", old_name, sqlite3_errmsg(db));
		succes = false;
	}

	sqlite3_finalize(stmt);
	return succes;
}

bool db_get_account_by_name(const char *name, Account *account){
	const char *sql = "SELECT id, name, balance, currency FROM accounts WHERE name = ?";
	sqlite3_stmt *stmt;
	if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparando la consulta: %s\n", sqlite3_errmsg(db));
		return false;
	}
	
	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	bool succes=false;
	
	if(sqlite3_step(stmt) == SQLITE_ROW){
		account->id = sqlite3_column_int(stmt, 0);
		strncpy(account->name, (const char *)sqlite3_column_text(stmt, 1), sizeof(account->name) -1);
		account->name[sizeof(account->name)-1] = '\0';
		account->balance = sqlite3_column_double(stmt, 2);
		strncpy(account->currency, (const char *)sqlite3_column_text(stmt, 3), sizeof(account->currency) -1);
		account->currency[sizeof(account->currency)-1] = '\0';
		succes = true;
	}
	else{
		fprintf(stderr, "La cuenta no existe o hubo un error\n");
	}

	sqlite3_finalize(stmt);
	return succes;
}

bool db_add_transaction(int account_id, TransactionType type, double amount, const char *description){
	sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	const char *sql_insert =
		"INSERT INTO transactions (account_id, type, amount, timestamp, description) "
		"VALUES (?, ?, ?, ?, ?);";
	
	sqlite3_stmt *stmt_insert;
	if(sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparando insert de transaccion: %s\n", sqlite3_errmsg(db));
		sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
		return false;
	}

	sqlite3_bind_int(stmt_insert, 1, account_id);
  sqlite3_bind_int(stmt_insert, 2, type);
	sqlite3_bind_double(stmt_insert, 3, amount);
	sqlite3_bind_int64(stmt_insert, 4, (int64_t)time(NULL));

	if(description != NULL){
		sqlite3_bind_text(stmt_insert, 5, description, -1, SQLITE_STATIC);
	}
	else{
		sqlite3_bind_text(stmt_insert, 5, "", -1, SQLITE_STATIC);
	}

	if(sqlite3_step(stmt_insert) != SQLITE_DONE){
		fprintf(stderr, "Error insertando transaccion: %s\n", sqlite3_errmsg(db));
		sqlite3_finalize(stmt_insert);
		sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
		return false;
	}
	sqlite3_finalize(stmt_insert);

	const char *sql_update;
	switch (type) {
		case INCOME:
			sql_update = "UPDATE accounts SET balance = balance + ? WHERE id = ?;";
			break;
		case EXPENSE:
			sql_update = "UPDATE accounts SET balance = balance - ? WHERE id = ?;";
			break;
		case RESET:
			sql_update = "UPDATE accounts SET balance = ? WHERE id = ?;";
			break;
	}
	
	sqlite3_stmt *stmt_update;
	if(sqlite3_prepare_v2(db, sql_update, -1, &stmt_update, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparando la actualizacion de balance: %s\n", sqlite3_errmsg(db));
		sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
		return false;
	}

	sqlite3_bind_double(stmt_update, 1, amount);
	sqlite3_bind_int(stmt_update, 2, account_id);

	if(sqlite3_step(stmt_update) != SQLITE_DONE){
		fprintf(stderr, "Error actualizando balance: %s\n", sqlite3_errmsg(db));
		sqlite3_finalize(stmt_update);
		sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
		return false;
	}
	sqlite3_finalize(stmt_update);
	
	sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);

	return true;
}

int db_get_all_accounts(Account **accounts){
	const char *sql = "SELECT id, name, balance, currency, display_order FROM accounts ORDER BY display_order ASC;";
	sqlite3_stmt *stmt;

	if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparando consulta: %s\n", sqlite3_errmsg(db));
		return -1;
	}
	
	int count = 0;
	int capacity = 10;
	Account *list = malloc(capacity *sizeof(Account));
	while(sqlite3_step(stmt) == SQLITE_ROW){
		if (count >= capacity) {
      capacity *= 2;
      list = realloc(list, capacity * sizeof(Account));
    }
        
    list[count].id = sqlite3_column_int(stmt, 0); // <--- Asignar el id correctamente[cite: 7]

    const char *db_name = (const char *)sqlite3_column_text(stmt, 1);
    strncpy(list[count].name, db_name ? db_name : "Desconocido", sizeof(list[count].name) - 1);
    list[count].name[sizeof(list[count].name) - 1] = '\0'; 
        
    list[count].balance = sqlite3_column_double(stmt, 2);

    const char *db_curr = (const char *)sqlite3_column_text(stmt, 3);
    strncpy(list[count].currency, db_curr ? db_curr : "ARS", sizeof(list[count].currency) - 1);
    list[count].currency[sizeof(list[count].currency) - 1] = '\0';
        
    count++;
	}
	sqlite3_finalize(stmt);
	*accounts = list;
	return count;
}
