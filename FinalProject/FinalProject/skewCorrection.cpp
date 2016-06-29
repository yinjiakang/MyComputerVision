#include "skewCorrection.h"

// Store line info.
struct myLine {
    double m; // Slope of the line
    double c; // Intercept of the line
    bool vertical; // True if the line is parallel to y-axis
};


// Line comparison function based on theta.
bool lineCmp(const Vec2f& a, const Vec2f& b) {
    const double threshold = 0.1;
    if ( abs(a[1] - b[1]) < threshold  ) {
        return a[0] < b[0];
    }
    return a[1] < b[1];
}

// Point comparison function
bool pointCmp(const Point2f& a, const Point2f& b) {
    return a.x + a.y < b.x + b.y;
}


Mat skewCorrection(Mat srcImg) {
    Mat dstImg, srcGray, srcBlur, edge;

    // Ref: http://docs.opencv.org/2.4/modules/imgproc/doc/geometric_transformations.html#resize
    resize(srcImg, srcImg, Size(), 1, 1);

    // Ref: http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/canny_detector/canny_detector.html
    // Transfrom to gray scale.
    cvtColor(srcImg, srcGray, CV_BGR2GRAY);

    // Blur the image to erase some noise.
    blur(srcGray, srcBlur, Size(3, 3));

    // Set thresholds for Canny
    const int lowThreshold = 125;
    const int highThreshold = 500;
    const int kernel_size = 3;
    Canny(srcBlur, edge, lowThreshold, highThreshold, kernel_size);

    // Ref: http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/hough_lines/hough_lines.html
    // Set parameters for hough transform
    const float rho = 1;
    const float theta = CV_PI / 100;
    const int threshold = 81;
    vector<Vec2f> lines;
    vector<myLine> mls;

    HoughLines(edge, lines, rho, theta, threshold, 0, 0);
    

    /*** Pre-process lines and output line equations ***/

    // Sort lines based primarily on theta
    sort(lines.begin(), lines.end(), lineCmp);

    // Move lines with theta near 3.1, which is vertical lines to the front.
    int last = lines.size() - 1;
    const double minTheta = 0.1;
    const double maxTheta = 3.1;
    const double rhoThreshold = 50;
    /* 
     * If the first line has theta near zero while the last line has theta near PI,
     * they are almost parallel, so move the last line to the front.
     */
    while ( lines[0][1] <= minTheta && lines[last][1] >= maxTheta ) {
        vector<Vec2f>::iterator it = lines.begin() + 1;
        for ( size_t i = 1; i < last; ++i ) {
            if ( lines[i][1] <= minTheta && abs(lines[0][0] - lines[i][0]) < rhoThreshold ) {
                ++it;
            }
        }
        lines.insert(it, lines[last]);
        it = lines.end() - 1;
        lines.erase(it);
    }

    for ( size_t i = 0; i < lines.size(); i++ ) {
        float rho = lines[i][0], theta = lines[i][1];

        // Eliminate lines that are close to each other.
        const float deltaRho = 150;
        const float deltaTheta = 0.25;
                      //  When rhos are very close and
        if ( i > 0 && abs(abs(rho) - abs(lines[i - 1][0])) < deltaRho &&
            ( abs(CV_PI - (theta + lines[i - 1][1])) < deltaTheta || abs(theta - lines[i - 1][1]) < deltaTheta ) ) {
            // When one theta is near PI and the other is near 0 or thetas are very close.
            vector<Vec2f>::iterator it = lines.begin() + i;
            lines.erase(it);
            --i;
            continue;
        }
        
        // Eliminate wrong lines.
        if ( i > 0 && i < lines.size() - 1 &&
             // When the line is not parallel to the two sides
             abs(theta - lines[i - 1][1]) > deltaTheta && abs(theta - lines[i + 1][1]) > deltaTheta &&
             abs(CV_PI - (theta + lines[i - 1][1])) > deltaTheta ) {
             // When one theta is near PI and the other is near 0
            vector<Vec2f>::iterator it = lines.begin() + i;
            lines.erase(it);
            --i;
            continue;
        }
        
        // cout << "rho: " << rho << "  theta: " << theta << endl;

        // Draw a line by two points.
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        pt1.x = cvRound(x0 + 1000 * (-b));
        pt1.y = cvRound(y0 + 1000 * (a));
        pt2.x = cvRound(x0 - 1000 * (-b));
        pt2.y = cvRound(y0 - 1000 * (a));
        // Draw red edges.
        //line(srcImg, pt1, pt2, Scalar(0, 0, 255), 3, CV_AA);

        struct myLine ml;
        // Deal with lines parallel to y-axis.
        if ( b == 0 ) {
            ml.vertical = 1;
            ml.c = rho;
            // printf("Line equation: x = %lf\n", rho);
        } else {
            ml.vertical = 0;
            // m for slope and c for y-intercept.
            double m = -a / b, c = rho / b;
            ml.m = m;
            ml.c = c;
            // printf("Line equation: y = (%lfx) + (%lf)\n", m ,c);
        }
        
        mls.push_back(ml);
    }



    /*** Output point coordinates ***/

    vector<Point2f> pts;
    // Calculate line-line intersection.
    // Ref: https://en.wikipedia.org/wiki/Line–line_intersection
    for ( int i = 0; i < 2; ++i ) {
        for ( int j = 2; j < 4; ++j ) {
            double a = mls[i].m, b = mls[j].m;
            double c = mls[i].c, d = mls[j].c;
            double x, y;
            // If slope = inf
            if ( mls[i].vertical == 1 ) {
                x = c;
                y = b * c + d;
            } else {
                x = (d - c) / (a - b);
                y = (a * d - b * c) / (a - b);
            }
            // printf("Point: (%lf, %lf)\n", x, y);
            Point2f pt1;
            pt1.x = x;
            pt1.y = y;
            pts.push_back(pt1);
            // Draw blue corner points
            //line(srcImg, pt1, pt1, Scalar(255, 0, 0), 12, CV_AA);
        }
    }

    // cout << "------------------" << endl << endl;

    // imshow("Display Image", srcImg);

    /*** Deskew ***/

    double rotationAngle = lines[0][1] / CV_PI * 180;
    const double rotationThreshold = 0.1;
    // Rotate 90 degree clockwise for horizontal papers, e.g. 2.jpg and 4.jpg.
    if ( lines[0][1] > rotationThreshold ) {
        rotationAngle -= 90;
    }

    // Calculate the center of the paper, which is later used for rotation.
    Point center;
    center.x = (pts[0].x + pts[1].x + pts[2].x + pts[3].x) / 4;
    center.y = (pts[0].y + pts[1].y + pts[2].y + pts[3].y) / 4;
    //line(srcImg, center, center, Scalar(0, 255, 0), 12, CV_AA);

    Mat rotatedImg;
    Mat rotationMatrix;

    // cout << endl << "Rotation angle: " << rotationAngle << endl;
    
    // Deal with 2.jpg in final project.
    if ( rotationAngle < -10 ) {
        rotationAngle += 180;
    }

    // Ref: http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/warp_affine/warp_affine.html
    rotationMatrix = getRotationMatrix2D(center, rotationAngle, 1);
    warpAffine(srcImg, rotatedImg, rotationMatrix, rotatedImg.size());
    // Map the points to the rotated image
    transform(pts, pts, rotationMatrix);

    if ( srcImg.rows < srcImg.cols ) {
        /*
         * Rotate 90 degree clockwise for horizontal papers and diminish their sizes.
         * Otherwise, the papers will exceed the bound of the images.
         */
        const float scale = 0.75;
        rotationMatrix = getRotationMatrix2D(center, 90, scale);
        warpAffine(rotatedImg, rotatedImg, rotationMatrix, rotatedImg.size() );
        transform(pts, pts, rotationMatrix);
    }

    /*** transform to rectangle ***/

    // Sort the points to determine the position of each point.
    sort(pts.begin(), pts.end(), pointCmp);


    // Set standard points
    
    const int lx = 130;
    const int ly = 160;
    const int rx = 510;
    const int ry = 720;
    


    const int width = rx - lx;
    const int height = ry - ly;
    Point2f topLeft;
    topLeft.x = lx;
    topLeft.y = ly;
    Point2f topRight;
    topRight.x = rx;
    topRight.y = ly;
    Point2f bottomLeft;
    bottomLeft.x = lx;
    bottomLeft.y = ry;
    Point2f bottomRight;
    bottomRight.x = rx;
    bottomRight.y = ry;
    vector<Point2f> standardPoints;
    standardPoints.push_back(topLeft);
    standardPoints.push_back(topRight);
    standardPoints.push_back(bottomLeft);
    standardPoints.push_back(bottomRight);

    // Ref: http://docs.opencv.org/2.4/modules/imgproc/doc/geometric_transformations.html
    // Assertion failed: https://groups.google.com/forum/?fromgroups=#!topic/android-opencv/CPDMJsmYVBI
    Mat perspectiveMatrix;
    perspectiveMatrix = getPerspectiveTransform(pts, standardPoints);

    Mat perspectiveImg(800, 800, rotatedImg.type());
    warpPerspective(rotatedImg, perspectiveImg, perspectiveMatrix, perspectiveImg.size());

    // Ref: http://stackoverflow.com/questions/8267191/how-to-crop-a-cvmat-in-opencv
    // Region of interest, which is used to crop the image.
    Rect myROI(lx, ly, width, height);

    Mat croppedImg = perspectiveImg(myROI);
    Mat boundaryRemovedImg(croppedImg, Rect(8, 8, croppedImg.cols - 16, croppedImg.rows - 16));

    // imshow("Display Image", croppedImg);
    // waitKey(0);
    
    return boundaryRemovedImg;
}
