import numpy as np
import cv2

def bird_Eyes(src):
    #Pegando o numero de linhas e colunas da imagem original
    rows = src.shape[0]
    cols = src.shape[1]
    #Definindo a regiao que pegarei da minha imagem de origem
    pts_src = np.float32([[0,rows],[0.3*cols,0.3*rows],[0.7*cols,0.3*rows],[cols,rows]])
    #Definindo para qual regiao quero gerar minha mudanca de perspectiva
    pts_dst = np.float32([[0,480],[0,0],[640,0],[640,480]])
    #Gerando minha matriz responsavel por realizar a mudanca de perspectiva
    M = cv2.getPerspectiveTransform(pts_src,pts_dst)
    #Gerando minha nova imagem na nova pespectiva
    bird_img = cv2.warpPerspective(src,M,(640,480),cv2.INTER_LINEAR,cv2.BORDER_DEFAULT)

    return bird_img

def select_Channel(src):
    #Aplicando filtro do tipo blur para retirar ruidos e homogenizar a imagem
    gauss_img = cv2.GaussianBlur(src,(3,3),0,0,cv2.BORDER_DEFAULT)
    #Passando imagem de RGB para HSV
    img_hsv = cv2.cvtColor(gauss_img,cv2.COLOR_BGR2HSV)
    #Gerando imagem binaria com a cor amarela encontrada
    img_bin = cv2.inRange(img_hsv,np.array([0,75,135]),np.array([145,255,208]))
    #Aplicando filtros morfologicos para tentar homogenizar a imagem binaria nas regioes de interesse
    img_bin = cv2.erode(img_bin,cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(5,5)))
    img_bin = cv2.dilate(img_bin,cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(5,5)))
    img_bin = cv2.dilate(img_bin,cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(5,5)))
    img_bin = cv2.erode(img_bin,cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(5,5)))
    
    return img_bin

def histogram_Line(src):
    #Pegando o numero de linhas e colunas da imagem original
    src_rows = src.shape[0]
    src_cols = src.shape[1]
    #Gerando uma imagem totalmente em branco do tamanho da src
    hist_img = np.zeros([src_rows,src_cols,3],dtype=np.uint8)
    hist_img[:] = 255
    #Sera meu vetor que contabilizara o numero de pixels brancos encontrados na coluna
    count = np.zeros(src_cols)
    #Irei passar coluna por coluna da src
    for col in range(0,src_cols):
        #Para cada coluna vou da metade de linhas da imagem ate o final linha por linha
        for row in range(int(src_rows*0.2),src_rows):
            #Cada pixel checo se e diferente de 0 (se e diferente de preto)
            if src[row][col] != 0:
                #Se for, contabilizo no meu vetor contador
                count[col] += 1
        cv2.circle(hist_img,(col,src_rows-1-int(count[col])),1,(0,0,255),3,8,0)
    #Vou pegar a posicao da coluna que possui o maior numero de pixels achados para determinar meu ponto inicial
    best_col = 1
    pont_ini = np.float32([src_rows,best_col])

    cv2.namedWindow("Histogram",cv2.WINDOW_NORMAL)
    cv2.imshow("Histogram",hist_img)
    return pont_ini
