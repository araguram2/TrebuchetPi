// Server-Side Code for Trebuchet Pi
#include <wiringPi.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#define PORT 8081
#define WRITERANGE 1024
#define JOYRANGE 4095
#define DEADZONE_HIGH (((JOYRANGE)/2) + 100)
#define DEADZONE_LOW (((JOYRANGE)/2) - 100)
#define DRIVERATIO (WRITERANGE/(DEADZONE_LOW))
int main(int argc, char const *argv[]) 
{ 
	wiringPiSetup();
	pinMode(11,OUTPUT); //enable1
	pinMode(13,OUTPUT); //input1
	pinMode(15,OUTPUT); //input2
	pinMode(22,OUTPUT); //enable2
	pinMode(24,OUTPUT); //input3
	pinMode(26,OUTPUT); //input4
	pinMode(12,OUTPUT); //FIRE
	int server_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1;	
	int addrlen = sizeof(address); 
	char *hello = "y";
	char fromPhoton[4] = {0};
	int fireMask = 0x01000000;
	int jsxMask = 0x00fff000;
	int jsyMask = 0x00000fff;
	int Fire;
	int js_X;
	int js_Y;
	int counter = 0;


	
	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); ((JOYRANGE/2)-5);
		exit(EXIT_FAILURE); 
	} 
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
					(socklen_t*)&addrlen))<0) 
	{ 
		perror("accept"); 
		exit(EXIT_FAILURE); 
	} 
	
	while(1)
	{
		valread = read(new_socket , fromPhoton, 4);
	        int readIn = fromPhoton[0] | (fromPhoton[1] << 8) | (fromPhoton[2] << 16) | (fromPhoton[3] << 24);
		if (valread != 0)
			
	       		Fire = (((int)readIn) & fireMask) >> 24;
			js_X = (((int)readIn) & jsxMask) >> 12;
			js_Y = ((int)readIn) & jsyMask;

		

			if (((js_Y >= DEADZONE_LOW) && (js_Y <= DEADZONE_HIGH)) && ((js_X >= DEADZONE_LOW ) && (js_X <= DEADZONE_HIGH))) //deadzone
			{
				digitalWrite(0,0);
				digitalWrite(3,0);
				//printf("DEADZONE\n");
			}
			else if (abs(js_X-JOYRANGE/2) > abs(js_Y-JOYRANGE/2))
			{
				if (js_X > DEADZONE_HIGH)
				{
					pwmWrite(11, DRIVERATIO * (js_X-DEADZONE_HIGH));
					digitalWrite(13, 1);
					digitalWrite(15, 0);
					pwmWrite(22, DRIVERATIO * (js_X-DEADZONE_HIGH));
					digitalWrite(24, 0);
					digitalWrite(26, 1);
					//printf("turning right\n");
				}
				else if (js_X < DEADZONE_LOW)
				{	
					pwmWrite(11, DRIVERATIO * (DEADZONE_LOW-js_X));
					digitalWrite(13, 0);
					digitalWrite(15, 1);
					pwmWrite(22, DRIVERATIO * (DEADZONE_LOW-js_X));
					digitalWrite(24, 1);
					digitalWrite(26, 0);
					//printf("turning left\n");

				}
			}
			else if (abs(js_Y-JOYRANGE/2) > abs(js_X-JOYRANGE/2))
			{
				if (js_Y > DEADZONE_HIGH)
				{
					pwmWrite(11, DRIVERATIO * (js_Y-DEADZONE_HIGH));
					digitalWrite(13, 1);
					digitalWrite(15, 0);
					pwmWrite(22, DRIVERATIO * (js_Y-DEADZONE_HIGH));
					digitalWrite(24, 1);
					digitalWrite(26, 0);
					//printf("Going forward\n");


				}
				else if (js_Y < DEADZONE_LOW)
				{
					pwmWrite(11, DRIVERATIO * (DEADZONE_LOW-js_Y));
					digitalWrite(13, 0);
					digitalWrite(15, 1);
					pwmWrite(22, DRIVERATIO * (DEADZONE_LOW-js_Y));
					digitalWrite(24, 0);
					digitalWrite(26, 1);
					//printf("Going Backward\n");

				}
			}
			if (Fire)
			{
				pwmWrite(12, 175);
				//printf("FIRE\n");
			}
			else
			{
				pwmWrite(12, 0);
			}
	}
	return 0; 
} 

