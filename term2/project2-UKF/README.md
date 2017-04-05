# Unscented Kalman Filter Project Starter Code
Self-Driving Car Engineer Nanodegree Program

---
## Project Description

The project "unscented Kalman filter" is based on the same structure as the extended Kalman filter.
It uses a main file that calls a function called ProcessMeasurement. Anything important happens in this function. The function is part of the class ukf.



## Dependencies

* cmake >= 2.8
 * All OSes: [click here for installation instructions](https://cmake.org/install/)
* make >= 3.8
  * Linux: make is installed by default on most Linux distros
  * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
  * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)
* gcc/g++ >= 4.8
  * Linux: gcc / g++ is installed by default on most Linux distros
  * Mac: same deal as make - [install Xcode command line tools]((https://developer.apple.com/xcode/features/)
  * Windows: recommend using [MinGW](http://www.mingw.org/)

## Basic Build Instructions

1. Clone this repo.
2. Make a build directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./UnscentedKF path/to/input.txt path/to/output.txt`. You can find some sample inputs in 'data/'.
    - eg. `./UnscentedKF ../data/sample-laser-radar-measurement-data-1.txt output.txt`


## Generating Additional Data

If you'd like to generate your own radar and lidar data, see the [utilities repo](https://github.com/udacity/CarND-Mercedes-SF-Utilities) for Matlab scripts that can generate additional data.

[//]: # (Image References)
[image1]: ./data/1-1-all.png
[image2]: ./data/1-2-est_mea.png
[image3]: ./data/1-3-est_gt.png
[image4]: ./data/2-1-all.png
[image5]: ./data/2-2-est_mea.png
[image6]: ./data/2-3-est_gt.png



## Result
* Input - sample-laser-radar-measurement-data-1.txt
Accuracy - RMSE:
[px, py, vx, vy] = [0.0597352, 0.0731396, 0.537907, 0.599281]

![alt text][image1]
![alt text][image2]
![alt text][image3]

* Input - sample-laser-radar-measurement-data-2.txt
Accuracy - RMSE:
[px, py, vx, vy] = [0.199075, 0.183845, 0.344491, 0.338491]

![alt text][image4]
![alt text][image5]
![alt text][image6]

