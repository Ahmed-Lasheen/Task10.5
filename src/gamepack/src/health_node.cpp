#include "ros/ros.h"
#include "std_msgs/String.h"
#include "turtlesim/Pose.h"
#include <cmath>

class DistanceCalculator
{
public:
    DistanceCalculator()
    {
        // Initialize the ROS node
        ros::NodeHandle nh;
    
        // Initialize the subscribers for the turtles' positions
        
        turtle1_sub_ = nh.subscribe("/turtle1/pose", 10, &DistanceCalculator::turtle1Callback, this);
        turtle2_sub_ = nh.subscribe("/turtle2/pose", 10, &DistanceCalculator::turtle2Callback, this);
        sayed =  nh.subscribe("effect", 1000,&DistanceCalculator::calculateAndPublishDistance, this);
        // Initialize the publisher for the distance
        distance_pub_ = nh.advertise<std_msgs::String>("health", 10);
    }

    void turtle1Callback(const turtlesim::Pose::ConstPtr& msg)
    {
        turtle1_pose_ = *msg;
        
    }

    void turtle2Callback(const turtlesim::Pose::ConstPtr& msg)
    {
        turtle2_pose_ = *msg;
       
    }
   

    void calculateAndPublishDistance(const std_msgs::String::ConstPtr& msg)
    {
        
        float dx = turtle2_pose_.x - turtle1_pose_.x;
        float dy = turtle2_pose_.y - turtle1_pose_.y;
        float distance = std::sqrt(dx * dx + dy * dy);
            
        if (distance <= 0.7)
        {
            std_msgs::String distance_msg;
            
            if (msg->data == "turtle1") {
                distance_msg.data = "turtle2";
            } else {
                distance_msg.data = "turtle1";
            }
            distance_pub_.publish(distance_msg);
            ROS_INFO(" ATTACK! ");
        }
    }

private:
    ros::Subscriber turtle1_sub_;
    ros::Subscriber turtle2_sub_;
    ros::Publisher distance_pub_;
    turtlesim::Pose turtle1_pose_;
    turtlesim::Pose turtle2_pose_;
    ros::Subscriber sayed;
    bool turt1;
    bool turt2;

};

int main(int argc, char **argv)
{ 
    ros::init(argc, argv, "health_cpp");

    DistanceCalculator turtles; // Create a single instance of DistanceCalculator
    ros::spin(); // Enter the loop to process callbacks
    return 0;
}
