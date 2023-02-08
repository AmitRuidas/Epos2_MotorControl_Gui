#include "ros/ros.h"
#include "pc7/Velocity.h"
#include <list>
#include "Definitions.h"
#include "wrap.h"
#include <signal.h>
#include <unistd.h>


unsigned int ulErrorCode1 = 0;
unsigned int ulErrorCode2 = 0;
void* g_pKeyHandle1=0;
void* g_pKeyHandle2=0;
unsigned short g_usNodeId1=1;
unsigned short g_usNodeId2=1;


void mySigintHandler(int sig)
{
  ROS_INFO("server shutdown.");
  ros::shutdown();
}

bool moveToVelocity(pc7::Velocity::Request &req, pc7::Velocity::Response &res)
{
	
        int lResult = MMC_FAILED;
        int absolute = 1;
        int relative = 0;
        int immediate = 1;
        int later = 0;
        int* old_velocity;
	int pNumber1=88;
	old_velocity=&pNumber1;
	        
 	
        string g_deviceName = "EPOS2"; //EPOS version
	string g_protocolStackName = "MAXON SERIAL V2"; //MAXON_RS232
	string g_interfaceName = "USB"; //RS232
	string g_portName = "USB0";//"/dev/ttyS0"; // /dev/ttyS1
	int g_baudrate = 1000000; //115200
		
	
        g_portName = "USB1"; //"/dev/ttyS1"; // /dev/ttyS1
	
	if((g_pKeyHandle2 = OpenDevice(&ulErrorCode2, g_deviceName, g_protocolStackName, g_interfaceName, g_portName, g_baudrate, g_pKeyHandle2))==MMC_SUCCESS)
        {
		LogError("OpenDevice2 failed", lResult, ulErrorCode2);
		return lResult;
	}
	
	else 
		LogInfo("Open Device 2 Successfull");
         printf("KeyHandle2 =  %p", g_pKeyHandle2);

	SetEnableState(g_pKeyHandle2, g_usNodeId1, &ulErrorCode2);
	ROS_INFO("Enabled MOTOR2");

	
	printf("KeyHandle2 =  %p", g_pKeyHandle2);
	ActivateProfileVelocityMode(g_pKeyHandle2, g_usNodeId1, &ulErrorCode2);
	ROS_INFO("Activated profile velocity mode for MOTOR2");

	get_velocity(g_pKeyHandle2, g_usNodeId1, old_velocity, &ulErrorCode2);
        ROS_INFO("previous: velocity = %d  node = %d", *old_velocity, g_usNodeId1);
	ROS_INFO("request: velocity = %ld", req.velocity_new);
        
        list<long> velocityList;

	velocityList.push_back(req.velocity_new);


	for(list<long>::iterator it = velocityList.begin(); it !=velocityList.end(); it++)
	{
		long targetVelocity = (*it);
		stringstream msg;
		msg << "move With velocity = " << targetVelocity << ", node = " << g_usNodeId2;
		LogInfo(msg.str());

		if(VCS_MoveWithVelocity(g_pKeyHandle2, g_usNodeId1, targetVelocity,/* relative, immediate,*/ &ulErrorCode2) == 0)
		{
			LogError("VCS_MoveWithVelocity", lResult, ulErrorCode2);
			lResult = MMC_FAILED;
			break;
		}

		sleep(1);
	}
	
	int* new_velocity; 
	int pNumber2=89;
	new_velocity=&pNumber2;

	ROS_INFO("new_velocity = %d", *new_velocity);
	get_velocity(g_pKeyHandle2, g_usNodeId2, new_velocity, &ulErrorCode2);
        ROS_INFO("new_velocity = %d", *new_velocity);
	res.velocity_old = *new_velocity;
	ROS_INFO("res.velocity_old = %ld", res.velocity_old);
}

int main(int argc, char **argv)
{
 
        int lResult = MMC_FAILED;       
	ros::init(argc, argv, "velocity_server");
	ros::NodeHandle n;

	ros::ServiceServer service = n.advertiseService("applyVelocity", moveToVelocity);

	ROS_INFO("Ready to move.");        

        signal(SIGINT, mySigintHandler);
	
	ros::spin();

	SetDisableState(g_pKeyHandle2, g_usNodeId1, &ulErrorCode2);
	if((lResult = CloseDevice(&ulErrorCode2,g_pKeyHandle2))!=MMC_SUCCESS)
	{
		LogError("CloseDevice2 failed", lResult, ulErrorCode2);
		return lResult;
	}
	
	return 0;
}
