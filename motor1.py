#!/usr/bin/env python
import rospy
from std_msgs.msg import String
from subprocess import Popen, PIPE
 
def callback(data):
    rospy.loginfo(rospy.get_caller_id() + 'rosrun pc7 velocity_server', data.data)
    #pop = subprocess.Popen(['rosrun', 'pc7', 'velocity_server']).communicate(pop)
     
def listener():
    rospy.init_node('Motor1', anonymous=True)
 
    rospy.Subscriber("vstart", String, callback)
 
#    rospy.spin()
  
if __name__ == '__main__':
    listener()

