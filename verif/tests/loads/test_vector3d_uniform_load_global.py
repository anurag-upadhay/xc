# -*- coding: utf-8 -*-

import numpy

movN2= movs_nodo_6gdl.vectorMovUVW(mdlr,2) # Desplazamiento del nodo 2
vI= element_vectors.getVectorIElem(mdlr,1) # Vector I del elemento 1
vProj= movN2.dot(vI)
delta= abs(vProj)

nodos.calculateNodalReactions(True)
reacN1= nodalReactions.vectorReacUVW(mdlr,1) # Reacción del nodo 1
vReacProj= reacN1.dot(vI)
RN= abs(vReacProj)

elem1= elementos.getElement(1)
elem1.getResistingForce()
scc0= elem1.getSections()[0]
N0= scc0.getStressResultantComponent("N")

F= (f*L)
deltateor= (f*L**2/(2*E*A))
ratio1= (abs((delta-deltateor)/deltateor))
ratio2= (abs((N0-F)/F))
ratio3= (abs((RN-F)/F))
