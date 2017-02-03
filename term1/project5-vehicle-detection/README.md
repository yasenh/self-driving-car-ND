# Vehicle Detection
[![Udacity - Self-Driving Car NanoDegree](https://s3.amazonaws.com/udacity-sdc/github/shield-carnd.svg)](http://www.udacity.com/drive)
---

**Vehicle Detection Project**

The goals / steps of this project are the following:

* Perform a Histogram of Oriented Gradients (HOG) feature extraction on a labeled training set of images and train a classifier Linear SVM classifier
* Optionally, you can also apply a color transform and append binned color features, as well as histograms of color, to your HOG feature vector. 
* Note: for those first two steps don't forget to normalize your features and randomize a selection for training and testing.
* Implement a sliding-window technique and use your trained classifier to search for vehicles in images.
* Run your pipeline on a video stream and create a heat map of recurring detections frame by frame to reject outliers and follow detected vehicles.
* Estimate a bounding box for vehicles detected.

Here are links to the labeled data for [vehicle](https://s3.amazonaws.com/udacity-sdc/Vehicle_Tracking/vehicles.zip) and [non-vehicle](https://s3.amazonaws.com/udacity-sdc/Vehicle_Tracking/non-vehicles.zip) examples to train your classifier.  These example images come from a combination of the [GTI vehicle image database](http://www.gti.ssr.upm.es/data/Vehicle_database.html), the [KITTI vision benchmark suite](http://www.cvlibs.net/datasets/kitti/), and examples extracted from the project video itself.   You are welcome and encouraged to take advantage of the recently released [Udacity labeled dataset](https://github.com/udacity/self-driving-car/tree/master/annotations) to augment your training data. 

Some example images for testing your pipeline on single frames are located in the `test_images` folder.  To help the reviewer examine your work, please save examples of the output from each stage of your pipeline in the folder called `ouput_images`, and include them in your writeup for the project by describing what each image shows.    The video called `project_video.mp4` is the video your pipeline should work well on. 


[//]: # (Image References)
[image1]: ./output_images/01-car-notcar.png
[image2]: ./output_images/02-car-HOG.png
[image3]: ./output_images/02-car-HOG2.png
[image4]: ./output_images/02-car-HOG3.png
[image5]: ./output_images/02-notcar-HOG.png
[image6]: ./output_images/02-notcar-HOG2.png
[image7]: ./output_images/02-notcar-HOG3.png
[image8]: ./output_images/03-YCrCb.png
[image9]: ./output_images/04-1.png
[image10]: ./output_images/04-2.png
[image11]: ./output_images/05-norm.png
[image12]: ./output_images/06-windows.png
[image13]: ./output_images/07-all-detections.png
[image14]: ./output_images/08-heatmap.png
[image15]: ./output_images/09-label.png
[image16]: ./output_images/10-1-test1.png
[image17]: ./output_images/10-1-test2.png
[image18]: ./output_images/10-1-test3.png
[image19]: ./output_images/11-result.png
[video1]: ./vehicle_tracking.mp4

---
###Writeup / README

####1. Provide a Writeup / README that includes all the rubric points and how you addressed each one.  You can submit your writeup as markdown or pdf.  [Here](https://github.com/udacity/CarND-Vehicle-Detection/blob/master/writeup_template.md) is a template writeup for this project you can use as a guide and a starting point.  

You're reading it!

###Histogram of Oriented Gradients (HOG)

####1. Explain how (and identify where in your code) you extracted HOG features from the training images.

The code for this step is contained in the code cell **#3 ~ #6 ** of the IPython notebook

I started by reading in all the `vehicle` and `non-vehicle` images.  Here is an example of one of each of the `vehicle` and `non-vehicle` classes:

![alt text][image1]

I then explored different color spaces and different `skimage.hog()` parameters (`orientations`, `pixels_per_cell`, and `cells_per_block`).  I grabbed random images from each of the two classes and displayed them to get a feel for what the `skimage.hog()` output looks like.

Here is an positive (training set with car) example using the `YCrCb` color space and HOG parameters of `orientations = 8`, `pixels_per_cell = (8, 8)` and `cells_per_block = (2, 2)`:

![alt text][image2]

Here is another positive example using the same parameters but `orientations = 64`. 

![alt text][image3]

One more positive example using the same parameters but `pixels_per_cell = (4, 4)`. 
![alt text][image4]

Some negetive examples:

![alt text][image5]
![alt text][image6]
![alt text][image7]

####2. Explain how you settled on your final choice of HOG parameters.

I tried various combinations of parameters and I choose `YCrCb` color space and HOG parameters of `orientations = 8`, `pixels_per_cell = (4, 4)` and `cells_per_block = (2, 2)` finally. From the training and testing result of several color spaces, I find that `LUV`, `YUV`, `YCrCb` have better performance than `RGB`,`HSV` and `HLS`. For HOG orientation, you will get minor improvement when increase it howerver the length of the feature vector will increase a lot and it applies to paramter `cells_per_block` as well. But when you decrease `pixels_per_cell` from  `(8, 8)` to `(4, 4)`, we will notice tremendous visible improvement, as the same as the example images above. It will cause more feature vector but worth to be implemented to reduce false alarm.

####3. Describe how (and identify where in your code) you trained a classifier using your selected HOG features (and color features if you used them).
The code for this step is contained in the code cell **#8 ~ #10 ** of the IPython notebook

The HOG feature is a great feature to distinguish vehicles from non-vehicle objects. However some non-vehicle object still have more or less similar HOG features as vehicles. For example:

![alt text][image9]
![alt text][image10]

So if we only rely on HOG features there will be some false detecitons, so I add spatical and histogram features. 

![alt text][image8]

I trained a linear SVM using `Spatial features`, `Histogram features` and `HOG features` with parametes `YCrCb color space`, `spatial_size = (32, 32)` and `hist_bins = 16`. All features will be normalized as a vector.

![alt text][image11]

After training, the whole model with parameter will be saved into a pickle file:

    with open(pickle_file, 'wb') as pfile:
        pickle.dump(
            {
                'model': svc,
                'scaler': X_scaler,
                'color_space': color_space,
                'orient': orient,
                'pix_per_cell': pix_per_cell,
                'cell_per_block': cell_per_block,
                'hog_channel': hog_channel,
                'spatial_size': spatial_size,
                'hist_bins': hist_bins,
                'spatial_feat': spatial_feat,
                'hist_feat': hist_feat,
                'hog_feat': hog_feat,
            }, 
            pfile, 
            pickle.HIGHEST_PROTOCOL)

###Sliding Window Search

####1. Describe how (and identify where in your code) you implemented a sliding window search.  How did you decide what scales to search and how much to overlap windows?

![alt text][image12]

Some experience when generate sliding windows and define search area

* Choose multiple size, for example I use (80, 50), (100, 70), (140, 90), (160, 100), (180, 130), (200, 150) and (280, 160). And the smaller size bounding boxes should be close to horizon line, and larger one close to host vehicle(camera).
* There will be more sliding window generated with the implementation of more overlap, more search area and more size of different sliding window defination. Generally speaking, more sliding window (and more features extracted) leads to higher detection accuracy but slower detection speed.
* For the area in front of the car, as you can imagine, the vehicle should be relatively large so the size of sliding window should increase as well.
* For long distace detection which is around the horizon line, it is very easy to lose dectction due to size, background and some other reason. So we should put more sliding windows in this area.

####2. Show some examples of test images to demonstrate how your pipeline is working.  What did you do to try to minimize false positives and reliably detect cars?

Ultimately I searched on two scales using YCrCb 3-channel HOG features plus spatially binned color and histograms of color in the feature vector, which provided a nice result.  Here are some example images:

![alt text][image16]
![alt text][image17]
![alt text][image18]
---

### Video Implementation

####1. Provide a link to your final video output.  Your pipeline should perform reasonably well on the entire project video (somewhat wobbly or unstable bounding boxes are ok as long as you are identifying the vehicles most of the time with minimal false positives.)
Here's a [link to my video result][video1]

####2. Describe how (and identify where in your code) you implemented some kind of filter for false positives and some method for combining overlapping bounding boxes.

I recorded the positions of positive detections in each frame of the video.  From the positive detections I created a heatmap and then thresholded that map to identify vehicle positions. 

    # Initialize a new deque object initialized left-to-right (using append()) with data from iterable.
    heatmaps = deque(maxlen = 10)
    
    # in image process pipeline
    # add heat according to raw detections 
    heatmap = add_heat(heatmap, hot_windows) 
    # insert to queue
    heatmaps.append(heatmap)
    # sum it up
    hm = sum(heatmaps)
    # filter false detections
    hm = apply_threshold(hm, 8)
    # cluster
    labels = label(hm)
    draw_img = draw_labeled_bboxes(draw_image2, labels)

In order to reduce noise and smooth detections, I am not use only one frame of detections. Actually, I implement a queue with maximum size of 10 to sum up heatmaps in continuous frames. The positive threshold is set as 8 which means we label it as a "car" if there are 8 positive detection in 10 frames and there maybe more than 1 detetion in single frame. I tried different queue size and threshold value and found 8/10 is a good choice.


Here's an example result showing the heatmap and bounding boxes overlaid on a frame of video:

#### All detections
![alt text][image13]

#### Heatmap
![alt text][image14]

#### Labels
![alt text][image15]

#### Final detection result
![alt text][image19]

---

###Discussion

####1. Briefly discuss any problems / issues you faced in your implementation of this project.  Where will your pipeline likely fail?  What could you do to make it more robust?

* In this project, a linear SVM classfier is trained based on spatial, histrogram and HOG features. The problems I met in project are mainly on false detections. 
* It is very trickly to play with parameters such as `pixels_per_cell`, `orientation`, `color_space`. But when you tuning multiple times (more than 50 times!), you will have a better understanding about how these parameters affect you final results.
* Another difficult point for me is to choose the size and position of sliding window. More window will have better detetion but more false alarm as well, and on the other hand you have a higher chance to miss detection with less windows. So you need to have some good strategy to design the architecture and tuning it back and forth.
* The process speed is pretty low (~1FPS), and it is becasue of too much features extracted and lots of sliding windows. There is one thing I can do to increase speed which is calculate the HOG feature only once, for the whole image or ROI, as a feature map , and then each sliding window just choose corresponding HOG features from feature map.
* Next step is to implement some deep neural network method such as SSD and YOLO to improve both detection accuracy and speed
