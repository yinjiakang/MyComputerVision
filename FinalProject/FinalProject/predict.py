import scipy.io as sio
import numpy as np
import sys
from sklearn.svm import SVC
from sklearn.svm import LinearSVC
from sklearn.ensemble import AdaBoostClassifier
from sklearn.ensemble import RandomForestClassifier
import pickle

if __name__ == '__main__':
    testId = np.genfromtxt("./result/pixels.csv", usecols=(0))
    testImg = np.genfromtxt("./result/pixels.csv", usecols=range(1, 785))
    testImg /= 255
    numberOfTestData = testImg.shape[0]

    training = 0
    if training == 1:
    # Read in .mat
        trainingImgMat = sio.loadmat('./mnist_dataset/mnist_train.mat')
        trainingLabelMat = sio.loadmat('./mnist_dataset/mnist_train_labels.mat')

        trainingImg = trainingImgMat["mnist_train"]
        trainingLabel = trainingLabelMat["mnist_train_labels"]

        # Normalize
        trainingImg /= 255

        numberOfTrainingData = trainingImg.shape[0]
    
        #clf = AdaBoostClassifier(base_estimator=RandomForestClassifier())
        clf = LinearSVC(dual=False)
        clf.fit(trainingImg, trainingLabel.ravel())

        with open('my_dumped_classifier.pkl', 'wb') as f:
            pickle.dump(clf, f)

        print("==Finish fitting==\n")

    else:
        with open('my_dumped_classifier.pkl', 'rb') as f:
            clf = pickle.load(f)

    print("=====Result=====")
    predict = clf.predict(testImg).reshape(numberOfTestData, 1)
    with open('./result/predictResult.csv', 'w') as file:
        for i in range(numberOfTestData):
            print("Image: %d, digit: %d" %(testId[i], predict[i]))
            file.write("%d %d\n" %(testId[i], predict[i]))
