import scipy.io as sio
import numpy as np
import sys
from sklearn.svm import SVC
from sklearn.svm import LinearSVC

if __name__ == '__main__':

    trainingdata = sio.loadmat('mnist_dataset/mnist_train.mat')
    traininglabeldata = sio.loadmat('mnist_dataset/mnist_train_labels.mat')
    testdata = sio.loadmat('mnist_dataset/mnist_test.mat')
    testlabeldata = sio.loadmat('mnist_dataset/mnist_test_labels.mat')

    trainingImg = trainingdata["mnist_train"]
    trainingLabel = traininglabeldata["mnist_train_labels"]
    testImg = testdata["mnist_test"]
    testLabel = testlabeldata["mnist_test_labels"]

    trainingImg /= 255
    testImg /= 255
    num_TestData = testImg.shape[0]
    
    clf = LinearSVC(dual=False)
    clf.fit(trainingImg, trainingLabel.ravel()) 

    predict = clf.predict(testImg).reshape(num_TestData, 1)
    score = clf.score(testImg, testLabel)
    wrongCount = num_TestData - np.count_nonzero(predict == testLabel)

    with open('svm_result.csv', 'w') as file:
        file.write("Error Rate: %f\n" %(wrongCount / 10000))
        file.write("Score: %f\n" %(score))
        for i in range(num_TestData):
            file.write("%d %d\n" %(predict[i], testLabel[i]))
