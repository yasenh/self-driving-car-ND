import pickle
import csv
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np

# test = np.array([1, 2, 3])
# test = np.append([test], [[4, 5, 6]], axis = 0)
#
# print(test)
# print(test.shape)

labels = []
imgs = np.zeros([1, 160, 320, 3])
empty_flag = True
with open( './driving_log.csv', 'rt') as f:
    reader = csv.reader(f)
    for line in reader:
        print(imgs.shape)
        img = mpimg.imread(line[0])
        #if load the first image
        if (empty_flag):
            imgs[0] = img
            empty_flag = False
        else:
            imgs = np.append(imgs, [img], axis = 0)
        label = line[3]
        labels.append(label)
f.close()


with open('train.p', 'wb') as handle:
    dict = {'features': imgs, 'angles': labels}
    pickle.dump(dict, handle)
