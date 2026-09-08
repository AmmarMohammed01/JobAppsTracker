/* jobapps-cli.c
 * PURPOSE:
 * - Print out usage instructions and accept input from user.
 * - Redirect user input to appropriate database function
*/

#include <stdio.h>
#include <stdlib.h> //atoi
#include <string.h> //strcspn, strtok
#include "jobapps-cli.h"
#include "db.h"

//private declarations. file-scope.
static void jacli_add_job_listing();
static void jacli_add_company();
static void jacli_add_platform();
static void jacli_add_job_status();

//static void jacli_info(); //relevant info
static void jacli_list_companies();
static void jacli_list_platforms();
static void jacli_list_job_listings();

static void jacli_list_status_types(); // NOT IMPLEMENTED YET
static void jacli_help();

void jacli_setup() {
	db_open();
}

void jacli_menu() {
	// MENU START INFO
	printf("OPTIONS:\n");
	printf("--------\n");
	printf("1. Add job listing\n");
	printf("2. Add company\n");
	printf("3. Add platform\n");
	printf("4. List all companies\n");
	printf("5. List all platforms\n");
	printf("6. List all job listings\n");
	printf("7. Add job status\n");
	printf("h. usage help\n");

	// REQUEST USER INPUT
	char option[10];
	//scanf("%c", &option);
	fgets(option, sizeof(option), stdin);
	printf("User option: %s\n", option);

	// MENU LOGIC
	if (option[0] == '1') { //add job listing
		jacli_add_job_listing();
	}
	else if (option[0] == '2') { // add company
		jacli_add_company();
	}
	else if (option[0] == '3') { // add platform
		jacli_add_platform();
	}
	else if (option[0] == '4') { // list all companies
		jacli_list_companies();
	}
	else if (option[0] == '5') { // list all platforms
		jacli_list_platforms();
	}
	else if (option[0] == '6') { // list all platforms
		jacli_list_job_listings();
	}
	else if (option[0] == '7') { // list all platforms
		jacli_add_job_status();
	}
	else if (option[0] == 'h') { //help
		jacli_help();
	}
}

static void jacli_help() {
	printf("You reached the help page\n");
}

// user_input should look like:
// - company name, platform name, job title, and date
// - company name, platform name, job title (date automatically set to today)
// each of the fields should be comma separated
static void jacli_add_job_listing() {
	char user_input[255]; //100 company_name, 100 job_title, 50 platform_name
	printf("Type: company_name`platform_name`job_title:\n");
	fgets(user_input, sizeof(user_input), stdin);

	user_input[strcspn(user_input, "\r\n")] = '\0';

	//Parse user input
	const char * delimeter = "`";

	char * companyName = strtok(user_input, delimeter);
	char * platformName = strtok(NULL, delimeter);
	char * jobTitle = strtok(NULL, delimeter);

	printf("c: %s\n", companyName);
	printf("p: %s\n", platformName);
	printf("j: %s\n", jobTitle);

	//Search if company_name is in databse, get company_id
	int companyId = db_select_company(companyName);
	if (companyId == -1) {
		printf("Company not found, adding company to database...\n");
		companyId = db_insert_company(companyName);
		if(companyId == -1) {
			fprintf(stderr, "Error inserting company.\n");
			return;
		}
	}
	printf("Company added w/ id = %d\n", companyId);
	
	//Search if platform_name is in database, get platform_id
	int platformId = db_select_platform(platformName);
	if (platformId == -1) {
		printf("Platform not found, adding platform to database...\n");
		platformId = db_insert_platform(platformName);
		if(platformId == -1) {
			fprintf(stderr, "Error inserting platform.\n");
			return;
		}
	}
	printf("Platform added w/ id = %d\n", platformId);
	
	//Send query to database with "company_id, platform_id, and job_title"
	int jobId = db_insert_job_listing(companyId, platformId, jobTitle);
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
- job id not found
- invalid date
- large description

ADD SUCCESS OUTPUT WHEN STATUS IS ADDED
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

	db_insert_job_status(jobId, status_datetime, status_description);
}

static void jacli_list_companies() {
	db_select_all("SELECT * FROM company");
}

static void jacli_list_platforms() {
	db_select_all("SELECT * FROM platform");
}

static void jacli_list_job_listings() {
	//print fields on top
	//replace ids with names

	//current output
	//[1] [4] [1] [Embedded Software Engineer] []

	//intended output
	//Job ID | Company Name | Platform Name | Job Title | Resume Name
	//1      | EmbCoExample | ExaJobFinder  | Embedded Software Engineer | NULL
	db_select_all("SELECT * FROM job_listing");
}

/*
static void jacli_list_status_types() {
	//db_select_all_status_descriptions();
}
*/

void jacli_close() {
	db_close();
}
