# -*- coding: utf-8 -*-
# Test de funcionamiento de la comprobación a cortante de una sección de hormigón armado.
#    Algunos resultados se comparan con los obtenidos del ejercicio ER-CS-01 de
#    www.areadecalculo.com. Según la norma EHE-08».

# Macros
from __future__ import division
from materials.ehe import auxEHE
from misc import banco_pruebas_scc3d
from solution import predefined_solutions


from materials.ehe import hormigonesEHE
from materials.ehe import EHE_reinforcing_steel
from materials.ehe import comprobVEHE08
from materials.ehe import cortanteEHE
import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from model import fix_node_6dof
from materials import typical_materials
import math

# Coeficientes de seguridad.
gammac= 1.5 # Coeficiente de minoración de la resistencia del hormigón.
gammas= 1.15 # Coeficiente de minoración de la resistencia del acero.

ancho= 0.3 # Ancho de la sección expresado en metros.
canto= 0.4 # Canto de la sección expresado en metros.
recub= 0.05 # Recubrimiento de la sección expresado en metros.
areaBarra=4e-4

NDato= -400e3 # Axil para comprobar el cortante.
MyDato= 1e5 # Momento para comprobar el cortante.
MzDato= 1e3 # Momento para comprobar el cortante.

numBarras= 3

''' 
print "numBarras= ",numBarras
   '''

prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
# Materials definition
materiales= mdlr.getMaterialLoader

tagHA25= hormigonesEHE.HA25.defDiagD(mdlr)
tagB500S= EHE_reinforcing_steel.B500S.defDiagD(mdlr)

respT= typical_materials.defElasticMaterial(mdlr,"respT",1e10) # Respuesta de la sección a torsión.
respVy= typical_materials.defElasticMaterial(mdlr,"respVy",1e6) # Respuesta de la sección a cortante según y.
respVz= typical_materials.defElasticMaterial(mdlr,"respVz",1e3) # Respuesta de la sección a cortante según y.
geomSecHA= mdlr.getMaterialLoader.newSectionGeometry("geomSecHA")
regiones= geomSecHA.getRegions
rg= regiones.newQuadRegion(hormigonesEHE.HA25.nmbDiagD)
rg.nDivIJ= 10
rg.nDivJK= 10
rg.pMin= geom.Pos2d(-ancho/2.0,-canto/2.0)
rg.pMax= geom.Pos2d(ancho/2.0,canto/2.0)
armaduras= geomSecHA.getReinfLayers
armaduraInf= armaduras.newStraightReinfLayer(EHE_reinforcing_steel.B500S.nmbDiagD)
armaduraInf.numReinfBars= numBarras
armaduraInf.barArea= areaBarra
armaduraInf.p1= geom.Pos2d(recub-ancho/2.0,recub-canto/2.0) # Armadura inferior.
armaduraInf.p2= geom.Pos2d(ancho/2.0-recub,recub-canto/2.0)
armaduraSup= armaduras.newStraightReinfLayer(EHE_reinforcing_steel.B500S.nmbDiagD)
armaduraSup.numReinfBars= numBarras
armaduraSup.barArea= areaBarra
armaduraSup.p1= geom.Pos2d(recub-ancho/2.0,canto/2.0-recub) # Armadura superior.
armaduraSup.p2= geom.Pos2d(ancho/2.0-recub,canto/2.0-recub)

secHA= materiales.newMaterial("fiberSectionShear3d","secHA")
fiberSectionRepr= secHA.getFiberSectionRepr()
fiberSectionRepr.setGeomNamed("geomSecHA")
secHA.setupFibers()
secHA.setRespVyByName("respVy")
secHA.setRespVzByName("respVz")
secHA.setRespTByName("respT")

banco_pruebas_scc3d.modeloSecc3d(mdlr,"secHA")


# Constraints
coacciones= mdlr.getConstraintLoader
fix_node_6dof.fixNode6DOF(coacciones,1)

# Loads definition
cargas= mdlr.getLoadLoader

casos= cargas.getLoadPatterns

#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"
#Load case definition
lp0= casos.newLoadPattern("default","0")
lp0.newNodalLoad(2,xc.Vector([NDato,0,0,0,MyDato,MzDato]))
lp1= casos.newLoadPattern("default","1")
lp1.newNodalLoad(2,xc.Vector([NDato,0,0,0,MzDato/10.0,MyDato/10.0]))
lp2= casos.newLoadPattern("default","2")
lp2.newNodalLoad(2,xc.Vector([NDato,0,0,0,0,0]))

#We add the load case to domain.
casos.addToDomain("0")


# Procedimiento de solución
analisis= predefined_solutions.simple_newton_raphson(prueba)
analOk= analisis.analyze(10)
if(analOk!=0):
  print "Error!; failed to converge."
  exit()



secHAParamsCortante= cortanteEHE.ParamsCortante()



elementos= mdlr.getElementLoader
ele1= elementos.getElement(1)
scc= ele1.getSection()
secHAParamsCortante.calcVuEHE08(mdlr,scc,"",hormigonesEHE.HA25,EHE_reinforcing_steel.B500S,NDato,math.sqrt(MyDato**2+MzDato**2),0,0)


Vu2A= secHAParamsCortante.Vu2


cargas= mdlr.getLoadLoader.removeFromDomain("0") # Quitamos la carga.
mdlr.resetLoadCase()
cargas= mdlr.getLoadLoader.addToDomain("1") # Añadimos la otra carga.


analOk= analisis.analyze(10)
if(analOk!=0):
  print "Error!; failed to converge."
  exit()


secHAParamsCortante.calcVuEHE08(mdlr,scc,"",hormigonesEHE.HA25,EHE_reinforcing_steel.B500S, 0,0,0,0)

Vu2B= secHAParamsCortante.Vu2


cargas= mdlr.getLoadLoader.removeFromDomain("1") # Quitamos la carga.
mdlr.resetLoadCase()
cargas= mdlr.getLoadLoader.addToDomain("2") # Añadimos la otra carga.


analOk= analisis.analyze(10)
if(analOk!=0):
  print "Error!; failed to converge."
  exit()


secHAParamsCortante.calcVuEHE08(mdlr,scc,"",hormigonesEHE.HA25,EHE_reinforcing_steel.B500S, 0,0,0,0)

Vu2C= secHAParamsCortante.Vu2


''' En lugar de tomar Vu2= 127.638 kN como indica en el documento
que sirve de base, tomamos 117.052 kN, el motivo es que aquí si
se tiene en cuenta el axil que toman las armaduras por lo que
el valor de la tensión media en el alma que se obtiene es inferior. '''
Vu2ATeor= 117.052e3
ratio1= ((Vu2A-Vu2ATeor)/Vu2ATeor)
Vu2BTeor= 90906.4
ratio2= ((Vu2B-Vu2BTeor)/Vu2BTeor)
Vu2CTeor= 127386
ratio3= ((Vu2C-Vu2CTeor)/Vu2CTeor)


'''
print "Vu2A= ",Vu2A/1e3," kN"
print "Vu2ATeor= ",Vu2ATeor/1e3," kN"
print "ratio1= ",ratio1
print "\nVu2B= ",Vu2B/1e3," kN"
print "Vu2BTeor= ",Vu2BTeor/1e3," kN"
print "ratio2= ",ratio2
print "\nVu2C= ",Vu2C/1e3," kN"
print "Vu2CTeor= ",Vu2CTeor/1e3," kN"
print "ratio3= ",ratio3
 '''


import os
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-5) & (abs(ratio2)<0.03) & (abs(ratio3)<1e-5):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."