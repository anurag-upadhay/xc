# -*- coding: utf-8 -*-
# Verification test of the addFiber function


import xc_base
import geom
import xc
from model import predefined_spaces
from solution import predefined_solutions
from materials import typical_materials

__author__= "Luis C. Pérez Tato (LCPT) and Ana Ortega (A_OO)"
__copyright__= "Copyright 2015, LCPT and AO_O"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com ana.ortega.ort@gmal.com"

yF= 2.0
zF= 3.0
F= 2100
MomY= F*zF
MomZ= F*yF
MomYFibersDq= 0.0
MomZFibersDq= 0.0
Es= 2.1e6
Area= 1.0
eps= F/(Area*Es)

# Problem type
feProblem= xc.FEProblem()
feProblem.logFileName= "/tmp/erase.log" # Ignore warning messages
preprocessor=  feProblem.getPreprocessor
nodes= preprocessor.getNodeHandler
modelSpace= predefined_spaces.StructuralMechanics3D(nodes)

nodes.defaultTag= 1 #First node number.
nod= nodes.newNodeXYZ(1,0,0)
nod= nodes.newNodeXYZ(1,0,0)


# Define materials
A= 0.0
yG= 0.0
zG= 0.0

IEA= 0.0; IK12= 0.0; IK13= 0.0
IK21= 0.0; IEIy= 0.0; IK23= 0.0
IK31= 0.0; IK32= 0.0; IEIz= 0.0

TEA= 0.0; TK12= 0.0; TK13= 0.0
TK21= 0.0; TEIy= 0.0; TK23= 0.0
TK31= 0.0; TK32= 0.0; TEIz= 0.0

elast0= typical_materials.defElasticMaterial(preprocessor, "elast0",Es)
# Sections
fiberSectionTest= preprocessor.getMaterialHandler.newMaterial("fiber_section_3d","fiberSectionTest")
fiberSectionTest.addFiber("elast0",Area,xc.Vector([yF,zF]))

A= fiberSectionTest.getArea
yG= fiberSectionTest.getCenterOfMassY()
zG= fiberSectionTest.getCenterOfMassZ()
initialTangentStiffness= fiberSectionTest.getInitialTangentStiffness()
IEA= initialTangentStiffness.at(1,1)
IK12= initialTangentStiffness.at(1,2)
IK13= initialTangentStiffness.at(1,3)
IK21= initialTangentStiffness.at(2,1)
IEIy= initialTangentStiffness.at(2,2)
IK23= initialTangentStiffness.at(2,3)
IK31= initialTangentStiffness.at(3,1)
IK32= initialTangentStiffness.at(3,2)
IEIz= initialTangentStiffness.at(3,3)

fiberSectionTest.setTrialSectionDeformation(xc.Vector([eps,0.0,0.0]))
tangentStiffness= fiberSectionTest.getTangentStiffness()
TEA=  tangentStiffness.at(1,1)
TK12=  tangentStiffness.at(1,2)
TK13=  tangentStiffness.at(1,3)
TK21=  tangentStiffness.at(2,1)
TEIy=  tangentStiffness.at(2,2)
TK23=  tangentStiffness.at(2,3)
TK31=  tangentStiffness.at(3,1)
TK32=  tangentStiffness.at(3,2)
TEIz=  tangentStiffness.at(3,3)

R= fiberSectionTest.getStressResultant()
fibers= fiberSectionTest.getFibers()
MomYFibersDq= fibers.getMy(0.0)
MomZFibersDq= fibers.getMz(0.0)


ratio1= R[0]-F
ratio2= R[1]-MomZ
ratio3= R[2]-MomY
ratio4= R[1]-MomZFibersDq
ratio5= R[2]-MomYFibersDq

'''
print "F= ",F
print "MomY= ",MomY
print "MomZ= ",MomZ
print "MomYFibersDq= ",MomYFibersDq
print "MomZFibersDq= ",MomZFibersDq
print "A= ",A
print "eps= ",eps
print "yG= ",yG
print "zG= ",zG,"\n\n"
print "IEA= ",IEA}   \print{" IK12= ",IK12} \print{" IK13= ",IK13
print "IK21= ",IK21} \print{" IEIy= ",IEIy} \print{" IK23= ",IK23
print "IK31= ",IK31} \print{" IK32= ",IK32} \print{" IEIz= ",IEIz,"\n\n"

print "TEA= ",TEA}   \print{" TK12= ",TK12} \print{" TK13= ",TK13
print "TK21= ",TK21} \print{" TEIy= ",TEIy} \print{" TK23= ",TK23
print "TK31= ",TK31} \print{" TK32= ",TK32} \print{" TEIz= ",TEIz,"\n\n"

print "R= ",R

print "ratio1= ",(ratio1)
print "ratio2= ",(ratio2)
print "ratio3= ",(ratio3)
print "ratio4= ",(ratio4)
'''

import os
from miscUtils import LogMessages as lmsg
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-15) & (abs(ratio2)<1e-15) & (abs(ratio3)<1e-15) & (abs(ratio4)<1e-15) & (abs(ratio5)<1e-15) :
  print "test ",fname,": ok."
else:
  lmsg.error(fname+' ERROR.')
