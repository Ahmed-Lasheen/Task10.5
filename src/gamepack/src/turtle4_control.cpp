#include <ros/ros.h>
//////////////
#include <geometry_msgs/Twist.h>
#include <turtlesim/Pose.h>
//////////////
#include <cmath>
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>  // For read()

#define KEYCODE_R 0x64
#define KEYCODE_L 0x61
#define KEYCODE_U 0x77
#define KEYCODE_D 0x73
#define KEYCODE_Q 0x78

////////////
class DistanceCalculator
{
public:
    DistanceCalculator()
    {
        sub1_ = nh_.subscribe("/turtle1/pose", 10, &DistanceCalculator::pose1Callback, this);
        sub2_ = nh_.subscribe("/turtle2/pose", 10, &DistanceCalculator::pose2Callback, this);
        sub3_ = nh_.subscribe("/turtle3/pose", 10, &DistanceCalculator::pose3Callback, this);
        sub4_ = nh_.subscribe("/turtle4/pose", 10, &DistanceCalculator::pose4Callback, this);
    }

private:
    ros::NodeHandle nh_;
    ros::Subscriber sub1_;
    ros::Subscriber sub2_;
    ros::Subscriber sub3_;
    ros::Subscriber sub4_;
    turtlesim::Pose pose1_;
    turtlesim::Pose pose2_;
    turtlesim::Pose pose3_;
    turtlesim::Pose pose4_;
    bool pose1_received_ = false;
    bool pose2_received_ = false;
    bool pose3_received_ = false;
    bool pose4_received_ = false;

    void pose1Callback(const turtlesim::Pose::ConstPtr& msg)
    {
        pose1_ = *msg;
        pose1_received_ = true;
        calculateDistance();
    }

    void pose2Callback(const turtlesim::Pose::ConstPtr& msg)
    {
        pose2_ = *msg;
        pose2_received_ = true;
        calculateDistance();
    }

    void pose3Callback(const turtlesim::Pose::ConstPtr& msg)
    {
        pose3_ = *msg;
        pose3_received_ = true;
        calculateDistance();
    }

    void pose4Callback(const turtlesim::Pose::ConstPtr& msg)
    {
        pose4_ = *msg;
        pose4_received_ = true;
        calculateDistance();
    }

    void calculateDistance()
    {
        if (pose1_received_ && pose2_received_ && pose3_received_ && pose4_received_)
        {
            double x1 = pose1_.x;
            double y1 = pose1_.y;
            double x2 = pose2_.x;
            double y2 = pose2_.y;
            double x3 = pose3_.x;
            double y3 = pose3_.y;
            double x4 = pose4_.x;
            double y4 = pose4_.y;
            double distance1 = std::sqrt(std::pow(x1 - x4, 2) + std::pow(y1 - y4, 2));
            double distance2 = std::sqrt(std::pow(x2 - x4, 2) + std::pow(y2 - y4, 2));
            double distance3 = std::sqrt(std::pow(x3 - x4, 2) + std::pow(y3 - y4, 2));
            ROS_INFO("Distance from turtle1: %.2f", distance1);
            ROS_INFO("Distance from turtle2: %.2f", distance2);
            ROS_INFO("Distance from turtle3: %.2f", distance3);
        }
    }
};
////////////

class TeleopTurtle
{
public:
    TeleopTurtle();
    void keyLoop();

    static void quit(int sig);

private:
    ros::NodeHandle nh_;
    double linear_, angular_, l_scale_, a_scale_;
    ros::Publisher twist_pub_;

    static int kfd;
    static struct termios cooked, raw;
};

int TeleopTurtle::kfd = 0;
struct termios TeleopTurtle::cooked, TeleopTurtle::raw;

TeleopTurtle::TeleopTurtle() :
    linear_(0),
    angular_(0),
    l_scale_(2.0),
    a_scale_(2.0)
{
    nh_.param("scale_angular", a_scale_, a_scale_);
    nh_.param("scale_linear", l_scale_, l_scale_);

    twist_pub_ = nh_.advertise<geometry_msgs::Twist>("turtle4/cmd_vel", 1);

    // Open the terminal for reading input
    kfd = 0;  // This could be a file descriptor for standard input
    tcgetattr(kfd, &cooked);
    memcpy(&raw, &cooked, sizeof(struct termios));
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VEOL] = 1;
    raw.c_cc[VEOF] = 2;
    tcsetattr(kfd, TCSANOW, &raw);
}

void TeleopTurtle::quit(int sig)
{
    (void)sig;
    tcsetattr(0, TCSANOW, &cooked);  // Use 0 for stdin
    ros::shutdown();
    exit(0);
}

void TeleopTurtle::keyLoop()
{
    char c;
    bool dirty = false;

    puts("Reading from keyboard");
    puts("---------------------------");
    puts("Use w a s d keys to move the turtle, and q to attack.");

    for (;;)
    {
        // get the next event from the keyboard
        if (read(kfd, &c, 1) < 0)
        {
            perror("read()");
            exit(-1);
        }

        linear_ = angular_ = 0;
        ROS_DEBUG("value: 0x%02X\n", c);

        switch (c)
        {
        case KEYCODE_L:
            ROS_DEBUG("LEFT");
            angular_ = 1.0;
            dirty = true;
            break;
        case KEYCODE_R:
            ROS_DEBUG("RIGHT");
            angular_ = -1.0;
            dirty = true;
            break;
        case KEYCODE_U:
            ROS_DEBUG("UP");
            linear_ = 1.0;
            dirty = true;
            break;
        case KEYCODE_D:
            ROS_DEBUG("DOWN");
            linear_ = -1.0;
            dirty = true;
            break;
        //////////////
        case KEYCODE_Q:
            ROS_DEBUG("ATTACK");
            DistanceCalculator dc;
            dirty = true;
            break;
        //////////////
        }

        geometry_msgs::Twist twist;
        twist.angular.z = a_scale_ * angular_;
        twist.linear.x = l_scale_ * linear_;
        if (dirty)
        {
            twist_pub_.publish(twist);
            dirty = false;
        }
    }
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "turtle4_control");
    TeleopTurtle teleop_turtle;

    signal(SIGINT, TeleopTurtle::quit);

    teleop_turtle.keyLoop();

    return 0;
}
