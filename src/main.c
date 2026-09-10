#include <stdio.h> //printf, scanf, fgets
#include "jobapps-cli.h" //jacli_menu, jacli_add_company

int main(int argc, char *argv[]) {
	int status = jacli_setup();
	if (status == 0) {
		jacli_menu(argc, argv);
	}
	jacli_close(status);

	return 0;
}
