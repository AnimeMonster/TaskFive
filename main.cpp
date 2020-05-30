#include <iostream>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

Scalar randomColor() {
    static RNG rng(12345);
    return Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
}

int main()
{
    Mat image, src, src_gray, grad;


    string imageName = ("/home/student/Desktop/test_marker.jpg");
    image = imread(imageName, IMREAD_COLOR);
    if (image.empty()) {
        printf("Error opening image : %s\n", imageName.c_str());
        return EXIT_FAILURE;
    }

    GaussianBlur(image, src, Size(5, 5), 1, 1, BORDER_DEFAULT);
    cvtColor(src, src_gray, COLOR_BGR2GRAY);
    Canny(src_gray, grad, 30, 90);

    vector<vector<Point>> contours;
    findContours(grad, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

    vector<vector<Point>> hull(contours.size());
    for (size_t i = 0; i < contours.size(); i++) {
        convexHull(contours[i], hull[i]);
    }

    Mat drawing = Mat::zeros(grad.size(), CV_8UC3);
    for (size_t i = 0; i < contours.size(); i++){
        drawContours(drawing, contours, (int)i, randomColor());
        drawContours(drawing, hull, (int)i, randomColor());
    }
    imshow("hull", drawing);
    char key = (char)waitKey(0);

    vector<vector<Point>> rects;
    vector<int> possrects;
    for (size_t i = 0; i < contours.size(); i++) {
        auto areaCont = contourArea(contours[i]);
        auto areaHull = contourArea(hull[i]);
        if (abs(areaHull - areaCont) / areaHull < 0.1 && areaHull > 2000) {
            contours[i] = hull[i];
            possrects.push_back(i);
        }
        else {
            contours[i] = {};
        }
    }

    Mat possrectsImg = Mat::zeros(grad.size(), CV_8UC3);
    for (int i : possrects) {
        drawContours(possrectsImg, hull, i, randomColor());
    }
    imshow("possible rects", possrectsImg);
    waitKey(0);

    for (int i : possrects) {
        auto possrect = hull[i];
        auto minrect = minAreaRect(hull[i]);
        auto minrectPoints = new Point2f[4];
        minrect.points(minrectPoints);
        auto areaRect = contourArea(possrect);
        vector<Point> minrecttrue = {minrectPoints[0], minrectPoints[1], minrectPoints[2], minrectPoints[3]};
        delete[] minrectPoints;
        auto areaMin = contourArea(minrecttrue);
        if (areaMin / areaRect > 0.9 && areaMin / areaRect < 1.1){
            rects.emplace_back(possrect);
        }
    }

    Mat rectsImg = Mat::zeros(grad.size(), CV_8UC3);
    for (int i = 0; i < rects.size(); i++) {
        drawContours(rectsImg, rects, i, Scalar(0, 0, 255));
    }
    imshow("rects", rectsImg);
    waitKey(0);

    return EXIT_SUCCESS;
}
