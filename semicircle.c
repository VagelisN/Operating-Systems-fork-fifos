#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc , char **argv)
{
	if(argc!=10 &&argc!=12 &&argc!=14) 
	{
		printf("wrong number of arguements given\n");
		exit(-1);
	}	
	FILE *fin=NULL,*fout=NULL;
	char orientation;
	int c,pointstoread=-1, offset=-1 ,aflag=0;
	float x,y,r;
	long size;
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
				x=atoi(optarg);
				y=atoi(argv[optind]);
				r=atoi(argv[optind+1]);
				orientation=*argv[optind+2];
				if(orientation!='W'&&orientation!='S'&&orientation!='N'&&orientation!='E')
				{
					printf("orientation has to be W, S, E or N\n");
					exit(-1);
				}
				break;
			case 'f':
				offset=atoi(optarg);
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
	if(offset!=-1) fseek(fin,offset,SEEK_SET);
	float a=0,b=0;
	int i;
	for (i = 0; i <pointstoread; i++)
	{
		fread(&a,1,sizeof(float),fin);
		fread(&b,1,sizeof(float),fin);
		if (orientation=='N')
		{
			if(b>=y)
			{
				if((a-x)*(a-x)+(b-y)*(b-y)<=(r*r))fprintf(fout,"%.2f\t%.2f\n",a,b);
			}
		}
		if (orientation=='S')
		{
			if(b<=y)
			{
				if((a-x)*(a-x)+(b-y)*(b-y)<=(r*r))fprintf(fout,"%.2f\t%.2f\n",a,b);
			}
		}
		if (orientation=='W')
		{
			if(a<=x)
			{
				if((a-x)*(a-x)+(b-y)*(b-y)<=(r*r))fprintf(fout,"%.2f\t%.2f\n",a,b);
			}
		}
		if (orientation=='E')
		{
			if(a>=x)
			{
				if((a-x)*(a-x)+(b-y)*(b-y)<=(r*r))fprintf(fout,"%.2f\t%.2f\n",a,b);
			}
		}
	}
	fclose(fin);
	fclose(fout);
	return 0;
}
