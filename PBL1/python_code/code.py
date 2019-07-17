import numpy as np
import matplotlib.pyplot as plt
import cv2
import sys



def main():
    sys.argv.append(' ')
    image_name = sys.argv[1]
    src = cv2.imread(image_name,cv2.IMREAD_COLOR)
    var = [1,2,3,4]
    print type(var)

    if src is None:
        print "Erro ao abrir imagem"
        return
    
    cv2.namedWindow('Imagem Original',cv2.WINDOW_NORMAL)
    cv2.imshow('Imagem Original',src)
    
    
    imgGray = cv2.cvtColor(src,cv2.COLOR_BGR2GRAY)
     
    cv2.namedWindow('Imagem Grayscale',cv2.WINDOW_NORMAL)
    cv2.imshow('Imagem Grayscale',imgGray)
    print imgGray.shape
    raio_min = int(imgGray.shape[0]*0.015)
    raio_max = int(imgGray.shape[0]*0.07)

    #min_threshold = 30
    min_Dist = int((imgGray.shape[0] / 8))

    imgGray = cv2.medianBlur(imgGray,5)

    circles = cv2.HoughCircles(imgGray,cv2.HOUGH_GRADIENT,1, min_Dist, 100, 90, raio_min, raio_max)
    #circles = np.uint16(np.around(circles))
    #print circles

    detected_img = src.copy()
    for (x, y, r) in circles[0]:
	# draw the circle in the output image, then draw a rectangle
	# corresponding to the center of the circle
	    cv2.circle(detected_img, (x, y), 20, (0, 255, 0), 4)
    #cv2.circle(detected_img,(circles[0][0],circles[0][1]),5) 
    cv2.namedWindow('Circulos',cv2.WINDOW_NORMAL)
    cv2.imshow('Circulos',detected_img)
    
    while True:
        k = cv2.waitKey(33)
        if k == 27 :
            break
    cv2.destroyAllWindows()

main()