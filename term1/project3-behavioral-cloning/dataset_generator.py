import pickle
import csv
import numpy as np
import cv2
import os

# input image info - height, width and channel, resize image to avoid out of memory issue
img_h = 64
img_w = 128
n_channel = 3

with open('./driving_log.csv', 'rt') as f:
    # get exact number of lines in log, so we can define the array size at the beginning
    n = len(f.readlines()) * 3
    print(n)

with open('./driving_log.csv', 'rt') as f:
    imgs = np.zeros([n, img_h, img_w, n_channel])
    labels = np.zeros([n])
    reader = csv.reader(f)
    count = 0
    for line in reader:
        # use center images, and left/right images with offset steering angle as training data
        img_center = cv2.imread(line[0])
        img_center = cv2.resize(img_center,(img_w, img_h))
        imgs[count] = img_center
        label_center = float(line[3])
        labels[count] = label_center
        count += 1

        #left
        img_left = cv2.imread(str(line[1]).replace(" ", ""))
        img_left = cv2.resize(img_left,(img_w, img_h))
        imgs[count] = img_left
        label_left = float(line[3]) + 0.08
        labels[count] = label_left
        count += 1
        #right
        img_right = cv2.imread(str(line[2]).replace(" ", ""))
        img_right = cv2.resize(img_right,(img_w, img_h))
        imgs[count] = img_right
        label_right = float(line[3]) - 0.08
        labels[count] = label_right
        count += 1

        #print log
        if (count % 30 == 0):
            print(count)
f.close()

print(imgs.shape)

training_file = 'train.p'
if os.path.isfile(training_file):
    os.remove(training_file)

# output to train.p
with open(training_file, 'wb') as handle:
    dict = {'features': imgs, 'angles': labels}
    pickle.dump(dict, handle, protocol=4)
handle.close()
