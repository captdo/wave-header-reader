
typedef struct riffChunk{
  uint8_t id[5];
  uint32_t size;
  uint8_t format[5];
}riff_t;

typedef struct fmtChunk{
  uint8_t id[5];
  uint32_t size;
  uint16_t aformat;
  uint16_t channels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
}fmt_t;

typedef struct dataChunk{
  uint8_t id[5];
  uint32_t size;
  uint32_t nsamples;
  uint8_t *data;
}data_t;

int openFile(char *path,FILE **fd);
int closeFile(FILE **fd);

int readNbytes(void *dest,FILE* fd,int n, int termination);
int readRiffch(riff_t *rc,FILE* fd);
int readFmtch(fmt_t *fc,FILE* fd);
int readDatach(data_t *dc, FILE* fd);
int readHeader(riff_t *rc, fmt_t *fc, data_t *dc, FILE* fd);
uint32_t calculateSamples(uint32_t bytes, uint32_t bps);
int fetchData(fmt_t *fc, data_t *dc,FILE* fd);


void printRiffch(riff_t *rc);
void printFmtch(fmt_t *fc);
void printDatach(data_t *dc);
