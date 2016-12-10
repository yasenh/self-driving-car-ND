## Files:
* dataset_generator.py - The script to generator training set (train.p)
* drive.py - The script to drive the car. I set the throttle as 0.1 to ensure stability.
* driving_log.csv - The log file that is generated by simulator. It contains the path of captured images, data of steering angle, throttle, break and speed.
* model.h5 - The model weights.
* model.json - The model architecture.
* model.py - The script used to create and train the model.
* README.md - explains the structure of your network and training approach.

## Model Architecture:
Input: 64 (height) x 128 (width) x 3 (rgb) normailized [-0.5, 0.5] image
Output: 1 number - predict steering angle

It is a sequential model, so the output shape of each layer is the input shape of next layer.

The model is based on the NVIDIA's paper - "End to End Learning for Self-Driving Cars" (https://arxiv.org/pdf/1604.07316v1.pdf).

It contains 3 convolution layers with relu activation, 3 maxpooling layers behind each convolution layer, 1 dropout layer, 1 flatten layer and 4 fully-connected layers.

|Layer (type)                       |  Output Shape         | Param #    |   Connected to
|-----------------------------------|-----------------------|------------|------------------------------
|convolution2d_input_1 (InputLayer) | (None, 64, 128, 3)    |   0	 |
|convolution2d_1 (Convolution2D)    | (None, 60, 124, 24)   |   1824     |   convolution2d_input_1[0][0]
|maxpooling2d_1 (MaxPooling2D)      | (None, 30, 62, 24)    |   0        |   convolution2d_1[0][0]
|convolution2d_2 (Convolution2D)    | (None, 28, 60, 48)    |   10416    |   maxpooling2d_1[0][0]
|maxpooling2d_2 (MaxPooling2D)      | (None, 14, 30, 48)    |   0        |   convolution2d_2[0][0]
|convolution2d_3 (Convolution2D)    | (None, 12, 28, 96)    |   41568    |   maxpooling2d_2[0][0]
|maxpooling2d_3 (MaxPooling2D)      | (None, 6, 14, 96)     |   0        |   convolution2d_3[0][0]
|dropout_1 (Dropout)                | (None, 6, 14, 96)     |   0        |   maxpooling2d_3[0][0]
|flatten (Flatten)                  | (None, 8064)          |   0        |   dropout_1[0][0]
|fc1 (Dense)                        | (None, 1164)          |   9387660  |   flatten[0][0]
|fc2 (Dense)                        | (None, 100)           |   116500   |   fc1[0][0]
|fc3 (Dense)                        | (None, 50)            |   5050     |   fc2[0][0]
|fc4 (Dense)                        | (None, 10)            |   510      |   fc3[0][0]
|predictions (Dense)                | (None, 1)             |   11       |   fc4[0][0]

Total params: 9563539


## Training set generator:
1. Use "dataset_generator.py" to generate serialized training data. I use the center images as main data source, and in order to get more data, I add some offset (+-0.08 ~ 0.10) to the steering angel to work with left and right images. The idea is pretty strightforward, if the center one capture the similar images as what left or right one get in training, it means the vehicle is not straight on road, so we need some steering angle to make the car go back to right pose.
2. If you use the 50Hz simulator, you will get around 50K or more images if you drive several laps.
3. pickle.dump(dict, handle, protocol=4), use "protocol = 4", so you can save the pickle file with more than 4GB size.

## Training process:
1. Log image data and steering angle in (10Hz/50Hz) simulator
2. Use "dataset_generator.py" to generate serialized training data.
3. In "model.py", load the training set, and then split it into training, validation and testing sets to avoid overfitting.
4. Don't forget to do resize and normalization on input images.
5. Train the model, and stop training if you find the training accuracy is decreasing but the validation does not change much or even increase, it means you maybe overfitting. You should stop training and change train paramters.
6. Save model and weights.

## Experiences:
1. Use 50Hz simulator, you can get more data and more smooth steering angle
2. Joystick has more smooth output than keyboard.
3. Write the code to generate serialized training dataset before you do the training. A good data set generator will really save a lot of time.
4. Resize the input image to 64 (height) x 128 (width) will save training time, and the performance is not bad. More pixels means more training time and memory cost, but not always means more accurate result.
5. Use images from different cameras to generator more training set.
6. It is very important to log some recovery scenarios, which is able to teach the model how to recovery from mistakes.
7. Again, do not forget to normalize data in both training and testing stages.
8. Training on GPU is much faster.
9. Choose the right model, I tried VGG, Resnet but it easily get overfitting, so I just use a more simple model and get a better result. I believe if you have a larger imput image size, these more complex network will have better performance on feature abstraction but you should always be careful about overfitting.
10. Be patient, Be patient, Be patient.





