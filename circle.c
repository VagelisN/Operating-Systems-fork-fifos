#include <stdio.h>
#include  <poll.h>
#include  <signal.h>
#include  <fcntl.h>
#include  <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>


int main(int argc , char **argv)
{
	if(argc!=9 &&argc!=11 &&argc!=13) 
	{
		printf("wrong number of arguements given\n");
		exit(-1);
	}	
	int c, pointstoread=-1, offset=-1 ,aflag=0;
	long size;
	FILE *fin=NULL,*fout=NULL;
	float x,y,r;
	while((c = getopt (argc, argv, "i:o:f:a:n:")) != -1)
	{
		switch(c)
		{
			case 'i':
				fin=fopen(optarg,"rb");
				if(fin==NULL)
				{
					printf("error opening file\n");
					exit(-1);
				}
				break;
			case 'o':
				fout=fopen(optarg,"w");
				if(fout==NULL)
				{
					printf("error opening file\n");
					exit(-1);
				}
				break;
			case 'a':
				aflag=1;
				x=atof(optarg);
				y=atof(argv[optind]);
				r=atof(argv[optind+1]);
				break;
			case 'f':
				offset=atoi(optarg);
				if(offset%(2*sizeof(float))!=0)
				{
					printf("offset mod 8 has to be zero\n");
					exit(-1);
				}
				break;
			case 'n':
				pointstoread=atoi(optarg);
				break;
			default:
				break;
		}
	}
	if(fin==NULL || fout==NULL||aflag==0)
	{
		printf("-i -o -a flags required\n");
		exit(-1);
	}
	if(pointstoread==-1)
	{
		fseek(fin,0,SEEK_END);
		size=ftell(fin);
		rewind(fin);
		if(offset!=-1)size=size-offset;
		pointstoread =(size/(2*sizeof(float)));
	}
	if(offset!=-1)
	{
		fseek(fin,offset,SEEK_SET);
	 }
	float a=0,b=0;
	int i;

	for(i = 0; i < pointstoread; i++)
	{
		fread(&a,1,sizeof(float),fin);
		fread(&b,1,sizeof(float),fin);
		if((a-x)*(a-x)+(b-y)*(b-y)<=(r*r))fprintf(fout,"%.2f\t%.2f\n",a,b);
	}
	fclose(fin);
	fclose(fout);
	return 0;
}
