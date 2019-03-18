#ifndef FUNCS_H
#define FUNCS_H


int arg_parsing(int ,char**,FILE**, int*, char**,char**,long*,int**); 
int plot(char**,FILE**,int,char*,int*,char**,int);
void handler_free(int,int,struct pollfd*,char **,char **,char** ,int *,int*,char *,FILE *,char *,char *, char *,char *);

#endif
