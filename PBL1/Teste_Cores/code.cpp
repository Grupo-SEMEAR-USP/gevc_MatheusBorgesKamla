#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
    char *image_name = argv[1];

    /*char* videoName = "../painel2.mp4";
    VideoCapture capture(videoName);

    if(!capture.isOpened()) 
    { 
        cerr << " ERR: Unable find input Video source." << endl;
		return -1;
	}*/

    namedWindow("Original Image", WINDOW_NORMAL);

    Mat gauss_image, hsv_image, imgThresholded, original_image;
    original_image = imread(image_name, IMREAD_COLOR);

    namedWindow("Control", WINDOW_NORMAL);
    resizeWindow("Control", Size(500, 800));

    int iLowH = 28;
    int iHighH = 204;

    int iLowS = 159;
    int iHighS = 255;

    int iLowV = 123;
    int iHighV = 253;

    /*while (waitKey(33) != 27)
    {
        capture.read(original_image);*/
        imshow("Original Image", original_image);

        GaussianBlur(original_image, gauss_image, Size(5, 5), 0, 0, BORDER_DEFAULT);

        cvtColor(gauss_image, hsv_image, CV_RGB2HSV);

        createTrackbar("LowH", "Control", &iLowH, 255);
        createTrackbar("HighH", "Control", &iHighH, 255);

        createTrackbar("LowS", "Control", &iLowS, 255);
        createTrackbar("HighS", "Control", &iHighS, 255);

        createTrackbar("LowV", "Control", &iLowV, 255);
        createTrackbar("HighV", "Control", &iHighV, 255);

        while (true)
        {
            inRange(hsv_image, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);
            imshow("Control", imgThresholded);
            break;
            
        }

        waitKey(0);
    //}

    return 0;
}
