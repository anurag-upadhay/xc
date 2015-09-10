# -*- coding: utf-8 -*-
# Test tomado del manual de Ansys
# Referencia:  Strength of Material, Part I, Elementary Theory & Problems, pg. 98, problem 4

E= 30e6 # Young modulus (psi)
l= 20*12 # Longitud de la barra en pulgadas
a= 10*12 # Longitud del voladizo
h= 30 # Canto de la viga en pulgadas.
A= 50.65 # Área de la viga en pulgadas cuadradas
I= 7892 # Inercia de la viga en pulgadas a la cuarta
w= 10000/12 # Load por unidad de longitud en libras/pulgada.

import xc_base
import geom
import xc
from model import predefined_spaces
from solution import predefined_solutions
from materials import typical_materials
from model import fix_node_6dof

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador   
nodos= mdlr.getNodeLoader
predefined_spaces.gdls_resist_materiales2D(nodos)
nodos.defaultTag= 1 #First node number.
nod= nodos.newNodeXY(0,0)
nod= nodos.newNodeXY(a,0.0)
nod= nodos.newNodeXY(a+(l/2),0.0)
nod= nodos.newNodeXY(a+l,0.0)
nod= nodos.newNodeXY(2*a+l,0.0)

# Definimos transformaciones geométricas
trfs= mdlr.getTransfCooLoader
lin= trfs.newLinearCrdTransf2d("lin")

    
# Materials definition
scc= typical_materials.defElasticSection2d(mdlr,"scc",A,E,I)


# Elements definition
elementos= mdlr.getElementLoader
elementos.defaultTransformation= "lin"
elementos.defaultMaterial= "scc"
#  sintaxis: beam2d_02[<tag>] 
elementos.defaultTag= 1 #Tag for next element.
beam2d= elementos.newElement("elastic_beam_2d",xc.ID([1,2]))
beam2d.h= h
        
beam2d= elementos.newElement("elastic_beam_2d",xc.ID([2,3]))
beam2d.h= h
        
beam2d= elementos.newElement("elastic_beam_2d",xc.ID([3,4]))
beam2d.h= h
        
beam2d= elementos.newElement("elastic_beam_2d",xc.ID([4,5]))
beam2d.h= h
    
# Constraints
coacciones= mdlr.getConstraintLoader
#
spc= coacciones.newSPConstraint(2,0,0.0) # Nodo 2
spc= coacciones.newSPConstraint(2,1,0.0)
spc= coacciones.newSPConstraint(4,1,0.0) # Nodo 4


# Loads definition
cargas= mdlr.getLoadLoader
casos= cargas.getLoadPatterns
#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"
#Load case definition
lp0= casos.newLoadPattern("default","0")
#casos.currentLoadPattern= "0"
eleLoad= lp0.newElementalLoad("beam2d_uniform_load")
eleLoad.elementTags= xc.ID([1,4])
eleLoad.transComponent= -w

#We add the load case to domain.
casos.addToDomain("0")

# Procedimiento de solución
import os
pth= os.path.dirname(__file__)
#print "pth= ", pth
if(not pth):
  pth= "."
execfile(pth+"/solu_transf_handler2.py")
    

nod3= nodos.getNode(3)
delta= nod3.getDisp[1] 

elem2= elementos.getElement(2)
elem2.getResistingForce()
sigma= elem2.getN2/A+elem2.getM2/I*h/2.0



# print delta
# print sigma
ratio1= (delta/0.182)
ratio2= (sigma/(-11400))

fname= os.path.basename(__file__)
if (abs(ratio1-1.0)<3e-3) & (abs(ratio2-1.0)<1e-3):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."
