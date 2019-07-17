#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
using namespace cv;
using namespace std;

Mat src_gray;
int thresh = 100;
float minimum_radius , maximum_radius;
RNG rng(12345);

void thresh_callback(int, void* );

int main( int argc, char** argv )
{
    char* imgName = argv[1];
    Mat src = imread(imgName,IMREAD_COLOR);
    if( src.empty() )
    {
      cout << "Erro ao abrir imagem\n" << endl;
      return -1;
    }

    cvtColor( src, src_gray, COLOR_BGR2GRAY );
    blur( src_gray, src_gray, Size(3,3) );
    
    namedWindow("Imagem Original",WINDOW_NORMAL);
	namedWindow("Imagem GrayScale",WINDOW_NORMAL);
	namedWindow("Contornos",WINDOW_NORMAL);

    imshow( "Imagem Original", src );
	imshow( "Imagem GrayScale", src_gray );

	minimum_radius = 0.015 * src.rows;
    maximum_radius = 0.070 * src.rows;

	createTrackbar( "Canny thresh:", "Contornos", &thresh, 255, thresh_callback );
	createTrackbar( "Canny thresh:", "Contornos", &thresh, 255, thresh_callback );
	createTrackbar( "Canny thresh:", "Contornos", &thresh, 255, thresh_callback );
    thresh_callback( 0, 0 );
    waitKey();
    return 0;
}
void thresh_callback(int, void* )
{
    Mat canny_output;
	float minimum_radius, maximum_radius;
	vector<Vec3f> circles;
    Canny( src_gray, canny_output, 30, thresh );
	HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, src_gray.rows/8, thresh, 30, minimum_radius, maximum_radius );
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( size_t i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
        drawContours( drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0 );
		cout<<"N_circles: " << circles.size() << endl;
		for(int j=0; j<circles.size(); j++){
			cout<<" Circulo " << j+1 << " : " << circles[j] << endl;
		}
    }
	cout <<"---------------------------------------------------------" << endl;
    imshow( "Contornos", drawing );
}