#include <mysql.h>
#include <stdio.h> //printf, scanf, fgets
#include <stdlib.h> //getenv
#include "jobapps-cli.h"

int main() {
	char option = jacli_menu();
	if (option == '1') {
		char companyName[50];
		printf("Type company name to add:\n");
		fgets(companyName, sizeof(companyName), stdin);

		jacli_add_company(companyName);
	}
	else if (option == 'h') {
		printf("You reached the help page\n");
	}

	return 0;
}

char jacli_menu() {
	printf("<<< jobapps-cli >>>\n");
	printf("For help type: h.\n");

	printf("OPTIONS:\n");
	printf("1. Add company\n");

	/*
	char input[50];
	fgets(input, sizeof(input), stdin); //include the '\n'
	printf("Your input: %s\n", input);
	*/

	char option;
	scanf("%c", &option);
	printf("User option: %c\n", option);

	return option;
}

void jacli_add_company(const char * companyName) {

}
