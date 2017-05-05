# CarND-Controls-PID
Self-Driving Car Engineer Nanodegree Program

---

## Dependencies

* cmake >= 3.5
 * All OSes: [click here for installation instructions](https://cmake.org/install/)
* make >= 4.1
  * Linux: make is installed by default on most Linux distros
  * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
  * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)
* gcc/g++ >= 5.4
  * Linux: gcc / g++ is installed by default on most Linux distros
  * Mac: same deal as make - [install Xcode command line tools]((https://developer.apple.com/xcode/features/)
  * Windows: recommend using [MinGW](http://www.mingw.org/)
* [uWebSockets](https://github.com/uWebSockets/uWebSockets) == 0.13, but the master branch will probably work just fine
  * Follow the instructions in the [uWebSockets README](https://github.com/uWebSockets/uWebSockets/blob/master/README.md) to get setup for your platform. You can download the zip of the appropriate version from the [releases page](https://github.com/uWebSockets/uWebSockets/releases). Here's a link to the [v0.13 zip](https://github.com/uWebSockets/uWebSockets/archive/v0.13.0.zip).
  * If you run OSX and have homebrew installed you can just run the ./install-mac.sh script to install this
* Simulator. You can download these from the [project intro page](https://github.com/udacity/CarND-PID-Control-Project/releases) in the classroom.

## Basic Build Instructions

1. Clone this repo.
2. Make a build directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./pid`. 

## Reflection

* Describes the effect of the P, I, D component of the PID algorithm in their implementation.

The "P" control stands for proportion, means output value is directly proportional to input value (error). 

However, the vehicle itself is a large integral link, which causes time delay. For example, the vehicle get close to center but will still keep running to the other side because of inertia, which may cause another side of error. 
So it may bring large overshoot and frequently oscillation if only proportional link is applied. 
That is, only the ”proportional” controller is often not enough, it only has the effect of amplification of the proportional term of the error magnitude, which is the need to add predictive link to predict error trend. 
When the vehicle approach target, the control system should decrease the regulation intensity.
 
The "D" control stand for derivative, which means output value is proportional to derivative of input signal. 
In other words, the effect of suppressing should be zero or negative direction when the error approaches zero because the derivation is decreasing. 
Thus, a proportional with derivative control system, the error suppression is possible to advance the control action to zero, or even negative, which can avoid large overshoot. 
So for a larger inertia or lag-controlled object, proportional with derivative (PD) control system can improve the dynamics of system in the regulation processing.

The "I" means integral, which accounts for past values of the error. 
For example, if the current output is not sufficiently strong, the integral of the error will accumulate over time, and the controller will respond by applying a stronger action.
The integral term accelerates the movement of the process towards setpoint and eliminates the "residual steady-state error" that occurs with a pure proportional controller.

The whole system, with the help of both steering PID controller and throttle PID controller, is able to handle the error between desired waypoint and actually position.

*. Discusses how they chose the final hyperparameters (P, I, D coefficients).

reference: https://robotics.stackexchange.com/questions/167/what-are-good-strategies-for-tuning-pid-loops

1. Set all gains to zero.
2. Increase the P gain until the response to a disturbance is steady oscillation.
3. Increase the D gain until the the oscillations go away (i.e. it's critically damped).
4. Repeat steps 2 and 3 until increasing the D gain does not stop the oscillations.
5. Set P and D to the last stable values.
6. Increase the I gain until it brings you to the setpoint with the number of oscillations desired (normally zero but a quicker response can be had if you don't mind a couple oscillations of overshoot)

I tried both manual tuning and twiddle, and noticed that manual tuning according to observation and experience usually provide better
performance than twiddle. My experience is: 

Firstly increase the P until vehicle oscillation, then decrease P gain a little bit, I multiply P by 0.8 normally.
Then add some D gain until the overshoot decrease rapidly. Finally increase I gain to overcome steady error and play around with these parameters until the vehicle drive itself safely.

[//]: # (Image References)
[image1]: ./misc/Screenshot1.png
[image2]: ./misc/Screenshot2.png

## Screenshots
![Demo1][image1]

![Demo2][image2]