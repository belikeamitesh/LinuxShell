#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

#define MAXIN 1024
#define MAXTOKENSIZE 64
#define MAX_NUM_TOKENS 64


void pipe1helper(int i, int o, char **pipearg){
	if(!fork()){
		dup2(i,0);
		close(i);
		dup2(o,1);
		close(o);
		execvp(pipearg[0],pipearg);
	}
}

void pipe1(int n, char ***pipeargs){
	int i=0,j,o,fd[2];
	for(j = 0; j < n-1; ++j){
		pipe(fd);
		pipe1helper(i,fd[1],pipeargs[j]);
		close(fd[1]);
		i = fd[0];
	}
	dup2(i,0);
	execvp(pipeargs[n-1][0],pipeargs[n-1]);
}




char **split(char *inputline)
{
	char *chpart = (char *)malloc(MAXTOKENSIZE * sizeof(char));
    char **splitchars = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	int i, charId = 0, partNo = 0;

	for(i =0; i < strlen(inputline); i++){

		char readChar = inputline[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			chpart[charId] = '\0';
			if (charId != 0){
				splitchars[partNo] = (char*)malloc(MAXTOKENSIZE *sizeof(char));
				strcpy(splitchars[partNo++], chpart);
				charId = 0; 
			}
		} else {
			chpart[charId++] = readChar;
		}
	}

	free(chpart);
	splitchars[partNo] = NULL ;
	return splitchars;
}

char ***psplit(char *inputline,int *n)
{
	char *pipearg = (char *)malloc(100* sizeof(char));
    char ***pipeargs = (char ***)malloc(10 * sizeof(char **));
	int i, charId = 0,j =0,k=0;

	for(j = 0; j < 10; ++j){
		pipeargs[j] = (char **)malloc(100 * sizeof(char *));
	}
	j = 0;
	for(i =0; i < strlen(inputline); i++){

		char readChar = inputline[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			pipearg[charId] = '\0';
			if (charId != 0){
				pipeargs[*n][k] = (char*)malloc(100*sizeof(char));
				strcpy(pipeargs[*n][k++], pipearg);
				charId = 0; 
			}
		} else if(readChar == '|'){
			++(*n);
			k = 0;
			
		}else {
			pipearg[charId++] = readChar;
		}
	}
	k = 0;
	++(*n);
	free(pipearg);
	pipeargs[*n][k] = NULL;
	return pipeargs;
}



int main(int argc, char* argv[]) {
	char  inputline[MAXIN];            
	char  **args;              
	int i;

	FILE* filep;
	if(argc == 2) {
		filep = fopen(argv[1],"r");
		if(filep < 0) {
			printf("The File does not exists.");
			return -1;
		}
	}
	char *path = (char *) malloc(sizeof(char) * 100);
	while(1) {
		fflush(stdin);			
		//bzero(inputline, sizeof(inputline));
		DIR *dir = NULL;
		struct dirent *sd = NULL;
		path = getcwd(path, 100);

		if(argc == 2) { 
			if(fgets(inputline, sizeof(inputline), filep) == NULL) { 
				break;	
			}
			inputline[strlen(inputline) - 1] = '\0';
		} else 
		{ 
			printf("~%s$ ",path);
			//printf("\n%s",inputline);
			if(fgets(inputline, sizeof(inputline), stdin) == NULL) { 
				putchar('\n');
				break;	
			}
		}
		

		inputline[strlen(inputline)] = '\n'; 
		args = split(inputline);

		int j = 0,flag = 0;
		while(args[j] != NULL){
			if(strcmp(args[j],"|")==0){
				flag = 1;
				break;
			}
			++j;
		}
		char ***pipeargs; 
		
		if(args[0] == NULL){
			continue;
		}
		else if(flag == 1){
			int n = 0;
			pipeargs = psplit(inputline,&n);
			pipe1(n,pipeargs);
			
		}
		else if(strcmp(args[0],"exit")==0){
			exit(0);
		}
		else if(strcmp(args[0],"source")==0){
			
			if(fork()==0)
			{
				//int fdin = open(args[1],O_RDONLY);
				//dup2(fdin,STDIN_FILENO);
				//close(fdin);
				char* arguments[3];
				arguments[0]= "./a.out";
				arguments[1]= args[1];
				arguments[2]=NULL;
				execvp(arguments[0],arguments);
			}
			wait(NULL);
		}
		else if(strcmp(args[0],"pwd")==0){
			printf("%s\n",path);
		}
		else if(strcmp(args[0],"ls")==0){
			dir = opendir((const char *)path);
			while((sd = readdir(dir))!= NULL){
				if(strcmp(sd->d_name,".") == 0 || strcmp(sd->d_name,"..") == 0 || sd->d_name[0]== '.'){
					continue;
				} else{
					printf("%s\t",sd->d_name);
				}
			}
			printf("\n");
		}
		else if(strcmp(args[0],"cd")==0){
			int result = chdir(args[1]);
		  	if(result != 0){
		    	switch(result){
		      		case EACCES: fprintf(stderr,"Cannot Access the File : Permission denied");
				    break;
				    case EIO: fprintf(stderr,"Error! An input output error occured");
				    break;
				    case ENAMETOOLONG: fprintf(stderr,"Path is to long");
				    break;
				    case ENOTDIR: fprintf(stderr,"A component of path not a directory"); 
				    break;
				    case ENOENT: fprintf(stderr,"No such file or directory");
				    break;
				    default: fprintf(stderr,"Couldn't find directory");
				    break;
		    	}
			}
		}
		else if(strcmp(args[0],"clear")==0){
			system("clear");
		}
		else if(strcmp(args[0],"mkdir")==0){
			char *p = args[1];
			int r = mkdir(p,0777);
		}
		else if(strcmp(args[0],"rm")==0){
			remove(args[1]);
		}
		
		else if(strcmp(args[0],"echo")==0){
			i = 1;
			while(args[i]!=NULL){
				printf("%s ",args[i]);
				++i;
			}
			printf("\n");
		}
		
		else if(strcmp(args[0],"cat")==0){
			int fd1=0,fd2=1;
			if(strcmp(args[1],"<")==0)
			{
				fd1 = open(args[2],O_RDONLY);
				if(args[3]!=NULL && strcmp(args[3],">")==0)
				{
					fd2= creat(args[4],0644);
				}
			}
			else if(strcmp(args[1],">")==0)
			{
				fd2 = creat(args[2],0644);
			}
			else{
				fd1 = open(args[1],O_RDONLY);
			}
			char c;
			if(fd1 < 0){
				fprintf(stderr,"Error : The file does Not exists.");
			}
			else{
				while(read(fd1,&c,1)==1){
					write(fd2,&c,1);
				}
				if(fd1 != 0)
				{
				close(fd1);
				}
				if(fd2!=1)
				{
					close(fd2);
				}
			}
			printf("\n");
		}
		
		else {
			int n = 0;
			while(args[n]){
				n++;
			}

			int id = fork();
			if(id < 0){
                //printf("fork error");
				printf("Error : The Command couldn't execute.");
			}
            //parent process
            else if(id > 0){
				if(strcmp(args[n-1],"&")==0){
					continue;
				}
				else{
					wait(NULL);
				}
			} 
            //child process
			else if(id==0){
				if(strcmp(args[n-1],"&")==0){
					args[n-1] = NULL;
				}
				execvp(args[0],args);
			}
		}
		
		for(i=0;args[i]!=NULL;i++){
			free(args[i]);
		}
		free(args);
	}
	return 0;

}