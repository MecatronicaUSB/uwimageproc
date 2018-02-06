import numpy as np
import cv2
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit, minimize_scalar
from scipy.interpolate import splev, splrep

# ### Se desarrollo una función para el cálculo de la Entropía, la cual proporciona una medida sobre la cantidad de información disponible en una imagen. Shannon (1948) definió la entropía como:
# 
# $ H( x ) = -( sum from (i=1) to (N) (p(x_i) cdot log(p(x_i)) ) ) $ 
# 
# El término log(1/p(x_i)) implica que la cantidad de información obtenida de un mensaje con probabilidad p, está inversamente relacionada con dicha probabilidad de ocurrencia del mensaje. La distribución de probabilidad de los valores se puede estimar contando el número de veces que aparece cada valor de gris en la imagen y dividiéndolo por la cantidad total de píxeles. Por lo anterior, se utiliza el histograma para el cálculo de la entropía.


def Entropia(imagen):
	histograma = cv2.calcHist([imagen],[0],None,[256],[0,256])
	histograma = histograma.ravel()/histograma.sum()
	AproxLogs = np.log2(histograma+0.00001)
	entropia = -1 * (histograma*AproxLogs).sum()
	return entropia


# ### ACLAHE es un algoritmo basado en CLAHE

def CLAHE(imagen,parametro,CL):
	clahe = cv2.createCLAHE(clipLimit=CL, tileGridSize=(parametro,parametro))
	imgch = clahe.apply(imagen)
	return imgch


# ### Función para graficar Entropia vs CL

def graficar(Xs,Ys,color):
	x=Xs
	x=x[2:51]
	y=Ys
	y=y[2:51]
	plt.plot(x,y,color)
	return x,y

def NombreGrafica(Xnombre,Ynombre,GraficaNombre):
	plt.xlabel(Xnombre)
	plt.ylabel(Ynombre)
	plt.title(GraficaNombre)
	plt.show()


# ###  Cálculo Primera y segunda derivada para y (Entropia)

def DerivadaY(Y):
	u = np.linspace(1, 49,49)
	def f(x,p0,p1,p2,p3):
		return p0*(np.exp(-p1*x))+ p2*(np.exp(-p3*x))     
	p0 = (7, 0.4, 0.9,5) # guess perameters 
	popt, pcov = curve_fit(f, u, Y, p0)
	x22 = np.linspace(1, 25, 25)
	y22 = f(x22, *popt)
	tck = splrep(x22, y22)
	x222 = np.linspace(1, 25, 49)
	y220 = splev(x222, tck,der=1)
	y222 = pow(y220,2)
	y221 = splev(x222, tck,der=2)
	return x22,x222,y220,y221,y222


# ### Primera y segunda derivada para x (Clip Limit)

def DerivadaX(X,x22,x222):
	u = np.linspace(1, 49,49)
	def f(x,p0,p1,p2,p3):
		return p0*(np.exp(-p1*x))+ p2*(np.exp(-p3*x))     
	p0 = (7, 0.4, 0.9,5) # guess perameters
	popt, pcov = curve_fit(f, u, X, p0)
	y22 = f(x22, *popt)
	tck2 = splrep(x22, y22)
	y223 = splev(x222, tck2,der=1)
	y224 = pow(y223,2)
	y225 = splev(x222, tck2,der=2)
	return y223,y224,y225


# ### Calculos de K (Curvatura)

def Curvatura(y220,y221,y222,y223,y224,y225):
	k1=y223 * y221
	k2=y220 * y225
	k3=k1 - k2
	k4=pow(pow(k3,2),0.5)
	k6=y224+y222
	k7=pow(k6,3)
	k8=pow(k7,0.5)
	k=k4/k8
	cl = np.argmax(k)
	return cl
