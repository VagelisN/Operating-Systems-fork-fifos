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
#include "funcs.h"

int main (int argc , char **argv)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~master process~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	if(argc!=7) 
	{
		printf("wrong number of arguements given\n");
		exit(-1);
	}
	int i,j=0,k,counter=0,workers,offset,*PointsToReadCount=NULL,*handlerids=NULL,rc,bytesread=0,plotcounter=0;
	char *tok=NULL,**commands=NULL,**colors=NULL,*line=NULL,*ch=NULL,**currcommand=NULL,*shape=NULL,*dirname=NULL,*filename=NULL,offsetbuff[20] ,ptoreadbuff[20],pathbuff[50],*buff=NULL;
	size_t len;
	pid_t pid;
	long size;	
	FILE *fin=NULL,*handlerout=NULL;
	struct pollfd *pfds=NULL;
	if(arg_parsing(argc,argv,&fin,&workers,&dirname,&filename,&size,&PointsToReadCount)!=0)exit(1);//command line arguement handling
	if((pfds=malloc(workers*sizeof(struct pollfd)))==NULL)exit(1);
	if((currcommand=malloc(6*sizeof(char*)))==NULL)exit(1);
	if((buff=malloc(1024*sizeof(char)))==NULL)exit(1);
	while ((getline(&line, &len,stdin))!= -1)//CLI
	{
		counter=1;
		ch=line;
		if(strcmp(ch,"exit\n")==0)break;
		while(*ch!=';')//count the number of individual shapes in every line given from stdin
		{
			if(*ch==',')
			{
				counter++;
			}
			ch++;
		}
		plotcounter++;
		if((commands=malloc(counter*sizeof(char*)))==NULL)exit(1);
		if((colors=malloc(counter*sizeof(char*)))==NULL)exit(1);//keep the color for gnuplot script
		if((handlerids=(int*)malloc(counter*sizeof(int)))==NULL)exit(1);//keep the handler ids to find the text files prodused by each handler for gnuplot script
		for (k = 0; k < counter; k++)
		{
			colors[k]=NULL;
		}	
		counter=0;
		for(tok=strtok(line,",;\n");tok!=NULL; tok=strtok(NULL,",;\n"))//split input in tokens
		{
			if((commands[counter]=malloc(sizeof(char)*(strlen(tok)+1)))==NULL)exit(1);
			strcpy(commands[counter],tok);
			counter++;
		}
		for (i = 0; i <counter; i++)//split each shape command
		{
			j=0;
			for(tok=strtok(commands[i]," ");tok!=NULL; tok=strtok(NULL," "))
			{
				if((currcommand[j]=malloc(sizeof(char)*strlen(tok)+1))==NULL)exit(1);
				strcpy(currcommand[j],tok);
				j++;
			}
			if(j==5)//5 args for circle and square 
			{
				if((colors[i]=malloc(sizeof(char)*(strlen(currcommand[4])+1)))==NULL)exit(1);
				strcpy(colors[i],currcommand[4]);
				currcommand[5]=NULL;
			}
			else//six args for the rest of the shapes
			{
				if((colors[i]=malloc(sizeof(char)*(strlen(currcommand[5])+1)))==NULL)exit(1);
				strcpy(colors[i],currcommand[5]);
			}
			if((shape=malloc(sizeof(char)*15))==NULL)exit(1);
			if(strcmp(currcommand[0],"circle")==0)strcpy(shape,"./circle");
			else if(strcmp(currcommand[0],"semicircle")==0)strcpy(shape,"./semicircle");
			else if(strcmp(currcommand[0],"square")==0)strcpy(shape,"./square");
			else if(strcmp(currcommand[0],"ellipse")==0)strcpy(shape,"./ellipse");
			else if(strcmp(currcommand[0],"ring")==0)strcpy(shape,"./ring");
			else 
			{
				printf("wrong shape given\n");
				exit(1);
			}
			pid=fork();
			/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~handler process~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
			if(pid==0)//In handler process
			{
				pid=getpid();
				sprintf(pathbuff,"./%s/%d.txt",dirname,pid);
				handlerout=fopen(pathbuff,"w");
				for (k = 0; k <workers ; k++)
				{
					sprintf(offsetbuff,"%d",(offset=(k*PointsToReadCount[k]*2*sizeof(float))));
					sprintf(ptoreadbuff,"%d",PointsToReadCount[k]);
					sprintf(pathbuff,"./%s/%d_w%d.fifo",dirname,pid,k);
					if ((mkfifo(pathbuff,0666))< 0)//make a named pipe for each worker
					{
						perror("Error creating the named pipe");
						exit(1);
					}
					if((pfds[k].fd=open(pathbuff,O_RDWR))<0)//open the pipes
						{
							printf("error opening file\n");
							exit(1);
						}
					/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~worker process~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
					if(fork()==0)//execute worker with args that are in currcommand
					{
	
						if(strcmp(currcommand[0],"circle")==0||strcmp(currcommand[0],"square")==0)
						{
							if(execl(shape,currcommand[0],"-a",currcommand[1],currcommand[2],currcommand[3],"-i",filename,"-o",pathbuff,"-f",offsetbuff,"-n",ptoreadbuff,NULL)==-1)
							{
									perror("exec failure ");
									exit(1);
							}
						}
						else
						{
							if(execl(shape,currcommand[0],"-a",currcommand[1],currcommand[2],currcommand[3],currcommand[4],"-i",filename,"-o",pathbuff,"-f",offsetbuff,"-n",ptoreadbuff,NULL)==-1)
							{
									perror("exec failure ");
									exit(1);
							}
						}
					}
					/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~end of worker process~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`*/
				}
				while(1)//wait to read from all the pipes
				{	
					for (k = 0; k <workers ; k++)
					{
						pfds[k].events=POLLIN;
						pfds[k].revents=0;
					}
					rc = poll(pfds,workers,100);
					if(rc<0)
					{
						printf("poll error\n");
						break;
					}
					else if(rc==0)//no more data to read pipes are closed
					{
						printf("poll timed out\n");
						break;
					}
					else if(rc >0)
					{
						for (k=0; k <workers ; k++)
						{
							
							if(pfds[k].revents & POLLIN)//as long as a pipe has data read from it
							{
								bytesread=read(pfds[k].fd,buff,1023);
								{
									buff[bytesread]='\0';
									fprintf(handlerout,"%s",buff);//print everything in the output file of the handler
								}
							}
						}
					}
				}
				handler_free(workers,counter,pfds,currcommand,colors,commands,handlerids,PointsToReadCount,shape,handlerout,line,buff,filename ,dirname);//everything from the master process was copied in the handler and has to be freed before exiting
				exit(1);
				/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ end of handler process ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
			}		
			else 
			{
				handlerids[i]=pid;//save the id of the handler
				for (k = 0; k < 6; k++)//free the current command 
				{
					free(currcommand[k]);
					currcommand[k]=NULL;
				}
				free(shape);
			}
		}
		for (k = 0;  k<counter; k++)wait(0);//wait for all handlers to finish
		if(plot(&buff,&fin,counter,dirname,handlerids,colors,plotcounter)!=0)exit(1);//create the gnuplot script and execute gnuplot with the script as input
		for (k = 0; k <counter ; k++)//free everything after the user has exited the CLI
		{
			free(colors[k]);
			free(commands[k]);
		}
		free(handlerids);
		free(colors);
		free(commands);
		commands=NULL;
	}
	free(pfds);
	free(line);
	free(buff);
	free(filename);
	free(currcommand);
	free(PointsToReadCount);
	if (fork()==0)//remove the directory created with all the temp files
	{
		execlp("rm" ,"rm", "-rf",dirname,NULL);
		exit(-1);
	}
	else wait(NULL);
	free(dirname);
	return 0;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~end of master process~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`*/
}
