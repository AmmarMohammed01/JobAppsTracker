# mysql_config --cflags
# mysql_config --libs

MYSQL_INCLUDE_PATH = /opt/homebrew/Cellar/mysql/26.7.0/include/mysql/ # I was going to leave at ./include, but found out to use ./include/mysql because "mysql_config --cflags"
MYSQL_LIB_PATH = /opt/homebrew/Cellar/mysql/26.7.0/lib/

INCLUDE_PATH = ./include/

all: main

main: src/jobapps-cli.c
	gcc src/jobapps-cli.c -I$(INCLUDE_PATH) -I$(MYSQL_INCLUDE_PATH) -L$(MYSQL_LIB_PATH) -lmysqlclient -o build/jacli

debug: src/jobapps-cli.c
	gcc -g src/jobapps-cli.c -I$(INCLUDE_PATH) -I$(MYSQL_INCLUDE_PATH) -L$(MYSQL_LIB_PATH) -lmysqlclient -o build/jacli

# jacli: src/jobapps-cli.c
#	gcc src/jobapps-cli.c -o build/jacli -Iinclude

clean:
	rm build/jobapps-cli

clean-d:
	rm build/jobapps-cli
	rm -rf build/jobapps-cli.dSYM
