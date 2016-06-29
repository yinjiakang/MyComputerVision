import scipy.io as sio
import numpy as np
import sys
from sklearn.ensemble import AdaBoostClassifier
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier

if __name__ == '__main__':
    np.set_printoptions(edgeitems=5)

    # Read in .mat
    # Ref: http://docs.scipy.org/doc/scipy/reference/generated/scipy.io.loadmat.html
    trainingImgMat = sio.loadmat('MNIST/mnist_train.mat')
    trainingLabelMat = sio.loadmat('MNIST/mnist_train_labels.mat')
    testImgMat = sio.loadmat('MNIST/mnist_test.mat')
    testLabelMat = sio.loadmat('MNIST/mnist_test_labels.mat')

    trainingImg = trainingImgMat["mnist_train"]
    trainingLabel = trainingLabelMat["mnist_train_labels"]
    testImg = testImgMat["mnist_test"]
    testLabel = testLabelMat["mnist_test_labels"]

    # Normalization
    trainingImg /= 255
    testImg /= 255

    numberOfTrainingData = trainingImg.shape[0]
    numberOfFeatures = trainingImg.shape[1]
    numberOfTestData = testImg.shape[0]

    # http://scikit-learn.org/stable/modules/generated/sklearn.ensemble.AdaBoostClassifier.html
    bdt = AdaBoostClassifier(DecisionTreeClassifier(criterion="entropy"))
    #bdt = AdaBoostClassifier(base_estimator=RandomForestClassifier())
    bdt.fit(trainingImg, trainingLabel.ravel())

    predict = bdt.predict(testImg).reshape(numberOfTestData, 1)
    score = bdt.score(testImg, testLabel)
    #predictProba = bdt.predict_proba(testImg)

    """
    trainingFullLabel = np.empty([10, numberOfTrainingData], dtype=np.int8)
    predictProbaFull = np.empty([numberOfTestData, 10])

    for i in range(10):
        trainingFullLabel[i] = (trainingLabel.transpose() == i)

    for i in range(10):
        bdt = AdaBoostClassifier(DecisionTreeClassifier(criterion="entropy"))
        bdt.fit(trainingImg, trainingFullLabel[i].ravel())

        predictProbaFull[:,i] = bdt.predict_proba(testImg)[:,1]
        
    predict = np.argmax(predictProbaFull, axis=1).reshape(numberOfTestData, 1)
    """

    wrongCount = numberOfTestData - np.count_nonzero(predict == testLabel)

    with open('adaboostResult.csv', 'w') as file:
        file.write("Error Rate: %f\n" %(wrongCount / 10000))
        file.write("Score: %f\n" %(score))
        for i in range(0, numberOfTestData):
            file.write("%d %d\n" %(predict[i], testLabel[i]))
