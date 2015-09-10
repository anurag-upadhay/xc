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
/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in XC::main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.4 $
// $Date: 2006/01/04 21:59:10 $
// $Source: /usr/local/cvs/OpenSees/SRC/domain/load/pattern/MultiSupportPattern.cpp,v $
                                                                        
// Written: fmk 11/00
// Revised:
//
// Purpose: This file contains the class definition for XC::MultiSupportPattern.
// MultiSupportPattern is an abstract class.

#include <domain/load/pattern/load_patterns/MultiSupportPattern.h>
#include <domain/load/groundMotion/GroundMotion.h>

#include <utility/actor/objectBroker/FEM_ObjectBroker.h>
#include <domain/domain/Domain.h>
#include <domain/constraints/SP_Constraint.h>
#include <domain/constraints/SP_ConstraintIter.h>
#include <cstdlib>
#include <utility/handler/ErrorHandler.h>

XC::MultiSupportPattern::MultiSupportPattern(int tag, int _classTag)
  :EQBasePattern(tag, _classTag), theMotionTags(0,16), dbMotions(0)
  {}


XC::MultiSupportPattern::MultiSupportPattern(int tag)
  :EQBasePattern(tag, PATTERN_TAG_MultiSupportPattern), theMotionTags(0,16), dbMotions(0)
  {}


XC::MultiSupportPattern::MultiSupportPattern(void)
  :EQBasePattern(0, PATTERN_TAG_MultiSupportPattern), theMotionTags(0,16), dbMotions(0)
  {}


void  XC::MultiSupportPattern::applyLoad(double time)
  {
    SP_Constraint *sp;
    SP_ConstraintIter &theIter = this->getSPs();
    while ((sp = theIter()) != 0)
      sp->applyConstraint(time);
  }
    

int XC::MultiSupportPattern::addMotion(GroundMotion &theMotion, int tag)
  {
    // ensure no motion with given tag already added
    if(theMotionTags.getLocation(tag) >= 0)
      {
        std::cerr << "XC::MultiSupportPattern::addMotion - could not add new, motion wih same tag exists\n";
        return -1;
      }

    theMotions.addMotion(theMotion);
    return 0;
  }



XC::GroundMotion *XC::MultiSupportPattern::getMotion(int tag)
  {
    int loc = theMotionTags.getLocation(tag);
    if(loc < 0)
      return nullptr;
    else
      return theMotions[loc];
  }


bool XC::MultiSupportPattern::addNodalLoad(NodalLoad *)
  {
    std::cerr << "XC::MultiSupportPattern::addNodalLoad() - cannot add XC::NodalLoad to EQ pattern\n";  
    return false;
  }

bool XC::MultiSupportPattern::addElementalLoad(ElementalLoad *)
  {
    std::cerr << "XC::MultiSupportPattern::addElementalLoad() - cannot add XC::ElementalLoad to EQ pattern\n";    
    return false;
  }

//! @brief Envía los miembros del objeto a través del canal que se pasa como parámetro.
int XC::MultiSupportPattern::sendData(CommParameters &cp)
  {
    int res= EQBasePattern::sendData(cp);
    res+= cp.sendID(theMotionTags,getDbTagData(),CommMetaData(17));
    res+= cp.sendInt(dbMotions,getDbTagData(),CommMetaData(18));
    return res;
  }

//! @brief Recibe los miembros del objeto a través del canal que se pasa como parámetro.
int XC::MultiSupportPattern::recvData(const CommParameters &cp)
  {
    int res= EQBasePattern::recvData(cp);
    res+= cp.receiveID(theMotionTags,getDbTagData(),CommMetaData(17));
    res+= cp.receiveInt(dbMotions,getDbTagData(),CommMetaData(18));
    return res;
  }

//! @brief Recibe el objeto a través del canal que se pasa como parámetro.
int XC::MultiSupportPattern::sendSelf(CommParameters &cp)
  {
    setDbTag(cp);
    const int dataTag= getDbTag();
    inicComm(3);
    int res= sendData(cp);

    res+= cp.sendIdData(getDbTagData(),dataTag);
    if(res < 0)
      std::cerr << nombre_clase() << "sendSelf() - failed to send data\n";
    return res;
  }

//! @brief Envía el objeto a través del canal que se pasa como parámetro.
int XC::MultiSupportPattern::recvSelf(const CommParameters &cp)
  {
    inicComm(3);
    const int dataTag= getDbTag();
    int res= cp.receiveIdData(getDbTagData(),dataTag);

    if(res<0)
      std::cerr << nombre_clase() << "::recvSelf - failed to receive ids.\n";
    else
      {
        setTag(getDbTagDataPos(0));
        res+= recvData(cp);
        if(res<0)
          std::cerr << nombre_clase() << "::recvSelf - failed to receive data.\n";
      }
    return res;
  }

void XC::MultiSupportPattern::Print(std::ostream &s, int flag)
  {
    s << "MultiSupportPattern  tag: " << this->getTag() << std::endl;
    SP_Constraint *sp;
    SP_ConstraintIter &theIter = this->getSPs();
    while ((sp = theIter()) != 0)
      sp->Print(s, flag);
  }