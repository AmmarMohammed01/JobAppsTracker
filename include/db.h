#ifndef __DB_H__
#define __DB_H__

int db_open();
void db_close(int status);

int db_select_company(const char * companyName);
int db_insert_company(const char * companyName);

int db_select_platform(const char * platformName);
int db_insert_platform(const char * platformName);

int db_insert_job_listing(const int companyId, const int platformId, const char * jobTitle, const int resumeId);
int db_insert_job_status(const int jobId, const char * status_date, const char * status_description);

int db_select_job_id(const int jobId);

int db_select_resume(const char * resumeName);
int db_insert_resume(const char * resumeName); //used when adding job listing
int db_insert_resume_full(const char * resumeName, const char * resumeLink); //jacli_resume_add

int db_insert_url(const int jobId, const char * websiteLink);

//MULTIPURPOSE
void db_select_all(const char * stmt_string);
int db_delete_by_id(const char * stmt_string, const int id);
//int db_select_by_id(const char * stmt_string, const int id);

#endif
