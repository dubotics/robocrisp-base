	# robocrisp-base

The code for the computer/client-side portion of the robocrisp robot

## Overview

The code for the base station will be organized into four separate components:

-   **Communications server:**  
    Responsible primarily for talking to the robot, and sending messages back and forth.
-   **Fake communications server:**  
    Identical to the communications server, except it fakes being connected to the robot
    so we can debug the base-station code without needing the robot.
-   **Robot control:**  
    Grabs input from joysticks and other computers, grabs encoder data from the communications
    server, and sends commands to the motors/actuators via the communications server.
-   **Video feed and analysis:**  
    Grabs the camera feed from the communications server, and displays it, possibly doing 
    some vision processing to highlight points of interest, the horizon, etc.
-   **Wifi cannon (??):**  
    Grabs GPS data from the communications server, and calibrates whatever method we use 
    to talk to the robot.
    
Each of the three client-side programs (robot control, video feed, wifi cannon) runs completely independently from each other, and will grab and send data via only the communications server. 

Therefore, each of the three client-side programs can be written in whatever programming language is most suitable, so long as they're able to talk to the server.

## Contributors

-   Michael Lee (michael0x2a)
-   Beck Pang (Beck-Sisyphus) let me change again on Linux
-   Collin J. Sutton (insaneinside)
-   Taylor Cramer (cramertj)
-   Kim Lum (kjlum)
-   David Wong(Netopia)
