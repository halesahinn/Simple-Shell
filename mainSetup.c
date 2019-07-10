#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <libgen.h>
#include <sys/types.h>
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
 
int argNumber;
int isRedirection(char *a[]){
    char *s = ">";
    char *p = ">>";
    char *t = "2>";
    int i = 0;
    while(a[i]) {
       if((strcmp(a[i], s) == 0)||(strcmp(a[i], p) == 0)||(strcmp(a[i], t) == 0)){
                 return 1; }
            i++; }
return 0;}

int isRedirectionReverse(char *a[]){
    char *r = "<";
    int i = 0;
    while(a[i]) {
       if((strcmp(a[i], r) == 0)){
                 return 1; }
            i++; }
return 0;}

int isRedirectionForward(char *a[]){
    char *r = ">";
    int i = 0;
    while(a[i]) {
       if((strcmp(a[i], r) == 0)){
                 return 1; }
            i++; }
return 0;}
int isPipeline(char *a[]){
    char *s = "|";
    int i = 0;
    while(a[i]) {
       if((strcmp(a[i], s) == 0)){

                 return 1; }
            i++; }
return 0;}

/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

void setup(char inputBuffer[], char *args[],int *background)
{
argNumber=0;
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
    
    ct = 0;
        
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);  

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	exit(-1);           /* terminate with error code of -1 */
    }

	//printf(">>%s<<",inputBuffer);
    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

        switch (inputBuffer[i]){
	    case ' ':
	    case '\t' :               /* argument separators */
		if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
		    ct++;
		}
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
		start = -1;
		break;

            case '\n':                 /* should be the final char examined */
		if (start != -1){
                    args[ct] = &inputBuffer[start];     
		    ct++;
		}
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
		break;

	    default :             /* some other character */
		if (start == -1)
		    start = i;
                if (inputBuffer[i] == '&'){
		    *background  = 1;
                    inputBuffer[i-1] = '\0';
		}
	} /* end of switch */
     }    /* end of for */
     args[ct] = NULL; /* just in case the input line was > 80 */
argNumber=ct;
	/*for (i = 0; i <= ct; i++)
		printf("args %d = %s\n",i,args[i]);*/
} /* end of setup routine */
 
int main(void)
{

            char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
            int background; /* equals 1 if a command is followed by '&' */
            char *args[MAX_LINE/2 + 1]; /*command line arguments */
            while (1){
                        
                        background = 0;
                        fprintf(stderr,"myshell: ");
                        /*setup() calls exit() when Control-D is entered */
                      setup(inputBuffer, args, &background);  

//Pipeline  "ls -al | sort"
if(isPipeline(args)==1){
char* bin= "/bin/";
char path[50];
strcpy( path, bin );
char* a = strcat( path, args[0] );
pid_t pid1;
int fd[2];
pipe(fd); //create a pipe
pid1 = fork();
if (pid1 < 0) {
  perror("Could not create process");
}
   if (pid1 == 0) {
        if (dup2(fd[1], STDOUT_FILENO) == -1) 
         perror("Failed to redirect stdout of ls");
      else if ((close(fd[0]) == -1) || (close(fd[1]) == -1)) 
         perror("Failed to close extra pipe descriptors on ls");
      else { 
         if(strcmp(args[1],"|")==0){
         execl(a,args[0], NULL);//execl
         }else{
         execl(a,args[0], args[1], NULL);//execl
         }perror("Failed to exec ls");}
}
   else{
      if (dup2(fd[0], STDIN_FILENO) == -1)               /* sort is the parent */
       perror("Failed to redirect stdin of sort");
      else if ((close(fd[0]) == -1) || (close(fd[1]) == -1)) 
       perror("Failed to close extra pipe file descriptors on sort"); 
      else {
       if(strcmp(args[1],"|")==0){
         execl(a,args[2], NULL);//execl
         }else{
         execl(a,args[3], NULL);//execl
         }perror("Failed to exec sort");}
}
}
//Ctrl+Z entered ^Z    
if(SIGTSTP==1){
if(background=0){//no background process
kill(getpid(),SIGTSTP);
}
perror("Could not stop foreground processes");
}
//move to fg
else if(strcmp(args[0],"fg")==0){

}
//clear the screen
if (strcmp(args[0],"clr")==0){
        system("clear");
}
//Exit from the program
else if (strcmp(args[0],"exit")==0){
if(background==1){
fprintf(stderr,"There are process runnin in background, Could'nt exit.");
}else{
        exit(0);
}
}
// ls > files.out - or - ls -al >> file.out
else if(isRedirection(args) && !(isRedirectionReverse(args))){
int i;
char cmd[1000] ="\0";
for(i=0; i < argNumber ; i++){
strcat(cmd, args[i]);
strcat(cmd, " "); }
char *commandLine[] = {"/bin/sh", "-c", cmd, NULL};
pid_t cpid;
cpid = fork();
if(cpid == 0){
if(execv("/bin/sh", commandLine) != 0){
fprintf(stderr,"Must be an execv error Rec\n"); }
}
if(cpid  != 0){
wait(NULL) ; } 

}
// ls < files.out
else if(isRedirectionReverse(args) && argNumber==3){

char cmd[1000] ="\0";
strcat(cmd, args[0]);
strcat(cmd, " ");
strcat(cmd, "$(cat ");
strcat(cmd, args[2]);
strcat(cmd, ")");
char *command[] = {"/bin/sh", "-c", cmd, NULL};
pid_t cpid;
cpid = fork();
if(cpid == 0){
if(execv("/bin/sh", command) != 0){ //exec command line in forked new process
fprintf(stderr,"Must be an execv error Rec\n"); }
}
if(cpid  != 0){  //youre in the parent
wait(NULL) ; } //parent wait
}

// ls < file.in > file.out
else if(isRedirectionReverse(args) && isRedirectionForward(args) && argNumber==5){
char cmd[1000] ="\0";
strcat(cmd, args[0]);
strcat(cmd, " ");
strcat(cmd, "$(cat ");
strcat(cmd, args[2]);
strcat(cmd, ")");  //create the reverse command
char *commandLine1[] = {"/bin/sh", "-c", cmd, NULL};
char cmd2[1000] ="\0";
strcat(cmd2,cmd);
strcat(cmd2, " > ");
strcat(cmd2, args[4]); //create the forward command
char *commandLine2[] = {"/bin/sh", "-c", cmd2, NULL};

pid_t cpid;
cpid = fork();
if(execv("/bin/sh", commandLine2) != 0){  //parent do the second  part of the process which is writing into file
fprintf(stderr,"Must be an redirection error\n"); }

if(cpid == 0){
if(execv("/bin/sh", commandLine1) != 0){    //child do the first part of the process
fprintf(stderr,"Must be an redirection error\n"); }
}
}
//Use execl()
else{
char* bin= "/bin/";
char path[50];
strcpy( path, bin );
char* a = strcat( path, args[0] );
   pid_t childpid;
   childpid = fork();

if (childpid == -1)  {
       perror("Failed to fork");
       return 1; 
   }

   if (childpid == 0) { // child code 
      if(execl(a, args[0],args[1],args[2],args[3],args[4],NULL)!=0){
    fprintf(stderr,"There is an execl error\n"); 
    exit(0);
}
       perror("Child failed to execl");
       return 1; 
   }
if(background  == 0){ //wait if it is background
wait(NULL) ; } 
}
 
}//end while
}//end main
