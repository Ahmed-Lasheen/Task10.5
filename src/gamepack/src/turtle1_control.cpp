#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/String.h>
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>  // For read()

#define KEYCODE_R 0x64
#define KEYCODE_L 0x61
#define KEYCODE_U 0x77
#define KEYCODE_D 0x73
#define KEYCODE_Q 0x71

void calculateAndPublishDistance();

ros::Publisher distance_pub_;

void calculateAndPublishDistance()
    {
            std_msgs::String distance_msg;
            distance_msg.data = "turtle1";
            distance_pub_.publish(distance_msg);
            ROS_INFO("ATTACK!");
            puts("ATTACK!");
    }

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

    twist_pub_ = nh_.advertise<geometry_msgs::Twist>("turtle1/cmd_vel", 1);
    distance_pub_ = nh_.advertise<std_msgs::String>("effect", 10);

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
        case KEYCODE_Q:
            calculateAndPublishDistance();
            dirty = true;
            break;
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
    ros::init(argc, argv, "turtle1_control");

    ros::NodeHandle nh;
    distance_pub_ = nh.advertise<std_msgs::String>("effect", 10);

    TeleopTurtle teleop_turtle;

    signal(SIGINT, TeleopTurtle::quit);

    teleop_turtle.keyLoop();

    return 0;
}
