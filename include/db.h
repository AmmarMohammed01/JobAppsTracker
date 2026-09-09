#ifndef __DB_H__
#define __DB_H__

void db_open();
void db_close();

int db_select_company(const char * companyName);
int db_insert_company(const char * companyName);

int db_select_platform(const char * platformName);
int db_insert_platform(const char * platformName);

void db_select_all(const char * stmt_string);

int db_insert_job_listing(const int companyId, const int platformId, const char * jobTitle);
int db_insert_job_status(const int jobId, const char * status_date, const char * status_description);

int db_select_job_id(const int jobId);

#endif
