# -*- coding: utf-8 -*-

import sqlite3 as sqlite
from sqliteUtils import sqlite_macros
from misc import ansysToEsfBarra

__author__= "Luis C. Pérez Tato (LCPT)"
__copyright__= "Copyright 2014, LCPT"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com"

import os
pth= os.path.dirname(__file__)
#print "pth= ", pth
if(not pth):
  pth= "."
fNameIn= pth+"/../aux/pilares.lst"
dbName= "/tmp/pilares.db"
tbName= "esfPilares"
os.system("rm -f " + dbName)
sqlite_macros.SQLTcreaDBase(dbName)
ansysToEsfBarra.ansysToEsfBarra(fNameIn,dbName,tbName)

i= 0
j= 0
e1= 0

conn= sqlite.connect(dbName)
conn.row_factory= sqlite.Row
otra= conn.cursor()
cur= conn.cursor()
cur.execute("select distinct ACCION from " + tbName)
rows= cur.fetchall()
for r in rows:
  i+=1
  idAccion= r['ACCION']
  sqlStr= "select * from "+ tbName + " where ACCION = '" + idAccion +"';"
  otra.execute(sqlStr)
  for row in otra:
    j+= 1


e1= sqlite_macros.SQLTexisteTabla(dbName,tbName)


ratio1= abs(i-6)/6 # 6 actions
ratio2= abs(j/6-144) # 144 elements

''' 
print "i= ",i
print "ratio1= ",ratio1
print "j= ",j
print "ratio2= ",ratio2
   '''

import os
from miscUtils import LogMessages as lmsg
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-15) & (abs(ratio2)<1e-15) & e1:
  print "test ",fname,": ok."
else:
  lmsg.error(fname+' ERROR.')

os.system("rm -f /tmp/pilares.db")
