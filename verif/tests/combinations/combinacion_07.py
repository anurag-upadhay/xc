# -*- coding: utf-8 -*-
# home made test
#    Prueba del funcionamiento de la base de datos
#    como almacén de combinaciones para acelerar el cálculo.

Ec= 2e5*9.81/1e-4 # Módulo de Young del hormigón en Pa.
nuC= 0.2 # Coeficiente de Poisson del hormigón EHE-08.
hLosa= 0.2 # Espesor.
densLosa= 2500*hLosa # Densidad de la losa kg/m2.
# Load
F= 5.5e4 # Magnitud de la carga en N

# Armadura activa
Ep= 190e9 # Módulo elástico expresado en MPa
Ap= 140e-6 # Área de la barra expresada en metros cuadrados
fMax= 1860e6 # Carga unitaria máxima del material expresada en MPa.
fy= 1171e6 # Tensión de límite elástico del material expresada en Pa.
tInic= 0.75**2*fMax # Pretensado final (incial al 75 por ciento y 25 por ciento de pérdidas totales).

import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from model import fix_node_6dof
from materials import typical_materials
from solution import database_helper as dbHelper

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
nodos= mdlr.getNodeLoader
predefined_spaces.gdls_resist_materiales3D(nodos)
nodos.defaultTag= 1 #First node number.
nod1= nodos.newNodeXYZ(0,0,0)
nod2= nodos.newNodeXYZ(1,0,0)
nod3= nodos.newNodeXYZ(2,0,0)
nod4= nodos.newNodeXYZ(3,0,0)
nod5= nodos.newNodeXYZ(0,1,0)
nod6= nodos.newNodeXYZ(1,1,0)
nod7= nodos.newNodeXYZ(2,1,0)
nod8= nodos.newNodeXYZ(3,1,0)
nod9= nodos.newNodeXYZ(0,2,0)
nod10= nodos.newNodeXYZ(1,2,0)
nod11= nodos.newNodeXYZ(2,2,0)
nod11= nodos.newNodeXYZ(3,2,0)


# Materials definition

hLosa= typical_materials.defElasticMembranePlateSection(mdlr,"hLosa",Ec,nuC,densLosa,hLosa)

typical_materials.defSteel02(mdlr,"aceroPret",Ep,fy,0.001,tInic)

elementos= mdlr.getElementLoader
# Losa de hormigón
elementos.defaultMaterial= "hLosa"
elementos.defaultTag= 1
elem= elementos.newElement("shell_mitc4",xc.ID([1,2,6,5]))
elem= elementos.newElement("shell_mitc4",xc.ID([2,3,7,6]))
elem= elementos.newElement("shell_mitc4",xc.ID([3,4,8,7]))
elem= elementos.newElement("shell_mitc4",xc.ID([5,6,10,9]))
elem= elementos.newElement("shell_mitc4",xc.ID([6,7,11,10]))
elem= elementos.newElement("shell_mitc4",xc.ID([7,8,12,11]))

# Armadura activa
elementos.defaultMaterial= "aceroPret"
elementos.dimElem= 3
truss= elementos.newElement("truss",xc.ID([1,2]));
truss.area= Ap
truss= elementos.newElement("truss",xc.ID([2,3]));
truss.area= Ap
truss= elementos.newElement("truss",xc.ID([3,4]));
truss.area= Ap
truss= elementos.newElement("truss",xc.ID([5,6]));
truss.area= Ap
truss= elementos.newElement("truss",xc.ID([6,7]));
truss.area= Ap
truss= elementos.newElement("truss",xc.ID([7,8]));
truss.area= Ap
truss= elementos.newElement("truss",xc.ID([9,10]));
truss.area= Ap
truss= elementos.newElement("truss",xc.ID([10,11]));
truss.area= Ap
truss= elementos.newElement("truss",xc.ID([11,12]));
truss.area= Ap

# Constraints
coacciones= mdlr.getConstraintLoader

fix_node_6dof.fixNode6DOF(coacciones,1)
fix_node_6dof.fixNode6DOF(coacciones,5)
fix_node_6dof.fixNode6DOF(coacciones,9)

# Loads definition
cargas= mdlr.getLoadLoader

casos= cargas.getLoadPatterns

#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"

lpG0= casos.newLoadPattern("default","G0")
lpG= casos.newLoadPattern("default","G")
lpSC= casos.newLoadPattern("default","SC")
lpVT= casos.newLoadPattern("default","VT")
lpNV= casos.newLoadPattern("default","NV")

#casos.currentLoadPattern= "G"
lpG.newNodalLoad(4,xc.Vector([F,0.0,0.0,0.0,0.0,0.0]))
lpG.newNodalLoad(8,xc.Vector([F,0.0,0.0,0.0,0.0,0.0]))
lpG.newNodalLoad(12,xc.Vector([F,0.0,0.0,0.0,0.0,0.0]))

#casos.currentLoadPattern= "SC"
lpSC.newNodalLoad(4,xc.Vector([F,0.0,0.0,0.0,0.0,0.0]))
lpSC.newNodalLoad(8,xc.Vector([F,0.0,0.0,0.0,0.0,0.0]))
lpSC.newNodalLoad(12,xc.Vector([F,0.0,0.0,0.0,0.0,0.0]))

#casos.currentLoadPattern= "VT"
lpVT.newNodalLoad(4,xc.Vector([F,0.0,0.0,0.0,0.0,0.0]))
lpVT.newNodalLoad(8,xc.Vector([F,0.0,0.0,0.0,0.0,0.0]))
lpVT.newNodalLoad(12,xc.Vector([F,0.0,0.0,0.0,0.0,0.0]))

#casos.currentLoadPattern= "NV"
lpNV.newNodalLoad(4,xc.Vector([F,0.0,0.0,0.0,0.0,0.0]))
lpNV.newNodalLoad(8,xc.Vector([F,0.0,0.0,0.0,0.0,0.0]))
lpNV.newNodalLoad(12,xc.Vector([F,0.0,0.0,0.0,0.0,0.0]))


# Combinaciones
combs= cargas.getLoadCombinations
comb= combs.newLoadCombination("ELU001","1.00*G")
comb= combs.newLoadCombination("ELU002","1.35*G")
comb= combs.newLoadCombination("ELU003","1.00*G + 1.50*SC")
comb= combs.newLoadCombination("ELU004","1.00*G + 1.50*SC + 0.90*NV")
comb= combs.newLoadCombination("ELU005","1.00*G + 1.50*SC + 0.90*VT")
comb= combs.newLoadCombination("ELU006","1.00*G + 1.50*SC + 0.90*VT + 0.90*NV")
comb= combs.newLoadCombination("ELU007","1.00*G + 1.50*VT")
comb= combs.newLoadCombination("ELU008","1.00*G + 1.50*VT + 0.90*NV")
comb= combs.newLoadCombination("ELU009","1.00*G + 1.05*SC + 1.50*VT")
comb= combs.newLoadCombination("ELU010","1.00*G + 1.05*SC + 1.50*VT + 0.90*NV")
comb= combs.newLoadCombination("ELU011","1.00*G + 1.50*NV")
comb= combs.newLoadCombination("ELU012","1.00*G + 0.90*VT + 1.50*NV")
comb= combs.newLoadCombination("ELU013","1.00*G + 1.05*SC + 1.50*NV")
comb= combs.newLoadCombination("ELU014","1.00*G + 1.05*SC + 0.90*VT + 1.50*NV")
comb= combs.newLoadCombination("ELU015","1.35*G + 1.50*SC")
comb= combs.newLoadCombination("ELU016","1.35*G + 1.50*SC + 0.90*NV")
comb= combs.newLoadCombination("ELU017","1.35*G + 1.50*SC + 0.90*VT")
comb= combs.newLoadCombination("ELU018","1.35*G + 1.50*SC + 0.90*VT + 0.90*NV")
comb= combs.newLoadCombination("ELU019","1.35*G + 1.50*VT")
comb= combs.newLoadCombination("ELU020","1.35*G + 1.50*VT + 0.90*NV")
comb= combs.newLoadCombination("ELU021","1.35*G + 1.05*SC + 1.50*VT")
comb= combs.newLoadCombination("ELU022","1.35*G + 1.05*SC + 1.50*VT + 0.90*NV")
comb= combs.newLoadCombination("ELU023","1.35*G + 1.50*NV")
comb= combs.newLoadCombination("ELU024","1.35*G + 0.90*VT + 1.50*NV")
comb= combs.newLoadCombination("ELU025","1.35*G + 1.05*SC + 1.50*NV")
comb= combs.newLoadCombination("ELU026","1.35*G + 1.05*SC + 0.90*VT + 1.50*NV")
 

printFlag= 0

solu= prueba.getSoluProc
solCtrl= solu.getSoluControl


solModels= solCtrl.getModelWrapperContainer
sm= solModels.newModelWrapper("sm")


cHandler= sm.newConstraintHandler("penalty_constraint_handler")
cHandler.alphaSP= 1.0e15
cHandler.alphaMP= 1.0e15
numberer= sm.newNumberer("default_numberer")
numberer.useAlgorithm("rcm")

solMethods= solCtrl.getSoluMethodContainer
smt= solMethods.newSoluMethod("smt","sm")
solAlgo= smt.newSolutionAlgorithm("newton_raphson_soln_algo")
ctest= smt.newConvergenceTest("norm_unbalance_conv_test")
ctest.tol= 1e-3
ctest.maxNumIter= 10
#ctest.printFlag= printFlag
integ= smt.newIntegrator("load_control_integrator",xc.Vector([]))
soe= smt.newSystemOfEqn("band_gen_lin_soe")
solver= soe.newSolver("band_gen_lin_lapack_solver")
analysis= solu.newAnalysis("static_analysis","smt","")


def resuelveCombEstatLin(comb,tagSaveFase0,hlp):
  mdlr.resetLoadCase()
  db.restore(tagSaveFase0)
  hlp.helpSolve(comb)
  comb.addToDomain()
  analOk= analysis.analyze(1)
  comb.removeFromDomain()
  hlp.db.save(comb.tag*100)



dXMin=1e9
dXMax=-1e9

def trataResultsComb(comb):
  nodos= mdlr.getNodeLoader
  nod8= nodos.getNode(8)
  deltaX= nod8.getDisp[0] # Desplazamiento del nodo 2 según z
  global dXMin
  dXMin=min(dXMin,deltaX)
  global dXMax
  dXMax=max(dXMax,deltaX)
  ''' 
    print "tagComb= ",comb.tag
    print "nmbComb= ",comb.getNombre
    print "dXMin= ",(dXMin*1e3)," mm\n"
    print "dXMax= ",(dXMax*1e3)," mm\n"
   '''

import os
os.system("rm -r -f /tmp/test_combinacion_07.db")
db= prueba.newDatabase("BerkeleyDB","/tmp/test_combinacion_07.db")
helper= dbHelper.DatabaseHelperSolve(db)

# Fase 0: pretensado
mdlr.resetLoadCase()
cargas= mdlr.getLoadLoader

combs= cargas.getLoadCombinations
comb= combs.newLoadCombination("FASE0","1.00*G0")
tagSaveFase0= comb.tag*100
comb.addToDomain()
analOk= analysis.analyze(1)
comb.removeFromDomain()

combs.remove("FASE0") # Eliminamos la combinación (ya no hace falta).
db.save(tagSaveFase0)



nombrePrevia="" 
tagPrevia= 0
tagSave= 0
for key in combs.getKeys():
  c= combs[key]
  resuelveCombEstatLin(c,tagSaveFase0,helper)
  trataResultsComb(comb)

ratio1= abs((dXMax-0.115734e-3)/0.115734e-3)
ratio2= abs((dXMin+0.0872328e-3)/0.0872328e-3)

''' 
print "dXMin= ",(dXMin*1e3)," mm\n"
print "dXMax= ",(dXMax*1e3)," mm\n"
print "ratio1= ",ratio1
print "ratio2= ",ratio2
   '''

fname= os.path.basename(__file__)
if (ratio1<1e-5) & (ratio2<1e-5) :
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."

os.system("rm -r -f /tmp/test_combinacion_07.db") # Your garbage you clean it