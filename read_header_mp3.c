#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include "mp3tag.h"

char *
SearchChar(const char *s1, const char moji, int size){
	int i;

	for(i = 0; i < size ; i++,s1++){
		if(*s1 == moji){
			return((char *)s1);
		}
	}
	return(NULL);
}

char *
FrameIdRead(const char *s1 ,const char *s2, int size){
	int i,n;

	n = strlen(s2);

	for(i = 0;i < size; i++,s1++){
		if((s1 = SearchChar(s1,s2[0],size)) == NULL)
			return(NULL);

		if(!memcmp(s1,s2,n)){
			return((char *)s1);
		}
		size -= i;
	}
	return(NULL);
}

int
TextConvert(ID3V2_20_TAG *tag){
	iconv_t ic;
	char *p_src;
	char *p_dst;
	size_t n_src, n_dst;

	if((tag->encode == 0)   // ISO-8859-1
	 	||(tag->encode == 3)){ // UTF-8
		strcpy(tag->encode_contents,tag->contents);
	}else if((tag->encode == 1) // UTF-16 with BOM
	 	||(tag->encode == 2)){  // UTF-16 without BOM
		if((ic = iconv_open("UTF-8","UTF-16")) == (iconv_t)-1) // to <- from
			return(-1);

		p_src = tag->contents;
		n_src = strlen(tag->contents);
		p_dst = tag->encode_contents;
		n_src = sizeof(tag->encode_contents);
		if(iconv(ic,&p_src,&n_src,&p_dst,&n_dst))
			return(-1);
		iconv_close(ic);
	}else{
			return(-1);
	}
	return(0);
}

int
ArtWork(char *wp, ID3V2_20_PIC *PIC,char *artwork_file_name){
	FILE *fp_img;
	unsigned char buff[128];
	char  img_file[256];
	int i, nPicSize;

	// set ID "PIC"
	memcpy(PIC->id,wp,3);
	PIC->id[3] = '\0';
	wp += 3; //skip "PIC"

	// set Frame Size
	memcpy(buff,wp,3);
	buff[3] = '\0';

	nPicSize = 0;
	for(i=0; i<3; i++){
		nPicSize *= 256;
		nPicSize += buff[i] & 0xff;
	}

#ifdef MIYA_DEBUG
	nPicSize = buff[0] << 16;
	nPicSize += buff[1] << 8;
	nPicSize += buff[2];
#endif
	PIC->size = nPicSize;
	wp += 3; //skip frame size

	// set Text encoding
	PIC->encoding = *wp;
	wp ++;

	//set Image Format("PNG" or "JPG")
	memcpy(PIC->format,wp,3);
	PIC->format[3] = '\0';
	wp += 3;  // skip image format 

        PIC->picture_type = *wp; 
	wp ++; // skip picture type

	//skip description
	while(*wp != '\0'){
		wp ++;  //$00
	}
	wp ++;  //(00)

	strcat(artwork_file_name,".");
	strcat(artwork_file_name,PIC->format);

	strcpy(img_file,ARTWORK_FILE_PATH);
	strcat(img_file,artwork_file_name);
#ifdef D_DEBUG
fprintf(stdout,"IMG:[%s]\n",img_file);
#endif

	if((fp_img = fopen(img_file,"wb")) == NULL){
		return(-1); 
	}
 	for(i=0; i < nPicSize; i++,wp++){ 
		fputc(*wp,fp_img);
	} 
	fclose(fp_img);

#ifdef D_DEBUG
fprintf(stdout,"id:[%s],size:[%d],encoding:[%d],format[%s],type:[%d]\n",
	PIC->id,
	PIC->size,
	PIC->encoding,
	PIC->format,
	PIC->picture_type);
#endif
return(0);
}

int
main(int argc,char *argv[]){
	FILE *fp;
	char *wp;
	unsigned char buff[1024];
	unsigned char *wkp; // after,change to malloc
	int i, j, bit_rate,play_time;
	int nTagSize ;
	int nContentsSize ;
	long file_size;

	ID3V2_HEADER ID3V2HEADER;
	ID3V2_20_PIC ID3V220PIC;
	BIT_RATE_INFO BITRATEINFO;
	MPEG_INFO MPEGINFO;
	ID3V2_20_TAG ID3V220TAG[TAG_ID_CNT] = {
        	{"TAL",3,1,0,'T',"",""}, // Album
        	{"TP1",3,1,0,'T',"",""}, // Artist 
        	{"TT2",3,1,0,'T',"",""}, // Title
        	{"TCO",3,1,0,'T',"",""}, // janle
        	{"TYE",3,1,0,'T',"",""}, //year
        	{"TRK",3,1,0,'T',"",""}, // Track No.
        	{"TEN",3,1,0,'T',"",""}, // Encoding user
        	{"PIC",3,0,0,'B',"",""}  // art work
	};

	if(argc != 3){
		fprintf(stdout,"Parameter Error[%d]{%s}{%s}\n",argc,argv[1],argv[2]);
		exit(1);
	}

	if((fp = fopen(argv[1],"rb")) == NULL){
		fprintf(stdout,"open error %s\n",argv[1]);
		exit(1);
	}
	wp = (char *)&ID3V2HEADER;
	for(i = 0; i < sizeof(ID3V2HEADER);i++,wp++){
		*wp = fgetc(fp);
	}

#ifdef D_DEBUG
	fprintf(stdout,"FILE:[%s]\n",argv[1]);
#endif

/* Read ID3V2 Header */
#ifdef D_DEBUG
	memcpy(buff,ID3V2HEADER.tag,3);
	buff[3] = '\0';
	fprintf(stdout,"TAG:[%s]\n",buff);

	memcpy(buff,ID3V2HEADER.version,2);
	buff[2] = '\0';
	fprintf(stdout,"Version:ID3V2.[%d.%d]\n",buff[0],buff[1]);

	fprintf(stdout,"flag:[%0x]\n",ID3V2HEADER.flags);
#endif
	if(ID3V2HEADER.version[0] != 2){
		fprintf(stdout,"TAG Version must be ID3V2.2.x,but ID3V2.[%d.%d]\n",
			ID3V2HEADER.version[0],ID3V2HEADER.version[1]);
		exit(1);
	}

	/* SIZE of variable header size */
	memcpy(buff,ID3V2HEADER.size,4);
	buff[4] = '\0';
	nTagSize = 0;
	for(i=0; i<4 ;i++){
		nTagSize *= 128;
		nTagSize += buff[i] & 0x7f;
	}
#ifdef D_DEBUG
	fprintf(stdout,"Size:[%d] TAG size:[%d]\n",nTagSize,nTagSize+sizeof(ID3V2_HEADER));
#endif

        /* Read all Frame */
	if((wkp = malloc(nTagSize*2)) == NULL){
		fprintf(stdout,"Memory not allocate\n");
		exit(-1);
	}
	memset(wkp,'\0',sizeof(nTagSize*2));
	for(i = 0; i < nTagSize; i++){
		wkp[i] = fgetc(fp);
	}
	
// Mp3 FRAME section
	fseek(fp,sizeof(ID3V2_HEADER)+nTagSize,SEEK_SET);
	fgets((char *)&MPEGINFO,sizeof(MPEG_INFO),fp);
	// get file size
	fseek(fp,0L,SEEK_SET);	// rewind pointer
	fseek(fp,0L,SEEK_END);
	file_size = ftell(fp);
	fclose(fp);

#ifdef D_DEBUG
	fprintf(stdout,"version->[%s]\n",MPEG_VERSION[MPEGINFO.version]);
	fprintf(stdout,"layer->[%s]\n",MPEG_LAYER[MPEGINFO.layer]);
#endif

	switch(MPEGINFO.version){
		case 0:
		case 2:
			bit_rate = MPEG2_BIT_RATE[MPEGINFO.bitrate][3-MPEGINFO.layer];
#ifdef D_DEBUG
			fprintf(stdout,"bit rate->[%d]kbps\n",bit_rate);
#endif
			break;
		case 3:
			bit_rate = MPEG1_BIT_RATE[MPEGINFO.bitrate][3-MPEGINFO.layer]; 
#ifdef D_DEBUG
			fprintf(stdout,"bit rate->[%d]kbps\n",bit_rate);
#endif
			break;
		defaults:
			bit_rate = 1;
			fprintf(stdout,"mis match MPEG version\n");
			break;
	}
	BITRATEINFO.bit_rate = bit_rate;	
	// playing time calcuration
	play_time = (file_size - nTagSize - sizeof(ID3V2_HEADER)) / (bit_rate / 8 * 1000);
	BITRATEINFO.play_time = play_time;	
#ifdef D_DEBUG
	fprintf(stdout,"Time %3d : %02d\n",play_time/60, play_time%60);
#endif
	
#ifdef D_DEBUG
fprintf(stdout,"synchead1=[%x]\n",MPEGINFO.synchead1);
fprintf(stdout,"synchead2=[%x]\n",MPEGINFO.synchead2);
fprintf(stdout,"version=[%x]\n",MPEGINFO.version);
fprintf(stdout,"layer=[%x]\n",MPEGINFO.layer);
fprintf(stdout,"errorprotect=[%x]\n",MPEGINFO.errorprotect);
fprintf(stdout,"bitrate=[%x]\n",MPEGINFO.bitrate);
fprintf(stdout,"samplerate=[%x]\n",MPEGINFO.samplerate);
fprintf(stdout,"padding=[%x]\n",MPEGINFO.padding);
fprintf(stdout,"extension=[%x]\n",MPEGINFO.extension);
fprintf(stdout,"channelmode=[%x]\n",MPEGINFO.channelmode);
fprintf(stdout,"extensionmode=[%x]\n",MPEGINFO.extensionmode);
fprintf(stdout,"copyright=[%x]\n",MPEGINFO.copyright);
fprintf(stdout,"original=[%x]\n",MPEGINFO.original);
fprintf(stdout,"emphasis=[%x]\n",MPEGINFO.emphasis);
#endif


/* Ther is no expanded header */
/* Read ID3V2-2.0 Audio Tag Header */

	for(i = 0 ; i < TAG_ID_CNT; i++){ 
		wp = FrameIdRead(wkp,ID3V220TAG[i].id,nTagSize);
		if(wp == NULL){
			sprintf(ID3V220TAG[i].encode_contents,"",ID3V220TAG[i].id);
		}else{
			if(!strcmp(ID3V220TAG[i].id,"PIC")){
				// Set Image File Name Alubum name + Artist  
				strcpy(ID3V220TAG[i].encode_contents,ID3V220TAG[0].encode_contents);
				strcat(ID3V220TAG[i].encode_contents,"_");
				strcat(ID3V220TAG[i].encode_contents,ID3V220TAG[1].encode_contents);
				ArtWork((char *)wp,&ID3V220PIC,ID3V220TAG[i].encode_contents);
#ifdef D_DEBUG
fprintf(stdout,"AFTER[%s]\n",ID3V220TAG[i].encode_contents);
#endif
			}else{
				// Skip Frame ID (TAL,TT2,etc)
				wp += strlen(ID3V220TAG[i].id);
				// value of length
				nContentsSize = 0;
				memcpy(buff,wp,3);
				buff[3] = '\0';
				for(j=0; j<3 ;j++){
					nContentsSize *= 128;
					nContentsSize += buff[j] & 0x7f;
				}
				// bytes for length(3byte)
				wp += ID3V220TAG[i].size;
				ID3V220TAG[i].encode = *wp;
				wp += ID3V220TAG[i].ch_size;
				memcpy(ID3V220TAG[i].contents,wp,nContentsSize);

				switch(ID3V220TAG[i].format){
					case 'T':	// Text Data
						TextConvert(&ID3V220TAG[i]);
						break;
					case 'B':	// Binary
						break;
				}
			}
		}
	}
	free(wkp);
	
#ifdef D_DEBUG
	for(i = 0 ; i < TAG_ID_CNT; i++){
		fprintf(stdout,"[%s] ",ID3V220TAG[i].encode_contents);
	}
	fprintf(stdout,"[Time %3d:%02d] [bit rate[%d]kbps]\n",
		BITRATEINFO.play_time/60, BITRATEINFO.play_time%60,BITRATEINFO.bit_rate);
#endif

	// Tag info, Bit rate info, mp3 filename
	if(InsertPostgres(&ID3V220TAG,&BITRATEINFO,argv[2])){
		exit(1);
	}
	
	exit(0);
}
