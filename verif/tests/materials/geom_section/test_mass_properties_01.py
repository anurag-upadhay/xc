# -*- coding: utf-8 -*-
# Home made test

import xc_base
import geom
import xc
from model import predefined_spaces
from materials import typical_materials

__author__= "Luis C. Pérez Tato (LCPT) and Ana Ortega (AOO)"
__copyright__= "Copyright 2015, LCPT and AOO"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com"

Es= 2.1e11
n= 15
Ec= Es/n
sideLength= 1.0

feProblem= xc.FEProblem()
preprocessor=  feProblem.getPreprocessor

concrete= typical_materials.defElasticMaterial(preprocessor, 'concrete',Ec)

sectionGeometryTest= preprocessor.getMaterialHandler.newSectionGeometry("sectionGeometryTest")
regions= sectionGeometryTest.getRegions
concrete= regions.newQuadRegion('concrete')
concrete.pMin= geom.Pos2d(0.0,0.0)
concrete.pMax= geom.Pos2d(1.0,1.0)

area= sectionGeometryTest.getAreaHomogenizedSection(Ec)
G= sectionGeometryTest.getCenterOfMassHomogenizedSection(Ec)
Iy= sectionGeometryTest.getIyHomogenizedSection(Ec)
Iz= sectionGeometryTest.getIzHomogenizedSection(Ec)
Pyz= sectionGeometryTest.getPyzHomogenizedSection(Ec)

areaTeor= sideLength*sideLength
yGTeor= sideLength/2.0
zGTeor= sideLength/2.0
iTeor= (sideLength)**4/12.0

ratio1= ((area-areaTeor)/areaTeor)
ratio2= ((G[0]-yGTeor)/yGTeor)
ratio3= ((G[1]-zGTeor)/zGTeor)
ratio4= ((Iy-iTeor)/iTeor)
ratio5= ((Iz-iTeor)/iTeor)

''' 
print "area= ",area
print "G= ",G
print "Iy= ",Iy
print "Iz= ",Iz
print "iTeor= ",iTeor
print "Pyz= ",Pyz
print "ratio1= ",ratio1
print "ratio2= ",ratio2
print "ratio3= ",ratio3
print "ratio4= ",ratio4
print "ratio5= ",ratio5
 '''

import os
from miscUtils import LogMessages as lmsg
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-15) & (abs(ratio2)<1e-15) & (abs(ratio3)<1e-15) & (abs(ratio4)<1e-15) & (abs(ratio5)<1e-15) :
  print "test_geom_caract_mec_01: ok."
else:
  print "test_geom_caract_mec_01: ERROR."
