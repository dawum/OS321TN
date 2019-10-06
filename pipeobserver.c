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

// Helper function to number of chars in string
int countChars(const char *s);

// Function to determine both commands to execute along with an array of their cooresponding arguments
void parseArgs(int argc, char *argv[], char* exec1, char* exec2, char* args1[], char* args2[]);

int main(int argc, char *argv[])
{

	// initialize executables 1 & 2 along with an array of args for each
	char* executable1 = "";
	char* executable2 = "";
	char* args1[8192] = {};
	char* args2[8192] = {};

	// Open and confirm that argv[1] is a file that can be opened and append if file exists. If not create with read write and execute permission. 
	int file = open(argv[1], O_WRONLY | O_APPEND | O_CREAT, 00700);

	if (file < 0){
		write(fileno(stderr), "Failed to open file\n", 29);
		exit(-1);
	}

	// Parse all argv[] components into 2 commands to execute and 2 arrays of args for each
	parseArgs(argc, argv, executable1, executable2, args1, args2);


printf("executable1 %p\n",executable1);

	close(file);
	//exit(-1);
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
void parseArgs(int argc, char *argv[], char* exec1, char* exec2, char* args1[], char* args2[])
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
	
		exec1 = argv[3];
		printf("exec1 %p\n",exec1);
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
		exec2 = argv[i];
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
