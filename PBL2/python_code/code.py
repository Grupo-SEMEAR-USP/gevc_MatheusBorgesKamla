import numpy as np
import cv2
import manipulations as mp

def main():
    video_name = '../road6.mp4'
    cap = cv2.VideoCapture(video_name)
    #Testo se a inicializacao do video foi realizada
    if cap.isOpened() != True:
        print "Erro ao abrir o video !"
        exit()
    
    src_window = "Original Video"
    cv2.namedWindow(src_window,cv2.WINDOW_NORMAL)
    result_window = "Result Video"
    #cv2.namedWindow(result_window,cv2.WINDOW_NORMAL)
    #Contador de frames do video
    frame_counter = 0
    while(True):
        if cv2.waitKey(33) == 27:
            break
        ret,frame = cap.read()
        frame_counter += 1
        #Faz com que fique em loop o video (comparo se o frame atual e o ultimo frame)
        if frame_counter == cap.get(cv2.CAP_PROP_FRAME_COUNT):
            #Se for volto meu contador de frama para 0 (inicio)
            frame_counter = 0 
            #Defino qual o frame quero voltar (o 0 que e o inicio)
            cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
        #Testo se o frame foi lido corretamente
        if ret is True:
            cv2.imshow(src_window,frame)

            bird_img = mp.bird_Eyes(frame)
            cv2.namedWindow("Bird Eyes Transformation",cv2.WINDOW_NORMAL)
            cv2.imshow("Bird Eyes Transformation",bird_img)

            bin_img = mp.select_Channel(bird_img)
            cv2.namedWindow("Channel Selection",cv2.WINDOW_NORMAL)
            cv2.imshow("Channel Selection",bin_img)
            ponts_ini = mp.histogram_Line(bin_img)
        #Se nao eu printo o erro e para reproducao do video
        else:
            print "Erro ao ler frame !"
            break
    
    cap.release()
    cv2.destroyAllWindows()

main()
        