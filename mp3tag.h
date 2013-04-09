#define TAG_ID_CNT 8
#define FRAME_HEADER_SIZE 7 //byte(32bit)
#define ARTWORK_FILE_PATH "/var/tmp/ArtWork/"

typedef struct{
	char  tag[3];
	char  version[2];
	char  flags;
	char  size[4];
}ID3V2_HEADER;

typedef struct{
	int	bit_rate;
	unsigned int	play_time;
}BIT_RATE_INFO;

typedef struct{
	char  id[5];    // use 4bytes ex.TP0'\0',TT2'\0'
	int   size;
	int   ch_size;
	char  encode;
	char  format;	//T Text B Binary
	unsigned char  contents[256];
	unsigned char  encode_contents[256];
}ID3V2_20_TAG;

/*
ID3V2_20_TAG ID3V220TAG[TAG_ID_CNT] = {
	{"TAL",3,1,0,'T',"",""}, // Album
	{"TP1",3,1,0,'T',"",""}, // Artist
	{"TT2",3,1,0,'T',"",""}, // Title
	{"TCO",3,1,0,'T',"",""}, // janle
	{"TYE",3,1,0,'T',"",""}, //year
	{"TRK",3,1,0,'T',"",""}, // Track No.
	{"TEN",3,1,0,'T',"",""}, // Encoding user
	{"PIC",3,0,0,'B',"",""}  // art work (set ArtWork image file name)
};

*/

typedef struct{
	char id[5];    // user 4 bytes "PIC'\0'"
	int  size;
        char encoding;
	char format[5]; // "JPG","PNG"
	char picture_type;
}ID3V2_20_PIC;

typedef struct{
    unsigned    synchead1 : 8;
    unsigned    errorprotect : 1;
    unsigned    layer : 2;
    unsigned    version : 2;
    unsigned    synchead2 : 3;
    unsigned    extension : 1;
    unsigned    padding : 1;
    unsigned    samplerate : 2;
    unsigned    bitrate : 4;
    unsigned    emphasis : 2;
    unsigned    original : 1;
    unsigned    copyright : 1;
    unsigned    extensionmode : 2;
    unsigned    channelmode : 2;
} MPEG_INFO;

static char *MPEG_VERSION[4] ={
	"MPEG バージョン2.5",
	" ",
	"MPEG バージョン2",
	"MPEG バージョン1"
};
static char *MPEG_LAYER[4] ={
	"",
	"レイヤー3",
	"レイヤー2",
	"レイヤー1"
};

static int MPEG1_BIT_RATE[][3]={
	{0,0,0},
	{32,32,32},
	{64,48,40},
	{96,56,48},
	{128,64,56},
	{160,80,64},
	{192,96,80},
	{224,112,96},
	{256,128,112},
	{288,160,128},
	{320,192,160},
	{352,224,192},
	{384,256,224},
	{416,320,256},
	{448,384,320},
	{0,0,0}
};
static int MPEG2_BIT_RATE[][3]={
	{0,0,0},
	{32,8,8},
	{48,16,16},
	{56,24,24},
	{64,32,32},
	{80,40,40},
	{96,48,48},
	{112,56,56},
	{128,64,64},
	{144,80,80},
	{160,96,96},
	{176,112,112},
	{192,128,128},
	{224,144,144},
	{256,160,160},
	{0,0,0}
};
