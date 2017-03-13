# Extended Kalman Filter Project
Self-Driving Car Engineer Nanodegree Program

---

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
4. Run it: `./ExtendedKF path/to/input.txt path/to/output.txt`. You can find
   some sample inputs in 'data/'.
    - eg. `./ExtendedKF ../data/sample-laser-radar-measurement-data-1.txt output.txt`

## Generating Additional Data

This is optional!

If you'd like to generate your own radar and lidar data, see the
[utilities repo](https://github.com/udacity/CarND-Mercedes-SF-Utilities) for
Matlab scripts that can generate additional data.

[//]: # (Image References)
[image1]: ./data/result1-1.png
[image2]: ./data/result1-2.png
[image3]: ./data/result2-1.png
[image4]: ./data/result2-2.png

## Result
* Input - sample-laser-radar-measurement-data-1.txt
Accuracy - RMSE:
[px, py, vx, vy] = [0.0702115, 0.0681311, 0.548794, 0.558757]

![alt text][image1]
![alt text][image2]

* Input - sample-laser-radar-measurement-data-2.txt
Accuracy - RMSE:
[px, py, vx, vy] = [0.182924, 0.193127, 0.482862, 0.839444]

![alt text][image3]
![alt text][image4]