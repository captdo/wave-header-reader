#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <endian.h>
#include <string.h>

#include "fileHandling.h"


#define SUCCESS 0
#define FAILURE 1

#define NOTERM 0
#define TERM 1


int openFile(char *path,FILE **fd)
{
  *fd=fopen(path,"r");
  if(*fd==NULL)
  {
    printf("failed to open file!Error %d: %s\n",errno,strerror(errno));
    return FAILURE;
  }

  return SUCCESS;
}

int closeFile(FILE **fd)
{
  if (fclose(*fd) != 0)
  {
    fprintf(stderr,"Failed to close file! Error %d: %s. \n",errno,strerror(errno));
    return FAILURE;
  }
  return SUCCESS;
}

int readNbytes(void *dest,FILE* fd,int n, int termination)
{

  if(fread(dest,n,1,fd)!=1)
  {
    fprintf(stderr,"Failed while reading %d bytes! Error %d: %s.\n",n,errno,strerror(errno));
    return FAILURE;
  }
  if(termination)
  {
    uint8_t *tdest = (uint8_t*)dest;
    tdest[4]='\0';
  }
  return SUCCESS;
}

int readRiffch(riff_t *rc,FILE* fd)
{
  if(fd == NULL)
  {
    printf("INVALID fd! Check performed before reading RIff!\n");
  }

  if(fseek(fd,0,SEEK_SET))
  {
    fprintf(stderr,"Failed setting position! Error %d: %s",errno,strerror(errno));
  }
  if(readNbytes((void*)rc->id,fd,4,TERM))
  {
    fprintf(stderr,"Failed to read Riff chunk ID!\n");
    return FAILURE;
  }
  if(readNbytes((void*)&rc->size,fd,4,NOTERM))
  {
    fprintf(stderr,"Failed to read Riff chunk size!\n");
    return FAILURE;
  }
  if(readNbytes((void*)&rc->format,fd,4,TERM))
  {
    fprintf(stderr,"Failed to read Riff chunk format!\n");
    return FAILURE;
  }
  return SUCCESS;
}

int readFmtch(fmt_t *fc,FILE* fd)
{
  if(fd == NULL)
  {
    fprintf(stderr,"Invalid pointer! check performed before reading fmt!\n");
    return FAILURE;
  }
  fseek(fd,12,SEEK_SET);
  if(readNbytes((void*)fc->id,fd,4,TERM))
  {
    fprintf(stderr,"Failed to read Fmt chunk ID!\n");
    return FAILURE;
  }
  if(readNbytes((void*)&fc->size,fd,4,NOTERM))
  {
    fprintf(stderr,"Failed to read Fmt chunk size!\n");
    return FAILURE;
  }
  if(readNbytes((void*)&fc->aformat,fd,2,NOTERM))
  {
    fprintf(stderr,"Failed to read Fmt chunk audio format!\n");
    return FAILURE;
  }
  if(readNbytes((void*)&fc->channels,fd,2,NOTERM))
  {
    fprintf(stderr,"Failed to read Fmt chunk channel count!\n");
    return FAILURE;
  }
  if(readNbytes((void*)&fc->sampleRate,fd,4,NOTERM))
  {
    fprintf(stderr,"Failed to read Fmt chunk sample rate!\n");
    return FAILURE;
  }
  if(readNbytes((void*)&fc->byteRate,fd,4,NOTERM))
  {
    fprintf(stderr,"Failed to read Fmt chunk byteRate!\n");
    return FAILURE;
  }
  if(readNbytes((void*)&fc->blockAlign,fd,2,NOTERM))
  {
    fprintf(stderr,"Failed to read Fmt chunk blockAlign!\n");
    return FAILURE;
  }
  if(readNbytes((void*)&fc->bitsPerSample,fd,2,NOTERM))
  {
    fprintf(stderr,"Failed to read Fmt chunk bits per sample!\n");
    return FAILURE;
  }
  return SUCCESS;
}

int readDatach(data_t *dc,FILE* fd)
{
  if(fd == NULL)
  {
    fprintf(stderr,"Invalid pointer! check performed before reading data ch!\n");
    return FAILURE;
  }
  fseek(fd,36,SEEK_SET);
  if(readNbytes((void*)dc->id,fd,4,TERM))
  {
    return FAILURE;
  }
  if(readNbytes((void*)&dc->size,fd,4,NOTERM))
  {
    return FAILURE;
  }
  return SUCCESS;
}

int readHeader(riff_t *rc, fmt_t *fc, data_t *dc, FILE* fd)
{
  if(fd == NULL)
  {
    fprintf(stderr,"Invalid pointer! check performed in readHeader!\n");
    return FAILURE;
  }
  if(readRiffch(rc,fd))
  {
    fprintf(stderr,"Failed to read RIFF chunk!\n");
    return FAILURE;
  }
  //read fmt chunk
  if(readFmtch(fc,fd))
  {
    fprintf(stderr,"Failed to read Fmt chunk!\n");
    return FAILURE;
  }
  if(readDatach(dc,fd))
  {
    fprintf(stderr,"Failed to read Data chunk!\n");
    return FAILURE;
  }
  return SUCCESS;
}
void printRiffch(riff_t *rc)
{
  printf("\nID: %s\n",rc->id);
  printf("Size: %d\n",le32toh(rc->size));
  printf("Format: %s\n",rc->format);
}
void printFmtch(fmt_t *fc)
{
  printf("\nfmt_ID: %s\n",fc->id);
  printf("fmt chunk size: %d\n",le32toh(fc->size));
  if(le16toh(fc->aformat) == 1)
  {
    printf("Audio Format: PCM (1) -> linear quantisation \n");
  }else{
    printf("Audio Format: %d -> compression \n",le16toh(fc->aformat));

  }

  switch(le16toh(fc->channels))
  {
    case 1:
            printf("Number of channels: 1 -> Mono\n");
            break;
    case 2:
            printf("Number of channels: 2 -> Stereo\n");
            break;
    default:
            printf("Number of channels: %d",le16toh(fc->channels));
  }
  printf("Sample rate: %d\n",le32toh(fc->sampleRate));
  printf("Byte rate: %d\n",le32toh(fc->byteRate));
  printf("Block align: %d\n",le16toh(fc->blockAlign));
  printf("Bits per sample: %d\n",le16toh(fc->bitsPerSample));
}
void printDatach(data_t *dc)
{
  printf("\nData ID: %s\n",dc->id);
  printf("Data size: %d\n",le32toh(dc->size));
}
uint32_t calculateSamples(uint32_t bytes, uint32_t bps)
{
  return bytes*8/(uint32_t)bps;
}

int fetchData(fmt_t *fc, data_t *dc,FILE *fd)
{

  if(fd == NULL)
  {
    fprintf(stderr,"Invalid pointer! check performed before fetching data!\n");
    return FAILURE;
  }
  dc->nsamples = calculateSamples(dc->size,fc->bitsPerSample);
  switch(fc->bitsPerSample)
  {
    case 8:
            dc->data = (uint8_t*)malloc(dc->nsamples*sizeof(uint8_t));
            if(!dc->data)
            {
              fprintf(stderr,"Memory allocation failed! Error %d: %s\n",errno,strerror(errno));
              return FAILURE;
            }
            if(readNbytes((void*)dc->data,fd,dc->nsamples,NOTERM))
            {
              return FAILURE;
            }
            break;
    //case 16:
    //case 32:
    default:
              fprintf(stderr,"Data format unsupported yet!\n");
              return FAILURE;
  }
  return SUCCESS;
}
