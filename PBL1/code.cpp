#include <iostream>
#include <stdio.h>
#include <math.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

void Ordena_Circulos(vector<Vec3f> &, vector<Vec3f> &);

int main(int argc, char **argv)
{
    //VideoCapture capture(0);
    //char* videoName = "painel2.mp4";
    char *imageName = argv[1];
    //VideoCapture capture(videoName);

    /*if(!capture.isOpened()) 
    { 
        cerr << " ERR: Unable find input Video source." << endl;
		return -1;
	}*/

    Mat src, gauss_image, imgGray, imgCircles, imgHSV, binary_image;

    int iLowH = 28;
    int iHighH = 204;

    int iLowS = 159;
    int iHighS = 255;

    int iLowV = 123;
    int iHighV = 253;

    namedWindow("Imagem Original", CV_WINDOW_NORMAL);
    namedWindow("Imagem GrayScale", CV_WINDOW_NORMAL);
    namedWindow("Imagem Binaria", WINDOW_NORMAL);
    namedWindow("Circulos Detectados", CV_WINDOW_NORMAL);

    int tot_pixels, found_pixels, point_aux;
    int lum_circulos[6];
    int leitura_painel[3];
    int upper_threshold = 90;

    vector<Vec3f> circles, ord_circles;
    float minimum_radius, maximum_radius;
    int pixel;

    //while(waitKey(33) != 27)
    //{
    //capture.read(src);
    src = imread(imageName, IMREAD_COLOR);

    if (src.empty())
    {
        cout << "Erro ao abrir imagem\n"
             << endl;
        return -1;
    }

    imgCircles = src.clone();

    cvtColor(src, imgGray, CV_BGR2GRAY);
    //Converto imagem para gray scale

    //blur( imgGray, imgGray, Size(3,3) );

    minimum_radius = 0.015 * src.rows;
    //Defino que o raio mínimo a ser encontrado sera o 1.5% do número de linhas da imagem original
    maximum_radius = 0.070 * src.rows;
    //Defino que o raio máximo a ser encontrado sera ate 7% do número de linhas da imagem original

    /// Apply the Hough Transform to find the circles
    HoughCircles(imgGray, circles, CV_HOUGH_GRADIENT, 1, imgGray.rows / 8, upper_threshold, 30, minimum_radius, maximum_radius);
    /*Recebe como parâmetros: imagem que deve ser em grayscale na qual procurará os contornos /// vetor do tipo 3-element floating-point
        (x, y, radius) que guardará o centro do circulo encontrado e o seu raio /// método de detecção utilizado - geralmente é o CV_HOUGH_GRADIENT, fator de escala de resolução se 1 mantem a escala da resolução
        do acumulador / se 2 os contornos serão buscado em metada da resolução da original /// Distância mínima entre os centros dos círculos detectados. 
        Se o parâmetro for muito pequeno, vários círculos vizinhos podem ser falsamente detectados além de um verdadeiro. Se for muito grande, alguns círculos podem ser perdidos.
        ///Primeiro parâmetro específico do método. No caso de CV_HOUGH_GRADIENT, é o limite mais alto dos dois passado para o detector de borda Canny ()
        //Segundo parâmetro passado para a Canny, quanto menor for mais contornos identificará, tendo a chance de achar mais falsos circulos
        //Valor mínimo de raios de circulos a ser achados/// valor máximo de raio a ser achado  */

    cout << "\n Circulos Detectados: " << circles.size() << endl;
    for (int j = 0; j < circles.size(); j++)
    {
        cout << " Circulo " << j << " : " << circles[j] << endl;
    }

    if (circles.size() >= 4)
    {
        Ordena_Circulos(circles, ord_circles);

        cout << "\n\nOrdenando: " << endl;
        for (int j = 0; j < circles.size(); j++)
        {
            cout << "Circulo " << j << " : " << ord_circles[j] << endl;
        }

        GaussianBlur(src, gauss_image, Size(5, 5), 0, 0, BORDER_DEFAULT);
        cvtColor(gauss_image, imgHSV, CV_RGB2HSV);
        inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), binary_image);

        cout << "\n\n-> Informacoes circulos " << endl;

        for (size_t i = 0; i < ord_circles.size(); i++)
        {
            //cvRound arredonda para int
            Point center(cvRound(ord_circles[i][0]), cvRound(ord_circles[i][1]));
            int radius = cvRound(ord_circles[i][2]);
            // circle center
            circle(imgCircles, center, 3, Scalar(0, 230, 238), -1, 8, 0);
            // circle outline
            circle(imgCircles, center, radius, Scalar(0, 230, 238), 3, 8, 0);

            found_pixels = 0;
            //Para cada pixel eu vou percorrer um quadrado de lados 2r em torno dos circulos
            for (int col = center.x - radius; col <= center.x + radius; ++col)
            {
                for (int row = center.y - radius; row <= center.y + radius; ++row)
                {
                    //Para cada pixel desse quadrado checo se ele pertence ao circulo (xponto - xcentro)² + (yponto - ycentro)² <= r²
                    if ((pow(col - center.x, 2) + pow(row - center.y, 2)) <= pow(radius, 2))
                    {
                        //Checo se o pixel e preto ou branco
                        point_aux = binary_image.at<uchar>(row, col);
                        if (point_aux > 0)
                            //Se for branco contabilizo o pixel como 'aceso'
                            found_pixels++;
                    }
                }
            }
            lum_circulos[i] = found_pixels;
            cout << "Luminosidade " << lum_circulos[i] << endl;
        }

        cout << "   -> Informacoes do painel " << endl;
        for (int i = 0; i < 3; i++)
        {
            if (lum_circulos[i] < lum_circulos[i+3])
                leitura_painel[i] = 1;
            else
                leitura_painel[i] = 0;

            if (leitura_painel[i] == 1)
            {
                cout << "        - LIGADO" << endl;
            }

            else
            {
                cout << "        - DESLIGADO" << endl;
            }
        }

        imshow("Circulos Detectados", imgCircles);
    }
    //else if (circles.size() >= 4)
    //    upper_threshold--;

    imshow("Imagem Original", src);
    imshow("Imagem GrayScale", imgGray);
    imshow("Imagem Binaria", binary_image);
    waitKey(0);
    //}

    return 0;
}

void Ordena_Circulos(vector<Vec3f> &circles, vector<Vec3f> &ord_circles)
{
    // Calculo do raio medio dos circulos detectados
    int raio_medio = 0;
    float raio_range = 0.2, y_med;
    for (int i = 0; i < circles.size(); i++)
    {
        raio_medio += circles[i][2];
        y_med += circles[i][1];
    }
    raio_medio = raio_medio / circles.size();
    cout << "\n\n-> Raio medio: " << raio_medio << " \nIntervalo de confiança: (" << (1 - raio_range) * raio_medio << "," << (1 + raio_range) * raio_medio << ") " << endl;

    y_med = y_med / circles.size();

    int cont_sup = 0, cont_inf = 0, indice_min;
    float min_x = 10000.0;
    ord_circles = circles;
    vector<Vec3f> sup_circles, inf_circles;

    for (int i = 0; i < circles.size(); i++)
    {
        if (circles[i][1] < y_med)
        {
            sup_circles.push_back(circles[i]);
        }
        else
        {
            inf_circles.push_back(circles[i]);
        }
    }

    while (cont_sup < sup_circles.size() /*&& cont_inf < inf_circles.size()*/)
    {
        for (int i = 0; i < sup_circles.size(); i++)
        {
            if (sup_circles[i][0] < min_x)
            {
                min_x = sup_circles[i][0];
                indice_min = i;
            }
        }
        ord_circles[cont_sup] = sup_circles[indice_min];
        sup_circles[indice_min][0] = 10000.0;
        cont_sup++;
        min_x = 10000.0;
        for (int i = 0; i < inf_circles.size(); i++)
        {
            if (inf_circles[i][0] < min_x)
            {
                min_x = inf_circles[i][0];
                indice_min = i;
            }
        }
        ord_circles[cont_inf + sup_circles.size()] = inf_circles[indice_min];
        inf_circles[indice_min][0] = 10000.0;
        cont_inf++;
        min_x = 10000.0;
    }
}
