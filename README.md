# Simple Path Planning in c++ game

* An Original developer's github: https://github.com/nguyenhopquang/CarRacing
## Original game
[![self-walking_robot](http://img.youtube.com/vi/9fcHq2ihfEE/0.jpg)](https://www.youtube.com/watch?v=9fcHq2ihfEE)
## Final result
[![self-walking_robot](http://img.youtube.com/vi/3OXcgFVJwHw/0.jpg)](https://www.youtube.com/watch?v=3OXcgFVJwHw)

## Procedure

### Kinematic Modeling

* Original game could move a car without considering kinematic model of a car.
* So I've modeled a car to rotate considering kinematic model.

### Waypoints

* I've used waypoints to plan the path.
* The car is modeled to follow the nearest waypoint.

### Path planning

* When waypoints are near enough to the obstacle car, path is changed linearly to the nearest lane where the obstacle car doesn't exist.






## Limitations
* The road is designed straight so there aren't many variables to be considered.
* I haven't considered longitudinal planning which is going to be next challenge for me.
* A planned path was linear, which is simple but bad in terms of acceleration user might feel and for avoiding obstacle.
* I didn't used Frenet frame. Since the road was straight, it was meaningless. But I'm 100% sure that I'll use it on my next path planning project. 


## What I've learned
* How to use SFML library
* Got more familiar with C++
* Basic concepts of path planning.
* How to deal with codes over 500 lines and with lots of functions.(Got used to handle the flow of long codes.)