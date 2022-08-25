/* 
 * Project 1: UNIX Shell
 * Programmer: Gregory L Whitman
 * Course: CMPE 320
 * Section: 1 (9:00-10:50am)
 * Instructor: S. Lee
*/ 

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
  Prototypes for custom shell commands
 */
int exitCommand(char **args);
int historyCommand(char **args);
int doLastCommand(char **args);

int place = 0;			// Place in history
char history[10][100] = {}; 	// Size of history

/*
   When a command is entered it will shut the shell down
 */
int exitCommand(char **args)
{
  return 0;	// Dont keep going and exit
}

/*
  This is where the code for the history command will go
*/
int historyCommand(char **args)
{
  if (place == 1)			// Error checking for no history
  {
    printf("There is no history yet\n");
    return 1;
  }

  if (place > 10)		// History is bigger than 10
  {
    for (int start = place - 10; start < place; start++)	// Increase start until = place
    {
       printf("%d: ", start);		// Print out line number
       printf("%s", history[start]);	// Print out command
    }
    return 1;				// Keep going
  }
  else				// History less than 10
  {
    for (int k=0; k < place; k++)	// Increase k until = place
    {
       printf("%d: ", k+1);	// Print out line number
       printf("%s", history[k]);// Print out command
    }
    return 1;
  }
}

/*
  This re runs previous command
*/
int doLastCommand(char **args)
{
  system(history[place-2]);	// Run previous command
  return 1;			// Keep going
}

/*
  Start the program and wait for it to be terminated
 */
int startShell(char **args)
{
  pid_t pid;	// Process/ID
  int status;	// Status of shell 0,1

  pid = fork();	// Fork process
  
  if (pid == 0) // Chiled process
  {
    if (execvp(args[0], args) == -1) // Execute command
    {
      perror("Could not execute command");	// Error
    }	
    exit(EXIT_FAILURE);		// Exit program
  } 
  else if (pid < 0) // Error with fork
  {
    perror("Error: fork failed.");	// Error
  } 
  else // Parent
  {
    do 
    {
      waitpid(pid, &status, WUNTRACED);			// Stopped or terminated
    } 
    while (!WIFEXITED(status) && !WIFSIGNALED(status));	// terminated normally or signaled to stop
  }
  
  return 1;	// Keep going
}

/*
   Checks to see if command entered is equivalent to
     one of the custom shell commands we made
 */
int runner(char **args)
{
  // No command given
  if (args[0] == NULL) 
  {
    printf("No Command was entered\n");
    return 1;		// No command entered
  }

  // History command
  if (strcmp(args[0], "history") == 0)	// Checks if they match
  {
    return historyCommand(args);	// Run specified method
  }

  // Exit shell/program 
  if (strcmp(args[0], "exit") == 0)	// Checks if they match
  {
    return exitCommand(args);		// Run specified method
  }

  // Do last command 
  if (strcmp(args[0], "!!") == 0)	// Checks if they match
  {
    return doLastCommand(args);		// Run specified method
  }

	// If no custom command was entered then
  return startShell(args);	// --> return the actual command entered
}

#define READLINE_BUFFERSIZE 1000
/*
   Reads the input enterd by user on commandline
 */
char *readCommandLine(void)
{
    char *buffer;		// Buffer fo the characters
    size_t bufsize = 32;	// Size of the buffer
    
    buffer = (char *)malloc(bufsize * sizeof(char));	// Creating memory for buffer

    if( buffer == NULL)				// Something went wrong
    {
        perror("Unable to allocate buffer");	// Error message
        exit(1);				// Exit the program
    }

    getline(&buffer, &bufsize, stdin);		// Loads line into buffer

    strcpy(history[place], buffer);		// Copy the buffer into history
    place++;
    
    return buffer;				// Return line typed in
}

#define TOKEN_BUFFERSIZE 200		// Size of buffer
#define TOKEN_DELIMETERS " \r\n\a\t"	// tab, newline, Carriage return
/**
   Breaks up command line into tokens aka tokenizing
 */
char **breakUpCommandLine(char *line)
{
  int bufferSize = TOKEN_BUFFERSIZE;			// Size of the buffer
  char **tokens = malloc(bufferSize * sizeof(char*));	// Creates memory for buffer
  char *tokenized;					// Broken up version of line
  int position = 0;					// Position in array of pointers

  tokenized = strtok(line, TOKEN_DELIMETERS);		// Breaks up line

  while (tokenized != NULL)				// Goes till no more line
  {
    tokens[position] = tokenized;			// Adds chunk to array of pointers at position
      position++;					// Increases position

    if (position >= bufferSize) 			// Out of memory need more
    {
      bufferSize = bufferSize + TOKEN_BUFFERSIZE;	// Doubles memory
      tokens = realloc(tokens, bufferSize * sizeof(char*));	// Reallocates memory with new size
    }

    tokenized = strtok(NULL, TOKEN_DELIMETERS);		// Adds null where a delimeter is spotted
  }
  tokens[position] = NULL;				// Adds null to end of chunks

  return tokens;					// All the chuncks broken up
}

/**
   Basic loop/begining of the shell, must get entered command
   tokenize it, then execute the command with a method
   that forks it. All the while checking if status is 1
   if not then exit shell. Also looks for !n command entered.
 */
void shellLoop(void)
{
  char *line;	// Original line typed into command line
  char **args;	// Tokenized arguments of command entered
  int status;	// Checks if program should run
  
  do
  {
    printf("osh> ");			// Always display the osh>
    line = readCommandLine();  		// Og line entered into shell

/**This section here is a little rough**
 *
 *  This is code isabout repeating !n where 
 *   n is a specific number in the history
 * 
 *   Always prints out: 
 *   Could not execute command: No such file or directory
 *       ***which is an error***
*/
  
    char *p = line;			// Set p to the command
    while (*p)				// Runs while p has characters
    {
      if (isdigit(*p))			// Checks for a digit
      {
        long val = strtol(p, &p, 10);	// Stores digit into value
        system(history[val]);		// Runs the specified command from history
      }
      else
      {
          p++;				// Move to next character
      }
    }

//**************************************    

      args = breakUpCommandLine(line);	// Tokenizing line
      status = runner(args);		// Checking then running command


      free(line);		// Free up memory of line
      free(args);		// Free up memory of args
  }
  while (status);	// Keep going while status is 1
}

/**
   Start of program
 */
int main(int argc, char **argv)
{

  shellLoop();		// Start of program/process

  return EXIT_SUCCESS;		// Exit program
}
