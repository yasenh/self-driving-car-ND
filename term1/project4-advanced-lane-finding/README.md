## Advanced Lane Finding
[![Udacity - Self-Driving Car NanoDegree](https://s3.amazonaws.com/udacity-sdc/github/shield-carnd.svg)](http://www.udacity.com/drive)

The goals / steps of this project are the following:

* Compute the camera calibration matrix and distortion coefficients given a set of chessboard images.
* Apply the distortion correction to the raw image.
* Use color transforms, gradients, etc., to create a thresholded binary image.
* Apply a perspective transform to rectify binary image ("birds-eye view"). 
* Detect lane pixels and fit to find lane boundary.
* Determine curvature of the lane and vehicle position with respect to center.
* Warp the detected lane boundaries back onto the original image.
* Output visual display of the lane boundaries and numerical estimation of lane curvature and vehicle position.

---

##Pipeline:
Original image -> undistort -> color and gradient threshold -> implement mask -> warp ->
search lines by using histogram or sliding window -> warp back -> result

![0-input](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/0-input.jpg)
![1-undistort](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/1-undistort.jpg)
![2-1-color_threshold](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/2-1-color_threshold.jpg)
![2-2-gradient_threshold](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/2-2-gradient_threshold.jpg)
![2-3-threshold_combine](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/2-3-threshold_combine.jpg)
![3-mask](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/3-mask.jpg)
![4-1-warped](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/4-1-warped.jpg)
![4-2-warped_left](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/4-2-warped_left.jpg)
![4-3-warped_right](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/4-3-warped_right.jpg)
![5-slide](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/5-slide.jpg)
![6-warped_after_window](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/6-warped_after_window.jpg)
![7-line](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/7-line.jpg)
![8-result.jpg](https://github.com/yasenh/self-driving-car-ND/blob/master/term1/project4-advanced-lane-finding/output_images/8-result.jpg)

##Detailed Description:
1. First of all, we need to calibrate the camera from these chessboard images.
2. Once we get intrinsic and extrinsic matrixes, we can test it on chessboard images and recorded images on road to verify it. Then we should save them as a pickle file so we don't need to calculate them everytime.
3. Before we start, we can define some class such as Line and SlidingWindow to store common and important information.
4. At the beginning of process pipeline, we need to convert input image to BGR order and undistort it for following stages.
5. Then we implement two thresholding method as filter, one is HSL color space and the other one is gradient thresholding. 
6. Because the R, G, and B components of an object’s color in a digital image are all correlated with the amount of light hitting the object, and therefore with each other, image descriptions in terms of those components make object discrimination difficult. Descriptions in terms of hue/lightness/chroma or hue/lightness/saturation are often more relevant. 
7. Now consider how you can use various aspects of your gradient measurements (x, y, magnitude, direction) to isolate lane-line pixels. Specifically, think about how you can use thresholds of the x and y gradients, the overall gradient magnitude, and the gradient direction to focus on pixels that are likely to be part of the lane lines. So we use both magnitude and direction on it!
8. In next step we combine this two thresholded image together. You can find gradient image provides very good edge information and color thresholded image perfectly fills in the blanks.
9. Because we know that some area cannot provide useful information for finding lines such as sky area. So we can add a simple mask to filter most of irrelenat area.
10. Set four corresponding point pairs, then calculate perspective and inverse perspective matrix.
11. Warp image
12. Then we need to implement sliding window algorithm as final filter. At the first several frames, we do not have enough line information, so we calcuate histogram every frame to find most possible sliding window center position. After we accumulate confident for some frames, we don't need to search bilnly. As soon as a high confidence detection of the lane lines has been achieved, that information should be propagated to the detection step for the next frame of the video, both as a means of saving time on detection and in order to reject outliers. It is a common sense that the line is continuous and the position will change gradually with the curvatue. So we only need to search around from the position of last frame, and the sliding window should be more or less continuous as well.
13. Sometime we may lost tracking, I use to method to improve robustness. One way when some of the sliding window cannot find valid points, we will keep the center position from last frame and search the current window based on the center position below it in following frames. Another way is when the valid points are too little in the whole frame, I assume the current sliding window maybe totally offset, so I set the confident as zero to restart blind search again in following frames.
14. After we get purely filtered line images, we can implement line fitting and calculate offset and curvature.
15. The final step is to draw lines, freespace and warp it back!

##TODO List:
1. The whole pipeline is not able to process videos in real-time, so the next step is to imporve it especially the strategy to search and fit lines. Another method is to implement it on GPU.
2. Even I design some failsafe method, however if the search algorithm cannot find lines in multiple continuous frames, it would cause the pipeline to fail. The reason is from both thresholding and line searching algorithm. I need to design a more robust way to deal with different scenarios.




