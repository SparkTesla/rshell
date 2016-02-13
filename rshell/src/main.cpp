#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>


//#include "class.h"

using namespace std;

/* Prints the prompt of the main function to issue 
 * bash commands so that the user utilize our version
 * of a bash shell in a terminal
 * */
void prompt()
{
  char *host = new char[20];
  
	if( gethostname(host, 20) == -1)
  {
     perror("Error with getting hostname!");
  }

  // print the prompt "$"
  cout << getlogin() << "@" << host << "$ "; 
}

//---------------------------------------
/* treat string:
 * FOR EXAMPLE:	
 * FROM    "		ls -a ; echo "I am a student."		&&exit	#this ia a comment.."
 * TO						"ls -a ; echo "I am a student." &&exit
 */
//---------------------------------------


void delete_blank(string &s) // delete the blanks in front of an at the end of a string
{
	size_t b = s.find_first_not_of(" ");
	size_t e = s.find_last_not_of(" ");

	if((b != std::string::npos) && (e != std::string::npos))
					s = s.substr(b,e-b+1);
	else if ((b == std::string::npos) && (e == std::string::npos))
					s.clear();
}

//delete the content after # and delete blanks in the from and at the end
void delete_comment(string &s)
{
	if(!s.empty())
	{
	  //delete comment from "#"
		string s2 ("#");
		size_t position_of_comment = s.find(s2);
		s = s.substr(0, position_of_comment);
		//replace '\t' to ' '
		replace(s.begin(), s.end(), '\t', ' ');
		//delete blank in the front and at the end
		delete_blank(s);
	}
}
//=======================
/* ===============================================
 * The position of the connectors ( ; && || ) in
 * the command
*/
//================================================
size_t find_semicolon (string s)
{
	string t(";");
	size_t p = s.find(t);
	return p;
}

size_t find_doubleand (string s)
{
	string t("&&");
	size_t p = s.find(t);
	return p;
}

size_t find_doublebar (string s)
{
	string t("||");
	size_t p = s.find(t);
	return p;
}
 
// to indicate the kind of connector 1)';' 2) '&&' 3) '||' 4) none
int connector = 0;

void get_command_queue(string &s, string &t)
{
	size_t c = find_semicolon(s);
	size_t a = find_doubleand(s);
	size_t b = find_doublebar(s);

	if((c!=std::string::npos && a!=std::string::npos && b!=std::string::npos && (c<a) && (c<b)) ||
		 (c!=std::string::npos && a==std::string::npos && b!=std::string::npos && (c<b)) ||
     (c!=std::string::npos && a!=std::string::npos && b==std::string::npos && (c<a)) ||
     (c!=std::string::npos && a==std::string::npos && b==std::string::npos))
	{
		t = s.substr(0,c);	//get one command from the whole line
		delete_blank(t);		//treat the first command; delete the blanks in the front and at the end
	  s.substr(c+1); 			//get rid of the command from the whole line
		connector = 1;			//set the kind of the connector
	}

	else if((c!=std::string::npos && a!=std::string::npos && b!=std::string::npos && (a<c) && (a<b)) ||
				  (c!=std::string::npos && a!=std::string::npos && b==std::string::npos && (a<c)) ||
					(c==std::string::npos && a!=std::string::npos && b!=std::string::npos && (a<b)) ||
					(c==std::string::npos && a!=std::string::npos && b==std::string::npos))
	{
	  t = s.substr(0,a);
		delete_blank(t);
		s = s.substr(a+2);
		connector = 2;
	}
	
	else if((c!=std::string::npos && a!=std::string::npos && b!=std::string::npos && (b<c) && (b<a)) ||
				  (c==std::string::npos && a!=std::string::npos && b!=std::string::npos && (b<a)) ||
					(c!=std::string::npos && a==std::string::npos && b!=std::string::npos && (b<c)) ||
					(c==std::string::npos && a==std::string::npos && b!=std::string::npos))
	{
	  t = s.substr(0,b);
		delete_blank(t);
		s = s.substr(b+2);
		connector = 3;
	}

	else if(c==std::string::npos && a==std::string::npos && b==std::string::npos)
	{
		t = s;	// if there is no connector in the whole command line
		delete_blank(t);
		s.clear();
		connector = 4;
	}
}

//-----------------------------------------------------------------
// Parsing the command into key commands
// ----------------------------------------------------------------
void parse(char  *line, char **argv) //set the pointer for execvp()
{
	*argv = line;
	// treat the quote in the command like: echo "I am a student", 
	// the blanks in the quotes should be ignored
	if(*line == '\"')
	{
		line++;
		while (*line != '\"')
		{
			if (*line != '\0') return;
			line++;
		}
	}
	
	line++;
	while (*line != '\0')
	{
		while(*line != ' ')
		{
			if(*line == '\"')
			{
				line++;
				while (*line != '\"')
				{
					if(*line != '\"')
									break;
					line++;
				}
			}
			if (*line == '\0') 
							break;
			line++;
			if(*line == '\0') 
							break;
		}
		if(*line == '\0') 
						break;
		*line = '\0';
		line++;
		while (*line == ' ')
		{
						line++;
		}
		if(*line == '\0') 
						break;
		argv++;
		*argv = line;
		if (*line == '\"')
		{
			line++;
			while(*line != '\"')
			{
				if (*line == '\0') 
								break;
				break;
			}
		}
		if (*line == '\0') 
						break;
		line++;
	}
	*argv = '\0';
}

/* There is a small bug need to be fixed. The quotes should not be included
 * in the *argv
 * For example: $ echo "I am a student."
 * The result now is "I am a atudent."   // <----the quotes should not be there
*/

//----------------------------------------------
// Execute any number of commands using a queue
//----------------------------------------------
int execute(char **execArgs)
{
	pid_t pid;
	int status;
	pid = fork();

	if(pid < 0)
	{
		cout << "ERROR: forking failed" << endl;
		return 0;
	}

	else if (pid == 0)
	{
		int a;
		a = execvp(*execArgs, execArgs);
		if(a < 0)
		{
			cout << "ERROR: execvp failed" << endl;
			return 0;
		}
		else 
						return 1;
	}

	else
	{
		while(wait(&status) != pid);
	}
}
//------------------------------------------------
// Retrieve the command from user
//------------------------------------------------
void input_treat_command(string &command)
{
	getline(cin, command);	// Get command line
	delete_comment(command);
}

//------------------------------------------------
// Our version of exit() in rshell
//------------------------------------------------
void check_command_exit(string command)
{
	if(command == "exit") 
					exit(0);
}


//-------------------------------------------------------------
//
//
//												Main Function
//
//
//-------------------------------------------------------------
int main(int argc, char **argv)
{
  int success;  // store the result of the execute

	while(1)
  {
		string command;
		string command_queue;
		connector = 0;				// initialize connector counter

		prompt();
		input_treat_command(command);
		check_command_exit(command);

		// if command is empty, not execute the following part, 
		// just back to the while loop
		if( !command.empty() )
		{
			// coonector != 4 indicates there is still command(s) in the command line
			while (connector != 4)
			{
				get_command_queue(command, command_queue);
				check_command_exit(command_queue);
				char c[1024];
				strcpy(c, command_queue.c_str());
				char *execArgs[64];
				parse(c,execArgs);

				success = execute(execArgs);
				
				// If the command being executed is followed by &&
				if (connector == 2 && success == 0)
								break;
				// If the command being executed is followed by ||
				else if(connector == 3 && success == 1) 
								break;
				else ;
			}
		}
		else ;
		if (success == -1) break;
		else ;
	}
	return 0;
}
