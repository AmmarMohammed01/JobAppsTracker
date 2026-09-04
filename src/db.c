#include <mysql.h>
#include <stdlib.h> //getenv
#include <stdio.h> //fprintf

#include "db.h"

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

void db_select_company(const char * companyName) {
	//SELECT * FROM company WHERE company_name = "$(companyName)";
	/*
	mysql_stmt_init();

	mysql_stmt_prepare();
	mysql_stmt_bind_param();

	mysql_stmt_execute();

	mysql_stmt_result_metadata(); //for SELECT
	
	mysql_stmt_fetch(); //call repeatedly until all rows fetched

	mysql_free_result(); // b/c stmt_result_metadata

	mysql_stmt_close(); // b/c stmt_init
	*/
}

void db_insert_company(const char * companyName) {
	//INSERT INTO company (company_name) VALUES ("$(comapnyName)");

}
