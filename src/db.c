#include <mysql.h>
#include <stdbool.h> //bool
#include <stdlib.h> //getenv
#include <stdio.h> //fprintf
#include <string.h> //strlen

#include "db.h"
#include "field_types.h"

static MYSQL * mysql = NULL; //this var isn't visible outside file = static

//0 - success, 1 - init failed, 2 - real_connect failed
int db_open() {
	const char *host = getenv("MYSQL_HOST");
	const char *user = getenv("MYSQL_USER");
	const char *password = getenv("MYSQL_PASSWORD");
	const char *database = getenv("MYSQL_DATABASE");

	printf("Connecting to database...\n");

	if (mysql_library_init(0, NULL, NULL)) {
		fprintf(stderr, "ERROR: Could not initialize MySQL client library\n");
		return 1; //MYSQL LIBRARY ISSUE
	}

	if( (mysql = mysql_init(mysql)) == NULL) { //page 68
		fprintf(stderr, "ERROR: Could not initialize mysql object\n");
		return 2; //MYSQL OBJ ISSUE
	}

	if (!mysql_real_connect(mysql,		//mysql
				host,	//hostname or ip address
			 	user,		//user
				password,	//passwd
			 	database,	//db name
				0,		//port
			 	NULL,		//unix_socket
			 	0) //client_flag
	   )//page 91 - 95
	{
		fprintf(stderr, "ERROR: Failed to connect to database: ERR_MSG: %s\n", mysql_error(mysql));
		printf("host: %s\n", host);
		printf("HINT 1: check if you ran 'brew services start mysql'\n");
		printf("HINT 2: check if you ran 'source setenv.sh' in the app directory\n");
		return 3; //DATABASE CONNECT ISSUE
	}
	//return 0;

	printf("Database connection established!\n");
	return 0; //SUCCESS
}

void db_close(int status) {
	if(status == 1) { //no library
		printf("Closing application...\n");
		return;
	}

	if(mysql != NULL) {
		printf("Closing mysql object...\n");
		mysql_close(mysql); //frees allocation of mysql pointer (from mysql_init)
		mysql = NULL;
	}

	printf("Closing mysql library...\n");
	mysql_library_end();
	printf("Closing application...\n");
}

// RETURN COMPANY_ID, else -1 for ERROR
int db_select_company(const char * companyName) {
	MYSQL_STMT * preparedStatement = mysql_stmt_init(mysql);
	const char *stmt_str = "SELECT company_id FROM company WHERE company_name = ?";
	unsigned long length = strlen(stmt_str); // NOTE: what is the value of the string <-- the value hasn't been prepared yet...

	//prepare and bind before execution
	if(mysql_stmt_prepare(preparedStatement, stmt_str, length)) { //success = 0
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));

	unsigned long str_length = strlen(companyName);
	/* STRING PARAM */
	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (char *)companyName;
	bind[0].buffer_length = 50;
	bind[0].is_null = 0;
	bind[0].length = &str_length;

	if ( mysql_stmt_bind_param(preparedStatement, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		return -1;
	}

	if( mysql_stmt_execute(preparedStatement) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_RES * result = mysql_stmt_result_metadata(preparedStatement); //for SELECT
	
	int companyId = -1;
	bool isNull = 0;
	bool error = 0;
	unsigned long resultLength = 0;

	MYSQL_BIND bindResult[1];
	memset(bindResult, 0, sizeof(bindResult));
	bindResult[0].buffer_type = MYSQL_TYPE_LONG;
	bindResult[0].buffer = (char *)&companyId;
	bindResult[0].is_null = &isNull;
	bindResult[0].length = &resultLength;
	bindResult[0].error = &error;

	if( mysql_stmt_bind_result(preparedStatement, bindResult) != 0) {
		fprintf(stderr, "Result bind failed: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		mysql_close(mysql);
		return -1;
	}

	//handle empty values, no results found
	int return_status;
	while ((return_status = mysql_stmt_fetch(preparedStatement)) == 0) {
		if (return_status == 1 || return_status == MYSQL_NO_DATA) {
			printf("No data left\n");
			break;
		}

		if (isNull) {
		    printf("company_id: NULL\n");
		} else {
		    printf("company_id: %d\n", companyId);
		}
	}

	mysql_free_result(result); // b/c stmt_result_metadata

	mysql_stmt_close(preparedStatement); // b/c stmt_init

	return companyId;
}

//RETURN -1 if ERROR, else COMPANY_ID
int db_insert_company(const char * companyName) {
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "INSERT INTO company (company_name) VALUES (?);";
	unsigned long length = strlen(stmt_string);
	
	if (mysql_stmt_prepare(stmt_handler, stmt_string, length)) {
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));

	unsigned long param_str_length = strlen(companyName);
	/* STRING PARAM */
	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (char *)companyName;
	bind[0].buffer_length = 50;
	bind[0].is_null = 0;
	bind[0].length = &param_str_length;

	if ( mysql_stmt_bind_param(stmt_handler, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		return -1;
	}

	if( (mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	int companyId = mysql_stmt_insert_id(stmt_handler);

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	return companyId;
}


int db_select_platform(const char * platformName) {
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "SELECT platform_id FROM platform WHERE platform_name = ?";
	unsigned long length = strlen(stmt_string); // NOTE: what is the value of the string <-- the value hasn't been prepared yet...

	//prepare and bind before execution
	if(mysql_stmt_prepare(stmt_handler, stmt_string, length)) { //success = 0
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));

	unsigned long param_str_length = strlen(platformName);
	/* STRING PARAM */
	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (char *)platformName; // (char *)str_data
	bind[0].buffer_length = 50;
	bind[0].is_null = 0;
	bind[0].length = &param_str_length;

	if ( mysql_stmt_bind_param(stmt_handler, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		return -1;
	}

	if(mysql_stmt_execute(stmt_handler)) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_RES * result = mysql_stmt_result_metadata(stmt_handler); //for SELECT
	
	int platformId = -1;
	bool isNull = 0;
	bool error = 0;
	unsigned long resultLength = 0;

	MYSQL_BIND bindResult[1];
	memset(bindResult, 0, sizeof(bindResult));
	bindResult[0].buffer_type = MYSQL_TYPE_LONG;
	bindResult[0].buffer = (char *)&platformId;
	bindResult[0].is_null = &isNull;
	bindResult[0].length = &resultLength;
	bindResult[0].error = &error;

	if( mysql_stmt_bind_result(stmt_handler, bindResult) != 0) {
		fprintf(stderr, "Result bind failed: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	//handle empty values, no results found
	int return_status;
	while ((return_status = mysql_stmt_fetch(stmt_handler)) == 0) {
		if (return_status == 1 || return_status == MYSQL_NO_DATA) {
			printf("No data left\n");
			break;
		}

		if (isNull) {
		    printf("platform_id: NULL\n");
		} else {
		    printf("platform_id: %d\n", platformId);
		}
	}

	mysql_free_result(result); // b/c stmt_result_metadata

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	return platformId;
}

//RETURN -1 IF ERROR, ELSE PLATFORM_ID
int db_insert_platform(const char * platformName) {
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "INSERT INTO platform (platform_name) VALUES (?);";
	unsigned long length = strlen(stmt_string);
	
	if (mysql_stmt_prepare(stmt_handler, stmt_string, length)) {
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));

	unsigned long param_str_length = strlen(platformName);
	/* STRING PARAM */
	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (char *)platformName; // (char *)str_data
	bind[0].buffer_length = 50;
	bind[0].is_null = 0;
	bind[0].length = &param_str_length;

	if ( mysql_stmt_bind_param(stmt_handler, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		return -1;
	}

	if( (mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	int platformId = mysql_stmt_insert_id(stmt_handler);

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	return platformId;
}

//RETURN -1 IF ERROR, ELSE RETURN NEWLY INSERTED JOBID
int db_insert_job_listing(const int companyId, const int platformId, const char * jobTitle, const int resumeId) {
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "INSERT INTO job (company_id, platform_id, job_title, resume_id) VALUES (?, ?, ?, ?);";
	unsigned long length = strlen(stmt_string);
	
	if (mysql_stmt_prepare(stmt_handler, stmt_string, length)) {
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_BIND bind[4];
	memset(bind, 0, sizeof(bind));

	/* INTEGER PARAM */
	/* This is a number type, so there is no need
	to specify buffer_length */
	bind[0].buffer_type= MYSQL_TYPE_LONG;
	bind[0].buffer= (char *)&companyId;
	bind[0].is_null= 0;
	bind[0].length= 0;

	/* INTEGER PARAM */
	bind[1].buffer_type= MYSQL_TYPE_LONG;
	bind[1].buffer= (char *)&platformId;
	bind[1].is_null= 0;
	bind[1].length= 0;

	unsigned long param_str_length = strlen(jobTitle);
	/* STRING PARAM */
	bind[2].buffer_type = MYSQL_TYPE_STRING;
	bind[2].buffer = (char *)jobTitle;
	bind[2].buffer_length = 50;
	bind[2].is_null = 0;
	bind[2].length = &param_str_length;

	/* INTEGER PARAM */
	bind[3].buffer_type= MYSQL_TYPE_LONG;
	bind[3].buffer= (char *)&resumeId;
	bind[3].is_null= 0;
	bind[3].length= 0;

	if ( mysql_stmt_bind_param(stmt_handler, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		return -1;
	}

	if( (mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	unsigned long long affectedRows = mysql_stmt_affected_rows(stmt_handler); //same as uint64_t

	int jobId = mysql_stmt_insert_id(stmt_handler); // RETURNS NO ERRORS

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	return jobId;
}

void db_select_all(const char * stmt_string) {
	unsigned long length = strlen(stmt_string);

	if(mysql_query(mysql, stmt_string)) { //0 success
		fprintf(stderr, "Error occured executing query. ERR_MSG %s\n", mysql_error(mysql));
		return;
	}

	MYSQL_RES * result = mysql_store_result(mysql);

	MYSQL_ROW row;
	unsigned int num_fields = mysql_num_fields(result);

	while( (row = mysql_fetch_row(result)) ) {
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(result);
		for(unsigned int i = 0; i < num_fields; i++)
		{
			printf("[%.*s] ", (int) lengths[i], row[i] ? row[i] : "NULL");
		}
		printf("\n");
	}

	mysql_free_result(result);
}

//return statusId
int db_insert_job_status(const int jobId, const char * status_datetime, const char * status_description) {
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "INSERT INTO job_status (job_id, status_datetime, status_description) VALUES (?, ?, ?)";
	unsigned long length = strlen(stmt_string);
	
	if (mysql_stmt_prepare(stmt_handler, stmt_string, length)) {
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_BIND bind[3];
	memset(bind, 0, sizeof(bind));

	/* INTEGER PARAM */
	bind[0].buffer_type= MYSQL_TYPE_LONG;
	bind[0].buffer= (char *)&jobId;
	bind[0].is_null= 0;
	bind[0].length= 0;

	/* DATE PARAM - p. 15-16*/
	//PERMISSABLE INPUT DATA TYPES FOR MYSQL BIND STRUCTS p. 130 table 6.1
	//MYSQL_TIME mt_status_datetime;
	// I found out I can just pass a string as the binding instead of datetime
	//bind[1].buffer_type= MYSQL_TYPE_DATETIME;
	//bind[1].buffer= (char *)&mt_status_datetime;
	unsigned long datetime_length = strlen(status_datetime);
	bind[1].buffer_type= MYSQL_TYPE_STRING;
	bind[1].buffer= (char *)status_datetime;
	bind[1].buffer_length = datetime_length;
	bind[1].is_null= 0;
	bind[1].length= 0;
	bind[1].length = &datetime_length;

	/* STRING PARAM */
	unsigned long description_length = strlen(status_description);
	bind[2].buffer_type = MYSQL_TYPE_STRING;
	bind[2].buffer = (char *)status_description;
	bind[2].buffer_length = 50;
	bind[2].is_null = 0;
	bind[2].length = &description_length;

	if ( mysql_stmt_bind_param(stmt_handler, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		return -1;
	}

	//supply data to be sent to the structure
	//mt_status_datetime.year

	if( (mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	int statusId = mysql_stmt_insert_id(stmt_handler);

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	return statusId;
}

//Search if job_id exists: -1 if not found, else found
int db_select_job_id(const int jobId) {
	MYSQL_STMT * preparedStatement = mysql_stmt_init(mysql);
	const char *stmt_str = "SELECT job_id FROM job WHERE job_id = ?";
	unsigned long stmt_length = strlen(stmt_str); // NOTE: what is the value of the string <-- the value hasn't been prepared yet...

	//prepare and bind before execution
	if(mysql_stmt_prepare(preparedStatement, stmt_str, stmt_length)) { //success = 0
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));

	/* INTEGER PARAM This is a number type, so there is no need to specify buffer_length */
	bind[0].buffer_type= MYSQL_TYPE_LONG;
	bind[0].buffer= (char *)&jobId;
	bind[0].is_null= 0;
	bind[0].length= 0;

	if ( mysql_stmt_bind_param(preparedStatement, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		return -1;
	}

	if( mysql_stmt_execute(preparedStatement) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_RES * result = mysql_stmt_result_metadata(preparedStatement); //for SELECT
	
	int resultId = -1;
	bool isNull = 0;
	bool error = 0;
	unsigned long resultLength = 0;

	MYSQL_BIND bindResult[1];
	memset(bindResult, 0, sizeof(bindResult));
	bindResult[0].buffer_type = MYSQL_TYPE_LONG;
	bindResult[0].buffer = (char *)&resultId;
	bindResult[0].is_null = &isNull;
	bindResult[0].length = &resultLength;
	bindResult[0].error = &error;

	if( mysql_stmt_bind_result(preparedStatement, bindResult) != 0) {
		fprintf(stderr, "Result bind failed: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		mysql_close(mysql);
		return -1;
	}

	//handle empty values, no results found
	int return_status;
	while ((return_status = mysql_stmt_fetch(preparedStatement)) == 0) {
		if (return_status == 1 || return_status == MYSQL_NO_DATA) {
			printf("No data left\n");
			break;
		}

		if (!isNull) {
		    printf("Found job_id: %d\n", resultId);
		}
	}

	mysql_free_result(result); // b/c stmt_result_metadata

	mysql_stmt_close(preparedStatement); // b/c stmt_init

	return resultId;
}

//If error return -1, else return rowsAffected
int db_delete_by_id(const char * stmt_string, const int id) {
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	unsigned long length = strlen(stmt_string);
	
	if (mysql_stmt_prepare(stmt_handler, stmt_string, length)) {
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));

	/* INTEGER PARAM */
	bind[0].buffer_type= MYSQL_TYPE_LONG;
	bind[0].buffer= (char *)&id;
	bind[0].is_null= 0;
	bind[0].length= 0;

	if ( mysql_stmt_bind_param(stmt_handler, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		return -1;
	}

	if( (mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	int rowsAffected = mysql_stmt_affected_rows(stmt_handler);

	return rowsAffected;
}


int db_select_resume(const char * resumeName) {
	MYSQL_STMT * preparedStatement = mysql_stmt_init(mysql);
	const char *stmt_str = "SELECT resume_id FROM resume WHERE resume_name = ?";
	unsigned long length = strlen(stmt_str); // NOTE: what is the value of the string <-- the value hasn't been prepared yet...

	//prepare and bind before execution
	if(mysql_stmt_prepare(preparedStatement, stmt_str, length)) { //success = 0
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));

	unsigned long str_length = strlen(resumeName);
	/* STRING PARAM */
	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (char *)resumeName;
	bind[0].buffer_length = 50;
	bind[0].is_null = 0;
	bind[0].length = &str_length;

	if ( mysql_stmt_bind_param(preparedStatement, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		return -1;
	}

	if( mysql_stmt_execute(preparedStatement) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_RES * result = mysql_stmt_result_metadata(preparedStatement); //for SELECT
	
	int resumeId = -1;
	bool isNull = 0;
	bool error = 0;
	unsigned long resultLength = 0;

	MYSQL_BIND bindResult[1];
	memset(bindResult, 0, sizeof(bindResult));
	bindResult[0].buffer_type = MYSQL_TYPE_LONG;
	bindResult[0].buffer = (char *)&resumeId;
	bindResult[0].is_null = &isNull;
	bindResult[0].length = &resultLength;
	bindResult[0].error = &error;

	if( mysql_stmt_bind_result(preparedStatement, bindResult) != 0) {
		fprintf(stderr, "Result bind failed: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		mysql_close(mysql);
		return -1;
	}

	//handle empty values, no results found
	int return_status;
	while ((return_status = mysql_stmt_fetch(preparedStatement)) == 0) {
		if (return_status == 1 || return_status == MYSQL_NO_DATA) {
			printf("No data left\n");
			break;
		}

		if(!isNull) {
		    printf("resume_id: %d\n", resumeId);
		}
	}

	mysql_free_result(result); // b/c stmt_result_metadata

	mysql_stmt_close(preparedStatement); // b/c stmt_init

	return resumeId;
}

int db_insert_resume(const char * resumeName) {
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "INSERT INTO resume (resume_name) VALUES (?);";
	unsigned long length = strlen(stmt_string);
	
	if (mysql_stmt_prepare(stmt_handler, stmt_string, length)) {
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));

	unsigned long param_str_length = strlen(resumeName);
	/* STRING PARAM */
	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (char *)resumeName;
	bind[0].buffer_length = 50;
	bind[0].is_null = 0;
	bind[0].length = &param_str_length;

	if ( mysql_stmt_bind_param(stmt_handler, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		return -1;
	}

	if( (mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	int resumeId = mysql_stmt_insert_id(stmt_handler);

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	return resumeId;
}

int db_insert_resume_full(const char * resumeName, const char * resumeLink) {
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "INSERT INTO resume (resume_name, resume_link) VALUES (?, ?);";
	unsigned long length = strlen(stmt_string);
	
	if (mysql_stmt_prepare(stmt_handler, stmt_string, length)) {
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_BIND bind[2];
	memset(bind, 0, sizeof(bind));

	unsigned long resume_name_length = strlen(resumeName);
	unsigned long resume_link_length = strlen(resumeLink); //I forgot to include this length earlier, TEST: see if a long dir link is stored. Example I tried earlier, software June 23 resume.
	/* STRING PARAM */
	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (char *)resumeName;
	bind[0].buffer_length = 100;
	bind[0].is_null = 0;
	bind[0].length = &resume_name_length;

	/* STRING PARAM */
	bind[1].buffer_type = MYSQL_TYPE_STRING;
	bind[1].buffer = (char *)resumeLink;
	bind[1].buffer_length = 500;
	bind[1].is_null = 0;
	bind[1].length = &resume_link_length;

	if ( mysql_stmt_bind_param(stmt_handler, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		return -1;
	}

	if( (mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	int resumeId = mysql_stmt_insert_id(stmt_handler);

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	return resumeId;
}

int db_insert_url(const int jobId, const char * websiteLink) {
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "INSERT INTO url (job_id, website_link) VALUES (?, ?);";
	unsigned long length = strlen(stmt_string);
	
	if (mysql_stmt_prepare(stmt_handler, stmt_string, length)) {
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	MYSQL_BIND bind[2];
	memset(bind, 0, sizeof(bind));

	unsigned long website_link_length = strlen(websiteLink);

	/* INTEGER PARAM This is a number type, so there is no need to specify buffer_length */
	bind[0].buffer_type= MYSQL_TYPE_LONG;
	bind[0].buffer= (char *)&jobId;
	bind[0].is_null= 0;
	bind[0].length= 0;

	/* STRING PARAM */
	bind[1].buffer_type = MYSQL_TYPE_STRING;
	bind[1].buffer = (char *)websiteLink;
	bind[1].buffer_length = 400;
	bind[1].is_null = 0;
	bind[1].length = &website_link_length;

	if ( mysql_stmt_bind_param(stmt_handler, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		return -1;
	}

	if( (mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return -1;
	}

	int linkId = mysql_stmt_insert_id(stmt_handler);

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	return linkId;
}
