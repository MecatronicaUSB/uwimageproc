# #  Algoritmo ACLAHE 
# 
# ## Reconstrucción del algoritmo ACLAHE desarrollado en el grupo de Mecatrónica de la Universidad Simón Bolívar basado en el paper: Un nuevo método para determinar los parámetros del CLAHE basados en la entropía de la imágen (Byong Seok Min, Dong Kyun Lim, Seung Jong Kim and Joo Heung Lee, 2013-7-5)
# 

# Las Siguientes librerías son la sutilizadas y su respectiva documentación
# * NumPy .... https://docs.scipy.org/doc/numpy-dev/dev/
# * Matplotlib ... http://matplotlib.org/1.4.2/
# * SciPy ... http://scipy.github.io/devdocs/hacking.html

import numpy as np
import cv2
import matplotlib.pyplot as plt
from ACLAHE import ParametrosACLAHE


img = cv2.imread('crowd.png',0)
BS , CL = ParametrosACLAHE(img)
clahe = cv2.createCLAHE(clipLimit=CL, tileGridSize=(BS,BS))
cl1 = clahe.apply(img)

cv2.imwrite('clahe_2.jpg',cl1)
