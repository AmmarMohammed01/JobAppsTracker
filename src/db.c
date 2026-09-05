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

void db_insert_company(const char * companyName) {
	//INSERT INTO company (company_name) VALUES (?);
	const char * insert_stmt = "INSERT INTO company (company_name) VALUES (?);";

}
