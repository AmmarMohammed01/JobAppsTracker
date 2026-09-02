# mysql_config --cflags
# mysql_config --libs

INCLUDE_PATH = /opt/homebrew/Cellar/mysql/26.7.0/include/mysql/ # I was going to leave at ./include, but found out to use ./include/mysql because "mysql_config --cflags"
LIB_PATH = /opt/homebrew/Cellar/mysql/26.7.0/lib/

all: main

main: main.c
	gcc main.c -I$(INCLUDE_PATH) -L$(LIB_PATH) -lmysqlclient -o my_client

debug: main.c
	gcc -g main.c -I$(INCLUDE_PATH) -L$(LIB_PATH) -lmysqlclient -o my_client

clean:
	rm my_client

clean-d:
	rm my_client
	rm -rf my_client.dSYM
