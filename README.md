## ARDrone Embedded Client

This is a simple UDP client that runs on the ARDrone's embedded Linux system. The C++ file needs to be compiled with a ARM GNU/Linux cross-compiler
or you may use the already compiled DroneClient executable.

####How to Install:

1. (Optional) Compile for ARM GNU/Linux using an ARM cross-compiler
	- `$ arm-g++ -o "DroneClient" DroneClient.cpp`
2. Copy compiled executable onto ARDrone using FTP. 
	- Boot up ARDrone
	- Connect to ARDrone ad-hoc network
	- FTP into 192.168.1.1 as anonymous user/password using FTP GUI or command line
	- Copy ARDrone executable to ARDrone
3. Run client on ARDrone
	- Open a Terminal/Shell
	- Telnet into the ARDrone and make DroneClient executable
		- `$ telnet 192.168.1.1`
		- `$ cd /data/video`
		- `$ chmod +x DroneClient`
	- Run the Drone Client: `$ ./DroneClient`

		
####Using the Drone Client:
- There are two types of use for the Drone client. 

1. Simple command line input by the user. Example:
	- `$ takeoff`
	- `$ pitch -.3`
	- `$ gaz .2`
	- `$ pitch 0`
	- `$ gaz 0`
	- `$ land`
	
2. ARDrone scripts which follow the same format as the command line input but also include a **wait** command. An example script **sample.ard**
is included here as well. To execute the script you simply add it as an argument:
	- `$ ./DroneClient sample.ard`


####How it works:
The ARDrone is designed to receive UDP packets on port 5556. The packets can contain one or more AT commands which the Drone will then
read and react to. For the native client to run, you simply take this same idea but send the packets to port 5556 on localhost. The Drone
sees this as the same as any remote client sending AT commands and thus the client works as it should.

####Help
If you run into any issues or have any questions, feel free to send me an email at conor.branagan[at]gmail.com.

	