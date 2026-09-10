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
//static void jacli_add_job_listing(); // REPLACED by jacli_entry_add
static void jacli_add_company();
static void jacli_add_platform();
static void jacli_add_job_status();

//static void jacli_info(); //relevant info
//static void jacli_list_companies(); //REPLACED by jacli_company_list_all
//static void jacli_list_platforms(); //REPLACED by jacli_platform_list_all
//static void jacli_list_job_listings(); // REPLACED by jacli_entry_list_all

//static void jacli_list_status_types(); // NOT IMPLEMENTED YET
static void jacli_help();

//NEW DECLARATIONS
//JOB ENTRY
static void jacli_entry_add(const char * company_name, const char * platform_name, const char * job_title, const char * resume_name);
static void jacli_entry_list_all();
static void jacli_entry_remove(const int job_id);
//static void jacli_entry_list(); static void jacli_entry_update();

//COMPANY
static void jacli_company_list_all();

//PLATFORM
static void jacli_platform_list_all();

//RESUME
static void jacli_resume_list_all();

//STATUS
static void jacli_status_list_all();

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
			printf("job listing options: add, list, update, remove\n");
			return;
		}

		if (strcmp(arg_values[2], "add") == 0) {
			if(arg_count < 4) {
				printf("Please type the following: " \
				"\"company name\", "\
				"\"platform name\", "\
				"\"job title\", "\
				"\"resume name\""\
				"\n");
				return;
			}
			else if (arg_count == 7) {
				printf("Correct # of args for 'job entry add'\n");
				const char * company_name = arg_values[3];
				const char * platform_name = arg_values[4];
				const char * job_title = arg_values[5];
				const char * resume_name = arg_values[6];

				printf("c: %s, p: %s, j: %s, r: %s\n", company_name, platform_name, job_title, resume_name);

				jacli_entry_add(company_name, platform_name, job_title, resume_name);

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
	printf("\tjacli entry add company_name platform_name job_title resume_name\n");
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
	printf("\n");

	printf("jacli status\n");
	printf("\tjacli status list all\n");
	printf("\n");
}

static void jacli_entry_add(const char * company_name, const char * platform_name, const char * job_title, const char * resume_name) {
	//Search if company_name is in databse, get company_id
	int company_id = db_select_company(company_name);
	if (company_id == -1) {
		printf("Company not found, adding company to database...\n");
		company_id = db_insert_company(company_name);
		if(company_id == -1) {
			fprintf(stderr, "Error inserting company.\n");
			return;
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
			return;
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
			return;
		}
	}
	printf("Resume added w/ id = %d\n", resume_id);
	
	//Send query to database with "company_id, platform_id, and job_title"
	int jobId = db_insert_job_listing(company_id, platform_id, job_title, resume_id);
	if (jobId == -1) {
		fprintf(stderr, "Error adding job listing!\n");
		return;
	}
	printf("Job successfully added w/ id = %d\n", jobId);
	// also add job_status of applied with date specified by user

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

/*
ADD ERROR HANDLING FOR USER INPUT:
- job id not found (DONE)
- invalid date (MySQL throws a Foreign Key error)
- large description (assumption is that description is cut-off)

ADD SUCCESS OUTPUT WHEN STATUS IS ADDED (DONE)
*/
static void jacli_add_job_status() {
	char user_input[100]; //5ish for jobId, around 16 for datetime, 50 for status description
	printf("Type: job_id`status_datetime`status_description:\n");
	fgets(user_input, sizeof(user_input), stdin);

	user_input[strcspn(user_input, "\r\n")] = '\0';

	//Parse user input
	const char * delimeter = "`";

	char * jobId_str = strtok(user_input, delimeter);
	char * status_datetime = strtok(NULL, delimeter);
	char * status_description = strtok(NULL, delimeter);

	const int jobId = atoi(jobId_str);
	printf("jobId: %d\n", jobId);
	printf("datetime: %s\n", status_datetime);
	printf("description: %s\n", status_description);

	if (db_select_job_id(jobId) == -1) {
		fprintf(stderr, "Job ID %d NOT found.\n", jobId);
		return;
	}

	int statusId;
	if ( (statusId = db_insert_job_status(jobId, status_datetime, status_description)) == -1) {
		printf("Unable to add job status!\n");
		return;
	}
	printf("Successfully added job status with id: %d\n", statusId);
}

static void jacli_entry_list_all() {
	//print fields on top
	//replace ids with names

	//current output
	//[1] [4] [1] [Embedded Software Engineer] []

	//intended output
	//Job ID | Company Name | Platform Name | Job Title | Resume Name
	//1      | EmbCoExample | ExaJobFinder  | Embedded Software Engineer | NULL
	db_select_all("SELECT * FROM job_listing");
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
	//if ((rows_affected = db_delete_job_listing(job_id)) == -1 ) {
	if ((rows_affected = db_delete_by_id("DELETE FROM job_listing WHERE job_id = ?", job_id)) == -1 ) {
		printf("ERROR DELETING JOB_LISTING W/ JOB_ID: %d\n", job_id);
		return;
	}
	else {
		printf("Successfully deleted job id %d row.\n", job_id); // NOTE: I assume this will always be 1, change the (s) or this code later.
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

static void jacli_status_list_all() {
	db_select_all("SELECT * FROM status");
}

void jacli_close(int status) {
	db_close(status);
}
