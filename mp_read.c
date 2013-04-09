#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FILE_NAME "./audio/02 Candy.mp3"
int
main(int argc,char *argv[]){
	FILE  *fp;
	char *wp;
	char buff[1024];
	char work_buff[4096];
	int i, j, mod;
	int nTagSize = 0;
	struct ID3V2_HEADER{
		char  tag[3];
		char  version[2];
		char  flags;
		char  size[4];
	}ID3V2HEADER;
	struct ID3V2_20_FRAME{
		char id[3];
		char size[3];
	}ID3V220FRAME;

	if((fp = fopen(FILE_NAME,"rb")) == NULL){
		fprintf(stdout,"open error %s\n",FILE_NAME);
		exit(1);
	}
	wp = (char *)&ID3V2HEADER;
	for(i = 0; i < sizeof(ID3V2HEADER);i++,wp++){
		*wp = fgetc(fp);
	}



	fprintf(stdout,"FILE:[%s]\n",FILE_NAME);
/* Read ID3V2 Header */
	memcpy(buff,ID3V2HEADER.tag,3);
	buff[3] = '\0';
	fprintf(stdout,"TAG:[%s]\n",buff);

	memcpy(buff,ID3V2HEADER.version,2);
	buff[2] = '\0';
	fprintf(stdout,"Version:ID3V2.[%d.%d]\n",buff[0],buff[1]);

	fprintf(stdout,"flag:[%0x]\n",ID3V2HEADER.flags);


	/* SIZE of variable header size */
	memcpy(buff,ID3V2HEADER.size,4);
	buff[4] = '\0';
	for(i=0; i<4 ;i++){
		nTagSize *= 128;
		nTagSize += buff[i] & 0x7f;
	}
	fprintf(stdout,"Size:[%d] TAG size:[%d]\n",nTagSize,nTagSize+10);


        /* Read all Frame */
	// wp = (char *)&ID3V220FRAME;
	memset(work_buff,'\0',sizeof(work_buff));
	for(i = 0; i < nTagSize; i++){
		work_buff[i] = fgetc(fp);
	}
	
	fclose(fp);

/* Read Frame Header */
	/* Ther is no expanded header */
	/* SIZE of variable header size */
	for(j = 0; j < 200; j++){
		mod = 0;
		memcpy((char *)&ID3V220FRAME,&work_buff[j],sizeof(ID3V220FRAME));
		memcpy(buff,ID3V220FRAME.id,sizeof(ID3V220FRAME.id));
		buff[sizeof(ID3V220FRAME.id)] = '\0';
		fprintf(stdout,"Frame ID:[%s]\n",buff);
		if(strcmp(buff,"")){
			mod = 1;
		}else{
			mod = 3;
		}

		memcpy(buff,ID3V220FRAME.size,sizeof(ID3V220FRAME.size));
		buff[sizeof(ID3V220FRAME.size)] = '\0';
		nTagSize = 0;
		for(i=0; i<3 ;i++){
			nTagSize *= 128;
			nTagSize += buff[i] & 0x7f;
		}
		fprintf(stdout,"Frame Size:[%d]\n",nTagSize);
		strcpy(buff,&work_buff[j+sizeof(ID3V220FRAME)]+mod);
		fprintf(stdout,"Contents:[%s]\n",buff);
		j += sizeof(ID3V220FRAME) + strlen(buff);
	}


#ifdef ORG
	for(i = 0; i < nTagSize;i++){
		if(work_buff[i] == '\0')
			work_buff[i] = ' ';
	}
	work_buff[nTagSize] = '\0';
	fprintf(stdout,"frame:[%s]\n",work_buff);
#endif


	exit(0);
}
