# -*- coding: utf-8 -*-
# Tomado del example VM66 del Ansys Verification Manual Release 9.0
import xc_base
import geom
import xc

from model import predefined_spaces
from solution import predefined_solutions
from materials import typical_materials
import math

__author__= "Luis C. Pérez Tato (LCPT)"
__copyright__= "Copyright 2014, LCPT"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com"

L= 16 # Cantilever length in inches.
b= 4 # Cantilever width in inches.
h= 1 # Cantilever depth in inches.
nuMat= 0 # Poisson's ratio.
EMat= 30E6 # Young modulus en psi.
espChapa= h # Thickness en m.
area= b*espChapa # Cross section area en m2
inertia1= 1/12.0*espChapa*b**3 # Moment of inertia in m4
inertia2= 1/12.0*b*espChapa**3 # Moment of inertia in m4
dens= 0.000728 # Density of the material en lb-sec2/in4
m= b*h*dens

NumDiv= 10

# Problem type
feProblem= xc.FEProblem()
preprocessor=  feProblem.getPreprocessor
nodes= preprocessor.getNodeHandler

modelSpace= predefined_spaces.StructuralMechanics3D(nodes)
# Define materials
elast= typical_materials.defElasticMembranePlateSection(preprocessor, "elast",EMat,nuMat,espChapa*dens,espChapa)

points= preprocessor.getMultiBlockTopology.getPoints
pt1= points.newPntIDPos3d(1, geom.Pos3d(0.0,0.0,0.0) )
pt2= points.newPntIDPos3d(2, geom.Pos3d(b,0.0,0.0) )
pt3= points.newPntIDPos3d(3, geom.Pos3d(b,L,0.0) )
pt4= points.newPntIDPos3d(4, geom.Pos3d(0,L,0.0) )
surfaces= preprocessor.getMultiBlockTopology.getSurfaces
surfaces.defaultTag= 1
s= surfaces.newQuadSurfacePts(1,2,3,4)
s.nDivI= 4
s.nDivJ= NumDiv


nodes.newSeedNode()

seedElemHandler= preprocessor.getElementHandler.seedElemHandler
seedElemHandler.defaultMaterial= "elast"
elem= seedElemHandler.newElement("ShellMITC4",xc.ID([0,0,0,0]))


f1= preprocessor.getSets.getSet("f1")
f1.genMesh(xc.meshDir.I)
# Constraints


ln= preprocessor.getMultiBlockTopology.getLineWithEndPoints(pt1.tag,pt2.tag)
lNodes= ln.getNodes()
for n in lNodes:
  n.fix(xc.ID([0,1,2,3,4,5]),xc.Vector([0,0,0,0,0,0])) # UX,UY,UZ,RX,RY,RZ


# Solution procedure
solu= feProblem.getSoluProc
solCtrl= solu.getSoluControl


solModels= solCtrl.getModelWrapperContainer
sm= solModels.newModelWrapper("sm")


cHandler= sm.newConstraintHandler("transformation_constraint_handler")

numberer= sm.newNumberer("default_numberer")
numberer.useAlgorithm("rcm")

analysisAggregations= solCtrl.getAnalysisAggregationContainer
analysisAggregation= analysisAggregations.newAnalysisAggregation("analysisAggregation","sm")
solAlgo= analysisAggregation.newSolutionAlgorithm("frequency_soln_algo")
integ= analysisAggregation.newIntegrator("eigen_integrator",xc.Vector([1.0,1,1.0,1.0]))

#soe= analysisAggregation.newSystemOfEqn("sym_band_eigen_soe")
#solver= soe.newSolver("sym_band_eigen_solver")
soe= analysisAggregation.newSystemOfEqn("band_arpackpp_soe")
soe.shift= 0.0
solver= soe.newSolver("band_arpackpp_solver")
solver.tol= 1e-3
solver.maxNumIter= 5

analysis= solu.newAnalysis("eigen_analysis","analysisAggregation","")


analOk= analysis.analyze(2)
eig1= analysis.getEigenvalue(1)





omega1= math.sqrt(eig1)
T1= 2*math.pi/omega1
f1calc= 1.0/T1



lambdA= 1.87510407
f1teor= lambdA**2/(2*math.pi*L**2)*math.sqrt(EMat*inertia2/m)
ratio1= abs(f1calc-f1teor)/f1teor

''' 
print "omega1= ",omega1
print "T1= ",T1
print "f1calc= ",f1calc
print "f1teor= ",f1teor
print "ratio1= ",ratio1
   '''

import os
from miscUtils import LogMessages as lmsg
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-3):
  print "test ",fname,": ok."
else:
  lmsg.error(fname+' ERROR.')
