import pickle
import csv
import numpy as np
import cv2

# with open( './driving_log.csv', 'rt') as f:
#     reader = csv.reader(f)
#     for line in reader:
#         print(line[0], line[3])

labels = []
img_h = 32
img_w = 64
n_channel = 3
imgs = np.zeros([1, img_h, img_w, n_channel])
empty_flag = True
with open( './driving_log.csv', 'rt') as f:
    reader = csv.reader(f)
    for line in reader:
        img_center = cv2.imread(line[0])
        img_center = cv2.resize(img_center,(img_w, img_h))
        #if load the first image
        if (empty_flag):
            imgs[0] = img_center
            empty_flag = False
        else:
            imgs = np.append(imgs, [img_center], axis = 0)
        label_center = float(line[3])
        labels.append(label_center)
        print(imgs.shape)
        #left
        img_left = cv2.imread(str(line[1]).replace(" ", ""))
        img_left = cv2.resize(img_left,(img_w, img_h))
        imgs = np.append(imgs, [img_left], axis = 0)
        label_left = float(line[3]) + 0.08
        labels.append(label_left)
        print(imgs.shape)
        #right
        img_right = cv2.imread(str(line[2]).replace(" ", ""))
        img_right = cv2.resize(img_right,(img_w, img_h))
        imgs = np.append(imgs, [img_right], axis = 0)
        label_right = float(line[3]) - 0.08
        labels.append(label_right)
        print(imgs.shape)
f.close()

with open('train.p', 'wb') as handle:
    dict = {'features': imgs, 'angles': labels}
    pickle.dump(dict, handle)
handle.close()
