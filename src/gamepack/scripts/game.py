#!/usr/bin/env python3
import rospy,os
from character import Character
from turtlesim.srv import Spawn
from std_msgs.msg import String
import threading
import pygame

def run_core():
    os.system("rosrun turtlesim turtlesim_node")
    #exit excution from terminal when gui closes
    os._exit(1)

def edit_health(name):
    if name.data == "turtle1":
        turtles[0].health -= 50
    else:
        turtles[1].health -=  50

def main():
    pygame.init()
    #runs from thread because its a blocking input
    thread = threading.Thread(target=run_core,daemon=True)
    thread.start()
    
    rospy.init_node('characters')
    health_check  = rospy.Subscriber("health",String,callback=edit_health)

    #services take time to initialize and fails if we dont use the wait
    rospy.wait_for_service('/spawn')
    spawn_turtle = rospy.ServiceProxy('/spawn', Spawn)

    spawn_turtle(10, 10, 0, 'turtle2')
    global turtles
    turtles = [Character("turtle1"),Character("turtle2")]

    print(rospy.is_shutdown() )
    rate = rospy.Rate(10)
    while not rospy.is_shutdown() :
        # Example movement and attack logic (can be replaced with actual control)
        print("Turtle1 health: ",turtles[0].health)
        print("Turtle2 health: ",turtles[1].health)
        ##if to close game and display winner
        
        if turtles[0].health < 1:
            winner = turtles[1].name
            break
        if turtles[1].health < 1:
            winner = turtles[0].name
            break
        rate.sleep()
    x  = 250
    y = 150
    screen = pygame.display.set_mode((x, y))
    pygame.display.set_caption("Winner Screen")
    white = (255, 255, 255)
    black = (0, 0, 0)

    # Set up fonts
    font = pygame.font.Font(None, 34)

    # Render the text
    winner_text = font.render(f"{winner} Win!", True, black)

    # Main loop
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        # Fill the screen with white
        screen.fill(white)

        # Get the text's rectangle and position it
        text_rect = winner_text.get_rect(center=(x/2, y/2))
        screen.blit(winner_text, text_rect)

        # Update the display
        pygame.display.flip()
    pygame.quit()
    exit()

if __name__ == '__main__':
    main()