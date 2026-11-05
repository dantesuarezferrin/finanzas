#ifndef MODELS_H
#define MODELS_H

#include <stdint.h>
typedef enum{
	INCOME,
	EXPENSE,
	RESET
} TransactionType;

typedef struct{
	int id;
	char name[64];
	double balance;
	char currency[8];
} Account;

typedef struct{
	int id;
	int account_id;
	TransactionType type;
	double amount;
	int64_t time;
	char desc[64];
} Transaction;

#endif
