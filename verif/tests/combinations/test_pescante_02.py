# -*- coding: utf-8 -*-

from __future__ import division

b= 0.4
h= 0.8
A= b*h
E= 200000*9.81/1e-4 # Módulo elástico aproximado del hormigón.
nu= 0.3 # Coeficiente de Poisson
G= E/(2*(1+nu)) # Módulo de elasticidad a cortante
Iy= (1/12.0*h*b**3) # Momento de inercia de la sección expresado en m4
Iz= (1/12.0*b*h**3) # Momento de inercia de la sección expresado en m4
J= 0.721e-8 # Momento de inercia a torsión expresado en m4
densHorm= 2500 # Densidad del hormigón armado.

import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from model import fix_node_6dof
from materials import typical_materials

#
#     3   2     4
#     ----|------
#         |
#         |
#         |
#         |1
#

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
nodos= mdlr.getNodeLoader
predefined_spaces.gdls_resist_materiales3D(nodos)
nodos.defaultTag= 1 #First node number.
nod1= nodos.newNodeXYZ(2.0,0.0,0.0)
nod2= nodos.newNodeXYZ(2.0,0.0,4.0)
nod3= nodos.newNodeXYZ(0.0,0.0,4.0)
nod4= nodos.newNodeXYZ(5.0,0.0,4.0)


trfs= mdlr.getTransfCooLoader
lin= trfs.newLinearCrdTransf3d("lin")
lin.xzVector= xc.Vector([0,1,0])
    
# Materials definition
scc= typical_materials.defElasticSection3d(mdlr,"scc",A,E,G,Iz,Iy,J)


# Elements definition
elementos= mdlr.getElementLoader
elementos.defaultTransformation= "lin"
elementos.defaultMaterial= "scc"
#  sintaxis: elastic_beam_3d[<tag>] 
elementos.defaultTag= 1
beam1= elementos.newElement("elastic_beam_3d",xc.ID([1,2]))
beam1.rho= densHorm*A
beam2= elementos.newElement("elastic_beam_3d",xc.ID([3,2])) 
beam2.rho= densHorm*A
beam3= elementos.newElement("elastic_beam_3d",xc.ID([2,4])) 
beam3.rho= densHorm*A

# Constraints
coacciones= mdlr.getConstraintLoader

fix_node_6dof.fixNode6DOF(coacciones,1)

# Loads definition
cargas= mdlr.getLoadLoader

casos= cargas.getLoadPatterns

#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"

# Peso propio
lpG= casos.newLoadPattern("default","G") 
# Sobrecarga de uso
lpSC= casos.newLoadPattern("default","SC") 
# Viento
lpVT= casos.newLoadPattern("default","VT") 
# Nieve
lpNV= casos.newLoadPattern("default","NV") 

casos.currentLoadPattern= "G"
elementos= mdlr.getSets.getSet("total").getElements
for e in elementos:
  pesoElem= (e.rho*(-10))
  e.vector3dUniformLoadGlobal(xc.Vector([0.0,0.0,pesoElem]))

beam2.vector3dUniformLoadGlobal(xc.Vector([0.0,0.0,-22e3]))
beam3.vector3dUniformLoadGlobal(xc.Vector([0.0,0.0,-22e3]))

casos.currentLoadPattern= "SC"
beam3.vector3dPointByRelDistLoadGlobal(0.99,xc.Vector([0.0,0.0,-100e3]))

casos.currentLoadPattern= "VT"
lpVT.newNodalLoad(3,xc.Vector([50e3,0.0,0.0,0.0,0.0,0.0]))

casos.currentLoadPattern= "NV"
beam2.vector3dUniformLoadGlobal(xc.Vector([0.0,0.0,-10e3]))
beam3.vector3dUniformLoadGlobal(xc.Vector([0.0,0.0,-10e3]))

# Combinaciones
combs= cargas.getLoadCombinations
import os
pth= os.path.dirname(__file__)
#print "pth= ", pth
if(not pth):
  pth= "."
execfile(pth+"/def_hip_elu.py")



def resuelveCombEstatLin(comb):
  mdlr.resetLoadCase()
  previa= comb.getCombPrevia()
  if(previa!=None):
    nombrePrevia= previa.getName
    tagPrevia= previa.tag
    ''' 
    print "nombrePrevia= ",nombrePrevia
    print "tag= ",comb.tag
    print "tagPrevia= ",tagPrevia
    print "descomp previa= ",getDescompCombPrevia
    print "resto sobre previa= ",getDescompRestoSobrePrevia
    '''
    tagSave= tagPrevia*100
    if(tagPrevia>0):
      db.restore(tagSave)

    comb.addToDomain()
    analisis= predefined_solutions.simple_static_linear(prueba)
    result= analisis.analyze(1)
    comb.removeFromDomain()


def trataResultsComb(comb):
  tagSave= comb.tag*100
  db.save(tagSave)
  elementos= mdlr.getElementLoader
  elem1= elementos.getElement(1)
  elem1.getResistingForce()
  global NMin1
  NMin1=min(NMin1,elem1.getN1)
  global NMin2
  NMin2=min(NMin2,elem1.getN2)
  ''' 
  print "tagComb= ",comb.tag
  print "nmbComb= ",comb.getName
  print "N1= ",(elem1.getN1/1e3)
  print "N2= ",(elem1.getN2/1e3)
  '''


NMin1= 1e9
NMin2= 1e9

import os
os.system("rm -r -f /tmp/test_pescante_02.db")
db= prueba.newDatabase("BerkeleyDB","/tmp/test_pescante_02.db")

nombrePrevia="" 
tagPrevia= 0
for c in combs.getKeys():
  resuelveCombEstatLin(combs[c])
  trataResultsComb(combs[c])
          
ratio1= ((NMin1+440.7e3)/440.7e3)
ratio2= ((NMin2+397.5e3)/397.5e3)

''' 
print "NMin1= ",NMin1
print "ratio1= ",ratio1
print "NMin2= ",NMin2
print "ratio2= ",ratio2
'''


fname= os.path.basename(__file__)
if (abs(ratio1)<1e-8) & (abs(ratio2)<1e-8):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."
os.system("rm -rf /tmp/test_pescante_02.db") # Your garbage you clean it