from keras.layers import Flatten, Dense, Convolution2D, MaxPooling2D, Dropout
from keras.models import Model, Sequential
from sklearn.model_selection import train_test_split
import pickle
import json
import os

# input image info - height, width and channel
img_h = 64
img_w = 128
n_channel = 3
input_shape = (img_h, img_w, n_channel)
# size of pooling area for max pooling
pool_size = (2, 2)

# both Sequential model and Function API are used for practice
# model is based on the NVIDIA's paper - "End to End Learning for Self-Driving Cars"
# https://arxiv.org/pdf/1604.07316v1.pdf
# add convolution layers to abstract feature map
base_model = Sequential()
base_model.add(Convolution2D(24, 5, 5,
                        activation = 'relu',
                        input_shape = input_shape))
base_model.add(MaxPooling2D(pool_size = pool_size))
base_model.add(Convolution2D(48, 3, 3,
                        activation = 'relu'))
base_model.add(MaxPooling2D(pool_size = pool_size))
base_model.add(Convolution2D(96, 3, 3,
                        activation = 'relu'))
base_model.add(MaxPooling2D(pool_size = pool_size))
# add dropout layer to avoid overfitting
base_model.add(Dropout(0.25))
# The base model can be placed by other pre-trained models such as VGG16
# use function API
x = base_model.output
# add the fully-connected layers
x = Flatten(name='flatten')(x)
x = Dense(1164, activation='relu', name='fc1')(x)
x = Dense(100, activation='relu', name='fc2')(x)
x = Dense(50, activation='relu', name='fc3')(x)
x = Dense(10, activation='relu', name='fc4')(x)
# It is a regression problem, so only 1 output
predictions = Dense(1, name='predictions')(x)
model = Model(input = base_model.input, output = predictions)
# print model architecture
model.summary()
# use "Adam" optimizer and mean square error for regression problem
model.compile(optimizer = 'adam', loss = 'mse')

# load training data
training_file = './train.p'
with open(training_file, mode='rb') as f:
    train = pickle.load(f)

X_train, Y_train = train['features'], train['angles']
# normalization to [-0.5, 0.5]
X_train /= 255
X_train -= 0.5

# split dataset to training, validation and testing set
train_features, test_features, train_labels, test_labels = train_test_split(
    X_train,
    Y_train,
    test_size=0.20,
    random_state=832289)

train_features, valid_features, train_labels, valid_labels = train_test_split(
    train_features,
    train_labels,
    test_size=0.20,
    random_state=832289)

# define output files
model_json_file = 'model.json'
model_h5_file = 'model.h5'

# remove old results
# TODO: check old model and weights, load them for finetuning
if os.path.isfile(model_json_file):
    os.remove(model_json_file)
if os.path.isfile(model_h5_file):
    os.remove(model_h5_file)

# set training parameters
nb_epoch = 10
batch_size = 32

# training the model
history = model.fit(train_features, train_labels,
                    batch_size = batch_size, nb_epoch = nb_epoch,
                    validation_data=(valid_features, valid_labels))
# evaluation
score = model.evaluate(test_features, test_labels)
print('Test score(loss):', score)

# Save your model architecture as model.json, and the weights as model.h5.
# serialize model to JSON
model_json = model.to_json()
with open(model_json_file, 'w') as f:
    json.dump(model_json, f)
# serialize weights to HDF5
model.save_weights(model_h5_file)
print("Saved model to disk")
