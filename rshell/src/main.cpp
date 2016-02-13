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
#include <signal.h>

using namespace std;

//------------------------------------------
//
//		prompt()
//
//------------------------------------------

void prompt() //print the prompt "$"
{
    char *host = new char[20];
	
	if ( gethostname(host, 20) == -1)
	{
		perror("Error with getting hostname");
	}

	cout << getlogin() << "@" << host << "$ ";
}

//----------------------------------------------------------------
//
//		delete_blank()
//
//----------------------------------------------------------------

//delete the blanks in front of and at the end of a string
void delete_blank(string &s)
{
    // Looking for blank spaces around command
    size_t b = s.find_first_not_of(" ");
    size_t e = s.find_last_not_of(" ");

    if ((b != std::string::npos) && (e != std::string::npos))
	    s = s.substr(b,e-b+1);
    else if ((b == std::string::npos) && (e == std::string::npos)) //when the string only contains blank space
		s.clear();
}


//----------------------------------------------------------------
//
//		delete_comment()
//
//----------------------------------------------------------------

//delete the content after # and delete blanks in the front and at the end
void delete_comment(string &s)
{
	if (!s.empty())
	{
		//delete comment from "#"
    	string s2 ("#");
    	size_t position_of_comment = s.find(s2);
    	s = s.substr(0, position_of_comment);
    	//replace '\t' to ' '
    	replace(s.begin(),s.end(),'\t',' ');
    	//delete blank in the front and at the end
    	delete_blank(s);
	}
}

//----------------------------------------------------------------
//
//	Find ; && || connectors and get the first command in the command line
//
//----------------------------------------------------------------

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

int connector = 0; // to indicate the kind of connector ;(1) or &&(2) or ||(3) or none(4)

void get_command_queue(string &s, string &t)
{
    size_t c = find_semicolon(s);
    size_t a = find_doubleand(s);
    size_t b = find_doublebar(s);

    if ((c!=std::string::npos && a!=std::string::npos && b!=std::string::npos && c<a && c<b) ||
        (c!=std::string::npos && a==std::string::npos && b!=std::string::npos && c<b) ||
        (c!=std::string::npos && a!=std::string::npos && b==std::string::npos && c<a) ||
        (c!=std::string::npos && a==std::string::npos && b==std::string::npos))
    {
        t = s.substr(0,c);	// get one command from the whole command line
		delete_blank(t);	// treat the first command: delete the blanks in the front and at the end
        s = s.substr(c+1);	// get rid of the command from the whole command line
        connector = 1;		// set the kind of the connector
    }

    else if ((c!=std::string::npos && a!=std::string::npos && b!=std::string::npos && a<c && a<b) ||
        	 (c!=std::string::npos && a!=std::string::npos && b==std::string::npos && a<c) ||
			 (c==std::string::npos && a!=std::string::npos && b!=std::string::npos && a<b) ||
			 (c==std::string::npos && a!=std::string::npos && b==std::string::npos))
    {
        t = s.substr(0,a);
		delete_blank(t);
        s = s.substr(a+2);
        connector = 2;
    }

    else if ((c!=std::string::npos && a!=std::string::npos && b!=std::string::npos && b<c && b<a) ||
			 (c==std::string::npos && a!=std::string::npos && b!=std::string::npos && b<a) ||
			 (c!=std::string::npos && a==std::string::npos && b!=std::string::npos && b<c) ||
			 (c==std::string::npos && a==std::string::npos && b!=std::string::npos))
    {
        t = s.substr(0,b);
		delete_blank(t);
        s = s.substr(b+2);
        connector = 3;
    }

    else if (c==std::string::npos && a==std::string::npos && b==std::string::npos)
    {
        t = s;				// if there is no connector in the whole command line
		delete_blank(t);
        s.clear();
        connector = 4;
    }

}

//----------------------------------------------------------------
//
//		Treat the command 
//
//----------------------------------------------------------------

void parse(char *line, char **argv)  // set the pointer for execvp()
{
	*argv = line;
   	if (*line == '\"') // treat the quote in the command like: echo "I am a student", the blanks in the quotes should be ignored
	{
		line++;
		while (*line != '\"')
		{
			if (*line == '\0') return;
			line++;
		}
	}
	line++;
	while (*line != '\0')
	{
		while (*line != ' ')
        {
			if (*line == '\"')
			{
				line++;
				while (*line != '\"')
				{
					if (*line == '\0') break;
					line++;
				}
            }
            if (*line == '\0') break;
            line++;
            if (*line == '\0') break;
        }
        if (*line == '\0') break;
		*line = '\0';
		line++;
		while (*line == ' ') line++;
		if (*line == '\0') break;
        argv++;
		*argv = line;
		if (*line == '\"')
        {
            line++;
            while (*line != '\"')
            {
                if (*line == '\0') break;
                line++;
            }
        }
        if (*line == '\0') break;
		line++;
	}
 	argv++;
 	*argv = '\0';
}

//----------------------------------------------------------------
//
//			Execute command
//
//----------------------------------------------------------------

pid_t pid;
int status;

int execute(char **execArgs)
{
	pid = fork();
	if(pid < 0)
	{
		perror("forking failed");
		return 0;		
	}
	else if (pid == 0)
	{
		int a;
		a = execvp(*execArgs, execArgs);
		if (a < 0)
		{
			perror("exec failed");
			return 0;			
			exit(1);
		}
		else return 1;
	}
    else
	{
		while(wait(&status)!=pid);
		return 1;
	}
}

//----------------------------------------------------------------
//
//		Input command
//
//----------------------------------------------------------------

void input_treat_command(string &command)
{
	getline(cin, command);		//get command line
	delete_comment(command);
}

//----------------------------------------------------------------
//
//		Storing commands that lead to bad PID
//
//----------------------------------------------------------------
//---------------------------FOR STORE WRONG COMMAND
vector<pid_t> wrong_pid;
pid_t this_pid;
//---------------------------FOR STORE WRONG COMMAND

//----------------------------------------------------------------
//
//			EXIT command
//
//----------------------------------------------------------------

void check_command_exit(string command)
{
	if(command == "exit") exit(0);
}

//----------------------------------------------------------------
//
//			Main Function
//
//----------------------------------------------------------------

int main ()
{
	int success;

	while(1)
	{
		string command;
		string command_queue;
		connector = 0;		// initialize connector

		prompt();
		input_treat_command(command);
		check_command_exit(command);

		if (!command.empty())	//if command is empty, not execute the following part, just back to the while loop
		{

			while (connector != 4)	// connector != 4 indicates there is still command(s) in the command line
			{
				get_command_queue(command, command_queue);
				delete_blank(command_queue);
				
				if (command_queue.empty()) break;
				
				check_command_exit(command_queue);
				
				char c[1024];
				strcpy(c, command_queue.c_str());
		    	char *execArgs[64];
		    	parse(c, execArgs);
				
				success = execute(execArgs);
				
//---------------------------------------------------------FOR STORE WRONG COMMAND			
				if (success == 1) this_pid = pid;
				if (success == 0)
				{
					wrong_pid.push_back(this_pid+1);
					cout << "*********************" << endl;
					cout << "This is a wrong command!" << endl;
					cout << "The child PID is " << wrong_pid.back() << endl;
					cout << "You will need exit multiple times!" << endl;
					cout << "*********************" << endl;
				}
//---------------------------------------------------------FOR STORE WRONG COMMAND
							

				if (connector == 2 && success == 0)
				{
					//kill_ps(pid, success);
					break;		// if the command being executed is followed by &&
				}				
				else if (connector == 3 && success == 1)
				{
					//kill_ps(pid, success);					
					break;	// if the command being executed is followed by ||
				}
				else ;//kill_ps(pid, success);

			}
		}
		else ;
		if (success == -1) break;
		else ;
	}
	return 0;
}

