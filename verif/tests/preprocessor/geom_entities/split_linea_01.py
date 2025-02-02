# -*- coding: utf-8 -*-

import xc_base
import geom
import xc
import math
import os
from solution import predefined_solutions
from model import predefined_spaces
from materials import typical_materials

__author__= "Luis C. Pérez Tato (LCPT)"
__copyright__= "Copyright 2014, LCPT"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com"

NumDiv= 25
CooMax= 10

# Problem type
feProblem= xc.FEProblem()
preprocessor=  feProblem.getPreprocessor
nodes= preprocessor.getNodeHandler
modelSpace= predefined_spaces.SolidMechanics3D(nodes)
# Define materials
elast= typical_materials.defElasticMaterial(preprocessor, "elast",3000)

nodes.newSeedNode()
seedElemHandler= preprocessor.getElementHandler.seedElemHandler
seedElemHandler.defaultMaterial= "elast"
seedElemHandler.dimElem= 3 # Dimension of element space
seedElemHandler.defaultTag= 1 #Tag for the next element.
truss= seedElemHandler.newElement("Truss",xc.ID([0,0]))
truss.area= 10.0

points= preprocessor.getMultiBlockTopology.getPoints
pt= points.newPntIDPos3d(1,geom.Pos3d(0.0,0.0,0.0))
pt= points.newPntIDPos3d(2,geom.Pos3d(CooMax,CooMax,CooMax))
pt= points.newPntIDPos3d(3,geom.Pos3d(CooMax/2,CooMax/2,CooMax/2))

lines= preprocessor.getMultiBlockTopology.getLines
lines.defaultTag= 1
l1= lines.newLine(1,2)
l1.nDiv= NumDiv

l1.splitAtPoint(points.get(3))
kpoints1= l1.getKPoints()
l2= lines.get(2)
kpoints2= l2.getKPoints()

ratio1= (kpoints1[0]-1)+(kpoints1[1]-3)
ratio2= (kpoints2[0]-3)+(kpoints2[1]-2)

''' 
print "K points linea 1: ",kpoints1
print "ratio1= ",ratio1
print "K points linea 2: ",kpoints2
print "ratio2= ",ratio2
'''

import os
from miscUtils import LogMessages as lmsg
fname= os.path.basename(__file__)
if((ratio1 < 1e-20) & (ratio2 < 1e-20)):
  print "test ",fname,": ok."
else:
  lmsg.error(fname+' ERROR.')


