#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include  <poll.h>
#include "funcs.h"

int arg_parsing(int argc, char** argv,FILE **fin,int *workers,char **dirname,char **filename,long *size,int **PointsToReadCount)
{
	int c,i,k;
	while((c = getopt (argc, argv, "i:w:d:")) != -1)
	{
		switch(c)
		{
			case 'i':
				*fin=fopen(optarg,"rb");			
				if((*filename=malloc(sizeof(char)*(strlen(optarg)+1)))==NULL)return 1;
				strcpy(*filename,optarg);
				if(fin==NULL)
				{
					printf("error opening file\n");
					return 1;
				}
				break;
			case 'w':
				*workers=atoi(optarg);
				if(*workers<1)
				{
					printf("-w has to be at least 1\n");
					return 1;
				}
				break;
			case 'd':
				if((*dirname=malloc(sizeof(char)*(strlen(optarg)+1)))==NULL)return 1;
				strcpy(*dirname,optarg);
				if (fork()==0)
				{
					execlp("mkdir" ,"mkdir",*dirname , NULL);
					return 1;
				}
				else wait(NULL);
				break;
			default:
				break;
		}		
	}
	fseek(*fin,0,SEEK_END);
	*size=ftell(*fin);
	(*size)=((*size)/(2*sizeof(float)));
	*PointsToReadCount=(int*)malloc((*workers)*sizeof(int));		
	for (i = 0; i <(*workers); i++)(*PointsToReadCount)[i]=((int)(*size / *workers));
	if( (int)(*size % *workers)!=0)//modulo is shared among all workers 
	{
		k=0;
		for (i = 0; i <(int)(*size % *workers); i++)
		{
			((*PointsToReadCount)[k])++;
			k++;
			if(k==*workers)k=0;
		}
	}
	rewind(*fin);
	fclose(*fin);
	return 0;
}

int plot(char** buff,FILE** fin,int counter,char* dirname,int* handlerids,char** colors,int plotcounter)
{
		int k;
		sprintf(*buff,"%d_script.gnuplot",plotcounter);
		*fin=fopen(*buff,"w");
		if(fin==NULL)
		{
			printf("error opening file\n");
			return 1;
		}
		fprintf(*fin,"set terminal png\nset size ratio -1\n");
		sprintf(*buff,"./%d_image.png",plotcounter);
		fprintf(*fin,"set output \"%s\"\nplot \\\n",*buff);
		for (k = 0; k < counter; k++)
		{
			if(k<counter-1)fprintf(*fin,"\"%s/%d.txt\" notitle with points pointsize 0.5 linecolor rgb \"%s\",\\\n",dirname,handlerids[k],colors[k]);
			else fprintf(*fin,"\"%s/%d.txt\" notitle with points pointsize 0.5 linecolor rgb \"%s\"\n",dirname,handlerids[k],colors[k]);
		}
		fclose(*fin);
		if (fork()==0)
		{
			sprintf(*buff,"%d_script.gnuplot",plotcounter);
			execlp("gnuplot","gnuplot","<",*buff,NULL);
			exit(-1);
		}
		else wait(0);
		if (fork()==0)
		{
			sprintf(*buff,"%d_script.gnuplot",plotcounter);
			execlp("rm","rm",*buff,NULL);
			exit(-1);
		}
		else wait(0);
		
		return 0;
}

void handler_free(int workers,int counter,struct pollfd *pfds,char **currcommand,char **colors,char** commands,int *handlerids,int *PointsToReadCount,char *shape,FILE *handlerout,char *line,char * buff, char *filename ,char *dirname)
{
	int k;
	for (k = 0; k <workers ; k++)
	{
		close(pfds[k].fd);
	}
	for (k = 0; k < 6; k++)
	{
		free(currcommand[k]);
	}
	for (k = 0; k <counter ; k++)
	{
		free(colors[k]);
		free(commands[k]);
	}
	free(pfds);
	free(handlerids);
	free(colors);
	free(commands);
	free(shape);
	fclose(handlerout);
	free(line);
	free(buff);
	free(filename);
	free(dirname);
	free(currcommand);
	free(PointsToReadCount);
}
