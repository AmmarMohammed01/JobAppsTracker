/* jobapps-cli.c
 * PURPOSE:
 * - Print out usage instructions and accept input from user.
 * - Redirect user input to appropriate database function
*/

#include <stdio.h>
#include <stdlib.h> //atoi
#include <string.h> //strcspn, strtok, strcmp
#include "jobapps-cli.h"
#include "db.h"

//private declarations. file-scope.
static void jacli_add_company();
static void jacli_add_platform();
//static void jacli_add_job_status(); //REPLACED BY jacli_status_add

static void jacli_help();

//NEW DECLARATIONS
//JOB ENTRY
static int jacli_entry_add(const char * company_name, const char * platform_name, const char * job_title, const char * resume_name);
static void jacli_entry_list_all();
static void jacli_entry_remove(const int job_id);
//static void jacli_entry_list(); static void jacli_entry_update();

//COMPANY
static void jacli_company_list_all();

//PLATFORM
static void jacli_platform_list_all();

//RESUME
static void jacli_resume_list_all();
static void jacli_resume_add(const char * resume_name, const char * resume_link);
static void jacli_resume_remove(const int resume_id);

//STATUS
static void jacli_status_list_all();
static void jacli_status_add(const int job_id, const char * status_datetime, const char * status_description);

//URL
static void jacli_url_list_all();
static void jacli_url_add(const int job_id, const char * website_link);

static void clear_screen(void) {
	//printf("\033[2J\033[H");
	//printf("\033[H\033[J");
	for (int i = 0; i < 50; i++) {
	    putchar('\n');
	}
	fflush(stdout);
}

int jacli_setup() {
	printf("<<< jobapps-cli >>>\n\n");
	int status = db_open();
	return status;
}

void jacli_menu(int arg_count, char *arg_values[]) {
	clear_screen();
	printf("\n<<< jobapps-cli >>>\n\n");

	// for(int i = 0; i < arg_count; i++) { printf("Arg #%d: %s\n", i, arg_values[i]); }

	//check if there are enough arguments!
	if (arg_count < 2) {
		printf("Read command help\n");
		jacli_help();
		return;
	}

	if (strcmp(arg_values[1], "help") == 0) {
		jacli_help();
		return;
	}

	else if (strcmp(arg_values[1], "entry") == 0) {
		if(arg_count < 3) {
			printf("job listing options: add, list, remove\n");
			return;
		}

		if (strcmp(arg_values[2], "add") == 0) {
			if(arg_count <= 3) {
				printf("Please type the following: " \
				"\"company name\", "\
				"\"job title\", "\
				"\"website link\", "\
				"\"resume name\""\
				"\"status_datetime\""\
				"\"platform name\", "\
				"\n");
				return;
			}
			else if (arg_count == 9) {
				printf("Correct # of args for 'job entry add'\n");
				const char * company_name = arg_values[3];
				const char * job_title = arg_values[4];
				const char * website_link = arg_values[5];
				const char * resume_name = arg_values[6];
				const char * status_datetime = arg_values[7];
				const char * platform_name = arg_values[8];

				printf("c: %s, p: %s, j: %s, r: %s\n", company_name, platform_name, job_title, resume_name);

				int returned_job_id = jacli_entry_add(company_name, platform_name, job_title, resume_name);

				if (returned_job_id == -1) {
					printf("Avoiding the addition of URL and status datetime since job was not added.\n");
					return;
				}

				jacli_url_add(returned_job_id, website_link);
				jacli_status_add(returned_job_id, status_datetime, "Applied");

				return;
			}
			else {
				printf("Invalid number of arguments for 'job entry add'\n");
				return;
			}
		}
		else if (strcmp(arg_values[2], "list") == 0) {
			if (arg_count == 4) {
				if (strcmp(arg_values[3], "all") == 0) {
					jacli_entry_list_all();
					return;
				}
				else {
					printf("'job entry list' undefined option\n");
					return;
				}
			}
		}
		else if (strcmp(arg_values[2], "remove") == 0) {
			if (arg_count == 4) {
				int job_id = atoi(arg_values[3]);
				jacli_entry_remove(job_id);
				return;
			}
			else {}
		}
	}

	else if (strcmp(arg_values[1], "company") == 0) {
		if (strcmp(arg_values[2], "list") == 0) {
			if (arg_count == 4) {
				if (strcmp(arg_values[3], "all") == 0) {
					jacli_company_list_all();
					return;
				}
				else {
					printf("'job company list' undefined option\n");
					return;
				}
			}
		}
	}

	else if (strcmp(arg_values[1], "platform") == 0) {
		if (strcmp(arg_values[2], "list") == 0) {
			if (arg_count == 4) {
				if (strcmp(arg_values[3], "all") == 0) {
					jacli_platform_list_all();
					return;
				}
				else {
					printf("'job platform list' undefined option\n");
					return;
				}
			}
		}

	}
	else if (strcmp(arg_values[1], "resume") == 0) {
		if (strcmp(arg_values[2], "list") == 0) {
			if (arg_count == 4) {
				if (strcmp(arg_values[3], "all") == 0) {
					jacli_resume_list_all();
					return;
				}
				else {
					printf("'job resume list' undefined option\n");
					return;
				}
			}
		}
		else if (strcmp(arg_values[2], "add") == 0) {
			if (arg_count == 5) {
				const char * resume_name = arg_values[3];
				const char * resume_link = arg_values[4];
				jacli_resume_add(resume_name, resume_link);
			}
			else {
				fprintf(stderr, "Invalid number of arguments for 'job resume add [resume_name] [resume_link]'\n");
			}
		}

	}
	else if (strcmp(arg_values[1], "status") == 0) {
		if (strcmp(arg_values[2], "list") == 0) {
			if (arg_count == 4) {
				if (strcmp(arg_values[3], "all") == 0) {
					jacli_status_list_all();
					return;
				}
				else {
					printf("'job status list' undefined option\n");
					return;
				}
			}
		}
		else if (strcmp(arg_values[2], "add") == 0) {
			if (arg_count == 6) {
				const char * job_id_str = arg_values[3];
				const int job_id = atoi(job_id_str); // NOTE: Need to check if atoi result is an INT!
				const char * status_datetime = arg_values[4];
				const char * status_description = arg_values[5];
				jacli_status_add(job_id, status_datetime, status_description);
			}
			else {
				printf("'job status add' undefined format\n");
				return;
			}
		}
	}
	else if (strcmp(arg_values[1], "url") == 0) {
		if (strcmp(arg_values[2], "list") == 0) {
			if (arg_count == 4) {
				if (strcmp(arg_values[3], "all") == 0) {
					jacli_url_list_all();
					return;
				}
				else {
					printf("'job url list' undefined option\n");
					return;
				}
			}
		}
		else if (strcmp(arg_values[2], "add") == 0) {
			if (arg_count == 5) {
				const char * job_id_str = arg_values[3];
				const int job_id = atoi(job_id_str); // NOTE: Need to check if atoi result is an INT!
				const char * website_link = arg_values[4];
				jacli_url_add(job_id, website_link);
			}
		}
	}
	else {
		printf("ERROR: Unable to interpret command\n");
		printf("For avaiable commands please refer to 'job help'\n");
		return;
	}
}

static void jacli_help() {
	printf("Current commands available:\n");
	printf("jacli entry\n");
	printf("\tjacli entry list all\n");
	printf("\tjacli entry add company_name job_title website_link, status_datetime, resume_name platform_name\n");
	printf("\tjacli entry remove job_id\n");
	printf("\n");

	printf("jacli company\n");
	printf("\tjacli company list all\n");
	printf("\n");

	printf("jacli platform\n");
	printf("\tjacli platform list all\n");
	printf("\n");

	printf("jacli resume\n");
	printf("\tjacli resume list all\n");
	printf("\tjacli resume add resume_name resume_link\n");
	//printf("\tjacli resume remove resume_id\n"); //TODO:
	printf("\n");

	printf("jacli status\n");
	printf("\tjacli status list all\n");
	printf("\tjacli status add job_id status_datetime status_description\n");
	//printf("\tjacli status remove status_id\n"); //TODO:
	printf("\n");

	printf("jacli url\n");
	printf("\tjacli url list all\n");
	printf("\tjacli url add job_id website_link\n");
	//printf("\tjacli url remove link_id\n"); //TODO:
	printf("\n");
}

static int jacli_entry_add(const char * company_name, const char * platform_name, const char * job_title, const char * resume_name) {
	//Search if company_name is in databse, get company_id
	int company_id = db_select_company(company_name);
	if (company_id == -1) {
		printf("Company not found, adding company to database...\n");
		company_id = db_insert_company(company_name);
		if(company_id == -1) {
			fprintf(stderr, "Error inserting company.\n");
			return -1;
		}
	}
	printf("Company added w/ id = %d\n", company_id);
	
	//Search if platform_name is in database, get platform_id
	int platform_id = db_select_platform(platform_name);
	if (platform_id == -1) {
		printf("Platform not found, adding platform to database...\n");
		platform_id = db_insert_platform(platform_name);
		if(platform_id == -1) {
			fprintf(stderr, "Error inserting platform.\n");
			return -1;
		}
	}
	printf("Platform added w/ id = %d\n", platform_id);

	//Search if platform_name is in database, get platform_id
	int resume_id = db_select_resume(resume_name);
	if (resume_id == -1) {
		printf("Resume not found, adding resume to database...\n");
		resume_id = db_insert_resume(resume_name);
		if(resume_id == -1) {
			fprintf(stderr, "Error inserting resume.\n");
			return -1;
		}
	}
	printf("Resume added w/ id = %d\n", resume_id);
	
	//Send query to database with "company_id, platform_id, and job_title"
	int jobId = db_insert_job_listing(company_id, platform_id, job_title, resume_id);
	if (jobId == -1) {
		fprintf(stderr, "Error adding job listing!\n");
		return -1;
	}
	printf("Job successfully added w/ id = %d\n", jobId);
	// also add job_status of applied with date specified by user
	return jobId;
}

static void jacli_add_company() {
	// Get input
	char companyName[50];
	printf("Type company name to add:\n");
	fgets(companyName, sizeof(companyName), stdin);

	// Prepare input
	companyName[strcspn(companyName, "\r\n")] = '\0'; //newline added by fgets can cause problems searching for value

	// Send input to see if exists
	int companyId = db_select_company(companyName);
	if (companyId == -1) {
		fprintf(stderr, "JACLI: Company ID for '%s' does not exist in database.\n", companyName);
		unsigned long long affectedRows = db_insert_company(companyName);
		printf("%llu company added.\n", affectedRows);
	}
	else {
		printf("JACLI: Found Company ID: %d\n", companyId);
	}

	int platformId = db_select_company(companyName);
	if (companyId == -1) {
		fprintf(stderr, "JACLI: Company ID for '%s' does not exist in database.\n", companyName);
		unsigned long long affectedRows = db_insert_company(companyName);
		printf("%llu company added.\n", affectedRows);
	}
	else {
		printf("JACLI: Found Company ID: %d\n", companyId);
	}

}

static void jacli_add_platform() {
	// Get input
	char platformName[50];
	printf("Type platform name to add:\n");
	fgets(platformName, sizeof(platformName), stdin);

	// Prepare input
	platformName[strcspn(platformName, "\r\n")] = '\0'; //newline added by fgets can cause problems searching for value

	// Send input to see if exists
	int platformId = db_select_platform(platformName);
	if (platformId == -1) {
		fprintf(stderr, "JACLI: Platform ID for '%s' does not exist in database.\n", platformName);
		unsigned long long affectedRows = db_insert_platform(platformName);
		printf("%llu platform added.\n", affectedRows);
	}
	else {
		printf("JACLI: Found Platform ID: %d\n", platformId);
	}

}

static void jacli_entry_list_all() {
	//print fields on top
	//replace ids with names

	//current output
	//[1] [4] [1] [Embedded Software Engineer] []

	//intended output
	//Job ID | Company Name | Platform Name | Job Title | Resume Name
	//1      | EmbCoExample | ExaJobFinder  | Embedded Software Engineer | NULL
	db_select_all("SELECT * FROM job");
}

static void jacli_entry_remove(const int job_id) {
	if (db_select_job_id(job_id) == -1) {
		fprintf(stderr, "Job ID %d NOT found.\n", job_id);
		return;
	}
	else {
		printf("Job ID #%d found\n", job_id);
	}

	int rows_affected;
	if ((rows_affected = db_delete_by_id("DELETE FROM job WHERE job_id = ?", job_id)) == -1 ) {
		printf("ERROR DELETING JOB W/ JOB_ID: %d\n", job_id);
		return;
	}
	else {
		printf("Successfully deleted job id %d row.\n", job_id);
	}
}

static void jacli_company_list_all() {
	db_select_all("SELECT * FROM company");
}

static void jacli_platform_list_all() {
	db_select_all("SELECT * FROM platform");
}

static void jacli_resume_list_all() {
	db_select_all("SELECT * FROM resume");
}

static void jacli_resume_add(const char * resume_name, const char * resume_link) {
	const int resume_id = db_insert_resume_full(resume_name, resume_link);
	if(resume_id == -1) {
		fprintf(stderr, "Error inserting resume.\n");
		return;
	}
	printf("Resume added w/ id = %d\n", resume_id);
}

//TEST: test this feature w/ maybe archived resume
static void jacli_resume_remove(const int resume_id) {
	int rows_affected;
	if ((rows_affected = db_delete_by_id("DELETE FROM resume WHERE resume_id = ?", resume_id)) == -1 ) {
		printf("ERROR DELETING RESUME W/ RESUME_ID: %d\n", resume_id);
		return;
	}
	else {
		printf("Successfully deleted resume id %d row.\n", resume_id);
	}
}

static void jacli_status_list_all() {
	db_select_all("SELECT * FROM status");
}

static void jacli_status_add(const int job_id, const char * status_datetime, const char * status_description) {
	if (db_select_job_id(job_id) == -1) {
		fprintf(stderr, "Job ID %d NOT found.\n", job_id);
		return;
	}

	int statusId;
	if ( (statusId = db_insert_job_status(job_id, status_datetime, status_description)) == -1) {
		printf("Unable to add job status!\n");
		return;
	}
	printf("Successfully added job status with id: %d\n", statusId);
}

static void jacli_url_list_all() {
	db_select_all("SELECT * FROM url");
}

static void jacli_url_add(const int job_id, const char * website_link) {
	const int link_id = db_insert_url(job_id, website_link);
	if(link_id == -1) {
		fprintf(stderr, "Error inserting url.\n");
		return;
	}
	printf("URL added w/ id = %d\n", link_id);
}

void jacli_close(int status) {
	db_close(status);
}
