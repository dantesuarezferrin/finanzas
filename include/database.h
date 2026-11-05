#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>
#include "models.h"

bool db_init(const char *db_path);
void db_close(void);

bool db_create_account(const char *name, double initial_balance, const char *currency);
bool db_delete_account(const char *name);
bool db_get_account_by_name(const char *name, Account *account);
bool db_add_transaction(int account_id, TransactionType type, double amount, const char *description);
int db_get_all_accounts(Account accounts[], int max_accounts);

#endif
