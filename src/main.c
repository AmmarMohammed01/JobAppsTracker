#include <stdio.h> //printf, scanf, fgets
#include "jobapps-cli.h" //jacli_menu, jacli_add_company

int main(int argc, char *argv[]) {
	printf("\n<<< jobapps-cli >>>\n\n");

	jacli_setup();
	jacli_menu(argc, argv);
	jacli_close();

	return 0;
}
