#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mp3tag.h"
#include "postgres.h"
#include "libpq-fe.h" 

int
InsertPostgres(ID3V2_20_TAG *id3v220tag, BIT_RATE_INFO *bitrate, char *mp3_file)
{
	ID3V2_20_TAG *wk_id3v220tag;
    	char dbName[] = "komimi";
    	char loginName[] = "kokuyo";
    	char password[] = "kokuyo";
	unsigned char album[128];
	unsigned char artist[128];
	unsigned char janle[64];
	unsigned char artwork[256];
	unsigned char mp3_filename[256];
	unsigned char title[128];
	unsigned char year[5];
	unsigned char track_no[12];
	unsigned char bit_rate[5];
	unsigned char time[10];
    	unsigned char seq_no[10];
	size_t length;
    	char sql[256];
    	int i, cnt, error;
    	PGconn *con;
    	PGresult *res;

    	/* connect Database */
    	con = PQsetdbLogin(
          	NULL,
          	NULL,
          	NULL,
          	NULL,
          	dbName,  // db name
          	loginName,  // login name
          	password); // password
    	if ( PQstatus(con) == CONNECTION_BAD ) {
        	fprintf(stderr,"Connection to database '%s' failed.\n",dbName);
        	fprintf(stderr,"%s",PQerrorMessage(con));
        	exit(1);
    	}

	wk_id3v220tag = id3v220tag;
	for(i=0; i<TAG_ID_CNT;i++,wk_id3v220tag++){
		switch(i){
			case 0:  // TAL Album
				length = strlen(wk_id3v220tag->encode_contents);
				PQescapeStringConn(con,album,wk_id3v220tag->encode_contents,length,&error);
				break;
			case 1:  // TP1 Artist
				length = strlen(wk_id3v220tag->encode_contents);
				PQescapeStringConn(con,artist,wk_id3v220tag->encode_contents,length,&error);
				break;
			case 2:  // Title
				length = strlen(wk_id3v220tag->encode_contents);
				PQescapeStringConn(con,title,wk_id3v220tag->encode_contents,length,&error);
				break;
			case 3:  // Janle
				length = strlen(wk_id3v220tag->encode_contents);
				PQescapeStringConn(con,janle,wk_id3v220tag->encode_contents,length,&error);
				break;
			case 4: // Year
				strcpy(year,wk_id3v220tag->encode_contents);
				break;
			case 5: // Track No.
				strcpy(track_no,wk_id3v220tag->encode_contents);
				break;
			case 7: // ArtWork file name
				strcpy(artwork,wk_id3v220tag->encode_contents);
				break;
			defaults:
				break;
		}
	}
	// bit rate
	sprintf(bit_rate,"%d",bitrate->bit_rate);
	sprintf(time,"%3d:%02d",bitrate->play_time/60,bitrate->play_time%60);

	// mp3 filename
	length = strlen(mp3_file);
	PQescapeStringConn(con,mp3_filename,mp3_file,length,&error);

    	/* set schema */
    	res = PQexec(con,"SET SEARCH_PATH TO MP3_TEST");
    	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        	fprintf(stderr,"%s",PQerrorMessage(con));
		PQclear(res);
    		PQfinish(con);
        	exit(1);
    	}
	PQclear(res);

	// is there same album and artist
	sprintf(sql,"select seq_no from album where album = '%s' and artist = '%s'",album,artist);
		
#ifdef D_DEBUG
fprintf(stdout,"[%s]\n",sql);
#endif
	res = PQexec(con,sql);
    	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        	fprintf(stderr,"%s",PQerrorMessage(con));
		PQclear(res);
    		PQfinish(con);
        	exit(1);
    	}
	// album count 
	cnt = PQntuples(res);
	if(!cnt){
		PQclear(res);
		// stored album data
		sprintf(sql,"insert into album \
			(album,artist,genre,artwork) values( \
			'%s', '%s','%s','%s')", \
			album,
			artist,
			janle,
			artwork);

#ifdef D_DEBUG
fprintf(stdout,"[%s]\n",sql);
#endif
		res = PQexec(con,sql);
    		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        		fprintf(stderr,"%s",PQerrorMessage(con));
			PQclear(res);
    			PQfinish(con);
        		exit(1);
    		}
		PQclear(res);

		// store song data
		sprintf(sql,"insert into songs \
			(seq_no,title,year,track_no,bit_rate,time,like_cnt,mp3_file) values( \
			currval('ALBUM_SEQ_ALBUM_NO_SEQ'), '%s','%s','%s','%s','%s',0,'%s')", \
			title,
			year,
			track_no,
			bit_rate,
			time,
			mp3_filename);
		res = PQexec(con,sql);
    		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        		fprintf(stderr,"%s",PQerrorMessage(con));
			PQclear(res);
    			PQfinish(con);
        		exit(1);
    		}
		PQclear(res);

	}else{
		// update album
		sprintf(seq_no,"%s",PQgetvalue(res,0,0));
		sprintf(sql,"update album set \
			album = '%s', \
			artist = '%s', \
			genre = '%s', \
			artwork = '%s' \
			where seq_no = '%s'",
			album,
			artist,
			janle,
			artwork,
			seq_no);
		PQclear(res);
		res = PQexec(con,sql);
    		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        		fprintf(stderr,"%s",PQerrorMessage(con));
			PQclear(res);
    			PQfinish(con);
        		exit(1);
    		}
		PQclear(res);

		// check same songs, seq_no + title
		sprintf(sql,"select title from songs \
			where seq_no = '%s' and title = '%s'",seq_no,title);
		res = PQexec(con,sql);
    		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        		fprintf(stderr,"%s",PQerrorMessage(con));
			PQclear(res);
    			PQfinish(con);
        		exit(1);
    		}
		// songs count 
		cnt = PQntuples(res);
		PQclear(res);
		if(!cnt){
			// store song data
			sprintf(sql,"insert into songs \
				(seq_no,title,year,track_no,bit_rate,time,like_cnt,mp3_file) values( \
				%s, '%s','%s','%s','%s','%s',0,'%s')", \
				seq_no,
				title,
				year,
				track_no,
				bit_rate,
				time,
				mp3_filename);
		}else{
			// update song data
			sprintf(sql,"update songs set \
				year = '%s', \
				track_no = '%s', \
				bit_rate = '%s', \
				time = '%s', \
				like_cnt = 0, \
				mp3_file = '%s' \
				where seq_no = '%s' and title = '%s'",
				year,
				track_no,
				bit_rate,
				time,
				mp3_filename,
				seq_no,
				title);
		}

		res = PQexec(con,sql);
    		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        		fprintf(stderr,"%s",PQerrorMessage(con));
			PQclear(res);
    			PQfinish(con);
        		exit(1);
    		}
		PQclear(res);
	}


#ifdef D_DEBUG
fprintf(stdout,"[%s]\n",sql);
#endif

    	PQfinish(con);

	return(0);
}  
