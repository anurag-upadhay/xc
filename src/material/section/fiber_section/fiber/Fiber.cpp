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
                                                                        
// $Revision: 1.3 $
// $Date: 2003/02/25 23:33:36 $
// $Source: material/section/fiber_section/fiber/Fiber.cpp,v $
                                                                        
                                                                        
// File: ~/fiber/Fiber.C
//
// Written: Remo Magalhaes de Souza
// Created: 10/98
// Revision: 
//
// Description: This file contains the implementation for the
// Fiber class. Fiber provides the abstraction of a section fiber.
// 
// What: "@(#) Fiber.C, revA"


#include "Fiber.h"
#include "boost/any.hpp"
#include "material/uniaxial/UniaxialMaterial.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/geom/pos_vec/Pos2d.h"

//! @brief constructor.
XC::Fiber::Fiber(int tag, int classTag)
  : TaggedObject(tag), MovableObject(classTag), dead(false) {}

//! @brief Lee un objeto Fiber desde archivo
bool XC::Fiber::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(Fiber) Procesando comando: " << cmd << std::endl;

    if(cmd == "material")
      {
        UniaxialMaterial *mat= getMaterial();
        if(mat)
          mat->LeeCmd(status);
        else
          std::cerr  << "(Fiber) Procesando comando: " << cmd 
                     << " puntero a material nulo en fibra: " << getTag() << std::endl;
        return true;
      }
    else if(cmd == "kill")
      {
        status.GetBloque(); //Ignoramos argumentos.
        kill();
        return true;
      }
    else if(cmd == "alive")
      {
        status.GetBloque(); //Ignoramos argumentos.
        alive();
        return true;
      }
    else
      return TaggedObject::procesa_comando(status);
  }

//! @brief Interpreta una cadena de caracteres y coloca los resultados en la pila.
bool XC::Fiber::interpreta(const std::string &str,const int &numValEsperados) const
  { return EntCmd::interpreta(str,numValEsperados); }

XC::Response *XC::Fiber::setResponse(const std::vector<std::string> &argv, Information &info)
  { return nullptr; }

int XC::Fiber::getResponse(int responseID, Information &info)
  { return -1; }

//! @brief Devuelve la fuerza con la que actúa la fibra.
double XC::Fiber::getFuerza(void) const
  { 
    double retval= 0.0;
    if(isAlive())
      retval= getArea()*getMaterial()->getStress();
    return retval;
  }

//! @brief Devuelve la fuerza con la que actúa la fibra.
double XC::Fiber::getDeformacion(void) const
  { return getMaterial()->getStrain(); }

//! @brief Devuelve la posición de la fibra.
Pos2d XC::Fiber::getPos(void) const
  { return Pos2d(getLocY(),getLocZ()); }

//! @brief Envia los datos a través del canal que se pasa como parámetro.
int XC::Fiber::sendData(CommParameters &cp)
  {
    setDbTagDataPos(0,getTag());
    setDbTagDataPos(1,dead);
    return 0;
  }


//! @brief Recibe los datos a través del canal que se pasa como parámetro.
int XC::Fiber::recvData(const CommParameters &cp)
  {    
    setTag(getDbTagDataPos(0));
    dead= static_cast<bool>(getDbTagDataPos(1));
    return 0;
  }
