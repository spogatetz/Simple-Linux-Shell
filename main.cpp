/* Simple Shell program: g++ shell.cpp -o myshell -lcrypt */ 
/* include c header files */
#include <stdlib.h>
#include <bits/stdc++.h>
#include <unistd.h> // for function fork()
#include <sys/wait.h> // for function wait()
#include <stdio.h>
#include <crypt.h>
// include c++ header files
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#define N 10 //increase N to 10 later for the final testing
#define PASSWORDCODE 1
#define COPYCODE 2
#define PROCESSCODE 3
#define FREEDISKCODE 4
#define SEARCHCODE 5
#define HISTORYCODE 6
#define LOGOUTCODE 7 

using namespace std;

void build_command();
void build_history(vector<string*> &command_log);
void save_history(vector<string*> &command_log);
void user_login();
void copy(string * parameters, size_t number_of_parameters);
void display_processes(string * parameters, size_t number_of_parameters);
void display_disk(string * parameters, size_t number_of_parameters);
void search_file(string * parameters, size_t number_of_parameters);
void show_history(string * parameters, size_t number_of_parameters, vector<string*> &command_log);
void type_prompt();
void change_password();
int read_command(string command, string *parameters, int &background, size_t &number_of_parameters, vector<string*> &command_log);
void exec_command(int opcode, string *parameters, size_t number_of_parameters, vector<string*> &command_log);

unordered_set<string> commands; // Use an unordered set for constant access time
string user;

int main()
{  
  	vector<string*> command_log;
  	int i=0,opcode=0;
  	int pid=0, status=0, background=0;
  	string command, parameters[3];
  	size_t number_of_parameters;
  	build_command();
  	build_history(command_log);
	/* read in the commands into a table or hash table */ 
	user_login();          /* Authenticate the user */
  	while (i < N) {        /* repeat maximum N times */
    		type_prompt( );    /* display prompt */
    		opcode = read_command(command, parameters, background, number_of_parameters, command_log);  
		/* input from terminal */
    		if (opcode>0) { /* valid command */
      			if (pid = fork() != 0) { /* Parent code */
        			if (!background)  {
          				pid = wait( &status); /* wait for child to exit */
	     				cout << "Parent: returned Child PID= " << pid << "\n";
					cout << "Parent: returned Child Status= " << status << "\n";
           				if (status == 0x0700) exit(0); /* status must be a hex number */
           				/* For example: LOGOUTCODE is 0x0500 is child terminated with the command exit(5) */
         			} /* end of parent code */
       			} else { /* Child code */
				cout << "Child: returned PID= " << pid << "\n";
           			exec_command (opcode, parameters, number_of_parameters, command_log);      /* execute command */ 
           			exit(0); 
      			} /* end of child code */
   		} else { cout << "Invalid command, try again\n"; } 
   		i++;
  	}
  	return 1;
}

void save_history(vector<string*> &command_log){
	ofstream command_file;
	command_file.open("previous_commands.txt", ios::trunc);
	if(command_file.is_open()){
		for(int i = 0; i < command_log.size(); i++){
			command_file << *command_log.at(i) << endl;
		}
	}

	//delete old pointers
	for( int i = 0 ; i < command_log.size(); i++){
		delete command_log.at(i);
	}
}

void build_history(vector<string*> &command_log){
	ifstream command_file;
	command_file.open("previous_commands.txt", ios::in);
	if(command_file.is_open()){
		string line;
		while(getline(command_file, line)){
			command_log.push_back(new string(line));
		}
	}
}

void build_command()
{
	//Read commands from file and insert into the unordered_set
	ifstream myfile;
	string line;
	myfile.open("commands.txt", ios::in);
	if(myfile.is_open()){
		while(getline(myfile, line)){
			commands.insert(line);
		}
		myfile.close();
	}
}

void user_login()
{
	string line;
	string password;
	string username;
	bool username_found = false;
	while(username_found == false){ // Loop until a correct username is supplied
		cout << "Enter your username: ";
		cin >> username;
		system("clear");
		ifstream password_file;
		password_file.open("password.txt", ios::in);
		if(password_file.is_open()){
			while(getline(password_file, line)){
				const char delim[2] = " ";
				string stored_username = strtok(&line[0], delim); 
				if(stored_username == username){
					password = line.substr(username.length() + 1, line.length() - username.length() - 1);
					username_found = true;		
				}
			}
			if(username_found == false){
				cout << "Username not found, please try again\n";
			}
		}
	}
	string entered_password;
	char* hash;
	string hash_as_string;
	do{ // Loop until correct password is supplied
		system("clear");
		cout << "Please enter your password: ";
		cin >> entered_password;
		hash = crypt(entered_password.c_str(), "12");
		hash_as_string = hash;
	}while(hash_as_string != password);
	user = username;
	system("clear");
	cin.ignore();
}

void type_prompt()
{
	cout << "SSH> ";
}

int read_command(string command, string *parameter, int &background, size_t &number_of_parameters, vector<string*> &command_log)
{
	// Check for an up arrow
	background = 0;
	int opcode=0;
	system("stty raw");
	char c;
	if((c = cin.peek()) == '\E'){ // Up arrow has been pressed, print previous command
		cout << '\r' << "Previous command entered: " << *command_log.back();
		cin.get(c);
		cin.get(c);
		cin.get(c);
		fflush(stdin);
		system("stty cooked");
		system("stty echo");
		cout << endl;
		type_prompt();
	} else { // Return to cooked mode
		system("stty cooked");
	}


	// Receive user input and process string
	getline(cin, command);
	if(command == ""){ // Prevent empty commands
		return opcode;
	}
	command_log.push_back(new string(command));
	const char delim[2] = " ";
	char* token = strtok(&command[0], delim);
	command = token;
	if(command.back() == '&'){
		//run in  background
		background = 1;
		command = command.substr(0, command.length() - 1); // Remove the & from command
	}

	// Gather parameters
	int param_index = 0;
	while(token != NULL) { 
		token = strtok(NULL, delim);
		if(token != NULL){
			parameter[param_index] = token;
			param_index += 1;
		}
	}

	
	// Search unordered_set for a matching command, then assign opcode
	number_of_parameters = param_index;
	unordered_set<string> :: iterator itr;
	for(itr = commands.begin(); itr != commands.end(); itr++){
		string text = *itr;
		string command_token = strtok(&text[0], delim);
		string opcode_token = strtok(NULL, delim);
		if(command == command_token){
			opcode = stoi(opcode_token);
		}
	}
	return opcode;	
}

void exec_command(int opcode, string *parameters, size_t number_of_parameters, vector<string*> &command_log)
{
	// Switch to issue command by opcode
	cout << "Child: Execute command function: 1" + (string)"\n";
	switch(opcode){
		case PASSWORDCODE:
			change_password();
			break;
		case COPYCODE:
			copy(parameters, number_of_parameters);
			break;
		case PROCESSCODE:
			display_processes(parameters, number_of_parameters);
			break;
		case FREEDISKCODE:
			display_disk(parameters, number_of_parameters);
			break;
		case SEARCHCODE:
			sleep(2);
			search_file(parameters, number_of_parameters);
			break;
		case HISTORYCODE:
			sleep(2);
			show_history(parameters, number_of_parameters, command_log);
			break;
		case LOGOUTCODE:
			save_history(command_log);
			cout << "Child: exit with status = LOGOUTCODE\n";
			exit(LOGOUTCODE);
			break;
	}
	return;
}

void show_history(string * parameters, size_t number_of_parameters, vector<string*> &command_log){
	cout << "Previous commands:" << endl;
	for(int i = 0;i < command_log.size();i++){
		cout << *command_log[i] << "\n";
	}
}

void search_file(string * parameters, size_t number_of_parameters){
	if(number_of_parameters == 2){
		string command = "grep " + parameters[0] + " " + parameters[1];
		system(command.c_str());
	} else {
		cout << "Invalid number of parameters" << endl;
	}
}

void display_disk(string * parameters, size_t number_of_parameters){
	if(number_of_parameters == 0){
		system("df -k");
	} else if (number_of_parameters == 1){
		string command = "df -k | grep " + parameters[0];
		system(command.c_str());
	} else {
		cout << "Invalid number of commands" << endl;
	}
}

void display_processes(string * parameters, size_t number_of_parameters){
	if(number_of_parameters == 0){
		system("ps -ef");
	} else if (number_of_parameters == 1){
		string command = "ps -ef | grep " + parameters[0];
		system(command.c_str());
	} else {
		cout << "Invalid number of parameters" << endl;
	}
}

void copy(string *parameters, size_t number_of_parameters){
	string file_one = parameters[0];
	string file_two = parameters[1];
	if(number_of_parameters == 2){
		string  command = "cp " + file_one + " " + file_two;
		system(command.c_str());
	}else if(number_of_parameters == 3){
		string file_three = parameters[2];
		string command = "cat " + file_one + " " + file_two + " > " + file_three;
		system(command.c_str());
	} else {
		cout << "Invalid number of parameters" << endl;
	}
}

void change_password(){
	// Gather new password from user
	cout << "Changing password for: " << user << endl;
	string new_pass;
	string new_pass_confirm;
	do{
		cout << "New Password: ";
		cin >> new_pass;
		cout << "Confirm Password: ";
		cin >> new_pass_confirm;
		if(new_pass != new_pass_confirm){
			cout << "\nPasswords do not match, try again. " << endl;
		}
	} while(new_pass != new_pass_confirm);

	// Store original lines in a vector
	string new_hash = crypt(new_pass.c_str(), "12");
	string line;
	fstream pwfile;
	vector<string> old_lines;
	pwfile.open("password.txt", ios::in);
	if(pwfile.is_open()){
		while(getline(pwfile, line)){
			old_lines.push_back(line);
		}
	}
	pwfile.close();

	// Update file with new password
	ofstream new_file;
	new_file.open("password.txt", ios::trunc);
	cout << new_file.is_open() << endl;
	if(new_file.is_open()){
		for(int i = 0; i < old_lines.size();i++){
			line = old_lines.at(i);
			if(line.substr(0, user.length()) == user){
				cout << "Password replaced" << endl;
				new_file << user << " " << new_hash << endl;		
			} else {
				new_file << old_lines.at(i);
			}
		}
	}
	new_file.close();
}
