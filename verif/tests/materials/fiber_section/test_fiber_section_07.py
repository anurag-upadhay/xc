# -*- coding: utf-8 -*-
''' Reinforced concrete section verification test empleando
   el material fiber_section_GJ. '''


import math
import xc_base
import geom
import xc
from misc import banco_pruebas_scc3d

__author__= "Luis C. Pérez Tato (LCPT) and Ana Ortega (A_OO)"
__copyright__= "Copyright 2015, LCPT and AO_O"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com ana.ortega.ort@gmal.com"

areaFi16= 2.01e-4 # Rebar area expressed in square meters.
brazo= 0.5 # Rebar lever arm with respect to section axis.


from materials.ehe import EHE_materials
from model import predefined_spaces
from solution import predefined_solutions

# Coeficientes de seguridad.
gammas= 1.15 # Partial safety factor for steel.



MzDato= 15e3
NDato= 0.0

prueba= xc.ProblemaEF()
preprocessor=  prueba.getPreprocessor
# Materials definition
tag= EHE_materials.B500S.defDiagD(preprocessor)
import os
pth= os.path.dirname(__file__)
if(not pth):
  pth= "."
#print "pth= ", pth
execfile(pth+"/geomSeccBarras.py")
secBarras= preprocessor.getMaterialLoader.newMaterial("fiber_section_GJ","secBarras")
fiberSectionRepr= secBarras.getFiberSectionRepr()
fiberSectionRepr.setGeomNamed("geomSecBarras")
secBarras.setupFibers()

banco_pruebas_scc3d.sectionModel(preprocessor, "secBarras")

# Constraints
modelSpace= predefined_spaces.getStructuralMechanics3DSpace(preprocessor)
modelSpace.fixNode000_000(1)
modelSpace.fixNodeF00_00F(2)

# Loads definition
cargas= preprocessor.getLoadLoader

casos= cargas.getLoadPatterns

#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"
#Load case definition
lp0= casos.newLoadPattern("default","0")
lp0.newNodalLoad(2,xc.Vector([0,0,0,0,0,MzDato]))

#We add the load case to domain.
casos.addToDomain("0")


# Solution procedure
analisis= predefined_solutions.simple_newton_raphson(prueba)
analOk= analisis.analyze(1)


nodes= preprocessor.getNodeLoader
nodes.calculateNodalReactions(True)
nodes= preprocessor.getNodeLoader

RN= nodes.getNode(1).getReaction[0] 
RM= nodes.getNode(1).getReaction[5] 
RN2= nodes.getNode(2).getReaction[0] 

elements= preprocessor.getElementLoader
ele1= elements.getElement(1)
scc= ele1.getSection()
esfN= scc.getFibers().getResultant()
esfMy= scc.getFibers().getMy(0.0)
esfMz= scc.getFibers().getMz(0.0)
defMz= scc.getSectionDeformationByName("defMz")
defN= scc.getSectionDeformationByName("defN")

ratio1= (RM+MzDato)/MzDato
ratio2= (esfMz-MzDato)/MzDato
ratio3= (esfN-NDato)

''' 
print "RN= ",(RN/1e3)," kN \n"
print "RN2= ",(RN2/1e3)," kN \n"
print "N= ",(esfN/1e3)," kN \n"
print "My= ",(esfMy/1e3)," kN m\n"
print "Mz= ",(esfMz/1e3)," kN m\n"
print "defMz= ",(defMz)
print "defN= ",(defN)
print "ratio1= ",(ratio1)
print "ratio2= ",(ratio2)
print "ratio3= ",(ratio3)
  '''

from miscUtils import LogMessages as lmsg
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-10) & (abs(ratio2)<1e-10) & (abs(ratio3)<1e-10) & (abs(RN)<1e-10) & (abs(RN2)<1e-10) & (abs(esfMy)<1e-10) :
  print "test ",fname,": ok."
else:
  lmsg.error(fname+' ERROR.')
