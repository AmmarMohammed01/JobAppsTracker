/* jobapps-cli.c
 * PURPOSE:
 * - Print out usage instructions and accept input from user.
 * - Redirect user input to appropriate database function
*/

#include <stdio.h>
#include "jobapps-cli.h"
#include "db.h"

void jacli_setup() {
	db_open();
}

char jacli_menu() {
	// MENU START INFO
	printf("OPTIONS:\n");
	printf("--------\n");
	printf("1. Add company\n");
	printf("h. usage help\n");

	// REQUEST USER INPUT
	char option;
	scanf("%c", &option);
	printf("User option: %c\n", option);

	// MENU LOGIC
	if (option == '1') {
		char companyName[50];
		printf("Type company name to add:\n");
		fgets(companyName, sizeof(companyName), stdin);

		jacli_add_company(companyName);
	}
	else if (option == 'h') {
		jacli_help();
	}

	return option;
}

void jacli_help() {
	printf("You reached the help page\n");
}

void jacli_add_company(const char * companyName) {
	db_insert_company(companyName);
}

void jacli_close() {
	db_close();
}
