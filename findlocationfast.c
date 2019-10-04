// findlocationfast.c
// Travis Carpenter and Nam Dawum
//
// A basic program that accepts a file and prefix as arguments and
// attempts to find said prefix in the file, outputting its cooresponding
// address when found and nothing if the prefix doesn't exist in file.

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


// Helper function that lseeks to the start of the next line
void seekNewLine(int file){
	char token = 0;
	while (token != 10){
		lseek(file, -2, SEEK_CUR);
		read(file, &token, 1);
	}
}

// Helper function that returns string length
int stringLength(char *string){
	int i = 0;
	while (string[i] != '\0'){
		++i;
	}
	return i;
}

// Helper function that converts char array to long
long arrayToLong(char *array) {
	int i=1;
	long result = (int)array[0] - '0';
	while(i<stringLength(array)){
		result = result * 10 + ((int)array[i] - '0');
		++i;
    }
	return result;
}

// Helper function that checks if the inputted prefix is properly formatted
int checkPrefix(int argc, char *argv[]){
	for (int i=0; i<=5; i++){
		if (isdigit(argv[2][i]) == 0){
			write(fileno(stderr), "Improperly formatted input prefix\n", 34);
			return -1;
		}
	}
	if (argv[2][6] != '\0'){
		write(fileno(stderr), "Improperly formatted input prefix\n", 34);
		return -1;
	}
	return 1;
}

// Helper function to get length of a char array
int getLength(const char* x){
	int i = -1;
	while (x[++i] != 0);
	return i;
}

// Helper function to form the address of a found prefix
void outputAddress(int file, long filePos){
	int i = 0;
	char prev[1] = "";
	char curr[1] = "";
	char address[27] = "";

	lseek(file, filePos, SEEK_SET);
	lseek(file, 6, SEEK_CUR);
	read(file, address, 26);

	// Determine length of address segment
	while(*curr != 32 && *prev != 32){
		prev[0] = curr[0];
		curr[0] = address[i];
		++i;
	}
	int length = getLength(curr);
	address[length] = '\0';

	// Ouput the address to stdout
	write(fileno(stdout), address, getLength(address));

	return;
}

// Binary search on file to attempt match with prefix
int binSearch(int file, long left, long right, int prefix)
{
	char leftToken[7] = "";
	char middleToken[7] = "";
	char rightToken[7] = "";
	long leftPrefix = 0;
	long middlePrefix = 0;
	long rightPrefix = 0;

	// Exit condition
	if (right >= left && right != (left + 1)) {
		long middle = (left + right) / 2;

		// Find left prefix value
		lseek(file, left, SEEK_SET);
		read(file, leftToken, 6);
		leftToken[6] = '\0';
		leftPrefix = arrayToLong(leftToken);

		// Find middle prefix value
		lseek(file, middle, SEEK_SET);
		seekNewLine(file);
		middle = lseek(file, 0, SEEK_CUR);
		read(file, middleToken, 6);
		middleToken[6] = '\0';
		middlePrefix = arrayToLong(middleToken);

		// Find right prefix value
		lseek(file, right, SEEK_SET);
		lseek(file, -32, SEEK_CUR);
		read(file, rightToken, 6);
		rightToken[6] = '\0';
		rightPrefix = arrayToLong(rightToken);

		// If we've passed prefix sought then it doesn't exist in file
		if (rightPrefix < prefix || leftPrefix > prefix)
			return -1;

		// If left prefix == prefix sought, write address to stdout and return 0
		if (leftPrefix == prefix){
			outputAddress(file, left);
			return 0;
		}

		// If right prefix == prefix sought, write address to stdout and return 0
		if (rightPrefix == prefix){
			outputAddress(file, (right - 32));
			return 0;
		}

		// If middle prefix == prefix sought, write address to stdout and return 0
		if (middlePrefix == prefix){
			outputAddress(file, middle);
			return 0;
		}

		// If middle prefix > prefix sought, recursively return
		// with binSearch on lower half
		if (middlePrefix > prefix){
			return binSearch(file, (left + 32), middle - 32, prefix);
		}

		// Else, middle prefix < prefix sought, resursively
		// return with binSearch on upper half
		return binSearch(file, middle, (right - 32), prefix);
	}

	// Prefix not found in file
	return -1;
}



int main(int argc, char *argv[])
{
	long prefix = arrayToLong(argv[2]);

	int file = open(argv[1], O_RDONLY);

	if (file < 0){
		write(fileno(stderr), "Failed to open file\n", 29);
		exit(-1);
	}

	// Find the EOF
	long end = lseek(file, 0, SEEK_END);

	// Check inputted prefix for proper format
	int val = checkPrefix(argc, argv);
	if (val < 0){
		close(file);
		exit(-1);
	}

	// Perform recursive binary search with 0 and end being the
	// upper and lower bounds of the search and argv[2] as the prefix
	int result = binSearch(file, 0, end, prefix);

	// Successful binSearch
	if (result == 0){
		close(file);
		exit(0);
	}

	// Not used
	if (result > 0){
		close(file);
		exit(1);
	}

	// Unsuccessful binSearch
	close(file);
	exit(-1);
}

