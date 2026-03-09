#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <zlib.h>
#include <stdlib.h>
#include "div_string.h"

#define INTOOUT  \
\
while(!feof(f)) { \
	n=fread(buf,1,1024,f); \
	fwrite(buf,1,n,out); \
} \



int pack(char *runtime, char *exefile, char *datafile, char *outfile) {

	char buf[1024];
	int n=0;
	char DX[2]="DX";

	int32_t exesize = 0;
	int32_t datsize=0;
	FILE *f=NULL;
	FILE *out = NULL;

	out=fopen(outfile,"wb");

	if(!out) {
		return 0;
	}

	f=fopen(runtime,"rb");

	if(!f) {
		printf("Could not open runtime\n");
		fclose(out);
		return 0;
	}

	// copy f to out
	INTOOUT

	fclose(f);

	f=fopen(exefile,"rb");

	if(!f) {
		printf("Could not open div bytecode\n");
		fclose(out);
		return 0;
	}

	fseek(f,0,SEEK_END);
	exesize=ftell(f);
	fseek(f,0,SEEK_SET);

	// cat f to out
	INTOOUT

	f=fopen(datafile,"rb");
	
	if(!f) {
		printf("Could not open div data %s\n",datafile);
		fclose(out);
		return 0;

	} else {
		fseek(f,0,SEEK_END);
		datsize=ftell(f);
		fseek(f,0,SEEK_SET);

		INTOOUT
		fclose(f);
	}

	fwrite(&exesize,4,1,out);
	fwrite(&datsize,4,1,out);

	fwrite(DX,2,1,out);

	fclose(out);

#ifndef WIN32
	chmod(outfile,strtol("0777",NULL,8));
#endif

	return 1;
}

#ifdef STANDALONE

int main(int argc, char *argv[]) {
	char runtime[255];
	char exefile[255];
	char datafile[255];
	char outfile[255];

	printf("argc: %d\n",argc);


	// output file
	if(argc>4) {
		DIV_STRCPY(outfile, argv[4]);
	} else {
		DIV_STRCPY(outfile,"out");
	}

	// runtime
	if(argc>1) {
		DIV_STRCPY(runtime, argv[1]);
	} else {
		DIV_STRCPY(runtime,"system/divrun-LINUX");
	}

	// bytecode
	if(argc>2) {
		DIV_STRCPY(exefile, argv[2]);
	} else {
		DIV_STRCPY(exefile, "system/EXEC.EXE");
	}

	// data file (pak in zip or zip)

	if(argc>2) {
		DIV_STRCPY(datafile, argv[3]);
	} else {
		DIV_STRCPY(datafile, "data.div");
	}

	return pack(runtime, exefile, datafile, outfile);

}

#endif
