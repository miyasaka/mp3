#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "postgres.h"
#include "libpq-fe.h" 

int main(int argc,char **argv)
{

    	char dbName[] = "komimi";
    	char loginName[] = "kokuyo";
    	char password[] = "kokuyo";
    	char sql[255];
	char wk[255];
    	int i, error;
    	PGconn *con;
    	PGresult *res;
    	char *row1;
	size_t length;

    /* DBとの接続 */
    	con = PQsetdbLogin(
          	NULL,
          	NULL,
          	NULL,
          	NULL,
          	dbName,  // db name
          	loginName,  // login name
          	password); // password
    	if ( PQstatus(con) == CONNECTION_BAD ) { /* 接続が失敗したときのエラー処理 */
        	fprintf(stderr,"Connection to database '%s' failed.\n",dbName);
        	fprintf(stderr,"%s",PQerrorMessage(con));
        	exit(1);
    	}

    	/* select文の発行 */
    	sprintf(sql,"set search_path to mp3_test");
    	res = PQexec(con,sql);
    	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        	fprintf(stderr,"%s",PQerrorMessage(con));
		PQclear(res);
    		PQfinish(con);
        	exit(1);
    	}
	PQclear(res);

	strcpy(wk,"SELECT count(*) FROM mp3_test' where song like '01 C%'");
	length = strlen(wk);
	fprintf(stdout,"len:[%d]\n",PQescapeStringConn(con,sql,wk,length,&error));
	fprintf(stdout,"sql:[%s]\n",sql);
	res = PQexec(con,sql);
    	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        	fprintf(stderr,"%s",PQerrorMessage(con));
		PQclear(res);
    		PQfinish(con);
        	exit(1);
    	}

   	fprintf(stdout,"Songs\n");
    	fprintf(stdout,"--------------------------------------\n");
    	for(i = 0; i < PQntuples(res) ;i++) {
        	fprintf(stdout,"[%s]\n",PQgetvalue(res,i,0));
    	}
    	PQclear(res);

    	PQfinish(con);

	return(0);
}  
