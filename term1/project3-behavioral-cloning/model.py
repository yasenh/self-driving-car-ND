from keras.layers import Flatten, Dense, Convolution2D, MaxPooling2D, Dropout
from keras.models import Model, Sequential
from sklearn.model_selection import train_test_split
import pickle
import json
import os

img_h = 64
img_w = 128
n_channel = 3

# size of pooling area for max pooling
pool_size = (2, 2)
input_shape = (img_h, img_w, 3)

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
base_model.add(Dropout(0.25))

x = base_model.output
#Add the fully-connected layers
x = Flatten(name='flatten')(x)
x = Dense(1164, activation='relu', name='fc1')(x)
x = Dense(100, activation='relu', name='fc2')(x)
x = Dense(50, activation='relu', name='fc3')(x)
x = Dense(10, activation='relu', name='fc4')(x)
predictions = Dense(1, name='predictions')(x)

model = Model(input = base_model.input, output = predictions)
model.summary()

model.compile(optimizer = 'adam', loss = 'mse')

training_file = './train.p'
with open(training_file, mode='rb') as f:
    train = pickle.load(f)

X_train, Y_train = train['features'], train['angles']
X_train /= 255
X_train -= 0.5

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

model_json_file = 'model.json'
model_h5_file = 'model.h5'

if os.path.isfile(model_json_file):
    os.remove(model_json_file)

if os.path.isfile(model_h5_file):
    os.remove(model_h5_file)

nb_epoch = 3
batch_size = 32

#fits the model on batches with real-time data augmentation:
history = model.fit(train_features, train_labels,
                    batch_size = batch_size, nb_epoch = nb_epoch,
                    validation_data=(valid_features, valid_labels))

score = model.evaluate(test_features, test_labels)
print('Test score:', score[0])
print('Test accuracy:', score[1])

# Save your model architecture as model.json, and the weights as model.h5.
# serialize model to JSON
model_json = model.to_json()
with open(model_json_file, 'w') as f:
    json.dump(model_json, f)
# serialize weights to HDF5
model.save_weights(model_h5_file)
print("Saved model to disk")
