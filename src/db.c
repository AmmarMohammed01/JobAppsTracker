#include <mysql.h>
#include <stdbool.h> //bool
#include <stdlib.h> //getenv
#include <stdio.h> //fprintf
#include <string.h> //strlen

#include "db.h"
#include "field_types.h"

static MYSQL * mysql = NULL; //this var isn't visible outside file = static

//0 - success, 1 - init failed, 2 - real_connect failed
void db_open() {
	const char *host = getenv("MYSQL_HOST");
	const char *user = getenv("MYSQL_USER");
	const char *password = getenv("MYSQL_PASSWORD");
	const char *database = getenv("MYSQL_DATABASE");

	if (mysql_library_init(0, NULL, NULL)) {
		fprintf(stderr, "Could not initialize MySQL client library\n");
		exit(1);
	}

	if( (mysql = mysql_init(mysql)) == NULL) { //page 68
		fprintf(stderr, "Could not initialize mysql object\n");
		//return 1;
		exit(1);
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
		fprintf(stderr, "Failed to connect to database: Error: %s\n", mysql_error(mysql));
		printf("host: %s\n", host);
		//return 2;
		exit(1);
	}
	//return 0;

	printf("Database connection established!\n");
}

void db_close() {
	if(mysql != NULL) {
		mysql_close(mysql); //frees allocation of mysql pointer (from mysql_init)
		mysql = NULL;
	}

	mysql_library_end();
}

// RETURN ID, else 0 for ERROR
int db_select_company(const char * companyName) {
	//SELECT * FROM company WHERE company_name = "$(companyName)";

	int return_status;

	MYSQL_STMT * preparedStatement = mysql_stmt_init(mysql);
	const char *stmt_str = "SELECT company_id FROM company WHERE company_name = ?";
	unsigned long length = strlen(stmt_str); // NOTE: what is the value of the string <-- the value hasn't been prepared yet...

	//prepare and bind before execution
	if(mysql_stmt_prepare(preparedStatement, stmt_str, length)) { //success = 0
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		mysql_close(mysql);
		return 1;
	}

	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));

	unsigned long str_length = strlen(companyName);
	/* STRING PARAM */
	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (char *)companyName; // (char *)str_data
	bind[0].buffer_length = 50;
	bind[0].is_null = 0;
	bind[0].length = &str_length;

	if ( mysql_stmt_bind_param(preparedStatement, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(preparedStatement));
		mysql_stmt_close(preparedStatement);
		return 1;
	}

	if( (return_status = mysql_stmt_execute(preparedStatement)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(preparedStatement));
		fprintf(stderr, "Return Status: %d\n", return_status);
		mysql_stmt_close(preparedStatement);
		mysql_close(mysql);
		return 1;
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
		return 1;
	}

	//handle empty values, no results found
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

unsigned long long db_insert_company(const char * companyName) {
	//int insert_complete = 0; //1 if complete

	//INSERT INTO company (company_name) VALUES (?);
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "INSERT INTO company (company_name) VALUES (?);";
	unsigned long length = strlen(stmt_string);
	
	if (mysql_stmt_prepare(stmt_handler, stmt_string, length)) {
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return 1;
	}

	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));

	unsigned long param_str_length = strlen(companyName);
	/* STRING PARAM */
	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (char *)companyName; // (char *)str_data
	bind[0].buffer_length = 50;
	bind[0].is_null = 0;
	bind[0].length = &param_str_length;

	if ( mysql_stmt_bind_param(stmt_handler, bind) ) {
		fprintf(stderr, "BINDING FAILED. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		return 1;
	}

	if( (mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return 1;
	}

	unsigned long long affectedRows = mysql_stmt_affected_rows(stmt_handler); //same as uint64_t

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	//return insert_complete;
	return affectedRows;
}


int db_select_platform(const char * platformName) {
	//SELECT * FROM platform WHERE platform_name = "$(platformName)";

	int return_status;

	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "SELECT platform_id FROM platform WHERE platform_name = ?";
	unsigned long length = strlen(stmt_string); // NOTE: what is the value of the string <-- the value hasn't been prepared yet...

	//prepare and bind before execution
	if(mysql_stmt_prepare(stmt_handler, stmt_string, length)) { //success = 0
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return 1;
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
		return 1;
	}

	if( (return_status = mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		fprintf(stderr, "Return Status: %d\n", return_status);
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return 1;
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
		return 1;
	}

	//handle empty values, no results found
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

unsigned long long db_insert_platform(const char * platformName) {
	MYSQL_STMT * stmt_handler = mysql_stmt_init(mysql);
	const char * stmt_string = "INSERT INTO platform (platform_name) VALUES (?);";
	unsigned long length = strlen(stmt_string);
	
	if (mysql_stmt_prepare(stmt_handler, stmt_string, length)) {
		fprintf(stderr, "Error in preparing statement! ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return 1;
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
		return 1;
	}

	if( (mysql_stmt_execute(stmt_handler)) ) {
		fprintf(stderr, "Error in statement execution. ERR_MSG: %s\n", mysql_stmt_error(stmt_handler));
		mysql_stmt_close(stmt_handler);
		mysql_close(mysql);
		return 1;
	}

	unsigned long long affectedRows = mysql_stmt_affected_rows(stmt_handler); //same as uint64_t

	mysql_stmt_close(stmt_handler); // b/c stmt_init

	return affectedRows;
}

void db_select_all_companies() {
	const char * stmt_string = "SELECT * FROM company";
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

void db_select_all_platforms() {
	const char * stmt_string = "SELECT * FROM platform";
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
