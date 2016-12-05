from keras.applications.vgg16 import VGG16
from keras.preprocessing import image
from keras.applications.vgg16 import preprocess_input
from keras.layers import Flatten, Dense, Convolution2D, Activation, MaxPooling2D, Dropout
from keras.models import Model, Sequential
import numpy as np
import pickle

img_h = 160
img_w = 320
n_channel = 3

#Get back the convolutional part of a VGG network trained on ImageNet
#base_model = VGG16(include_top = False, weights = 'imagenet', input_shape = (img_h, img_w, n_channel))

# number of convolutional filters to use
nb_filters = 32
# size of pooling area for max pooling
pool_size = (2, 2)
# convolution kernel size
kernel_size = (3, 3)
input_shape = (img_h, img_w, 3)


base_model = Sequential()
base_model.add(Convolution2D(nb_filters, kernel_size[0], kernel_size[1],
                        border_mode = 'valid',
                        input_shape = input_shape))
base_model.add(Activation('relu'))
base_model.add(Convolution2D(nb_filters, kernel_size[0], kernel_size[1],
                        border_mode = 'valid',
                        input_shape = input_shape))
base_model.add(Activation('relu'))
base_model.add(MaxPooling2D(pool_size=pool_size))
base_model.add(Dropout(0.5))

x = base_model.output
#Add the fully-connected layers
x = Flatten(name='flatten')(x)
x = Dense(4096, activation='relu', name='fc1')(x)
x = Dense(4096, activation='relu', name='fc2')(x)
predictions = Dense(1, activation='sigmoid', name='predictions')(x)

#Create your own model
model = Model(input = base_model.input, output = predictions)

#In the summary, weights and layers from VGG part will be hidden, but they will be fit during the training
#model.summary()

# first: train only the top layers (which were randomly initialized)
# i.e. freeze all convolutional InceptionV3 layers
for layer in base_model.layers:
    layer.trainable = False

model.compile(optimizer = 'adam', loss = 'mse', metrics=['accuracy'])

training_file = './train.p'
with open(training_file, mode='rb') as f:
    train = pickle.load(f)

X_train, Y_train = train['features'], train['angles']
X_train /= 255
X_train -= 0.5


nb_epoch = 5
batch_size = 32
# fits the model on batches with real-time data augmentation:
history = model.fit(X_train, Y_train,
                    batch_size = batch_size, nb_epoch = nb_epoch)

# Save your model architecture as model.json, and the weights as model.h5.
# serialize model to JSON
model_json = model.to_json()
with open("model.json", "w") as json_file:
    json_file.write(model_json)
# serialize weights to HDF5
model.save_weights("model.h5")
print("Saved model to disk")

#reference
#https://keras.io/applications/
#1. install the latest keras from the offical website "sudo python setup.py install", the pip install the old version which does not support input_shape
