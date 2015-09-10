//----------------------------------------------------------------------------
//  programa XC; cálculo mediante el método de los elementos finitos orientado
//  a la solución de problemas estructurales.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
//  El programa deriva del denominado OpenSees <http://opensees.berkeley.edu>
//  desarrollado por el «Pacific earthquake engineering research center».
//
//  Salvo las restricciones que puedan derivarse del copyright del
//  programa original (ver archivo copyright_opensees.txt) este
//  software es libre: usted puede redistribuirlo y/o modificarlo 
//  bajo los términos de la Licencia Pública General GNU publicada 
//  por la Fundación para el Software Libre, ya sea la versión 3 
//  de la Licencia, o (a su elección) cualquier versión posterior.
//
//  Este software se distribuye con la esperanza de que sea útil, pero 
//  SIN GARANTÍA ALGUNA; ni siquiera la garantía implícita
//  MERCANTIL o de APTITUD PARA UN PROPÓSITO DETERMINADO. 
//  Consulte los detalles de la Licencia Pública General GNU para obtener 
//  una información más detallada. 
//
// Debería haber recibido una copia de la Licencia Pública General GNU 
// junto a este programa. 
// En caso contrario, consulte <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------
//BidimMecLoad.cpp

#include "BidimMecLoad.h"
#include <utility/matrix/Matrix.h>
#include <utility/matrix/Vector.h>
#include "xc_utils/src/base/CmdStatus.h"
#include "utility/matrix/ID.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "domain/mesh/element/coordTransformation/CrdTransf.h"
#include "utility/actor/actor/MovableVector.h"
#include "domain/mesh/node/Node.h"

XC::BidimMecLoad::BidimMecLoad(int tag,int classTag,const double &wt,const double &wa1,const double &wa2,const ID &theElementTags)
  :BidimLoad(tag, classTag, theElementTags), Trans(wt), Axial1(wa1), Axial2(wa2) {}

XC::BidimMecLoad::BidimMecLoad(int tag,int classTag)
  :BidimLoad(tag, classTag), Trans(0.0), Axial1(0.0), Axial2(0.0) {}

//! @brief Lee un objeto BidimMecLoad desde archivo
bool XC::BidimMecLoad::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(BidimMecLoad) Procesando comando: " << cmd << std::endl;
    if(cmd == "trans")
      {
        Trans= interpretaDouble(status.GetString());
        return true;
      }
    else if(cmd == "axial1")
      {
        Axial1= interpretaDouble(status.GetString());
        return true;
      }
    else if(cmd == "axial2")
      {
        Axial1= interpretaDouble(status.GetString());
        return true;
      }
    else
      return BidimLoad::procesa_comando(status);
  }

void XC::BidimMecLoad::Print(std::ostream &s, int flag) const
  {
    s << "BidimMecLoad - Reference load" << std::endl;
    s << "  Transverse: " << Trans << std::endl;
    s << "  Axial1:      " << Axial1 << std::endl;
    s << "  Axial2:      " << Axial2 << std::endl;
    BidimLoad::Print(s,flag);
  }

//! @brief Envía los datos a través del canal que se pasa como parámetro.
int XC::BidimMecLoad::sendData(CommParameters &cp)
  {
    int res= BidimLoad::sendData(cp);
    res+= cp.sendDoubles(Trans,Axial1,Axial2,getDbTagData(),CommMetaData(5));
    return res;
  }

//! @brief Recibe los datos a través del canal que se pasa como parámetro.
int XC::BidimMecLoad::recvData(const CommParameters &cp)
  {
    int res= BidimLoad::recvData(cp);
    res+= cp.receiveDoubles(Trans,Axial1,Axial2,getDbTagData(),CommMetaData(5));
    return res;
  }

//! Devuelve la propiedad del objeto cuyo código se pasa
//! como parámetro.
any_const_ptr XC::BidimMecLoad::GetProp(const std::string &cod) const
  {
    if(cod == "axial1")
      return any_const_ptr(Axial1);
    else if(cod == "axial2")
      return any_const_ptr(Axial2);
    else if(cod == "trans")
      return any_const_ptr(Trans);
    else
      return BidimLoad::GetProp(cod);
  }