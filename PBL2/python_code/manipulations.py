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
    for col in range(int(0.1*src_cols),int(0.9*src_cols)):
        #Para cada coluna vou da metade de linhas da imagem ate o final linha por linha
        '''for row in range(int(src_rows*0.8),src_rows):
            #Cada pixel checo se e diferente de 0 (se e diferente de preto)
            if src[col][row] != 0:
                #Se for, contabilizo no meu vetor contador
                count[col] += 1'''
        #Somo os valores de cada pixel
        count[col] = np.sum(src[int(src_rows*0.5):src_rows,col])/255
        #Desenho os circulos vermelhos do histograma, quanto maior a altura do circulo mais pixels foram encontrados naquela coluna
        cv2.circle(hist_img,(col,src_rows-1-int(count[col])),1,(0,0,255),3,8,0)
    #Vou pegar a posicao da coluna que possui o maior numero de pixels achados
    best_col = np.argmax(count)
    #Numero de colunas vizinhas a best_col que irei percorrer calculando uma media ponderada do pixels na coluna
    neighbor = 30
    #Variaveis utilizadas para realizar media ponderada
    num = 0
    dem = 0
    #Percorro os pixels vizinhos a best_col
    for i in range(best_col-neighbor,best_col+neighbor):
        #Somatorio do numero de pixels na coluna vezes seu indice
        num += i*count[i]
        #Somatorio do numero de pixels na coluna
        dem += count[i]
    #Se nao possui pixels achados definirei o ponto inicial como -1
    if num == 0 and dem == 0:
        num = -1
        dem = 1
    #Realizo a media ponderada
    ini_col = int(num/dem)
    #Gero meu ponto novo
    center_ini = (ini_col,src_rows)

    return center_ini, hist_img

def sliding_Window(src, bin_src, center_ini, num_rectangle):
    vec_navegation = [] #vetor que guardarei os pontos centrais dos retangulos calculados
    #Imagem que desenharei os retangulos
    sliding_img = src
    #Lado e alturas que utilizarei para meus retangulos
    l_rectangle = 110
    h_rectangle = 20
    #Definindo os pontos para meu retangulo incial (tem que ser diagonalmente opostos)
    pont_ini1 = (center_ini[0]-l_rectangle/2,center_ini[1]+h_rectangle/2)
    pont_ini2 = (center_ini[0]+l_rectangle/2,center_ini[1]-h_rectangle/2)
    #Desenhando retangulo inicial a partir do ponto inicial
    cv2.rectangle(sliding_img,pont_ini1,pont_ini2,(255,255,0),2,8,0)
    #Vetor que sera utilizado para contabilizar os pixels brancos dentre de cada quadrado
    count = np.zeros(l_rectangle)
    #Variavel que sempre ira guardar o centro do retangulo anterior, inicialmente sendo o centro inicial vindo do histograma
    previus_center = center_ini
    #Variavel que contabilizara descontinuidades
    desc_cont = 0
    #Guarda o ponto antes de haver uma descontinuidade
    previus_center_desc = (0,0)
    #Loop em que calculo os quadrados
    for rec in range(0,num_rectangle):
        #Para calcular o novo centro de cada quadrado, matenho sua coordenada x e so subo a sua y
        new_center = (previus_center[0],previus_center[1]-h_rectangle)
        #Gerando meus pontos diagonalmente opostos para os novos retangulos
        p1 = (new_center[0]-l_rectangle/2,new_center[1]-h_rectangle/2)
        p2 = (new_center[0]+l_rectangle/2,new_center[1]+h_rectangle/2)
        #Percorro a area do novo quadrado contabilizando o numero de pixels brancos por coluna
        count_num = 0 #Somatorio do numero de pixels achados na coluna vezes seu indice
        count_den = 0 #Somatorio numero de pixels na coluna
        for col in range(p1[0],p1[0]+l_rectangle):
            count[col-p1[0]] = np.sum(bin_src[p1[1]:p2[1],col])/255
            count_num += col*count[col-p1[0]]
            count_den += count[col-p1[0]]
        #Se o somatorio de numeros de pixels achados no quadrado foi maior que uma certa porcentagem de sua area
        if count_den > (0.05*l_rectangle*h_rectangle):
            #Atualizo a coluna (coord. x) do meu novo centro fazendo uma media ponderada 
            new_center = (int(count_num/count_den),previus_center[1]-h_rectangle)
            #Atualizo p1 e p2 para desenhar o quadrado
            p1 = (new_center[0]-l_rectangle/2,new_center[1]-h_rectangle/2)
            p2 = (new_center[0]+l_rectangle/2,new_center[1]+h_rectangle/2)
            #Guardo o centro do retangulo calculado
            vec_navegation.append(new_center)
            cv2.rectangle(sliding_img,p1,p2,(0,100,255),2,8,0)

            #Tratamento da descontinuidade (checo se o contador de descontinuidade e maior que zero)
            if desc_cont > 0 :
                #print "previus_center = ", previus_center_desc, " new_center = ", new_center
                #Calulo a distancia entre as colunas do ponto antes da descontinuidade e depois da continuidade
                dist_x = new_center[0]-previus_center[0]
                #Divido a distancia pelo o numero de quadrados presentes na descontinuidade
                div = dist_x/desc_cont
                #print "dist_x = ",dist_x," desc_cont = ",desc_cont," div = ",div
                #Calculo as coordenadas y (altura) para os pontos/quadrados da minha descontinuidade guardando no vetor y_desc
                y_desc = np.zeros(desc_cont)
                for i in range(1,desc_cont+1):
                    y_desc[i-1] = previus_center_desc[1] - i*h_rectangle
                #Se div == 0 quer dizer que a distancia da coluna entre meu ponto antes da continuidade e depois (dist_x) e menor que o numero de pontos/quadrados naquela descontinuidade
                if div == 0 :
                    #Cont guarda o tanto de pontos que ja foi calculado da descontinuidade     
                    cont = 0
                    #Nesse caso eu vou deslocando de 1 em 1 minha coordenada x(colunas) do ponto antes da descontinuidade 
                    #ate chegar na coordenada x do ponto apos a descontinuidade
                    for l in range(1,abs(dist_x)+1):
                        if l<desc_cont:
                            p1_desc = (int(previus_center_desc[0]+ l -l_rectangle/2),int(y_desc[l-1]-h_rectangle/2))
                            p2_desc = (int(previus_center_desc[0]+ l +l_rectangle/2),int(y_desc[l-1]+h_rectangle/2))
                            cv2.rectangle(sliding_img,p1_desc,p2_desc,(255,255,255),2,8,0)
                            vec_navegation.append((previus_center_desc[0]+ l,int(y_desc[l-1])))
                            cont+=1
                    #Caso o numero de quadrados na descontinuidade ainda nao tenha sido completado (cont guarda quanto ja foram calculados)
                    #o resto que faltar eu determino a coordenada x sendo do ponto apos a descontinuidade mesmo
                    for i in range(cont+1,desc_cont+1):
                        p1_desc = (int(new_center[0] -l_rectangle/2),int(y_desc[i-1]-h_rectangle/2))
                        p2_desc = (int(new_center[0] +l_rectangle/2),int(y_desc[i-1]+h_rectangle/2))
                        vec_navegation.append((new_center[0],int(y_desc[i-1])))
                        cv2.rectangle(sliding_img,p1_desc,p2_desc,(255,255,255),2,8,0)
                else:
                    #Caso div>0 ou div<0 utilizo a funcao np.arange em que irei gerar um vetor que gerara as colunas que vao variando da coluna anterior
                    #a descontinuidade ate depois com um acrescimo ou decrescimo de div (depende de ser positivo ou negativo)
                    vec_newx = np.arange(previus_center_desc[0],new_center[0],div)
                    #Contador para o numeros de pontos/quadrados calculados para essa descontinuidade
                    cont = 0
                    #Desenho os quadrados de acordo com as colunas gerados pela np.arange
                    for l in range(0,vec_newx.shape[0]):
                        if cont < desc_cont:
                            p1_desc = (int(vec_newx[l] -l_rectangle/2),int(y_desc[cont]-h_rectangle/2))
                            p2_desc = (int(vec_newx[l] +l_rectangle/2),int(y_desc[cont]+h_rectangle/2))
                            cv2.rectangle(sliding_img,p1_desc,p2_desc,(255,100,0),2,8,0)
                            vec_navegation.append((vec_newx[l],int(y_desc[cont])))
                            cont += 1
                    #Se ainda houver quadrados faltando a ser desenhado eu desenho seguindo a coluna depois da continuidade
                    for i in range(cont+1,desc_cont+1):
                            p1_desc = (int(new_center[0] -l_rectangle/2),int(y_desc[i-1]-h_rectangle/2))
                            p2_desc = (int(new_center[0] +l_rectangle/2),int(y_desc[i-1]+h_rectangle/2))
                            vec_navegation.append((new_center[0],int(y_desc[i-1])))
                            cv2.rectangle(sliding_img,p1_desc,p2_desc,(255,100,0),2,8,0)
            #O centro antigo passa a ser o que eu acabei de desenhar
            previus_center = new_center
            #Zero o numero de descontinuidade
            desc_cont = 0
        else:
            #Se nao, estarei em uma descontinuidade
            #Checo se estou acabando de entrar em uma descontinuidade
            if desc_cont == 0:
                #Se estiver guardo o ponto anterior a descontinuidade
                previus_center_desc = previus_center
            #Atualizo o ponto anterior pelo novo que foi calculado
            previus_center = new_center
            #Contabilizo o numero de pontos/quadrados na descontinuidade
            desc_cont += 1
    #print "-----------"
    return sliding_img, vec_navegation

def inverse_bird_Eyes(src, vec_nav):
    out = src
    rows = out.shape[0]
    cols = out.shape[1]

    #Definindo a regiao que pegarei da minha imagem de origem
    pts_src = np.float32([[0,rows],[0.3*cols,0.3*rows],[0.7*cols,0.3*rows],[cols,rows]])
    #Definindo para qual regiao quero gerar minha mudanca de perspectiva
    pts_dst = np.float32([[0,480],[0,0],[640,0],[640,480]])
    #Gerando minha matriz responsavel por realizar a mudanca de perspectiva
    M = cv2.getPerspectiveTransform(pts_dst,pts_src)
    #Modificacoes que devem ser feitas para que a vec_nav_final seja aceita para perspectiveTransform
    #Passo para float 32bits
    vec_nav_final = np.array(vec_nav,dtype='float32')
    #Testo se ele conseguiu calcular os pontos da slidingwindow (se nao perdeu as linhas)
    if vec_nav_final.shape[0] != 0:
        #Passo para 3 dimensoes
        vec_nav_final = vec_nav_final[None,:,:]
        #Passo meu vetor de pontos da navegacao que estava na bird's eyes perspectiva para mesma perpectiva da imagem original
        vec_nav_final = cv2.perspectiveTransform(vec_nav_final,M)
        return vec_nav_final
    else:
        #Se tiver perdido retorno None
        return None

def shift_Central(src, vec_nav):
    #Checo se existe os pontos (se nao perdeu as linhas)
    if vec_nav is not None:
        #Desenho meu poligo de pontos indicando a sua direcao
        result_img = cv2.polylines(src,np.int32([vec_nav]),0,(255,0,0),3,8,0)
        col_central_line = int(src.shape[1]/2)
        result_img = cv2.line(result_img,(col_central_line,0),(col_central_line,src.shape[0]),(0,255,0),3,8,0)
        num = 0
        dem = 0
        for i in range(0,vec_nav.shape[0]):
            num += col_central_line - vec_nav[i][0]
            dem += 1
        shif = num/dem
        current_side = 0
        if shif >= 0:
            current_side = 1
        else:
            current_side = -1
    else:
        #Se nao so retorno a imagem original
        result_img = src
        #Como perdi o pontilhado, nao sei quanto tenho que deslocar
        shif = -1
        #Passo shift como 0 pois pegarei na main o shift anterior
        current_side = 0
    return result_img, shif, current_side


