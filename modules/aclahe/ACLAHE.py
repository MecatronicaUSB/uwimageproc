
import numpy as np
import cv2
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit, minimize_scalar
from scipy.interpolate import splev, splrep
from functions import Entropia, CLAHE, graficar, NombreGrafica, DerivadaY, DerivadaX, Curvatura

def ParametrosACLAHE(imagen):

# Preprocesamiento de la imágen

# Se lee la imagen en escala de grises
	#img=cv2.imread('crowd.png',0)
	imgfilt = cv2.GaussianBlur(imagen,(3,3),0)


# Creare vector que contenga el block size y clip limit

	filas,columnas=imgfilt.shape
	bl=[2,4,8,16,32]
	cl=np.arange(0,25,0.5)


# Se realizará un ciclo para recorrer la imágen para los diversos Block Sizes y CLip Limit. Los mismos son almacenados en la siguiente matriz 

# donde la primera columna indica el tamaño del BS y seguido en la misma fila los valores del CL utilizados. Una fila despues se almacena de 

# igual manera los valores de entropía.

	resultados=np.zeros((10,51),np.float32)


# Ciclo para pasar cada uno de los bloques através de las funciones

	n=0 #Para saber donde poner el valor de los block size
	v=1 #Contar las posiciones en el eje y
	for k in bl:
		m=1 #Contar las posiciones en el eje x     
		for i in cl:
	        	resultados[n,m] = i
		imgx = CLAHE(imgfilt,k,i)
		x = Entropia(imgx)
		resultados[v,m] = x
		m = m+1
		n = n+2
		v = v+2

# Se procede a graficar Entropia vs CL para los 5 Block Size


	# Block Size 2x2
	x2,y2=graficar(resultados[0],resultados[1],'r')
	#Block Size 4x4
	x4,y4=graficar(resultados[2],resultados[3],'g')
	# Block Size 8x8
	x8,y8=graficar(resultados[4],resultados[5],'c-')
	#Block Size 16x16
	x16,y16=graficar(resultados[6],resultados[7],'m--')
	#Block Size 32x32
	x32,y32=graficar(resultados[8],resultados[9],'k-')
	#Nombre Grafica
	NombreGrafica('Clip Limit','Entropy','Entropy vs Clip Limit')


#  Calculo del Clip Limit 

	#maxima curvatura  para BS-2x2
	x22,x222,y220,y221,y222=DerivadaY(y2)
	y223,y224,y225=DerivadaX(x2,x22,x222)
	cl2=Curvatura(y220,y221,y222,y223,y224,y225)
	#maxima curvatura  para BS-4x4
	x22,x222,y220,y221,y222=DerivadaY(y4)
	y223,y224,y225=DerivadaX(x4,x22,x222)
	cl4=Curvatura(y220,y221,y222,y223,y224,y225)
	#maxima curvatura  para BS-8x8
	x22,x222,y220,y221,y222=DerivadaY(y8)
	y223,y224,y225=DerivadaX(x8,x22,x222)
	cl8=Curvatura(y220,y221,y222,y223,y224,y225)
	#maxima curvatura  para BS-16x16
	x22,x222,y220,y221,y222=DerivadaY(y16)
	y223,y224,y225=DerivadaX(x16,x22,x222)
	cl16=Curvatura(y220,y221,y222,y223,y224,y225)
	#maxima curvatura  para BS-32x32
	x22,x222,y220,y221,y222=DerivadaY(y32)
	y223,y224,y225=DerivadaX(x32,x22,x222)
	cl32=Curvatura(y220,y221,y222,y223,y224,y225)


# Una vez obtenido los puntos, buscar el que presenta mayor clip limit ya que es el que tendrá el punto de máxima curvatura

	d=cl2
	Xcl=[cl4,cl8,cl16,cl32]
	for i in Xcl:
		if d<i:
			d=i


# Búsqueda del Block Size

	# UNA VEZ FIJADO EL CLIP LIMIT, BUSCO EL BLOCK SIZE 
	resultados2=np.zeros((2,5),np.float16)
	# Ciclo para pasar cada uno de los bloques através de las funciones
	n=0 #Para saber donde poner el valor de los block size
	v=1 #Contar las posiciones en el eje y
	m=0 #Contar las posiciones en el eje x 
	for k in bl:    
		resultados2[n,m]=k
		imgx=CLAHE(imgfilt,k,d)
		x=Entropia(imgx)
		resultados2[v,m]=x
		m=m+1                
	# Se procede a graficar Entropia vs BS para el CL fijado"
	xbs=resultados2[0]
	ybs=resultados2[1]
	max_y = max(ybs)  # Find the maximum y value
	i=0
	for item in ybs:
		if item == max_y:
			w=i
		i+=1
	BS=xbs[w]

	# El Block Size es:
	BS=int(round(BS))
	# El Clip Limit es:
	CL=d

	return BS,CL
