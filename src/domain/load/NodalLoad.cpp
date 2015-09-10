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
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.6 $
// $Date: 2005/11/23 22:48:50 $
// $Source: /usr/local/cvs/OpenSees/SRC/domain/mesh/node/NodalLoad.cpp,v $
                                                                        
                                                                        
// Written: fmk 
// Created: 11/96
// Revision: A
//
// Purpose: This file contains the implementation of XC::NodalLoad

#include "NodalLoad.h"
#include "domain/component/Parameter.h"
#include <domain/mesh/node/Node.h>
#include <utility/matrix/Vector.h>
#include <cstdlib>
#include <domain/domain/Domain.h>
#include <domain/mesh/element/Information.h>
#include "xc_utils/src/base/CmdStatus.h"
#include "utility/matrix/ID.h"
#include "utility/matrix/Matrix.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/geom/pos_vec/Pos3d.h"
#include "xc_utils/src/geom/pos_vec/Vector3d.h"
#include "xc_utils/src/geom/pos_vec/SVD3d.h"
#include "utility/actor/actor/ArrayCommMetaData.h"

void XC::NodalLoad::borra_load(void)
  {
    if(load) delete load;
    load= nullptr;
  }

void XC::NodalLoad::set_load(const Vector &theLoad)
  {
    borra_load();
    load= new Vector(theLoad);
    if(!load)
      {
        std::cerr << "FATAL XC::NodalLoad::NodalLoad(int node, const Vector &theLoad) -";
        std::cerr << " ran out of memory for load on Node " << myNode << std::endl;
        exit(-1);
      }
  }

// AddingSensitivity:BEGIN /////////////////////////////////////
 XC::Vector XC::NodalLoad::gradientVector(1);
// AddingSensitivity:END ///////////////////////////////////////

//! @brief Constructor.
XC::NodalLoad::NodalLoad(int tag, int theClassTag)
:Load(tag,theClassTag), myNode(0), myNodePtr(nullptr), load(nullptr), konstant(false)
  {
    // AddingSensitivity:BEGIN /////////////////////////////////////
    parameterID = 0;
    // AddingSensitivity:END ///////////////////////////////////////
  }

//! @brief Constructor.
XC::NodalLoad::NodalLoad(int tag, int node, int theClassTag)
  :Load(tag,theClassTag), myNode(node), myNodePtr(nullptr), load(nullptr), konstant(false)
  {
    // AddingSensitivity:BEGIN /////////////////////////////////////
    parameterID = 0;
    // AddingSensitivity:END ///////////////////////////////////////
  }


//! @brief Constructor.
XC::NodalLoad::NodalLoad(int tag, int node, const Vector &theLoad, bool isLoadConstant)
  :Load(tag, LOAD_TAG_NodalLoad), myNode(node), myNodePtr(nullptr), load(nullptr), konstant(isLoadConstant)
  {
    set_load(theLoad);
    // AddingSensitivity:BEGIN /////////////////////////////////////
    parameterID = 0;
    // AddingSensitivity:END ///////////////////////////////////////
  }


//! @brief Lee un objeto XC::NodalLoad desde archivo
//!
//! Soporta los comandos:
//! -val: Lee las componentes del vector de cargas.
bool XC::NodalLoad::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(NodalLoad) Procesando comando: " << cmd << std::endl;
    if(cmd == "nod") //Nodo sobre el que actúa.
      {
        const int tmp= interpretaInt(status.GetString());
        if(!myNodePtr)
          myNode= tmp;
        else
          if(myNode!=tmp)
            std::cerr << "La carga nodal ya tiene nodo asignado, no puede asignarse a otro." << std::endl;
        return true;
      }
    else if(cmd == "nodPtr") //Puntero al nodo sobre el que actúa.
      {
        if(!myNodePtr)
          myNodePtr= get_node_ptr();
        if(myNodePtr)
          myNodePtr->LeeCmd(status);
        else
          {
            std::cerr << "El puntero a nodo es nulo." << std::endl;
            status.GetBloque();
          }
        return true;
      }
    else if(cmd == "val") //Valor del vector de cargas.
      {
        Vector tmp_load(crea_vector_double(status.GetString()));
        set_load(tmp_load);
        return true;
      }
    else if(cmd == "svd3d") //Sistema de vectores deslizantes en espacio3d.
      {
        if(!myNodePtr)
          myNodePtr= get_node_ptr();
        if(myNodePtr)
          {
            const SVD3d tmp= interpretaSVD3d(status.GetString());
            const Vector3d R= tmp.getResultante();
            const Pos3d pt= myNodePtr->getPosFinal3d();
            const Vector3d M= tmp.getMomento(pt);
            Vector tmp_load(6);
            tmp_load[0]= R.x();
            tmp_load[1]= R.y();
            tmp_load[2]= R.z();
            tmp_load[3]= M.x();
            tmp_load[4]= M.y();
            tmp_load[5]= M.z();
            set_load(tmp_load);
          }
        else
          {
            std::cerr << "El puntero a nodo es nulo." << std::endl;
            status.GetBloque();
          }
        return true;
      }
    else
      return Load::procesa_comando(status);
  }

//! @brief Destructor.
XC::NodalLoad::~NodalLoad(void)
  { borra_load(); }

void  XC::NodalLoad::setDomain(Domain *newDomain)
  {
    // first get myNodePtr
    if(newDomain == 0)
      return;

    // invoke the ancestor class method
    this->DomainComponent::setDomain(newDomain);    

    /*
    if(newDomain)
      {
        myNodePtr = newDomain->getNode(myNode);
        if(myNodePtr == 0)
          {
            std::cerr << *newDomain;
            std::cerr << "WARNING XC::NodalLoad::setDomain() - No associated XC::Node node " ;
            std::cerr << " for XC::NodalLoad " << *this;
            //        std::cerr << *newDomain;
            return;
          }
      }
    */
  }

int XC::NodalLoad::getNodeTag(void) const
  { return myNode; }

//! @brief Devuelve las componentes del vector fuerza.
const XC::Vector &XC::NodalLoad::getForce(void) const
  {
    static Vector retval(3);
    retval.Zero();
    if(!myNodePtr)
      myNodePtr= const_cast<NodalLoad *>(this)->get_node_ptr();
    if(load && myNodePtr)
      {
        const size_t numGdl= myNodePtr->getNumberDOF();
        const size_t dim= myNodePtr->getCrds().Size();
        switch(numGdl)
          {
          case 1:
            retval(0)= (*load)(0);
            break;
          case 2:
            retval(0)= (*load)(0);
            retval(1)= (*load)(1);
            break;
          case 3:
            if(dim == 2)
              {
                retval(0)= (*load)(0);
                retval(1)= (*load)(1);
              }
            else if(dim == 3)
              {
                retval(0)= (*load)(0);
                retval(1)= (*load)(1);
                retval(2)= (*load)(2);
              }
            break;
          case 6:
            retval(0)= (*load)(0);
            retval(1)= (*load)(1);
            retval(2)= (*load)(2);
            break;
          default:
            std::cerr << "Error en BeamMecLoad::getForce." << std::endl;
            break;
          }
      }
    else
      {
        if(!load)
          std::cerr << "NodalLoad::getForce; no se ha definido la carga." << std::endl;
        if(!myNodePtr)
          std::cerr << "NodalLoad::getForce; el puntero a nodo es nulo." << std::endl;
      }
    return retval;
  }

//! @brief Devuelve las componentes del vector fuerza.
const XC::Vector &XC::NodalLoad::getMoment(void) const
  {
    static Vector retval(3);
    retval.Zero();
    if(!myNodePtr)
      myNodePtr= const_cast<NodalLoad *>(this)->get_node_ptr();
    if(load && myNodePtr)
      {
        const size_t numGdl= myNodePtr->getNumberDOF();
        const size_t dim= myNodePtr->getCrds().Size();
        switch(numGdl)
          {
          case 3:
            if(dim == 2)
              retval(0)= (*load)(2);
            break;
          case 6:
            retval(0)= (*load)(3);
            retval(1)= (*load)(4);
            retval(2)= (*load)(5);
            break;
          default:
            std::cerr << "Error en BeamMecLoad::getMoment." << std::endl;
            break;
          }
      }
    else
      {
        if(!load)
          std::cerr << "NodalLoad::getMoment; no se ha definido la carga." << std::endl;
        if(!myNodePtr)
          std::cerr << "NodalLoad::getMoment; el puntero a nodo es nulo." << std::endl;
      }
    return retval;
  }

//! @brief Devuelve el puntero a nodo que corresponde al tag
//! que se guarda en myNode.
XC::Node *XC::NodalLoad::get_node_ptr(void)
  {
    Node *retval= nullptr;
    Domain *theDomain= getDomain();
    if(!theDomain)
      std::cerr << "NodalLoad::get_node_ptr(), puntero a dominio nulo.";
    else
      {
        retval= theDomain->getNode(myNode);
        if(!retval)
          {
            std::cerr << "NodalLoad::get_node_ptr() - No se encontró el nodo:"
                      << myNode << std::endl;
          }
      }
    return retval;
  }

void XC::NodalLoad::applyLoad(double loadFactor)
  {
    if(!myNodePtr)
      myNodePtr= get_node_ptr();

    assert(load);
    // add the load times the load factor to nodal unbalanced load
    if(konstant == false)
      myNodePtr->addUnbalancedLoad(*load,loadFactor);
    else
      myNodePtr->addUnbalancedLoad(*load,1.0);
  } 

//! @brief Devuelve un vector para almacenar los dbTags
//! de los miembros de la clase.
XC::DbTagData &XC::NodalLoad::getDbTagData(void) const
  {
    static DbTagData retval(7);
    return retval;
  }

//! @brief Envía los miembros del objeto a través del
//! canal que se pasa como parámetro.
int XC::NodalLoad::sendData(CommParameters &cp)
  {
    int res= Load::sendData(cp);
    res+= cp.sendInts(myNode,parameterID,getDbTagData(),CommMetaData(2));
    res+= cp.sendVectorPtr(load,getDbTagData(),ArrayCommMetaData(3,4,5));
    res+= cp.sendBool(konstant,getDbTagData(),CommMetaData(6));
    return res;
  }

//! @brief Recibe los miembros del objeto a través del
//! canal que se pasa como parámetro.
int XC::NodalLoad::recvData(const CommParameters &cp)
  {
    int res= Load::recvData(cp);
    res+= cp.receiveInts(myNode,parameterID,getDbTagData(),CommMetaData(2));
    load= cp.receiveVectorPtr(load,getDbTagData(),ArrayCommMetaData(3,4,5));
    res+= cp.receiveBool(konstant,getDbTagData(),CommMetaData(6));
    return res;
  }

int XC::NodalLoad::sendSelf(CommParameters &cp)
  {
    inicComm(7);
    int result= sendData(cp);
    
    const int dataTag= getDbTag();
    result+= cp.sendIdData(getDbTagData(),dataTag);
    if(result < 0)
      std::cerr << "NodalLoad::sendSelf - failed to send data\n";
    return result;
  }

int XC::NodalLoad::recvSelf(const CommParameters &cp)
  {        
    inicComm(7);
    const int dataTag= getDbTag();
    int res= cp.receiveIdData(getDbTagData(),dataTag);
    if(res<0)
      std::cerr << "NodalLoad::recvSelf() - failed to recv data\n";
    else
      res+= recvData(cp);
    return res;
  }


void XC::NodalLoad::Print(std::ostream &s, int flag)
  {
     s << "Nodal Load: " << myNode;
     if(load)
       s << " load : " << *load;
  }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
//!
//! Soporta los códigos:
//! nod: Devuelve el nodo sobre el que actúa la carga.
//! load: Devuelve el valor de la carga.
any_const_ptr XC::NodalLoad::GetProp(const std::string &cod) const
  {
    if(cod=="nod") 
      return any_const_ptr(*myNodePtr);
    else if(cod=="numNod")
      return any_const_ptr(myNode);
    else if(cod=="load")
      return get_prop_vector(load);
    else if(cod == "getForce")
      return get_prop_vector(getForce());
    else if(cod == "getMoment")
      return get_prop_vector(getMoment());
    else
      return Load::GetProp(cod);
  }


// AddingSensitivity:BEGIN /////////////////////////////////////
int XC::NodalLoad::setParameter(const std::vector<std::string> &argv, Parameter &param)
  {
    if(argv.size() < 1)
      return -1;
    else if(argv[0] == "1")
      return param.addObject(1, this);
    else if(argv[0] == "2")
      return param.addObject(2, this);
    else if(argv[0] == "3")
      return param.addObject(3, this);
    else if(argv[0] == "4")
      return param.addObject(4, this);
    else if(argv[0] == "5")
      return param.addObject(5, this);
    else if(argv[0] == "6")
      return param.addObject(6, this);
    else
      return -1;
  }

int XC::NodalLoad::updateParameter(int parameterID, Information &info)
  {
    //int nn; 
    //Domain *theDomain;
    switch (parameterID)
      {
      case -1:
        return -1;
      case 1:
        (*load)(0) = info.theDouble;
        return 0;
      case 2:
        (*load)(1) = info.theDouble;
        return 0;
      case 3:
        (*load)(2) = info.theDouble;
        return 0;
      default:
        return -1;
      }
  }


int XC::NodalLoad::activateParameter(int passedParameterID)
  {
    parameterID = passedParameterID;
    return 0;
  }


const XC::Vector &XC::NodalLoad::getExternalForceSensitivity(int gradNumber)
  {
    gradientVector(0) = (double)parameterID;
    return gradientVector;
  }


// AddingSensitivity:END //////////////////////////////////////
