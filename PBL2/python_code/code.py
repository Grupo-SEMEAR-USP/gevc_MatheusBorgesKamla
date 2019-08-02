import numpy as np
import cv2
import manipulations as mp
import operator



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
    side_ant = 0
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

            pont_ini, hist_img = mp.histogram_Line(bin_img)
            cv2.namedWindow("Histogram",cv2.WINDOW_NORMAL)
            cv2.imshow("Histogram",hist_img)

            sliding_img, vec_nav = mp.sliding_Window(bird_img, bin_img, pont_ini, 20)
            #Ordenando meu vetor com os pontos calculado dos retangulos centrais
            vec_nav.sort(key = operator.itemgetter(1),reverse = True)
            
            cv2.namedWindow("Sliding Window",cv2.WINDOW_NORMAL)
            cv2.imshow("Sliding Window", sliding_img)

            vec_nav_final = mp.inverse_bird_Eyes(frame,vec_nav)
            if vec_nav_final is not None:
                #Passando para 2 dimensoes meu vetor de navegacao
                vec_nav_final = vec_nav_final[0,:,:]
            
            result_img, shift, side = mp.shift_Central(frame,vec_nav_final)
            #Se tiver perdido o pontilhado passho shift igual a 0 e assim pego o side_anterior
            if side == 0:
                side = side_ant
            print "---------------\nDeslocar: ",shift,"\nPosicao: ",side, "\n"
            side_ant = side
            cv2.namedWindow("Result Image",cv2.WINDOW_NORMAL)
            cv2.imshow("Result Image", result_img)
        #Se nao eu printo o erro e para reproducao do video
        else:
            print "Erro ao ler frame !"
            break
    
    cap.release()
    cv2.destroyAllWindows()

main()
        