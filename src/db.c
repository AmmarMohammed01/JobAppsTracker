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

/*
void db_select_all_old(const char * stmt_string) {
	// EXECUTE STATEMENT
	unsigned long length = strlen(stmt_string);
	if(mysql_query(mysql, stmt_string)) { //0 success
		fprintf(stderr, "Error occured executing query. ERR_MSG %s\n", mysql_error(mysql));
		return;
	}

	// STORE RESULT
	MYSQL_RES * result = mysql_store_result(mysql);

	// PRINT FIELD NAMES
	unsigned int num_fields = mysql_num_fields(result);
	MYSQL_FIELD * fields;
	fields = mysql_fetch_fields(result);

	for (unsigned int i = 0; i < num_fields; i++) {
		printf("%s ", fields[i].name);
	}
	printf("\n");

	// PRINT ROW VALUES
	MYSQL_ROW row;
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
*/

void db_select_all(const char *stmt_string)
{
	// EXECUTE STATEMENT
    if (mysql_query(mysql, stmt_string) != 0) {
        fprintf(stderr, "Query error: %s\n", mysql_error(mysql));
        return;
    }

	// STORE RESULT
    MYSQL_RES *result = mysql_store_result(mysql);

    if (result == NULL) {
        fprintf(stderr, "Result error: %s\n", mysql_error(mysql));
        return;
    }

    unsigned int num_fields = mysql_num_fields(result);
    MYSQL_FIELD *fields = mysql_fetch_fields(result);
    my_ulonglong num_rows = mysql_num_rows(result);

    size_t *widths = calloc(num_fields, sizeof(size_t));

    if (widths == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        mysql_free_result(result);
        return;
    }

    /* Start with the width of each column name. */
    for (unsigned int i = 0; i < num_fields; i++) {
        widths[i] = strlen(fields[i].name);
    }

    /*
     * First pass: find the widest value in every column.
     */
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != NULL) {
        unsigned long *lengths = mysql_fetch_lengths(result);

        for (unsigned int i = 0; i < num_fields; i++) {
            size_t length;

            if (row[i] != NULL) {
                length = lengths[i];
            } else {
                length = strlen("NULL");
            }

            if (length > widths[i]) {
                widths[i] = length;
            }
        }
    }

    /* Reset the result cursor for the second pass. */
    mysql_data_seek(result, 0);

    /* Print top separator. */
    printf("+");
    for (unsigned int i = 0; i < num_fields; i++) {
        printf("-%-*s-+", (int) widths[i], "");
    }
    printf("\n");

    /* Print column names. */
    printf("|");
    for (unsigned int i = 0; i < num_fields; i++) {
        printf(" %-*s |",
               (int) widths[i],
               fields[i].name);
    }
    printf("\n");

    /* Print separator. */
    printf("+");
    for (unsigned int i = 0; i < num_fields; i++) {
        printf("-%-*s-+", (int) widths[i], "");
    }
    printf("\n");

    /* Second pass: print the rows. */
    while ((row = mysql_fetch_row(result)) != NULL) {
        unsigned long *lengths = mysql_fetch_lengths(result);

        printf("|");

        for (unsigned int i = 0; i < num_fields; i++) {
            if (row[i] != NULL) {
                printf(" %-*.*s |",
                       (int) widths[i],
                       (int) lengths[i],
                       row[i]);
            } else {
                printf(" %-*s |",
                       (int) widths[i],
                       "NULL");
            }
        }

        printf("\n");
    }

    /* Print bottom separator. */
    printf("+");
    for (unsigned int i = 0; i < num_fields; i++) {
        printf("-%-*s-+", (int) widths[i], "");
    }
    printf("\n");

    free(widths);
    mysql_free_result(result);
}

//return statusId
int db_insert_job_status(const int jobId, const char * status_datetime, const char * status_description) {
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "INSERT INTO status (job_id, status_datetime, status_description) VALUES (?, ?, ?)";
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

void db_select_entries_by_date(const char * date) {
	/*
	select job_id from status WHERE status_datetime = ?; //datetime user gives
	^ use the job_ids to know which job entry to select

	select * from job where job_id = (select job_id from status where status_datetime = ?);

	select * from job where job_id = (select job_id from status where status_datetime BETWEEN '2026-08-27 00:00' AND '2026-08-27 23:59');
	ERROR 1242 (21000): Subquery returns more than 1 row

	I got it working, statement below:
	SELECT *
	  FROM job as J, status as S
	 WHERE J.job_id = S.job_id AND S.status_datetime BETWEEN '? 00:00' AND '? 23:59';

	SELECT J.job_id, J.company_id, J.job_title, S.status_datetime
	  FROM job as J, status as S
	 WHERE J.job_id = S.job_id AND S.status_datetime BETWEEN '? 00:00' AND '? 23:59';
	*/
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "SELECT * FROM job as J, status as S WHERE J.job_id = S.job_id AND S.status_datetime BETWEEN '? 00:00' AND '? 23:59'";
	unsigned long length = strlen(stmt_string);

	if (mysql_stmt_prepare(stmt_handler, stmt_string, length)) {
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return;
	}

	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));

	unsigned long datetime_str_length = strlen(date);

	/* STRING PARAM */
	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (char *)date;
	bind[0].buffer_length = 400;
	bind[0].is_null = 0;
	bind[0].length = &datetime_str_length;

	if ( mysql_stmt_bind_param(stmt_handler, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		return;
	}

	if( (mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return;
	}

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	return;
}

void db_select_status_by_job_id(const int job_id) {
	//TODO: NEED TO HANDLE SCENARIO WHERE JOB_ID IS NOT FOUND IN STATUS TABLE
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "select * from status where job_id = ? ORDER BY status_datetime";
	unsigned long stmt_length = strlen(stmt_string);

	if (mysql_stmt_prepare(stmt_handler, stmt_string, stmt_length)) {
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return;
	}

	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));

	/* INTEGER PARAM */
	bind[0].buffer_type= MYSQL_TYPE_LONG;
	bind[0].buffer= (char *)&job_id;
	bind[0].is_null= 0;
	bind[0].length= 0;

	if ( mysql_stmt_bind_param(stmt_handler, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		return;
	}

	if( (mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return;
	}

	MYSQL_RES * result = mysql_stmt_result_metadata(stmt_handler); //for SELECT

	MYSQL_BIND bindResult[4];
	memset(bindResult, 0, sizeof(bindResult));

	int status_id;
	int job_id_result;
	char datetime[20];
	char description[50];
	bool is_null[4];
	unsigned long length[4];

	/* INTEGER PARAM */
	bindResult[0].buffer_type= MYSQL_TYPE_LONG;
	bindResult[0].buffer= (char *)&status_id;
	bindResult[0].is_null= &is_null[0];
	bindResult[0].length= &length[0];

	/* INTEGER PARAM */
	bindResult[1].buffer_type= MYSQL_TYPE_LONG;
	bindResult[1].buffer= (char *)&job_id_result;
	bindResult[1].is_null= &is_null[1];
	bindResult[1].length= &length[1];

	bindResult[2].buffer_type= MYSQL_TYPE_STRING;
	bindResult[2].buffer= (char *)datetime;
	bindResult[2].buffer_length = 20;
	bindResult[2].is_null= &is_null[2];
	bindResult[2].length= &length[2];

	bindResult[3].buffer_type= MYSQL_TYPE_STRING;
	bindResult[3].buffer= (char *)description;
	bindResult[3].buffer_length = 50;
	bindResult[3].is_null= &is_null[3];
	bindResult[3].length= &length[3];

	if( mysql_stmt_bind_result(stmt_handler, bindResult) != 0) {
		fprintf(stderr, "Result bind failed: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return;
	}

	//page 150
	/* Now buffer all results to client (optional step) */
	if (mysql_stmt_store_result(stmt_handler))
	{
		fprintf(stderr, " mysql_stmt_store_result() failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(stmt_handler));
		return; //exit(0);
	}

	/* Fetch all rows */
	int row_count= 0;

	fprintf(stdout, "Fetching results ...\n");
	while (!mysql_stmt_fetch(stmt_handler))
	{
		row_count++;
		fprintf(stdout, " row %d\n", row_count);
		/* column 1 */
		fprintf(stdout, " status_id (integer) : ");
		if (is_null[0])
			fprintf(stdout, " NULL\n");
		else
			fprintf(stdout, " %d(%ld)\n", status_id, length[0]);
		/* column 2 */
		fprintf(stdout, " job_id (integer) : ");
		if (is_null[1])
			fprintf(stdout, " NULL\n");
		else
			fprintf(stdout, " %d(%ld)\n", job_id_result, length[1]);
		/* column 3 */
		fprintf(stdout, " status_datetime (datetime) : ");
		if (is_null[2])
			fprintf(stdout, " NULL\n");
		else
			fprintf(stdout, " %s(%ld)\n", datetime, length[2]);
		/* column 4 */
		fprintf(stdout, " status_description (string): ");
		if (is_null[3])
			fprintf(stdout, " NULL\n");
		else
			fprintf(stdout, " %s(%ld)\n", description, length[3]);
		fprintf(stdout, "\n");
	}

	/* Validate rows fetched */
	fprintf(stdout, " total rows fetched: %d\n", row_count);
	if (row_count != 2)
	{
		fprintf(stderr, " MySQL failed to return all rows\n");
		exit(0);
	}

	/* Free the prepared result metadata */
	mysql_free_result(result);

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	return;
}
