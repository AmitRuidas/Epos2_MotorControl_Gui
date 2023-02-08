#include "ros/ros.h"
#include "pc7/Velocity.h"
#include <list>
#include "Definitions.h"
#include "wrap.h"
#include <signal.h>
#include <unistd.h>
#include<time.h>

		//If You Want To add a motor plss uncommand the commanded line & now Your 3rd Motor is Ready//

unsigned int ulErrorCode1 = 0;
unsigned int ulErrorCode2 = 0;
//unsigned int ulErrorCode3 = 0;
void* g_pKeyHandle1=0;
void* g_pKeyHandle2=0;
//void* g_pKeyHandle3=0;
unsigned short g_usNodeId1=1;
unsigned short g_usNodeId2=1;
//unsigned short g_usNodeId3=0;


void mySigintHandler(int sig)
{
  ROS_INFO("server shutdown.");
  ros::shutdown();
}

int start_motors()
{
	int lResult = MMC_FAILED;
	string g_deviceName = "EPOS2"; //EPOS version
	string g_protocolStackName = "MAXON SERIAL V2"; //MAXON_RS232
	string g_interfaceName = "USB"; //RS232
	string g_portName = "USB0";//"/dev/bus/usb/001/002";//"/dev/ttyS0"; // /dev/ttyS1
	int g_baudrate = 1000000; //115200

	if((g_pKeyHandle1 = OpenDevice(&ulErrorCode1, g_deviceName, g_protocolStackName, g_interfaceName, g_portName, g_baudrate, 		g_pKeyHandle1))==MMC_SUCCESS)
        {
		LogError("OpenDevice1 failed", lResult, ulErrorCode1);
		return lResult;
	}
	else 
		LogInfo("Open Device 1 Successfull");
	
	
}

void enable_motors()
{
	SetEnableState(g_pKeyHandle1, g_usNodeId1, &ulErrorCode1);
	ROS_INFO("Enabled MOTOR1");
	

}

void set_profile_velocity_mode()
{
	ActivateProfileVelocityMode(g_pKeyHandle1, g_usNodeId1, &ulErrorCode1);	
	ROS_INFO("Activated profile velocity mode for MOTOR1");
	
}

void disable_motors()
{
	SetDisableState(g_pKeyHandle1, g_usNodeId1, &ulErrorCode1);
}

int stop_motors()
{
	int lResult = MMC_FAILED;
	if((lResult = CloseDevice(&ulErrorCode1,g_pKeyHandle1))!=MMC_SUCCESS)
	{
		LogError("CloseDevice1 failed", lResult, ulErrorCode1);
		return lResult;
	}

	
}

bool moveWithVelocity(pc7::Velocity::Request &req, pc7::Velocity::Response &res)
{
	
	int lResult = MMC_FAILED;
	int absolute = 1;
        int relative = 0;
        int immediate = 1;
        int later = 0;
        
	ROS_INFO("request: velocity = %ld", req.velocity_new);
        
        list<long> velocityList;

	velocityList.push_back(req.velocity_new);

	for(list<long>::iterator it = velocityList.begin(); it !=velocityList.end(); it++)
	{
		long targetVelocity = (*it);
		stringstream msg;
		msg << "move With velocity = " << targetVelocity << ", node = " << g_usNodeId1;
		LogInfo(msg.str());

		if(VCS_MoveWithVelocity(g_pKeyHandle1, g_usNodeId1, targetVelocity,/* relative, immediate,*/ &ulErrorCode1) == 0)
		{
			LogError("VCS_MoveWithVelocity", lResult, ulErrorCode1);
			lResult = MMC_FAILED;
			break;
		}

		sleep(1);
	}
	
	
	int pNumber2 = 89;
	int* new_velocity = &pNumber2;

	get_velocity(g_pKeyHandle1, g_usNodeId1, new_velocity, &ulErrorCode1);
        ROS_INFO("new_velocity of MOTOR 1= %d", *new_velocity);
	res.velocity_old = *new_velocity;
	ROS_INFO("res.velocity_old of MOTOR1= %ld", res.velocity_old);
	
}

int main(int argc, char **argv)
{
 
        int lResult = MMC_FAILED;  	     
	ros::init(argc, argv, "velocity_server");
	ros::NodeHandle n;
	ros::Rate r(1); // 1 hz

	start_motors();
	enable_motors();
	set_profile_velocity_mode();	
	
	ros::ServiceServer service = n.advertiseService("applyVelocity", moveWithVelocity);

	ROS_INFO("Ready to move.");     

        signal(SIGINT, mySigintHandler);
	
        ros::spin();
	ros::spinOnce();
	r.sleep();

        disable_motors();
	stop_motors();
	
	
	return 0;
}
