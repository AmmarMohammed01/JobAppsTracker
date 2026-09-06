#ifndef __DB_H__
#define __DB_H__

void db_open();
void db_close();

int db_select_company(const char * companyName);
unsigned long long db_insert_company(const char * companyName);

int db_select_platform(const char * companyName);
unsigned long long db_insert_platform(const char * companyName);

#endif
