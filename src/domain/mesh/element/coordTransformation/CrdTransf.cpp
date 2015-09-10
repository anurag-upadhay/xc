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
// $Date: 2005/12/15 00:30:38 $
// $Source: /usr/local/cvs/OpenSees/SRC/domain/mesh/element/coordTransformation/CrdTransf.cpp,v $


// File: ~/crdTransf/CrdTransf.C
//
// Written: Remo Magalhaes de Souza (rmsouza@ce.berkeley.edu)
// Created: 04/2000
// Revision: A
//
// Description: This file contains the implementation for the XC::CrdTransf class.
// CrdTransf provides the abstraction of a frame
// coordinate transformation. It is an abstract base class and
// thus no objects of  it's type can be instatiated.

#include <domain/mesh/element/coordTransformation/CrdTransf.h>
#include <domain/mesh/node/Node.h>
#include <utility/matrix/Vector.h>
#include <utility/matrix/Matrix.h>
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/base/utils_any.h"
#include "xc_utils/src/geom/pos_vec/MatrizPos3d.h"
#include "utility/matrix/ID.h"
#include "utility/actor/actor/MovableVector.h"
#include "utility/actor/actor/ArrayCommMetaData.h"
#include "xc_utils/src/nucleo/InterpreteRPN.h"

//! @brief Constructor.
XC::CrdTransf::CrdTransf(int tag, int classTag, int dim_joint_offset)
  : TaggedObject(tag), MovableObject(classTag),
    nodeIPtr(nullptr), nodeJPtr(nullptr),L(0.0),nodeIOffset(dim_joint_offset), nodeJOffset(dim_joint_offset), initialDispChecked(false) {}

//! @brief Lee el objeto desde archivo.
bool XC::CrdTransf::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(CrdTransf) Procesando comando: " << cmd << std::endl;

    if(cmd == "set_rigid_joint_offsetI")
      {
        const Vector rigJntOffset1= Vector(convert_to_vector_double(interpretaVectorAny(status.GetBloque())));
	set_rigid_joint_offsetI(rigJntOffset1);
        return true;
      }
    else if(cmd == "set_rigid_joint_offsetJ")
      {
        const Vector rigJntOffset2= Vector(convert_to_vector_double(interpretaVectorAny(status.GetBloque())));
	set_rigid_joint_offsetJ(rigJntOffset2);
        return true;
      }
    else
      return TaggedObject::procesa_comando(status);
  }

//! @brief Destructor virtual.
XC::CrdTransf::~CrdTransf(void)
  {}

//! @brief Asigna los punteros a nodo dorsal y frontal.
int XC::CrdTransf::set_ptr_nodos(Node *nodeIPointer, Node *nodeJPointer)
  {
    nodeIPtr = nodeIPointer;
    nodeJPtr = nodeJPointer;

    if ((!nodeIPtr) || (!nodeJPtr))
      {
        std::cerr << "\nLinearCrdTransf2d::initialize";
        std::cerr << "\ninvalid pointers to the element nodes\n";
        return -1;
      }
    return 0;
  }

const XC::Matrix &XC::CrdTransf::getPointsGlobalCoordFromLocal(const Matrix &localCoords) const
  {
    static Matrix retval;
    const size_t numPts= localCoords.noRows(); //Número de puntos a transformar.
    const size_t dim= localCoords.noCols(); //Dimensión del espacio.
    retval.resize(numPts,dim);
    Vector xl(dim);
    Vector xg(dim);
    for(size_t i= 0;i<numPts;i++)
      {
        for(size_t j= 0;j<dim;j++)
          xl[j]= localCoords(i,j);
        xg= getPointGlobalCoordFromLocal(xl);
        for(size_t j= 0;j<dim;j++)
          retval(i,j)= xg[j];
      }
    return retval;
  }

double XC::CrdTransf::getPointBasicCoordFromGlobal(const Vector &globalCoords) const
  {
    Vector cl= getPointLocalCoordFromGlobal(globalCoords);
    return cl[0]/L;
  }

const XC::Vector &XC::CrdTransf::getBasicDisplSensitivity(int gradNumber)
  {
    std::cerr << "WARNING XC::CrdTransf::getBasicDisplSensitivity() - this method "
        << " should not be called." << std::endl;

    static XC::Vector dummy(1);
    return dummy;
  }

const XC::Vector &XC::CrdTransf::getGlobalResistingForceShapeSensitivity(const XC::Vector &pb, const XC::Vector &p0)
  {
    std::cerr << "ERROR XC::CrdTransf::getGlobalResistingForceSensitivity() - has not been"
        << " implemented yet for the chosen transformation." << std::endl;

    static XC::Vector dummy(1);
    return dummy;
  }

const XC::Vector &XC::CrdTransf::getBasicTrialDispShapeSensitivity(void)
  {
    std::cerr << "ERROR CrdTransf::getBasicTrialDispShapeSensitivity() - has not been"
        << " implemented yet for the chosen transformation." << std::endl;

    static Vector dummy(1);
    return dummy;
  }

//! @brief Envia los miembros del objeto a través del canal que se pasa como parámetro.
int XC::CrdTransf::sendData(CommParameters &cp)
  {
    //data(0)= getTag();
    int res= cp.sendDouble(L,getDbTagData(),CommMetaData(1));
    res+= cp.sendVector(nodeIOffset,getDbTagData(),CommMetaData(2));
    res+= cp.sendVector(nodeJOffset,getDbTagData(),CommMetaData(3));
    res+= cp.sendVector(nodeIInitialDisp,getDbTagData(),CommMetaData(4));
    res+= cp.sendVector(nodeJInitialDisp,getDbTagData(),CommMetaData(5));
    res+= cp.sendBool(initialDispChecked,getDbTagData(),CommMetaData(6));
    if(res<0)
      std::cerr << "CrdTransf::sendData - failed to send data.\n";
    return res;
  }

//! @brief Recibe los miembros del objeto a través del canal que se pasa como parámetro.
int XC::CrdTransf::recvData(const CommParameters &cp)
  {
    //setTag(data(0));
    int res= cp.receiveDouble(L,getDbTagData(),CommMetaData(1));
    res+= cp.receiveVector(nodeIOffset,getDbTagData(),CommMetaData(2));
    res+= cp.receiveVector(nodeJOffset,getDbTagData(),CommMetaData(3));
    res+= cp.receiveVector(nodeIInitialDisp,getDbTagData(),CommMetaData(4));
    res+= cp.receiveVector(nodeJInitialDisp,getDbTagData(),CommMetaData(5));
    res+= cp.receiveBool(initialDispChecked,getDbTagData(),CommMetaData(6));
    if(res<0)
      std::cerr << "CrdTransf::recvData - failed to receive data.\n";
    return res;    
  }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
any_const_ptr XC::CrdTransf::GetProp(const std::string &cod) const
  {
    if(verborrea>4)
      std::clog << "CrdTransf::GetProp (" << nombre_clase() << "::GetProp) Buscando propiedad: " << cod << std::endl;
    if(cod=="getInitialLength")
      {
        tmp_gp_dbl= getInitialLength();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getDeformedLength")
      {
        tmp_gp_dbl= getDeformedLength();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getTrialDisp")
      return get_prop_vector(getBasicTrialDisp());
    else if(cod=="getIncrDisp")
      return get_prop_vector(getBasicIncrDisp());
    else if(cod=="getIncrDeltaDisp")
      return get_prop_vector(getBasicIncrDeltaDisp());
    else if(cod=="getTrialVel")
      return get_prop_vector(getBasicTrialVel());
    else if(cod=="getTrialAccel")
      return get_prop_vector(getBasicTrialAccel());
    else if(cod=="getVectorI")
      return get_prop_vector(getI());
    else if(cod=="getVectorJ")
      return get_prop_vector(getJ());
    else if(cod == "getPointGlobalCoordFromLocal")
      {
        if(InterpreteRPN::Pila().size()>0)
          {
            const Vector local_coord= convert_to_vector(InterpreteRPN::Pila().Pop());
            return get_prop_vector(getPointGlobalCoordFromLocal(local_coord));
          }
        else
          {
            err_num_argumentos(std::cerr,1,"GetProp",cod);
            return any_const_ptr();
          }
      }
    else if(cod == "getPointGlobalCoordFromBasic")
      {
        const double xi= popDouble(cod);
        return get_prop_vector(getPointGlobalCoordFromBasic(xi));
      }
    else if(cod == "getPointLocalCoordFromGlobal")
      {
        if(InterpreteRPN::Pila().size()>0)
          {
            const Vector global_coord= convert_to_vector(InterpreteRPN::Pila().Pop());
            return get_prop_vector(getPointLocalCoordFromGlobal(global_coord));
          }
        else
          {
            err_num_argumentos(std::cerr,1,"GetProp",cod);
            return any_const_ptr();
          }
      }
    else if(cod == "getPointBasicCoordFromGlobal")
      {
        if(InterpreteRPN::Pila().size()>0)
          {
            const Vector global_coord= convert_to_vector(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= getPointBasicCoordFromGlobal(global_coord);
            return any_const_ptr(tmp_gp_dbl);
          }
        else
          {
            err_num_argumentos(std::cerr,1,"GetProp",cod);
            return any_const_ptr();
          }
      }
    else if(cod == "getPointGlobalDisplFromBasic")
      {
        if(InterpreteRPN::Pila().size()>1)
          {
            const Vector local_coord= convert_to_vector(InterpreteRPN::Pila().Pop());
            const double xi= convert_to_double(InterpreteRPN::Pila().Pop());
            return get_prop_vector(getPointGlobalDisplFromBasic(xi,local_coord));
          }
        else
          {
            err_num_argumentos(std::cerr,2,"GetProp",cod);
            return any_const_ptr();
          }
      }
    else if(cod=="getVectorGlobalCoordFromLocal")
      {
        if(InterpreteRPN::Pila().size()>0)
          {
            const Vector local_coord= convert_to_vector(InterpreteRPN::Pila().Pop());
            return get_prop_vector(getVectorGlobalCoordFromLocal(local_coord));
          }
        else
          {
            err_num_argumentos(std::cerr,1,"GetProp",cod);
            return any_const_ptr();
          }
      }
    else if(cod=="getVectorLocalCoordFromGlobal")
      {
        if(InterpreteRPN::Pila().size()>0)
          {
            const Vector global_coord= convert_to_vector(InterpreteRPN::Pila().Pop());
            return get_prop_vector(getVectorLocalCoordFromGlobal(global_coord));
          }
        else
          {
            err_num_argumentos(std::cerr,1,"GetProp",cod);
            return any_const_ptr();
          }
      }
    else if(cod=="getPointLocalCoordFromGlobal")
      {
        if(InterpreteRPN::Pila().size()>0)
          {
            const Vector global_coord= convert_to_vector(InterpreteRPN::Pila().Pop());
            return get_prop_vector(getPointLocalCoordFromGlobal(global_coord));
          }
        else
          {
            err_num_argumentos(std::cerr,1,"GetProp",cod);
            return any_const_ptr();
          }
      }
    else
      return TaggedObject::GetProp(cod);
  }