# -*- coding: utf-8 -*-
'''Verification test taken from example 2-005 of 
   the SAP 2000 verification manual.'''


__author__= "Luis C. Pérez Tato (LCPT) and Ana Ortega (AOO)"
__copyright__= "Copyright 2015, LCPT and AOO"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com"

# feProblem.setVerbosityLevel(0)
NumDivI= 8
NumDivJ= 8
CooMaxX= 2
CooMaxY= 2
E= 17472000 # Elastic modulus en lb/in2
nu= 0.3 # Poisson's ratio
G= 6720000
thickness= 0.0001 # Cross section depth expressed in inches.
unifLoad= 0.0001 # Uniform load in lb/in2.
ptLoad= 0.0004 # Punctual load in lb.

import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from materials import typical_materials

feProblem= xc.FEProblem()
preprocessor=  feProblem.getPreprocessor
nodes= preprocessor.getNodeHandler

# Problem type
modelSpace= predefined_spaces.StructuralMechanics3D(nodes)
# Define materials
elast= typical_materials.defElasticMaterial(preprocessor, "elast",E)

nodes.newSeedNode()

# Define materials
nmb1= typical_materials.defElasticMembranePlateSection(preprocessor, "memb1",E,nu,0.0,thickness)



seedElemHandler= preprocessor.getElementHandler.seedElemHandler
seedElemHandler.defaultMaterial= "memb1"
seedElemHandler.defaultTag= 1
elem= seedElemHandler.newElement("ShellMITC4",xc.ID([0,0,0,0]))



points= preprocessor.getMultiBlockTopology.getPoints
pt1= points.newPntFromPos3d(geom.Pos3d(0.0,0.0,0.0))
pt2= points.newPntFromPos3d(geom.Pos3d(CooMaxX,0.0,0.0))
pt3= points.newPntFromPos3d(geom.Pos3d(CooMaxX,CooMaxY,0.0))
pt4= points.newPntFromPos3d(geom.Pos3d(0.0,CooMaxY,0.0))
surfaces= preprocessor.getMultiBlockTopology.getSurfaces
surfaces.defaultTag= 1
s= surfaces.newQuadSurfacePts(pt1.tag,pt2.tag,pt3.tag,pt4.tag)
s.nDivI= NumDivI
s.nDivJ= NumDivJ


f1= preprocessor.getSets.getSet("f1")
f1.genMesh(xc.meshDir.I)

#Constraints
sides= s.getEdges
#Edge iterator
for l in sides:
  for i in l.getEdge.getNodeTags():
    modelSpace.fixNode000_FFF(i)

# Loads definition
loadHandler= preprocessor.getLoadHandler
lPatterns= loadHandler.getLoadPatterns

#Load modulation.
ts= lPatterns.newTimeSeries("constant_ts","ts")
lPatterns.currentTimeSeries= "ts"
#Load case definition
lp0= lPatterns.newLoadPattern("default","0")
#lPatterns.currentLoadPattern= "0"


f1= preprocessor.getSets.getSet("f1")
nNodes= s.getNumNodes

node= s.getNodeIJK(1,NumDivI/2+1,NumDivJ/2+1)
# print "Central node: ", node.tag
# print "Central node coordinates: ", node.getCoo
lp0.newNodalLoad(node.tag,xc.Vector([0,0,-ptLoad,0,0,0])) # Concentrated load


nElems= s.getNumElements
#We add the load case to domain.
lPatterns.addToDomain("0")


# Solution procedure
analisis= predefined_solutions.simple_static_linear(feProblem)
analOk= analisis.analyze(1)

node= s.getNodeIJK(1,NumDivI/2+1,NumDivJ/2+1)
# print "Central node: ", node.tag
# print "Central node coordinates: ", node.getCoo
# print "Central node displacements: ", node.getDisp
UZ= node.getDisp[2]


UZTeor= -11.6
ratio1= (abs((UZ-UZTeor)/UZTeor))
ratio2= (abs((nElems-64)/64))

''' 
print "UZ= ",UZ
print "Number of nodes: ",nNodes
print "Number of elements: ",nElems
print "ratio1: ",ratio1
   '''

import os
from miscUtils import LogMessages as lmsg
fname= os.path.basename(__file__)
if (abs(ratio1)<6e-3) & (abs(ratio2)<1e-9):
  print "test ",fname,": ok."
else:
  lmsg.error(fname+' ERROR.')
