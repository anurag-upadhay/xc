# -*- coding: utf-8 -*-
# Tomado del Ejemplo A47 del SOLVIA Verification Manual
from __future__ import division

import xc_base
import geom
import xc
from model import predefined_spaces
from solution import predefined_solutions
from materials import typical_materials
import math

L= 1 # Longitud de la ménsula en metros
b= 0.05 # Ancho de la sección en metros
h= 0.10 # Canto de la sección en metros
A= b*h # Área de la sección en m2
I= 1/12.0*b*h**3 # Momento de inercia en m4
theta= math.radians(30)
E=2.0E11 # Módulo elástico en N/m2
dens= 7800 # Densidad del acero en kg/m3
m= A*dens

NumDiv= 10

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
nodos= mdlr.getNodeLoader
predefined_spaces.gdls_resist_materiales2D(nodos)

# Materials definition
scc= typical_materials.defElasticSection2d(mdlr,"scc",A,E,I)


nodos.newSeedNode()

# Definimos transformaciones geométricas
trfs= mdlr.getTransfCooLoader
lin= trfs.newLinearCrdTransf2d("lin")


elementos= mdlr.getElementLoader
# Definimos elemento semilla
seedElemLoader= mdlr.getElementLoader.seedElemLoader
seedElemLoader.defaultTransformation= "lin"
seedElemLoader.defaultMaterial= "scc"
seedElemLoader.defaultTag= 1 #Tag for next element.
beam2d= seedElemLoader.newElement("elastic_beam_2d",xc.ID([0,0]))
beam2d.h= h
beam2d.rho= m


puntos= mdlr.getCad.getPoints
pt= puntos.newPntIDPos3d(1,geom.Pos3d(0.0,0.0,0.0))
pt= puntos.newPntIDPos3d(2,geom.Pos3d(L*math.cos(theta),L*math.sin(theta),0.0))
lines= mdlr.getCad.getLines
lines.defaultTag= 1
l= lines.newLine(1,2)
l.nDiv= NumDiv


# Constraints
coacciones= mdlr.getConstraintLoader

#
spc= coacciones.newSPConstraint(1,0,0.0) # Nodo 2,gdl 0
spc= coacciones.newSPConstraint(1,1,0.0) # Nodo 2,gdl 1
spc= coacciones.newSPConstraint(1,2,0.0) # Nodo 2,gdl 2


setTotal= mdlr.getSets.getSet("total")
setTotal.genMesh(xc.meshDir.I)

# Procedimiento de solución
import os
pth= os.path.dirname(__file__)
#print "pth= ", pth
if(not pth):
  pth= "."
execfile(pth+"/arpackpp_solver.py")


omega= eig1**0.5
T= 2*math.pi/omega
fcalc= 1/T


lambdA= 1.87510407
fteor= lambdA**2/(2*math.pi*L**2)*math.sqrt(E*I/m)
ratio1= (fcalc-fteor)/fteor

'''
print "fteor= ",fteor
print "eig1= ",(omega)
print "omega= ",(omega)
print "T= ",(T)
print "fcalc= ",(fcalc)
''' 

import os
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-2):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."