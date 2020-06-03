#include <argp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fileHandling.h"

#define NOPATH 1
#define PATH   0

#define SUCCESS 0
#define FAILURE 1


static char *progname       = "hread2";
static const char pdoc[]    = "Read contents of audio file headers and load samples into memory";
static const char adoc[]    = "FILE...";

struct argp_option options[] ={
  {"path",'\0',"path",0,"path to file; this is the default argument"},
  {0}
};

typedef struct arguments {
  char* path;
  int pathFlag;
}args_t;

typedef struct program_ds{
  args_t args;
  riff_t rc;
  fmt_t fc;
  data_t dc;
  FILE* fd;
  FILE** pfd;
}pds_t;

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

int main(int argc,char** argv)
{
  pds_t pds;

  pds.fd = NULL;
  pds.pfd = &pds.fd;

  /*initial assumption is that path is not provided*/
  pds.args.pathFlag     = NOPATH;

  if(argp_parse(&argp,argc,argv,0,0,&pds.args)){
    fprintf(stderr,"Failed while parsing arguments! Exiting..\n");
    return FAILURE;
  }
  if(pds.args.pathFlag)
  {
    argp_help(&argp,stderr,ARGP_HELP_SHORT_USAGE|ARGP_HELP_LONG,progname);
    return FAILURE;
  }
  int retc = openFile(pds.args.path,pds.pfd);
  if(retc)
  {
    printf("I didnt open file!\n");
    return FAILURE;
  }

  if(readHeader(&pds.rc,&pds.fc,&pds.dc,pds.fd))
  {
    fprintf(stderr,"%s: Failed to read WAVE header!\n",progname);
    return FAILURE;
  }

  printRiffch(&pds.rc);
  printFmtch(&pds.fc);
  printDatach(&pds.dc);

  if(closeFile(pds.pfd))
  {
    return FAILURE;
  }

  return SUCCESS;
  }
