// pipeobserver.c
// Travis Carpenter and Dawum Nam
//
// A program that emulates the UNIX pipe command

#include <stdio.h>
#include <sys/mman.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>

	// initialize executables 1 & 2 along with an array of args for each
	char* executable1 = "";
	char* executable2 = "";
	char* args1[8192] = {};
	char* args2[8192] = {};
//	static char* exec1[] = {"ls","-a",NULL};
//	static char* exec2[]  = {"wc", "-l", NULL}; 
	char* exec1[8193] = {};
	char* exec2[8193] = {};


// Helper function to number of chars in string
int countChars(const char *s);

// Function to determine both commands to execute along with an array of their cooresponding arguments
static void parseArgs(int argc, char *argv[]);

int main(int argc, char *argv[])
{

	// Open and confirm that argv[1] is a file that can be opened and append if file exists. If not create with read write and execute permission. 
	int file = open(argv[1], O_WRONLY | O_APPEND | O_CREAT, 00700);
	int pipe1[2];

	if (file < 0){
		write(fileno(stderr), "Failed to open file\n", 29);
		exit(-1);
	}

	// Parse all argv[] components into 2 commands to execute and 2 arrays of args for each
	parseArgs(argc, argv);


	// Form the exectuables with the exec and args parsed with parseArgs
	int i = 1;
	exec1[0] = executable1;
	while(true){
		exec1[i] = args1[i-1];
		if (args1[i] == 0)
			break;
		i++;
	}

	i = 1;
	exec2[0] = executable2;
	while(true){
		exec2[i] = args2[i-1];
		if (args2[i] == 0)
			break;
		i++;
	}


	// create first pipe
	if (pipe(pipe1) == -1)
	{
		write(fileno(stderr),"Pipe Creation failed",20);
		exit(-1);
	}

	// Create child A
	pid_t childA = fork();

	// Child A Failed
	if (childA < 0)
	{
		write(fileno(stderr),"Fork failed",20);
		exit(-1);
	}
	// Parent of ChildA = process
	else if (childA > 0)
	{
		close(pipe1[1]);  // write of pipe 1 is closed for good. 

		pid_t childB = fork();

		// Child B Failed
		if (childB < 0)
		{
			write(fileno(stderr),"Fork failed",20);
			exit(EXIT_FAILURE);
		}
		// Parent of Child B = process
		else if (childB > 0)
		{
			close(pipe1[0]); // close write end of pipe1
			close(file);
			wait(NULL); //wait child to exit
			exit(EXIT_SUCCESS);
		}
		// ChildB
		else
		{
			//char buf[1];
			dup2(pipe1[0],STDIN_FILENO); //redirect stdoin of childB to read end of pipe1

			int pipe2[2];

			// create second pipe
			if (pipe(pipe2) == -1)
			{
				write(fileno(stderr),"Pipe Creation failed",20);
				exit(-1);
			}
			
			pid_t GchildA = fork(); // Create GChildA
			
			// GChildA Failed.
			if (GchildA < 0)
			{
				write(fileno(stderr),"fork failed",20);
				exit(-1);
			}
			
			// Child B
			else if (GchildA > 0)
			{
				close(file);
				close(pipe2[1]);
				wait(NULL);

				pid_t GchildB = fork();

				// GchildB Failed
				if (GchildB < 0)
				{
					write(fileno(stderr), "fork failed", 20);
					exit(EXIT_FAILURE);
				}

				// Child B
				else if (GchildB > 0)
				{
					close(pipe2[0]);
					wait(NULL);
					exit(EXIT_SUCCESS);
				}

				// GchildB
				else
				{
					dup2(pipe2[0],STDIN_FILENO);
					execvp(exec2[0],exec2);
					close(pipe2[0]);
					exit(EXIT_SUCCESS);
				}
				
			}
			// GChildA
			else
			{
				close(pipe2[0]); 
				dup2(pipe2[1],STDOUT_FILENO);
				char buf[1];
				while(read(pipe1[0],&buf,1) == 1)
				{
					write(file,buf,1);
					write(pipe2[1],buf,1);
				}
				close(file);
				close(pipe1[0]);
				close(pipe2[0]);
				close(pipe2[1]);
				exit(EXIT_SUCCESS);
			}
			
			/*
			while (read(pipe1[0],&buf,1) == 1)
			{
				write(fileno(stdout),buf,1);
			}
			close(pipe1[0]); */ 
		}
	}
	// ChildA
	else
	{
		close(pipe1[0]); // close 1/3
		dup2(pipe1[1],STDOUT_FILENO); //redirect stdout of childA to write end of pipe1
		execvp(exec1[0],exec1); // run executable 1
		close(pipe1[1]); // close 2/3 
		close(file); // close 3/3
		exit(EXIT_SUCCESS); // write successful!
	}
}

// Helper function to number of chars in string
int countChars(const char *s)
{
    int i = 0;
    while (s != NULL && s[i] != '\0')
    {
        i++;
    } 
    return i;
}
// Function to determine both commands to execute along with an array of their cooresponding arguments
static void parseArgs(int argc, char *argv[])
{
	int j = 0;			// counter for forming args1[] and args2[]
	int i = 4;			// counter for iterating through argv[]
	int numBracketPairs = 1;	// counter for bracket pairings
	// Form exec1 //

	// Check that argv[2] is "["
	if (argv[2][0] != 133 && argv[2][1] != 0){
		printf("Improperly formed argument list, closing.\n");
		exit(-1);
	}
	// If argv[2] is "[", set exec1 = argv[3]
	else{

		executable1 = argv[3];

	}

	// Form args1[] //
	while(i < argc){
		// If argv[i] is the final closing bracket, "]", args1[] is finished being formed. Increment argv[] counter
		if (argv[i][0] == 93 && argv[i][1] == 0 && numBracketPairs == 1){
			i++;
			numBracketPairs--;
			break;
		}
		// If argv[i] is "]" but not the final closing bracket, add it to args1[] and decrement numBracketPairs, increment argv[] and args1[] counters
		else if (argv[i][0] == 93 && argv[i][1] == 0 && numBracketPairs != 1){
			args1[j] = argv[i];
			i++;
			j++;
			numBracketPairs--;
		}

		else{
			// If argv[i] is another "[", add it to args1[] and increment numBrackPairs, argv[] and args1[] counters
			if (argv[i][0] == 91 && argv[i][1] == 0){
				args1[j] = argv[i];
				numBracketPairs++;
				i++;
				j++;
			}
			// add argv[i] to args1[] and increment argv[] and args1[] counters
			else{
				args1[j] = argv[i];
				i++;
				j++;
			}
		}
	}
	// If not all brackets were paired properly, argv[] is improperly formed
	if (numBracketPairs != 0){
		printf("Improperly formed argument list, closing.\n");
		exit(-1);
	}

	// For Testing Purposes //
//	for (int g=0; g<=j; g++)
//		printf("args1[%d] = %s\n",g, args1[g]);


	// Form exec2 //

	// Check if only one bracket command was entered
	if (argv[i] == 0){
		printf("Improperly formed argument list, closing.\n");
		exit(-1);
	}

	// Check that argv[i] is "["
	if (argv[i][0] != 133 && argv[i][1] != 0){
		printf("Improperly formed argument list, closing.\n");
		exit(-1);
	}
	// If argv[i] is "[", increment i, set exec2 to argv[i] and increment numBracketPairs and i
	else{
		i++;
		executable2 = argv[i];
		numBracketPairs++;
		i++;
	}

	// Reset j to 0
	j = 0;

	// Form args2[] //
	while(i < argc){
		// If argv[i] is the final closing bracket, "]", args2[] is finished being formed. Increment argv[] counter
		if (argv[i][0] == 93 && argv[i][1] == 0 && numBracketPairs == 1){
			i++;
			numBracketPairs--;
			break;
		}
		// If argv[i] is "]" but not the final closing bracket, add it to args2[] and decrement numBracketPairs, increment argv[] and args2[] counters
		else if (argv[i][0] == 93 && argv[i][1] == 0 && numBracketPairs != 1){
			args2[j] = argv[i];
			i++;
			j++;
			numBracketPairs--;
		}

		else{
			// If argv[i] is another "[", add it to args2[] and increment numBrackPairs, argv[] and args2[] counters
			if (argv[i][0] == 91 && argv[i][1] == 0){
				args2[j] = argv[i];
				numBracketPairs++;
				i++;
				j++;
			}
			// add argv[i] to args1[] and increment argv[] and args1[] counters
			else{
				args2[j] = argv[i];
				i++;
				j++;
			}
		}
	}

	// If not all brackets were paired properly, argv[] is improperly formed
	if (numBracketPairs != 0){
		printf("Improperly formed argument list, closing.\n");
		exit(-1);
	}


	// For Testing Purposes//
//	for (int g=0; g<=j; g++)
//		printf("args2[%d] = %s\n",g, args2[g]);

}
