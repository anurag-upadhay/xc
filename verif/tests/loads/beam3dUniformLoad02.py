# -*- coding: utf-8 -*-
# home made test

import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from materials import typical_materials
from materials.sections import section_properties
from materials.sections import defSeccAggregation

__author__= "Luis C. Pérez Tato (LCPT) and Ana Ortega (AOO)"
__copyright__= "Copyright 2015, LCPT and AOO"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com"

L= 2 # Bar length.
P= 10e3 # Carga uniforme transversal.
n= 1e6 # Carga uniforme axial.

feProblem= xc.FEProblem()
preprocessor=  feProblem.getPreprocessor   
nodes= preprocessor.getNodeHandler

seccTest= section_properties.RectangularSection("test",b=.20,h=.30) # Section geometry.
matSeccTest= typical_materials.MaterialData("mattest",E=7E9,nu=0.3,rho=2500) # Section material.


# Problem type
modelSpace= predefined_spaces.StructuralMechanics3D(nodes)
# Definimos el material
defSeccAggregation.defSeccAggregation3d(preprocessor, seccTest,matSeccTest)
nodes.defaultTag= 1 #First node number.
nod= nodes.newNodeXYZ(0,0,0)
nod= nodes.newNodeXYZ(L,0,0)

# Geometric transformations
lin= modelSpace.newLinearCrdTransf("lin",xc.Vector([0,0,1]))    
# Elements definition
elements= preprocessor.getElementHandler
elements.defaultTransformation= "lin"
elements.defaultMaterial= seccTest.sectionName
elements.numSections= 3 # Number of sections along the element.
elements.defaultTag= 1
beam3d= elements.newElement("ForceBeamColumn3d",xc.ID([1,2]))
    
# Constraints
modelSpace.fixNode000_000(1)


# Loads definition
loadHandler= preprocessor.getLoadHandler
lPatterns= loadHandler.getLoadPatterns
ts= lPatterns.newTimeSeries("constant_ts","ts")
lPatterns.currentTimeSeries= "ts"
#Load case definition
lp0= lPatterns.newLoadPattern("default","0")
#lPatterns.currentLoadPattern= "0"
eleLoad= lp0.newElementalLoad("beam3d_uniform_load")
eleLoad.elementTags= xc.ID([1]) 
eleLoad.transComponent= -P 
eleLoad.axialComponent= n
#We add the load case to domain.
lPatterns.addToDomain("0")



# Solution procedure
analisis= predefined_solutions.simple_static_modified_newton(feProblem)
result= analisis.analyze(1)

nodes.calculateNodalReactions(True,1e-7)
nod2= nodes.getNode(2)
delta0= nod2.getDisp[0] 
delta1= nod2.getDisp[1] 
nod1= nodes.getNode(1)
RN= nod1.getReaction[0] 
RN2= nod2.getReaction[0] 


elem1= elements.getElement(1)
elem1.getResistingForce()
scc0= elem1.getSections()[0]

N0= scc0.getStressResultantComponent("N")

F= (n*L)
delta0Teor= (n*L**2/2.0/matSeccTest.E/seccTest.A())
ratio0= ((delta0-delta0Teor)/delta0Teor)
Q= P*L
delta1Teor= (-Q*L**3/8/matSeccTest.E/seccTest.Iz())
ratio1= ((delta1-delta1Teor)/delta1Teor)
ratio2= (abs((N0-F)/F))
ratio3= (abs((RN+F)/F))


''' 
print "delta0= ",delta0
print "delta0Teor= ",delta0Teor
print "ratio0= ",ratio0
print "delta1= ",delta1
print "delta1Teor= ",delta1Teor
print "ratio1= ",ratio1
print "F= ",F
print "N0= ",N0
print "ratio2= ",ratio2
print "RN= ",RN
print "RN2= ",RN2
print "ratio3= ",ratio3
 '''

import os
from miscUtils import LogMessages as lmsg
fname= os.path.basename(__file__)
if (abs(ratio0)<1e-4) & (abs(ratio1)<0.03) & (abs(ratio2)<1e-14) & (abs(ratio3)<1e-14):
  print "test ",fname,": ok."
else:
  lmsg.error(fname+' ERROR.')
