# -*- coding: utf-8 -*-
# Test from Ansys manual
# Reference:  Strength of Material, Part I, Elementary Theory and Problems, pg. 26, problem 10

import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from materials import typical_materials
import math

__author__= "Luis C. Pérez Tato (LCPT) and Ana Ortega (AOO)"
__copyright__= "Copyright 2015, LCPT and AOO"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com"

E= 30e6 # Young modulus (psi)
l= 10 # Cable length in inches
sigmaPret= 1500 # Prestressing force (pounds)
area= 2
fPret= sigmaPret*area # Prestressing force (pounds)
F= 100 # Prestressing force (pounds)

feProblem= xc.FEProblem()
preprocessor=  feProblem.getPreprocessor
nodes= preprocessor.getNodeHandler

# Problem type
modelSpace= predefined_spaces.StructuralMechanics3D(nodes)

# Model definition
nodes.defaultTag= 1 #First node number.
nod= nodes.newNodeXYZ(0,0,0)
nod= nodes.newNodeXYZ(l/2,0.0,0)
nod= nodes.newNodeXYZ(l,0.0,0)

# Materials definition
typical_materials.defCableMaterial(preprocessor, "cable",E,sigmaPret,0.0)
    
''' We define nodes at the points where loads will be applied.
    We will not compute stresses so we can use an arbitrary
    cross section of unit area.'''
    
# Elements definition
elements= preprocessor.getElementHandler
elements.defaultMaterial= "cable"
elements.dimElem= 3 # Dimension of element space
truss1= elements.newElement("CorotTruss",xc.ID([1,2]))
truss1.area= area
truss2= elements.newElement("CorotTruss",xc.ID([2,3]))
truss2.area= area
     
# Constraints
constraints= preprocessor.getBoundaryCondHandler
modelSpace.fixNode000_000(1)
modelSpace.fixNodeFFF_000(2)
modelSpace.fixNode000_000(3)

# Loads definition
loadHandler= preprocessor.getLoadHandler
lPatterns= loadHandler.getLoadPatterns
#Load modulation.
ts= lPatterns.newTimeSeries("constant_ts","ts")
lPatterns.currentTimeSeries= "ts"
lPattern= "0"
lp0= lPatterns.newLoadPattern("default",lPattern)
#lPatterns.currentLoadPattern= lPattern
lp0.newNodalLoad(2,xc.Vector([0,-F,0,0,0,0]))
lPatterns.addToDomain(lPattern) # Append load pattern to domain.

# Solution procedure
analisis= predefined_solutions.simple_newton_raphson(feProblem)
result= analisis.analyze(10)


nodes.calculateNodalReactions(True,1e-7)
nod3= nodes.getNode(3)
R1X= nod3.getReaction[0]
R1Y= nod3.getReaction[1] 
nod1= nodes.getNode(1)
R2X= nod1.getReaction[0]
R2Y= nod1.getReaction[1] 
nod2= nodes.getNode(2)
deltaX= nod2.getDisp[0]
deltaY= nod2.getDisp[1]  

ele1= elements.getElement(1)
tension= ele1.getN()


alpha= -math.atan2(deltaY,l/2)
tensTeor= F/(2*math.sin(alpha))
ratio1= (abs(R1X+R2X)/fPret)
ratio2= (abs(tension-tensTeor)/tensTeor)
ratio3= (abs(R1Y-F/2.0)/(F/2))
ratio4= (abs(R2Y-F/2.0)/(F/2))
    
''' 
print "alpha= ",rad2deg((alpha))
print "R1X= ",R1X
print "R1Y= ",R1Y
print "R2X= ",R2X
print "R2Y= ",R2Y
print "deltaX= ",deltaX
print "deltaY= ",deltaY
print "tensTeor= ",(tensTeor)
print "tension= ",(tension)
print "ratio1= ",(ratio1)
print "ratio2= ",(ratio2)
print "ratio3= ",(ratio3)
print "ratio4= ",(ratio4)
   '''
    
import os
from miscUtils import LogMessages as lmsg
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-11) & (abs(ratio2)<1e-11) & (abs(ratio3)<1e-11) & (abs(ratio4)<1e-11):
  print "test ",fname,": ok."
else:
  lmsg.error(fname+' ERROR.')
