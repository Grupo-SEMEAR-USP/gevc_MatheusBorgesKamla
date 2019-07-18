#include <iostream>
#include <math.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
/*#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include "std_msgs/Float32.h"
#include "std_msgs/Int64.h"*/

using namespace cv;
using namespace std;

#define PI 3.1415926

char* source_window = "Original Video";
char* result_window = "Result Video";

/*
    Variável que armazena a identidade da linha analisada
    0: nenhuma linha
    1: linha continua
    2: linha pontilhada
*/
int current_line_view;

/*
    Variável que armazena o lado em que o veículo se encontra na pista
    -1: esquerda
     1: direita
*/
int current_side;

void bird_Eyes(Mat&, Mat&); 

int iLowV = 0;
int iHighV = 255;
void select_Channel(Mat&, Mat&, int, int);

Point2i button;
int previous_x = -1;
void histogram_Line(Mat&);

/* Constantes da funções de sliding_Window */
int h_rectangle = 20;
int l_rectangle = 110;
int num_rectangle = 15; // Quando atualizar, lembrar de atualizar o tamanho dos vetores control_line[] e alpha[]

vector<Point2f> Central_Line;
vector<Point2i> iCentral_Line;
int control_line[20];
double alpha[20];
void sliding_Window_Line(Mat&, Mat&);

vector<Point2f> final_navegavel;
vector<Point2i> ifinal_navegavel;
void inverse_bird_Eyes(Mat&, Mat&, Mat&);

float shift_Central(Mat&);

int main( int argc, char** argv )
{
    /*ros::init(argc, argv, "shift_central");
    ros::NodeHandle nh;
    image_transport::ImageTransport it_(nh);
    ros::Publisher vision_pub = nh.advertise<std_msgs::Float32>("/shift_central_topic", 100);
    ros::Publisher line_view_pub = nh.advertise<std_msgs::Int64>("/line_view_topic", 100);
    ros::Publisher side_pub = nh.advertise<std_msgs::Int64>("/side_topic", 100);
    image_transport::Publisher image_pub_ = it_.advertise("/road_vision", 1);
    cv_bridge::CvImagePtr cv_ptr(new cv_bridge::CvImage);*/

    //std_msgs::Float32 shift_msg;
    float shift_result;
    //std_msgs::Int64 line_msg;
    //std_msgs::Int64 side_msg;

    Mat src; // Frame original lido da câmera
    Mat bird_img; // Bird Eyes Transformation
    Mat color_img; // Manipulação dos canais de cor
    Mat sliding_img; // Sliding window
    Mat result_img; // Resultado 

    /* Abertura do vídeo */ 
    char* videoName = "road6.mp4";
    VideoCapture cap(videoName);
    //VideoCapture cap(1);
    if ( !cap.isOpened() )
    {
        cout << "Erro ao abrir o video" << endl;
        return -1;
    }

    /* Janelas */
    namedWindow(source_window, CV_WINDOW_NORMAL);
    namedWindow(result_window, CV_WINDOW_NORMAL);

    for(int j = 0; j<num_rectangle; j++)
        control_line[j] = -1;

    //ros::Rate loop_rate(100); //100 hz
    while ( (waitKey(33) != 27) /*&& ros::ok()*/)
    {
        //ros::Time time = ros::Time::now();

        cap.read(src);
       
        bird_Eyes(src,bird_img);

        select_Channel(bird_img, color_img, iLowV, iHighV);
        
        histogram_Line(color_img);

        sliding_img = bird_img.clone();

        sliding_Window_Line(color_img, sliding_img);

        inverse_bird_Eyes(src, bird_img, result_img);

        shift_result = shift_Central(result_img);

        // Deslocamento entre a linha da pista e o robô
        /*shift_msg.data = shift_result;
        ROS_INFO_STREAM("Deslocamento: " <<  shift_msg.data);
        vision_pub.publish(shift_msg);*/

        cout << "Deslocamento: " << shift_result << endl; 
        
        // Linha analisada
        /*line_msg.data = current_line_view;
        ROS_INFO_STREAM("Linha atual: " << line_msg.data);
        line_view_pub.publish(line_msg);*/

        cout << "Linha atual: " << current_line_view << endl; 

        // Lado do robô
        /*side_msg.data = current_side;
        ROS_INFO_STREAM("Linha atual: " << side_msg.data);
        side_pub.publish(side_msg);*/

        cout << "Posicao: " << current_side << endl; 
        
        imshow(source_window, src);
        imshow(result_window, result_img);

        /*cv_ptr->encoding = "bgr8";
        cv_ptr->header.stamp = time;
        cv_ptr->header.frame_id = "/road_vision";
        cv_ptr->image = result_img;
        image_pub_.publish(cv_ptr->toImageMsg());*/

        Central_Line.clear();
        iCentral_Line.clear();
        final_navegavel.clear();
        ifinal_navegavel.clear();

        /*ros::spinOnce();
        loop_rate.sleep();*/
        //waitKey(0);
    }

    //waitKey(0);
}

void select_Channel(Mat& in, Mat& out, int low, int high)
{
    Mat tr;
    Mat imgThresholded;

    GaussianBlur(in,tr,Size(3,3),0,0,BORDER_DEFAULT);
    cvtColor(tr,tr,CV_RGB2HSV); // conversão para HLS

    inRange(tr, Scalar(0, 75, 135), Scalar(145, 255, 208), imgThresholded); //Para o vídeo modelo
    //inRange(tr, Scalar(0, 0, 210), Scalar(179, 255, 255), imgThresholded); //Para o Helvis
      
    //morphological opening (remove small objects from the foreground)
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

    //morphological closing (fill small holes in the foreground)
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    /*namedWindow("H channel", CV_WINDOW_NORMAL);
    namedWindow("S channel", CV_WINDOW_NORMAL);
    namedWindow("L channel", CV_WINDOW_NORMAL);
    imshow("H channel", hls_planes[0]);
    imshow("S channel", hls_planes[1]);
    imshow("L channel", hls_planes[2]);*/

    //inRange(hls_planes[1], Scalar(low), Scalar(high), out); // selecão do canal H
    //inRange(hls_planes[1], 30, 180, tr); // selecão do canal L
    //bitwise_not ( imgThresholded, imgThresholded );

    out = imgThresholded;

    char* color_window = "Selecao do Canal e das Intensidades de Cor";
    namedWindow(color_window, CV_WINDOW_NORMAL);
    imshow(color_window, out);
}

void bird_Eyes(Mat& in, Mat& out)
{
    Mat tr; // variável de manipulação interna da função

    int Rows = in.rows;
    int Cols = in.cols;

    Point2f src_vertices[4];
    src_vertices[0] = Point(        0,      Rows); // 
    src_vertices[1] = Point(0.30*Cols, 0.30*Rows); // 
    src_vertices[2] = Point(0.70*Cols, 0.30*Rows); // 
    src_vertices[3] = Point(     Cols,      Rows); // 

    Point2f dst_vertices[4];
    dst_vertices[0] = Point(  0, 480);
    dst_vertices[1] = Point(  0,   0);
    dst_vertices[2] = Point(640,   0);
    dst_vertices[3] = Point(640, 480);

    Mat M = getPerspectiveTransform(src_vertices, dst_vertices);
    tr = Mat(480, 640, CV_8UC3);
    warpPerspective(in, tr, M, tr.size(), INTER_LINEAR, BORDER_CONSTANT);

    out = tr;
    
    char* bird_window = "Bird Eyes Transformation";
    namedWindow(bird_window, CV_WINDOW_NORMAL);
    imshow(bird_window, out);
}

void histogram_Line(Mat& in)
{
    Mat histImage(in.rows, in.cols, CV_8UC3, Scalar(255,255,255) );

    int count[in.cols];
    long int count_Num = 0;
    int count_Den = 0;
    int i_best_col = -1;
    int n_best_col = 0;
    //int normalized_count[in.cols];
    float inicial_Row = 0.5 * in.rows;

    for (int col = 0; col < in.cols; ++col)
    {
        count[col]=0;

        for (int row = inicial_Row ; row < in.rows; ++row)
        {
            if ( (int)(in.at<uchar>(row,col)) != 0)
            {
                ++count[col];
                count_Num+=(col*count[col]);
                count_Den+=count[col];
            }
        }

        //if (count[col] != 0)
            //cout << "Coluna " << col << " tem " << count[col] << " pixeis" << endl; 

        if (count[col] > n_best_col)
        {
            i_best_col = col;
            n_best_col = count[col];
        }             

        circle(histImage, Point(col, in.rows - count[col]),1,Scalar(0,0,255),3,8,0);
    }

    button.y = in.rows;
    
    /*if ( ( (count_Den > 10) || (previous_x == -1))  && (count_Den != 0) )
    {
        //cout << "Count_num: " << count_Num << "  Count_den: " << count_Den << endl;
        button.x = count_Num / count_Den;
        previous_x = button.x;
    }*/
    if ( ( (count_Den > 10) || (previous_x == -1) ) )
    {
        button.x = i_best_col;
        previous_x = button.x;
    }
    else   
        button.x = previous_x;

    //cout << "Ponto inicial (" << button.x << "," << button.y << " ) " << endl;

    char* histogram_window = "Histogram 2";
    namedWindow(histogram_window, CV_WINDOW_NORMAL);
    imshow(histogram_window, histImage);
}

void sliding_Window_Line(Mat& in, Mat& out)
{
    // Desenha o quadrado inicial resultante do histograma
    Point2d initial_Center = Point(button.x, button.y);
    Point2d last_good;
    int ilast_good = -1;
    Point2d next_good;
    int inext_good = -1;
    int count_dct = 0;

    Point P1, P2; // pontos auxiliares para desenho dos retângulos
    P1.x = button.x - (l_rectangle/2);
    P1.y = button.y - (h_rectangle/2);
    P2.x = button.x + (l_rectangle/2);
    P2.y = button.y;
    rectangle(out, P1, P2, Scalar(255,255,0), 2, 8, 0);

    Point2d previous_Center = initial_Center;
    Point2d new_Center;
    int count[l_rectangle];
    int count_Num;
    int count_Den;

    for (int i=0; i<num_rectangle; i++)
    {
        count_Num = 0;
        count_Den = 0;

        new_Center.x = previous_Center.x;
        new_Center.y = previous_Center.y - h_rectangle;

        P1.x = new_Center.x - (l_rectangle/2);
        P1.y = new_Center.y - (h_rectangle/2);
        P2.x = new_Center.x + (l_rectangle/2);
        P2.y = new_Center.y + (h_rectangle/2);

        for (int col = P1.x ; col <= (P1.x + l_rectangle); col++)
        {
            count[col-P1.x] = 0;
            for (int row = P1.y ; row <= (P1.y + h_rectangle); row++)
            {
                if ( (int)(in.at<uchar>(row,col)) != 0)
                {
                    ++count[col-P1.x];
                    count_Num+=(col*count[col-P1.x]);
                    count_Den+=count[col-P1.x];
                } 
            }
            //cout << "A coluna " << col << "(indice " << col-P1.x << " ) tem  " << count[col-P1.x] << endl;
        }

        //cout << " O quadrado " << i << " tem " << count_Den << " pontos " << endl;
        if ( count_Den > ( 0.05 * (l_rectangle * h_rectangle) ) )
        {
            new_Center.x = count_Num / count_Den;

            control_line[i] = new_Center.x;
        }
        else
        {
            control_line[i] = -1;
            //cout << "Centro anterior [ " << i << " ]: ( " << previous_Central_Line[i].x << " , " << previous_Central_Line[i].y << " ) " << endl;
            //cout << "Centro anterior [ " << i << " ]: " << previous_Central_Line[1].x << endl;               
        }

        Central_Line.push_back(new_Center);
        iCentral_Line.push_back(new_Center); 

        /*if (i>0)
        {
            double aux = ( Central_Line[i].y - iCentral_Line[i-1].y )/( Central_Line[i].x - iCentral_Line[i-1].x ) ;
            alpha[i-1] = ( atan(aux) * 180 ) / (double) PI;
            //cout << "Alpha [ " << i-1 << " ]: " << alpha[i-1] << endl;
        }*/

        if(control_line[i] != -1)
        { 
            P1.x = new_Center.x - (l_rectangle/2);
            P1.y = new_Center.y - (h_rectangle/2);
            P2.x = new_Center.x + (l_rectangle/2);
            P2.y = new_Center.y + (h_rectangle/2);

            rectangle(out, P1, P2, Scalar(0,100,255), 2, 8, 0);
        }
        else    
            count_dct++;

        previous_Center = new_Center;
    }

    if ( count_dct > (0.1 * num_rectangle) )
    {
        if ( count_dct > (0.9 * num_rectangle) )
            current_line_view = 0;
        else 
            current_line_view = 2;
    }
    else if ( (current_side == -1 ) && (current_side == -1) )
        current_line_view = 2;
    else if ( (current_side == -1 ) && (current_side == 1) )
        current_line_view = 3;

    //cout << "Descontinuidades: " << count_dct << endl;
    //cout << "10 pcento do n retangulos: " << 0.1 * num_rectangle << endl;
    //cout << "Linha em análise: " << current_line_view << endl;

    int num_rec_dct = 0;
    double alfa_aux;
    for (int k=0; k<num_rectangle; k++)
    {
        if (control_line[k] == -1)
        {
            num_rec_dct++;

            if( (k > 0) && (ilast_good == -1) )
            {
                last_good = Central_Line[k-1];
                ilast_good = k-1;
            }
            else if (k == 0)
            {
                last_good.x = initial_Center.x;
                last_good.y = initial_Center.y - h_rectangle;
                ilast_good = 0;

                P1.x = last_good.x - (l_rectangle/2);
                P1.y = last_good.y - (h_rectangle/2);
                P2.x = last_good.x + (l_rectangle/2);
                P2.y = last_good.y + (h_rectangle/2);

                rectangle(out, P1, P2, Scalar(0,100,255), 2, 8, 0);
            }
        }     
        else
        {
            if( (ilast_good != -1) && (inext_good == -1) )
            {
                next_good = Central_Line[k];
                inext_good = k;
                break;
            }
        }       
    }


    alfa_aux =  ( next_good.y - last_good.y) / (next_good.x - last_good.x) ; 

    /*cout << "Ultimo bom retangulo: " << ilast_good << endl;
    cout << "Retangulo " << ilast_good << " : ( " << last_good.x << " , " << last_good.y << " )" << endl;
    cout << "Proximo bom retangulo: " << inext_good << endl;
    cout << "Retangulo " << inext_good << " : ( " << next_good.x << " , " << next_good.y << " )" << endl;*/

    for(int k = 1; k <= num_rec_dct; k++)
    {
        Central_Line[ilast_good + k].x = last_good.x + ( Central_Line[ilast_good + k].y - last_good.y) / alfa_aux;
        //cout << "Retangulo " << ilast_good + k << " : ( " << Central_Line[ilast_good + k].x << " , " << Central_Line[ilast_good + k].y << " )" << endl;

        P1.x = Central_Line[ilast_good + k].x - (l_rectangle/2);
        P1.y = Central_Line[ilast_good + k].y - (h_rectangle/2);
        P2.x = Central_Line[ilast_good + k].x + (l_rectangle/2);
        P2.y = Central_Line[ilast_good + k].y + (h_rectangle/2);

        rectangle(out, P1, P2, Scalar(0,150,255), 2, 8, 0);
    }

    for(int k = inext_good; k < num_rectangle; k++)
    {
        if(control_line[k] == -1)
        {
            P1.x = Central_Line[k].x - (l_rectangle/2);
            P1.y = Central_Line[k].y - (h_rectangle/2);
            P2.x = Central_Line[k].x + (l_rectangle/2);
            P2.y = Central_Line[k].y + (h_rectangle/2);

            rectangle(out, P1, P2, Scalar(0,180,255), 2, 8, 0);
        }
    }

    char* sliding_window = "Sliding Window";
    namedWindow(sliding_window, CV_WINDOW_NORMAL);
    imshow(sliding_window, out);
}

void inverse_bird_Eyes(Mat& src, Mat& in, Mat& out)
{
    out = src.clone();

    int Rows = src.rows;
    int Cols = src.cols;

    Point2f src_vertices[4];
    src_vertices[0] = Point(        0,      Rows); // 
    src_vertices[1] = Point(0.30*Cols, 0.30*Rows); // 
    src_vertices[2] = Point(0.70*Cols, 0.30*Rows); // 
    src_vertices[3] = Point(     Cols,      Rows); // 

    Point2f dst_vertices[4];
    dst_vertices[0] = Point(  0, 480);
    dst_vertices[1] = Point(  0,   0);
    dst_vertices[2] = Point(640,   0);
    dst_vertices[3] = Point(640, 480);

    Mat M = getPerspectiveTransform(dst_vertices, src_vertices);

    perspectiveTransform(Central_Line, final_navegavel, M);

    for(int i=0 ; i<final_navegavel.size() ; i++)
    {
        ifinal_navegavel.push_back(Point(final_navegavel[i].x, final_navegavel[i].y));
        /*cout << "Ponto inicial: ( " << right_Line[i].x << " ; " << right_Line[i].y << " ) " << endl;
        cout << "Ponto navegavel:   ( " << navegavel_int[i].x << " ; " << navegavel_int[i].y << " ) " << endl;*/
    }

    polylines(out,ifinal_navegavel,0,Scalar(255,0,0),3,8,0);
}

float shift_Central(Mat& in)
{ 
    float frame_center = in.cols / 2;
    float shift;
    float shift_num = 0;
    float shift_den = 0;

    line(in,Point(frame_center,0),Point(frame_center,in.rows),Scalar(0,255,0),3,8,0);

    for(int i=0; i < final_navegavel.size(); i++)
    {      
        shift_num  += frame_center - final_navegavel[i].x;
        shift_den++;
    }

    shift = shift_num/shift_den;

    if (shift >= 0)
        current_side = 1;
    else   
        current_side = -1;

    //cout << "Deslocamento da linha central (em pixeis): " << shift << endl;
    //cout << "Lado (-1: esquerda// 1: direita): " << current_side << endl;

    return shift;
}
