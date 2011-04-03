#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
using namespace std;

// Value definitions for string values
enum StringValue {
	evNotDefined,
	evHelp,
	evQuit,
	evTakeoff,
	evLand,
	evYaw,
	evPitch,
	evRoll,
	evGaz,
	evReset,
	evFile,
	evWait,
};

// Function was missing for some reason
void* memcpy(void* dest, const void* src, size_t count);

// Used to associate strings with enums
static map<string, StringValue> s_mapStringValues;

// Initialization method
static void initMap();
void *send_command(void *ptr);
void handleInput(string szInput, string val);
void runScript(string name);

// Initial Drone values
float roll = 0, pitch = 0, gaz = 0, yaw = 0;
int seq = 1, secWait = 0;

// Networking vars
int socketId, serverPort, bufferSize, size;
sockaddr_in serverAddr, clientAddr;
char buffer[200];

int main( int argc, char *argv[] ) {
	string script_name;


	// Initialize the string map
	initMap();

	// Initialize variables
	string szInput = "", hostName = "localhost";
	socketId = socket(AF_INET,SOCK_DGRAM,0);
	serverPort = 5556;
	pthread_t command_thread;

	sockaddr &serverAddrCast = (sockaddr &)serverAddr;

	//specify server address, port
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(serverPort);
	struct hostent*hp=gethostbyname(hostName.c_str());

	memcpy(( char *) &serverAddr.sin_addr, (char *) hp->h_addr, hp->h_length);
	size = sizeof(serverAddr);

	// Immediately reset the trim of the drone
	bufferSize = sprintf(buffer, "AT*FTRIM=%d,\r", seq);
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
	seq++;
	bufferSize = 0;

	// Start a thread that sends a command every 3 seconds
	pthread_create(&command_thread, NULL, send_command, NULL);

	// Check for any arguments (would be a script)
	if(argc == 2) {
		script_name = argv[1];
		// Run script in argument
		runScript(script_name);
	} else {
		// If no script given, use user input area
		while(1) {
			cout << "Enter a command or 'help' for list of commands: ";
			cin >> szInput;
			cout << "\n";
			switch(s_mapStringValues[szInput]) {
				case evFile:
					cout << "Enter a file name for the script: ";
					cin >> script_name;
					runScript(script_name);
					break;
				case evQuit:
					return 0;
					break;
				default: handleInput(szInput, ""); break;
			}

			cout << endl;
			// Send the command as UDP packet to Drone
			sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
			seq++;
			bufferSize = 0;
			cout << endl;
		}
	}

	close(socketId);

	return 0;
}

// Maps user inputs to enums
void initMap() {
	s_mapStringValues["help"] = evHelp;
	s_mapStringValues["takeoff"] = evTakeoff;
	s_mapStringValues["land"] = evLand;
	s_mapStringValues["yaw"] = evYaw;
	s_mapStringValues["pitch"] = evPitch;
	s_mapStringValues["roll"] = evRoll;
	s_mapStringValues["gaz"] = evGaz;
	s_mapStringValues["reset"] = evReset;
	s_mapStringValues["file"] = evFile;
	s_mapStringValues["q"] = evQuit;
	s_mapStringValues["wait"] = evWait;
}

// Thread method that sends commands every 100ms or so
void *send_command( void * ptr) {
	sockaddr &serverAddrCast = (sockaddr &)serverAddr;
	// Sends a command every 1.5 seconds (right now it just sets yaz, pitch, etc as they are)
	while(1) {
		bufferSize = sprintf(buffer, "AT*PCMD=%d,%d,%d,%d,%d,%d\r",
								seq,
								1,
								*(int*)(&roll),
								*(int*)(&pitch),
								*(int*)(&gaz),
								*(int*)(&yaw)	);
		sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
		seq++;
		bufferSize = sprintf(buffer, "AT*COMWDG=%d\r", seq);
		sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
		seq++;
		usleep(100000);
	}
}

// Main function for handling user input
void handleInput(string szInput, string val) {
	switch(s_mapStringValues[szInput]) {
		case evHelp:
			cout << "List of available commands:" << endl;
			cout << "reset: Resets the drone's emergency state and all values (pitch, roll, gaz, yaw)" << endl;
			cout << "takeoff: Tell the Drone to takeoff" << endl;
			cout << "land: Tell the Drone to land" << endl;
			cout << "yaw: Set the yaw (spin) for the Drone" << endl;
			cout << "pitch: Set the pitch (speed forward/backward) for the Drone" << endl;
			cout << "roll: Set the roll (speed left/right) for the Drone" << endl;
			cout << "gaz: Set the gaz (height) for the Drone" << endl;
			cout << "q: Exit program" << endl;
			break;
		case evTakeoff:
			bufferSize = sprintf(buffer, "AT*REF=%d,%d\r", seq, 290718208);
			break;
		case evLand:
			bufferSize = sprintf(buffer, "AT*REF=%d,%d\r", seq, 290717696);
			break;
		case evReset:
			roll = 0;
			pitch = 0;
			gaz = 0;
			yaw = 0;
			bufferSize = sprintf(buffer, "AT*REF=%d,%d\r", seq, 290717952);
			break;
		case evWait:
		case evYaw:
		case evPitch:
		case evRoll:
		case evGaz:
			if(val == "") {
				cout << "Enter a floating point value (-1 to 1): ";
				switch(s_mapStringValues[szInput]) {
					case evYaw: cin >> yaw; break;
					case evPitch: cin >> pitch; break;
					case evRoll: cin >> roll; break;
					case evGaz: cin >> gaz; break;
					default: break;
				}
			} else {
				switch(s_mapStringValues[szInput]) {
					case evYaw: yaw = strtod(val.c_str(), NULL); break;
					case evPitch: pitch = strtod(val.c_str(), NULL); break;
					case evRoll: roll = strtod(val.c_str(), NULL); break;
					case evGaz: gaz = strtod(val.c_str(), NULL); break;
					case evWait:
						secWait = strtol(val.c_str(), NULL, 10);
						char sleepCommand[50];
						// Uses /bin/sleep system command because there were problems
						// with the C++ sleep() command
						sprintf(sleepCommand, "/bin/sleep %d", secWait);
						int result;
						result = system(sleepCommand);
						break;
					default: break;
				}
			}
			bufferSize = sprintf(buffer, "AT*PCMD=%d,%d,%d,%d,%d,%d\r",
									seq,
									1,
									*(int*)(&roll),
									*(int*)(&pitch),
									*(int*)(&gaz),
									*(int*)(&yaw)	);
			break;
		default:
			bufferSize = 0;
			cout << "Invalid command" << endl;
			break;
	}
}


// Main function for running a script
void runScript(string name) {
	sockaddr &serverAddrCast = (sockaddr &)serverAddr;
	string line;
	ifstream script_file (name.c_str());
	// Work through the script line by line
	if(script_file.is_open()) {
		while(script_file.good()) {
			getline(script_file, line);
			cout << endl << "Read line: " << line << endl;
			if(line != "") {
				string sub1, sub2;
				int numSubs = 0;
				istringstream iss(line);
				do {
					if(numSubs == 0)
						iss >> sub1;
					if(numSubs == 1)
						iss >> sub2;
					numSubs++;
				} while(iss && numSubs < 2);

				if(numSubs == 2) {
					handleInput(sub1, sub2);
				} else {
					handleInput(line, "");
				}

				if(bufferSize != 0) {
					cout << "Sending command to localhost:" << serverPort << " ";
					for(unsigned int i = 0; i < sizeof(buffer); i++) {
						cout << buffer[i];
					}
					cout << endl;

					sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
				}
				seq++;
				bufferSize = 0;
			}
		}
		script_file.close();
	} else {
		cout << "Unable to open file";
	}
}

// When compiling for ARM, I had an error saying memcpy was not found,
// so I'm just including it here
void* memcpy(void* dest, const void* src, size_t count) {
        char* dst8 = (char*)dest;
        char* src8 = (char*)src;
        --src8;
        --dst8;

        while (count--) {
            *++dst8 = *++src8;
        }
        return dest;
}

