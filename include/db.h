#ifndef __DB_H__
#define __DB_H__

//int db_open();
void db_open();
void db_close();
void db_select_company(const char * companyName);
void db_insert_company(const char * companyName);

#endif
