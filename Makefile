# mysql_config --cflags
# mysql_config --libs

MYSQL_INCLUDE_PATH = /opt/homebrew/Cellar/mysql/26.7.0/include/mysql/ # I was going to leave at ./include, but found out to use ./include/mysql because "mysql_config --cflags"
MYSQL_LIB_PATH = /opt/homebrew/Cellar/mysql/26.7.0/lib/

INCLUDE_PATH = ./include/
SRC = src/main.c src/jobapps-cli.c src/db.c

all: main

main: src/main.c
	gcc $(SRC) -I$(INCLUDE_PATH) -I$(MYSQL_INCLUDE_PATH) -L$(MYSQL_LIB_PATH) -lmysqlclient -o build/jacli

debug: src/jobapps-cli.c
	gcc -g $(SRC) -I$(INCLUDE_PATH) -I$(MYSQL_INCLUDE_PATH) -L$(MYSQL_LIB_PATH) -lmysqlclient -o build/jacli

clean:
	rm build/jobapps-cli

clean-d:
	rm build/jobapps-cli
	rm -rf build/jobapps-cli.dSYM
