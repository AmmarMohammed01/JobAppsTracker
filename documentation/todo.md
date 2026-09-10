# TODO

- add job listing
    - find company id from company name
    - find platform id from platform name
    - assemble insert: company id, platform id, job title
    - create a job status w/ date and description of "applied"

- track resume jobs are applied with

- allow users to categorize jobs by industry/field type

## 2026-09-06
Currently implemented:
- find company id from company name
- find platform id from platform name

## 2026-09-08

I need to write out the workflow so I can understand what need to be done:

### ADD COMMANDS

```bash
job entry add company platform title resume

job entry add "TheCorp" "ThePlat" "Backend Engineer" "~/resume/TheResume.pdf"
job entry add "TheCorp" "ThePlat" "Backend Engineer" "TheResume"
```

```text
First, user should be able to add a job listing.
A job listing has:
- company name --> company_id from database
- platform name --> platform_id from database
- job title
- optional to enter resume name

- A job_id is automatically generated

If company name and platform name are not found in the
database, the system should automatically add them to 
the company and platform tables respectively.

A job status should be automatically created immediately with the following information:
- status datetime = today
- status description = "Applied"
- job_id to target the new job_listing (rename to 'job' table)

- A status_id is automatically generated
```

```bash
job status add job_id status_datetime status_description

job status add 49 2026-09-08 18:49 Applied
job status add 49 2026-09-08 Applied
```

```text
Secondly, the user should be able to add job statuses
to keep track on updates related to the job_listing

Creating a job status:
- status datetime = any date and time "YYYY-MM-DD HH:MM"
- status description = "Applied", "Phone Screening", "Interview", "Rejected", "Offer"
- job_id to target the new job_listing

- A status_id is automatically generated
```

```bash
job resume add resume_name resume_file_location

job resume add TheResume ~/resume/TheResume.pdf
```

```text
Thirdly, the user should be able to keep track of
which resume they used to apply to a job listing.

Creating a resume entry: [NEED TO IMPLEMENT]
- resume name (required)
- resume file location (optional)

- A resume_id is automatically generated

After a resume is created, the user can specify which
job listing he applied for.
[NEED TO IMPLEMENT]
```

```bash
job url add job_id website_link

job url add 49 example://ThePlat.example/TheCorp/SoftwareEngineer
```

```text
Fourthly, the user should be able to keep track of
the website link navigated to apply to the job.
The user can also add additional links related to the job listing.

Creating a URL entry:
- job_id to target the job listing
- website_link

- A url_id will automatically be generated
```

```bash
job category add job_id category_name
job category add 49 Software

job ?group? add
```

```text
Optionally, the user can group job entires into categories.

Category Examples:
- Software Jobs
- Hardware Jobs
- Embedded Jobs
- Marketing Jobs
- ...

Create a category entry:
```

### LIST COMMANDS
Get a list of job entries
```bash
job entry list all
job entry list job_id

job entry list 49
```

```bash
job entry list range

job entry list "today"
job entry list "yesterday"

job entry list "2026-09-09"
job entry list "9/9/2026" # MM/DD/YYYY, MM/DD/YY, DD/MM/YY, DD/MM/YYYY

job entry list "this week" | "current week"
job entry list "last week"

job entry list "this month"
job entry list "last month"

job entry list "Jan" | "Jan." | "Jan. 2026" | "January 2026"

job entry list "this year"
job entry list "last year"
job entry list year
job entry list "2026"

job entry list "2026-08-31 to 2026-09-09"

```

Get a list of resume entries
```bash
job resume list all
job resume list resume_id       # get by id
job resume list resume_name     # get by name

job resume list sort interviews "high to low"
job resume list sort interviews "low to high"

job resume list sort interviews "ignored" | "no interviews" # just applied, no other status available | just applied, then rejected
```

Get a list of companies applied to
```bash
job company list all    # show times applied, times interviewed, times rejected
# maybe show frequency applied, how often user applies to it
```

Get a list of platforms used:
```bash
job platform list all   # see how many times platform was used, how many times application made it to interview stage
```

Get a list of jobs by categories
```bash
job category list "marketing"
job category list "software engineering"
job category list "embedded engineering"
job category list "hardware engineering"
```

### ADDITIONAL COMMANDS
Could consider udpate and delete commands
