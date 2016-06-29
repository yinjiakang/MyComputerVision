import scipy.io as sio
import numpy as np
import sys
from sklearn.svm import SVC
from sklearn.svm import LinearSVC

if __name__ == '__main__':
    # Read in .mat
    trainingImgMat = sio.loadmat('MNIST/mnist_train.mat')
    trainingLabelMat = sio.loadmat('MNIST/mnist_train_labels.mat')
    testImgMat = sio.loadmat('MNIST/mnist_test.mat')
    testLabelMat = sio.loadmat('MNIST/mnist_test_labels.mat')

    trainingImg = trainingImgMat["mnist_train"]
    trainingLabel = trainingLabelMat["mnist_train_labels"]
    testImg = testImgMat["mnist_test"]
    testLabel = testLabelMat["mnist_test_labels"]

    # Normalize
    trainingImg /= 255
    testImg /= 255

    numberOfTrainingData = trainingImg.shape[0]
    numberOFFeatures = trainingImg.shape[1]
    numberOfTestData = testImg.shape[0]
    
    # SVM Ref: http://scikit-learn.org/stable/modules/generated/sklearn.svm.SVC.html
    #clf = SVC()
    # LinearSVM Ref: http://scikit-learn.org/stable/modules/generated/sklearn.svm.LinearSVC.html
    clf = LinearSVC(dual=False)
    clf.fit(trainingImg, trainingLabel.ravel()) 
    print("==Finish fitting==\n")
    predict = clf.predict(testImg).reshape(numberOfTestData, 1)
    score = clf.score(testImg, testLabel)
    wrongCount = numberOfTestData - np.count_nonzero(predict == testLabel)

    with open('svmResult.csv', 'w') as file:
        file.write("Error Rate: %f\n" %(wrongCount / 10000))
        file.write("Score: %f\n" %(score))
        for i in range(numberOfTestData):
            file.write("%d %d\n" %(predict[i], testLabel[i]))
