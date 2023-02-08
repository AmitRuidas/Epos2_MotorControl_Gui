#include "ros/ros.h"
#include "pc7/Position.h"
#include <list>
#include "Definitions.h"
#include "wrap.h"
#include <signal.h>
#include <unistd.h>
#include <time.h>

unsigned int ulErrorCode3 = 0;
void* g_pKeyHandle3=0;
unsigned short g_usNodeId3=1;

void mySigintHandler(int sig)
{
  ROS_INFO("server shutdown.");
  ros::shutdown();
}
int start_motors()
{
	int lResult = MMC_FAILED;
	string g_deviceName = "EPOS2"; 			//EPOS version
	string g_protocolStackName = "MAXON SERIAL V2";	 //MAXON_RS232
	string g_interfaceName = "USB"; 	//RS232
	string g_portName = "USB2";	 //"/dev/bus/usb/001/002";//"/dev/ttyS0"; // /dev/ttyS1 //"/dev/bus/usb/005"
	int g_baudrate = 1000000;	 //115200				//SetDefaultParameters();

	if((g_pKeyHandle3 = OpenDevice(&ulErrorCode3, g_deviceName, g_protocolStackName, g_interfaceName, g_portName, g_baudrate, 		g_pKeyHandle3))==MMC_SUCCESS)
        {
		LogError("OpenDevice3 failed", lResult, ulErrorCode3);
		return lResult;
	}
	else 
		LogInfo("Open Device 3 Successfull");
}
void enable_motors()
{
	SetEnableState(g_pKeyHandle3, g_usNodeId3, &ulErrorCode3);
	ROS_INFO("Enabled Position_MOTOR3");
}
void set_profile_position_mode()
{
	ActivateProfilePositionMode(g_pKeyHandle3, g_usNodeId3, &ulErrorCode3);
	ROS_INFO("Activated profile position mode for MOTOR3");
}
void disable_motors()
{
	SetDisableState(g_pKeyHandle3, g_usNodeId3, &ulErrorCode3);
}
int stop_motors()
{
	int lResult = MMC_FAILED;
	if((lResult = CloseDevice(&ulErrorCode3,g_pKeyHandle3))!=MMC_SUCCESS)
	{
		LogError("CloseDevice", lResult, ulErrorCode3);
		return lResult;
	}
}
bool moveToPosition(pc7::Position::Request &req, pc7::Position::Response &res)
{
        int lResult = MMC_FAILED;
        int absolute = 1;
        int relative = 0;
        int immediate = 1;
        int later = 0; 
        ROS_INFO("request: position"); 

	ROS_INFO("request: position = %ld", req.position_new);
        list<long> positionList;
	positionList.push_back(req.position_new);
	for(list<long>::iterator it = positionList.begin(); it !=positionList.end(); it++)
	{
		long targetPosition = (*it);
		stringstream msg;
		msg << "move to position = " << targetPosition << ", node = " << g_usNodeId3;
		LogInfo(msg.str());
		if(VCS_MoveToPosition(g_pKeyHandle3, g_usNodeId3, targetPosition, relative, immediate, &ulErrorCode3) == 0)
		{
			LogError("VCS_MoveToPosition", lResult, ulErrorCode3);
			lResult = MMC_FAILED;
			break;
		}
		sleep(1);
	}
	int pNumber4 = 99;
	int* new_position3 = &pNumber4;
	get_position(g_pKeyHandle3, g_usNodeId3, new_position3, &ulErrorCode3);
        ROS_INFO("new_position = %d", *new_position3);
	res.position_old = *new_position3;
}

int main(int argc, char **argv)
{
  	int lResult = MMC_FAILED;	
	ros::init(argc, argv, "position_server");
	ros::NodeHandle n;
	//ros::Rate r(1); // 1 hz

	start_motors();
	enable_motors();
	set_profile_position_mode();

	ros::ServiceServer sub = n.advertiseService("applyPosition", moveToPosition);
	//ros::Subscriber sub = n.subscribe("username2", moveToPosition);
	ROS_INFO("Ready to move."); 

	signal(SIGINT, mySigintHandler);
	
	ros::spin();

	//ros::spinOnce();
	//r.sleep();

	disable_motors();
	stop_motors();

        return 0;
}
