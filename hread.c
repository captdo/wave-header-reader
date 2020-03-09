#include <argp.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <endian.h>

#define SUCCESS 0
#define FAILURE 1

#define PATH 0
#define NOPATH 1

static char *progname       = "hread";
static const char pdoc[]    = "Read contents of audio file headers";
static const char adoc[]    = "FILE...";

static FILE *fd;

struct argp_option options[] ={
  {"path",'\0',"path",0,"path to file; this is the default argument"},
  {0}
};

typedef struct arguments {
  char* path;
  int pathFlag;
}args_t;

static int parse_opt(int key,char *arg,struct argp_state *state){

    args_t *args = state->input;

    switch (key)
    {
      case '\0':
                args->pathFlag = PATH;
                args->path = arg;
                printf("path: %s\n",args->path);
                break;

      case ARGP_KEY_END:
                if(state->arg_num < 1)
                {
                  fprintf(stderr,"Too few arguments!\n");
                  argp_state_help(state,stderr,ARGP_HELP_SHORT_USAGE|ARGP_HELP_LONG);
                  return FAILURE;
                }
    }
    return SUCCESS;
  }
struct argp argp = {options,parse_opt,adoc,pdoc};

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

static int readRiff(riff_t *rc){
  fseek(fd,0,SEEK_SET);
  if(fread(rc->id,4,1,fd)!=1)
  {
    fprintf(stderr,"%s: Failed to read riff ID! Error %d: %s.\n",progname,errno,strerror(errno));
    return FAILURE;
  }
  rc->id[4] = '\0';
  if(fread(&rc->size,4,1,fd)!=1)
  {
    fprintf(stderr,"%s: Failed to read riff size! Error %d: %s.\n",progname,errno,strerror(errno));
    return FAILURE;
  }
  if(fread(rc->format,4,1,fd)!=1)
  {
    fprintf(stderr,"%s: Failed to read riff format! Error %d: %s.\n",progname,errno,strerror(errno));
    return FAILURE;
  }
  rc->format[4] = '\0';
  return SUCCESS;
}

static int readFmt(fmt_t *fc){
  fseek(fd,12,SEEK_SET);
  if(fread(fc->id,4,1,fd)!=1)
  {
    fprintf(stderr,"%s: Failed to read fmd ID! Error %d: %s.\n",progname,errno,strerror(errno));
    return FAILURE;
  }
  fc->id[4] = '\0';
  if(fread(&fc->size,4,1,fd)!=1)
  {
    fprintf(stderr,"%s: Failed to read fmd size! Error %d: %s.\n",progname,errno,strerror(errno));
    return FAILURE;
  }
  if(fread(&fc->aformat,2,1,fd)!=1)
  {
    fprintf(stderr,"%s: Failed to read Audio Format! Error %d: %s.\n",progname,errno,strerror(errno));
    return FAILURE;
  }
  if(fread(&fc->channels,2,1,fd)!=1)
  {
    fprintf(stderr,"%s: Failed to read channel count! Error %d: %s.\n",progname,errno,strerror(errno));
    return FAILURE;
  }
  if(fread(&fc->sampleRate,4,1,fd)!=1)
  {
    fprintf(stderr,"%s: Failed to read sample rate! Error %d: %s.\n",progname,errno,strerror(errno));
    return FAILURE;
  }
  if(fread(&fc->byteRate,4,1,fd)!=1)
  {
    fprintf(stderr,"%s: Failed to read byte rate! Error %d: %s.\n",progname,errno,strerror(errno));
    return FAILURE;
  }
  if(fread(&fc->blockAlign,2,1,fd)!=1)
  {
    fprintf(stderr,"%s: Failed to read blockAlign! Error %d: %s.\n",progname,errno,strerror(errno));
    return FAILURE;
  }
  if(fread(&fc->bitsPerSample,2,1,fd)!=1)
  {
    fprintf(stderr,"%s: Failed to read fmd ID! Error %d: %s.\n",progname,errno,strerror(errno));
    return FAILURE;
  }
  return SUCCESS;
}

static int openFile(char *path){
  fd = fopen(path,"r");
  if(!fd){
    fprintf(stderr,"%s: Failed to open file! Error %d: %s.\n",progname,errno,strerror(errno));
    return FAILURE;
  }
  return SUCCESS;
}

static int closeFile(){
  if (fclose(fd) != 0)
  {
    fprintf(stderr,"%s: Failed to close file! Error %d: %s. \n",progname,errno,strerror(errno));
    return FAILURE;
  }
  return SUCCESS;
}

static void printRif(riff_t *rc){
  printf("ID: %s\n",rc->id);
  printf("Size: %d\n",le32toh(rc->size));
  printf("Format: %s\n",rc->format);
}
static void printFmt(fmt_t *fc){
  printf("fmt_ID: %s\n",fc->id);
  printf("fmt chunk size: %d\n",le32toh(fc->size));
  printf("Audio Format: %d\n",le16toh(fc->aformat));
  printf("Number of channels: %d\n",le16toh(fc->channels));
  printf("Sample rate: %d\n",le32toh(fc->sampleRate));
  printf("Byte rate: %d\n",le32toh(fc->byteRate));
  printf("Block align: %d\n",le16toh(fc->blockAlign));
  printf("Bits per sample: %d\n",le16toh(fc->bitsPerSample));
}
static int readHeader(riff_t *rc,fmt_t *fc){
  //read riff chunk
  if(readRiff(rc))
  {
    fprintf(stderr,"%s: Failed to read RIFF chunk!\n",progname);
    return FAILURE;
  }
  //read fmt chunk
  if(readFmt(fc))
  {
    return FAILURE;
  }
  return SUCCESS;
}

int main(int argc, char** argv){
  args_t args;
  riff_t rc;
  fmt_t fc;

  /*initial assumption is that path is not provided*/
  args.pathFlag     = NOPATH;

  if(argp_parse(&argp,argc,argv,0,0,&args)){
    fprintf(stderr,"Failed while parsing arguments! Exiting..\n");
    return FAILURE;
  }

  if(args.pathFlag)
  {
    argp_help(&argp,stderr,ARGP_HELP_SHORT_USAGE|ARGP_HELP_LONG,progname);
    return FAILURE;
  }

  if(openFile(args.path))
  {
    return FAILURE;
  }

  if(readHeader(&rc,&fc))
  {
    fprintf(stderr,"%s: Failed to read WAVE header!\n",progname);
    return FAILURE;
  }

  printRif(&rc);
  printFmt(&fc);

  if(closeFile())
  {
    return FAILURE;
  }

  return SUCCESS;
}
