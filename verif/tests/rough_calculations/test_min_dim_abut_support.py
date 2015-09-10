# -*- coding: utf-8 -*-
import rough_calculations.ng_min_dim_of_abutment_support


b2= rough_calculations.ng_min_dim_of_abutment_support.getBminPontFlotant(45,"E","Z3a","COII")

b3= rough_calculations.ng_min_dim_of_abutment_support.getBminPontAppuiFixe((44.87+52.46)/2.0,0.1,"E","Z3a","COII")

ratio1= (b2-0.3404)/0.3404
ratio2= (b3-0.32102328)/0.32102328

# print "b2= ", b2
# print "b3= ", b3
# print "ratio1= ",ratio1
# print "ratio2= ",ratio2

import os
fname= os.path.basename(__file__)
if abs(ratio1)<1e-10 and abs(ratio2)<1e-10 :
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."
