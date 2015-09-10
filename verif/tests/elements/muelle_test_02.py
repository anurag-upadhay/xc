# -*- coding: utf-8 -*-
# home made test
K= 1000.0 # Constante del muelle
l= 100.0 # Distancia entre nodos
F= 1.0 # Magnitud de la fuerza
D= F/K # Magnitud del desplazamiento

import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from materials import typical_materials



# Model definition
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
nodos= mdlr.getNodeLoader
# Problem type
predefined_spaces.gdls_elasticidad2D(nodos)

nodos.defaultTag= 1 #First node number.
nod= nodos.newNodeXY(0,0)
nod= nodos.newNodeXY(l,0.0)

# Materials definition
elast= typical_materials.defElasticMaterial(mdlr,"elast",K)
    
''' Se definen nodos en los puntos de aplicación de
    la carga. Puesto que no se van a determinar tensiones
    se emplea una sección arbitraria de área unidad '''
    
# Elements definition
elementos= mdlr.getElementLoader
elementos.defaultMaterial= "elast"
elementos.dimElem= 2
elementos.defaultTag= 1 #First node number.
muelle= elementos.newElement("muelle",xc.ID([1,2]));
    
# Constraints
coacciones= mdlr.getConstraintLoader
#
spc= coacciones.newSPConstraint(1,0,0.0) # Nodo 1
spc= coacciones.newSPConstraint(1,1,0.0)
spc= coacciones.newSPConstraint(2,1,0.0) # Nodo 2

# Loads definition
cargas= mdlr.getLoadLoader
casos= cargas.getLoadPatterns
#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"
#Load case definition
lp0= casos.newLoadPattern("default","0")
#casos.currentLoadPattern= "0"
lp0.newSPConstraint(2,0,D)

#We add the load case to domain.
casos.addToDomain("0")
# Solution
analisis= predefined_solutions.simple_static_linear(prueba)
result= analisis.analyze(1)



nodos.calculateNodalReactions(True)
nodos= mdlr.getNodeLoader
nod2= nodos.getNode(2)
deltax= nod2.getDisp[0] 
deltay= nod2.getDisp[1] 
nod1= nodos.getNode(1)
R= nod1.getReaction[0] 

elementos= mdlr.getElementLoader
elem1= elementos.getElement(1)
elem1.getResistingForce()
Ax= elem1.getMaterial().getStrain() # Alargamiento del muelle

ratio1= (F+R)/F
ratio2= (F-(K*deltax))/F
ratio3= (Ax-deltax)/deltax

''' 
print "R= ",R
print "dx= ",deltax
print "dy= ",deltay
print "Ax= ",Ax
print "ratio1= ",(ratio1)
print "ratio2= ",(ratio2)
print "ratio3= ",(ratio3)
   '''
import os
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-10) & (abs(ratio2)<1e-10) & (abs(ratio3)<1e-10):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."