/*
 * Author: Ammar Mohammed
 * Program: MySQL Client for Job Application Tracking Database
 * Date Started: Sep. 1, 2026
 * */
#include <mysql.h>
#include <stdio.h> //printf, fprintf
#include <stdlib.h> //getenv

int main() {
	const char *host = getenv("MYSQL_HOST");
	const char *user = getenv("MYSQL_USER");
	const char *password = getenv("MYSQL_PASSWORD");
	const char *database = getenv("MYSQL_DATABASE");

	if (mysql_library_init(0, NULL, NULL)) {
		fprintf(stderr, "Could not initialize MySQL client library\n");
		exit(1);
	}

	MYSQL * mysql = NULL;
	if( (mysql = mysql_init(mysql)) == NULL) { //page 68
		fprintf(stderr, "Could not initialize mysql object\n");
	}

	//const char *db_name = "name_was_here";
	//mysql_select_db(mysql, db_name); //already set in mysql_real_connect

     	//something about mysql_options() and using a my.cnf
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
	}

	//TEST CONNECTION
	/*
	const char * hostInfo = mysql_get_host_info(mysql);
	printf("%s\n", hostInfo); //"Localhost via UNIX socket
	*/

	const char *sql_statement = "SELECT * FROM platform";
	MYSQL_RES *result;
	unsigned int num_fields;
	unsigned int num_rows;

	if (mysql_query(mysql, sql_statement) ) //if not 0 (0 is success)
	{
		fprintf(stderr, "Error occured in running sql statement.\n");
	}
	else
	{
		result = mysql_store_result(mysql);
		if(result) //rows are found
		{
			num_fields = mysql_num_fields(result);
			//retrieve rows, then call mysql_free_result(result);
			printf("# of fields: %d\n", num_fields);

			MYSQL_FIELD *field;
			while((field = mysql_fetch_field(result))) //page 55
			{
				printf("field name %s\n", field->name);
			}

			//page 57-59
			MYSQL_ROW row;
			unsigned long *lengths;

			while((row = mysql_fetch_row(result)))
			{
				lengths = mysql_fetch_lengths(result); //IMPORTANT: VALID ONLY FOR CURRENT ROW
				for(unsigned int i = 0; i < num_fields; i++) {
					printf("[%.*s] ", (int) lengths[i], row[i] ? row[i] : "NULL");
				}
				printf("\n");
			}

			/*
			# of fields: 2
			field name platform_id
			field name platform_name
			[1] [LinkedIn]
			[2] [Indeed]
			*/

			mysql_free_result(result);
		}
		else
		{
			if(mysql_field_count(mysql) == 0)
			{
				//query doesn't return data - such as in not SELECT
				num_rows = mysql_affected_rows(mysql);
				printf("# of rows affected: %d\n", num_rows);
			}
			else //mysql_store_result should have returned data
			{
				fprintf(stderr, "Error: %s\n", mysql_error(mysql));
			}
		}
	}


	mysql_close(mysql); //frees allocation of mysql pointer (from mysql_init)

	mysql_library_end();

	return 0;
}
