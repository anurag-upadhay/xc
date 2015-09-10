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
                                                                        
// $Revision: 1.9 $
// $Date: 2003/03/04 00:48:16 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/section/SectionForceDeformation.cpp,v $
                                                                        
                                                                        
// File: ~/material/SectionForceDeformation.C
//
// Written: MHS 
// Created: Feb 2000
// Revision: A
//
// Description: This file contains the class implementation for SectionForceDeformation.
//
// What: "@(#) SectionForceDeformation.C, revA"

#include <material/section/SectionForceDeformation.h>
#include <domain/mesh/element/Information.h>
#include <domain/mesh/element/truss_beam_column/nonlinearBeamColumn/matrixutil/MatrixUtil.h>
#include <utility/matrix/Matrix.h>
#include <utility/matrix/Vector.h>
#include <utility/recorder/response/MaterialResponse.h>
#include "xc_utils/src/base/CmdStatus.h"
#include <deque>
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/base/utils_any.h"
#include "material/section/ResponseId.h"
#include "modelador/loaders/MaterialLoader.h"
#include "utility/actor/actor/MovableMatrix.h"
#include "utility/actor/actor/MatrixCommMetaData.h"
#include "xc_utils/src/nucleo/InterpreteRPN.h"

const XC::Matrix *ptr_section_tangent= nullptr;
const XC::Matrix *ptr_initial_tangent= nullptr;

//! @brief Constructor.
XC::SectionForceDeformation::SectionForceDeformation(int tag, int classTag,MaterialLoader *mat_ldr)
  : Material(tag,classTag), fDefault(nullptr), material_loader(mat_ldr) {}

//! @brief Constructor de copia.
XC::SectionForceDeformation::SectionForceDeformation(const SectionForceDeformation &otro)
  : Material(otro), fDefault(nullptr), material_loader(otro.material_loader)
  {
    if(otro.fDefault)
      fDefault= new Matrix(*otro.fDefault);
  }

//! @brief Operador asignación.
XC::SectionForceDeformation &XC::SectionForceDeformation::operator=(const SectionForceDeformation &otro)
  {
    Material::operator=(otro);
    if(fDefault) delete fDefault;
    if(otro.fDefault) fDefault= new Matrix(*otro.fDefault);
    material_loader= otro.material_loader;
    return *this;
  }

//! @brief Lee un objeto SectionForceDeformation desde archivo
bool XC::SectionForceDeformation::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(SectionForceDeformation) Procesando comando: " << cmd << std::endl;
    if(cmd == "set_trial_section_deformation")
      {
        std::clog << "El comando: " << cmd << " está pensado para pruebas." << std::endl;
        const Vector def= interpreta_xc_vector(status.GetString());
        setTrialSectionDeformation(def);
        return true;
      }
    else if(cmd == "get_stress_resultant")
      {
        status.GetBloque(); //Ignoramos argumentos.
        getStressResultant();
        return true;
      }
    else if(cmd == "get_section_tangent")
      {
        status.GetBloque(); //Ignoramos argumentos.
        ptr_section_tangent= &getSectionTangent();
        return true;
      }
    else if(cmd == "get_initial_tangent")
      {
        status.GetBloque(); //Ignoramos argumentos.
        ptr_initial_tangent= &getInitialTangent();
        return true;
      }
    else
      return Material::procesa_comando(status);
  }

XC::SectionForceDeformation::~SectionForceDeformation(void)
  {
    if(fDefault) delete fDefault;
    fDefault= nullptr;
  }

//! @brief Devuelve una cadena de caracteres con los nombres de los esfuerzos a los
//! que la sección aporta rigidez separados por comas.
std::string XC::SectionForceDeformation::getTypeString(void) const
  { return getType().getString(); }

//! @brief Agrega al vector de deformaciones generalizadas inicial el
//! que se pasa como parámetro.
int XC::SectionForceDeformation::addInitialSectionDeformation(const Vector &def)
  { return setInitialSectionDeformation(getInitialSectionDeformation()+def); }



//! @brief Devuelve la matriz de flexibilidad tangente de la sección.
const XC::Matrix &XC::SectionForceDeformation::getSectionFlexibility(void) const
  {
    int order = this->getOrder();
  
    if(!fDefault)
      {                
        fDefault= new Matrix(order,order);
        if(!fDefault)
          {
            std::cerr << "SectionForceDeformation::getSectionFlexibility -- failed to allocate flexibility matrix\n";
             exit(-1);
           }
      }
    const Matrix &k = this->getSectionTangent();
    switch(order)
      {
      case 1:
        if(k(0,0) != 0.0)
          (*fDefault)(0,0) = 1.0/k(0,0);
        break;
      case 2:
        invert2by2Matrix(k,*fDefault);
        break;
      case 3:
        invert3by3Matrix(k,*fDefault);
        break;
      default:
        invertMatrix(order,k,*fDefault);
        break;
      }
    return *fDefault;
  }

//! @brief Devuelve la matriz de flexibilidad inicial de la sección.
const XC::Matrix &XC::SectionForceDeformation::getInitialFlexibility(void) const
  {
    int order = this->getOrder();
    if(!fDefault)
      {                
        fDefault = new Matrix(order,order);
        if(fDefault == 0)
          {
            std::cerr << "XC::SectionForceDeformation::getInitialFlexibility -- failed to allocate flexibility matrix\n";
            exit(-1);
          }
      }
    const Matrix &k = this->getInitialTangent();
  
    switch(order)
      {
      case 1:
        if(k(0,0) != 0.0)
          (*fDefault)(0,0) = 1.0/k(0,0);
        break;
      case 2:
        invert2by2Matrix(k,*fDefault);
        break;
      case 3:
        invert3by3Matrix(k,*fDefault);
        break;
      default:
        invertMatrix(order,k,*fDefault);
        break;
      }
    return *fDefault;
  }

//! @brief Devuelve la densidad (masa por unidad de longitud/area/volumen)
//! de la sección.
double XC::SectionForceDeformation::getRho(void) const
  { return 0.0; }


/*
int 
XC::SectionForceDeformation::setResponse(const std::vector<std::string> &argv, Information &sectInfo)
{
    // deformations
    if ((strcmp(argv[0],"deformations") ==0) || 
        (strcmp(argv[0],"deformation") ==0)) {

        XC::Vector *theVector = new XC::Vector(this->getOrder());
        if (theVector == 0) {
            std::cerr << "WARNING XC::SectionForceDeformation::setResponse() - out of memory\n";
            return -1;
        } 
        sectInfo.theVector = theVector;
        sectInfo.theType = VectorType;        
        return 1;
    } 

    // stress resultants
    else if ((strcmp(argv[0],"forces") ==0) ||
             (strcmp(argv[0],"force") ==0)) {

        XC::Vector *theVector = new XC::Vector(this->getOrder());
        if (theVector == 0) {
            std::cerr << "WARNING XC::SectionForceDeformation::setResponse() - out of memory\n";
            return -1;
        } 
        sectInfo.theVector = theVector;
        sectInfo.theType = VectorType;        
        return 2;
    } 

        // tangent stiffness
        else if (argv[0] == "stiff" ||
                argv[0] == "stiffness") {
                int order = this->getOrder();
                XC::Matrix *newMatrix = new XC::Matrix(order,order);
                if (newMatrix == 0) {
                        std::cerr << "WARNING XC::SectionForceDeformation::setResponse() - out of memory\n";
                        return -1;
                } 
                sectInfo.theMatrix = newMatrix;
                sectInfo.theType = MatrixType;        
                return 3;
        }

    // otherwise response quantity is unknown for the Section class
    else
        return -1;    
}
*/

//! @brief Devuelve la respuesta de la sección
XC::Response *XC::SectionForceDeformation::setResponse(const std::vector<std::string> &argv, Information &sectInfo)
  {
    // deformations
    if(argv[0] == "deformations" || argv[0] == "deformation")
      return new MaterialResponse(this, 1, this->getSectionDeformation());
    // forces
    else if(argv[0] == "forces" || argv[0] == "force")
      return new MaterialResponse(this, 2, this->getStressResultant());
    // tangent
    else if(argv[0] == "stiff" || argv[0] == "stiffness")
      return new MaterialResponse(this, 3, this->getSectionTangent());
    // force and deformation
    else if (argv[0] == "forceAndDeformation")
      return new MaterialResponse(this, 4, XC::Vector(2*this->getOrder()));
    else
      return 0;
  }

//! @brief Devuelve la respuesta de la sección
int XC::SectionForceDeformation::getResponse(int responseID, Information &secInfo)
  {
    switch (responseID)
      {
      case 1:
        return secInfo.setVector(this->getSectionDeformation());
      case 2:
        return secInfo.setVector(this->getStressResultant());
      case 3:
        return secInfo.setMatrix(this->getSectionTangent());
      case 4:
        {
          Vector &theVec= *(secInfo.theVector);
          const Vector &e = this->getSectionDeformation();
          const Vector &s = this->getStressResultant();
          int order = this->getOrder();
          for(int i = 0; i < order; i++)
            {
              theVec(i) = e(i);
              theVec(i+order) = s(i);
            }
          return secInfo.setVector(theVec);
        }
      default:
        return -1;
      }
  }

//! @brief Devuelve la deformación correspondiente a la posición que se pasa como parámetro.
double XC::SectionForceDeformation::getStrain(const double &,const double &) const
  {
    std::cerr << "No se ha implementado la funcion getStrain para la clase: "
              << nombre_clase() << std::endl;
    return 0.0;
  }

//! @brief Devuelve la componente del vector de deformaciones que
//! corresponde al índice que se pasa como parámetro.
double XC::SectionForceDeformation::getSectionDeformation(const int &defID) const
  {
    double retval= 0.0;
    const int order= getOrder();
    const Vector &e= getSectionDeformation(); //Vector de deformaciones.
    const ResponseId &code= getType();
    for(register int i= 0;i<order;i++)
      if(code(i) == defID)
        retval+= e(i);
    return retval;
  }

//! @brief Devuelve la componente del vector resultante de tensiones que
//! corresponde al índice que se pasa como parámetro.
double XC::SectionForceDeformation::getStressResultant(const int &defID) const
  {
    double retval= 0.0;
    const int order= getOrder();
    const Vector &f= getStressResultant(); //Vector de esfuerzos.
    const ResponseId &code= getType();
    for(register int i= 0;i<order;i++)
      if(code(i) == defID)
        retval+= f(i);
    return retval;
  }

//! Devuelve los esfuerzos de la placa.
double XC::SectionForceDeformation::getStressResultantByName(const std::string &cod) const
  {
    double retval= 0.0;
    if(cod == "n1") //Esfuerzo axil por unidad de longitud, paralelo al eje 1.
      retval= getStressResultant(MEMBRANE_RESPONSE_n1);
    else if(cod == "n2") //Esfuerzo axil por unidad de longitud, paralelo al eje 2.
      retval= getStressResultant(MEMBRANE_RESPONSE_n2);
    else if(cod == "n12")
      retval= getStressResultant(MEMBRANE_RESPONSE_n12);
    else if(cod == "m1") //Flector por unidad de longitud, en torno al eje 1.
      retval= getStressResultant(PLATE_RESPONSE_m1);
    else if(cod == "m2") //Flector por unidad de longitud, en torno al eje 2.
      retval= getStressResultant(PLATE_RESPONSE_m2);
    else if(cod == "m12")
      retval= getStressResultant(PLATE_RESPONSE_m12);
    else if(cod == "q13")
      retval= getStressResultant(PLATE_RESPONSE_q13);
    else if(cod == "q23")
      retval= getStressResultant(PLATE_RESPONSE_q23);
    else if(cod == "P" || cod == "N") //Esfuerzo axil (barra prinsmatica).
      retval= getStressResultant(SECTION_RESPONSE_P);
    else if(cod == "Mz") //Flector en torno al eje z.
      retval= getStressResultant(SECTION_RESPONSE_MZ);
    else if(cod == "My") //Flector en torno al eje y.
      retval= getStressResultant(SECTION_RESPONSE_MY);
    else if(cod == "Vz")
      retval= getStressResultant(SECTION_RESPONSE_VZ);
    else if(cod == "Vy")
      retval= getStressResultant(SECTION_RESPONSE_VY);
    else if(cod == "T" || cod == "Mx")
      retval= getStressResultant(SECTION_RESPONSE_T);

    return retval;
  }

//! @brief Returns section's deformation.
double XC::SectionForceDeformation::getSectionDeformationByName(const std::string &cod) const
  {
    double retval= 0.0;
    if(cod == "defn1")
      retval= getSectionDeformation(MEMBRANE_RESPONSE_n1);
    else if(cod == "defn2")
      retval= getSectionDeformation(MEMBRANE_RESPONSE_n2);
    else if(cod == "defm1") //Flector en torno al eje 1.
      retval= getSectionDeformation(PLATE_RESPONSE_m1);
    else if(cod == "defm2") //Flector en torno al eje 2.
      retval= getSectionDeformation(PLATE_RESPONSE_m2);
    else if(cod == "defq13")
      retval= getSectionDeformation(PLATE_RESPONSE_q13);
    else if(cod == "defq23")
      retval= getSectionDeformation(PLATE_RESPONSE_q23);
    else if(cod == "defm12")
      retval= getSectionDeformation(PLATE_RESPONSE_m12);
    else if(cod == "defn12")
      retval= getSectionDeformation(MEMBRANE_RESPONSE_n12);
    else if(cod == "defP" || cod == "defN") //Esfuerzo axil (barra prismática).
      retval= getSectionDeformation(SECTION_RESPONSE_P);
    else if(cod == "defMz") //Giro en torno al eje z.
      retval= getSectionDeformation(SECTION_RESPONSE_MZ);
    else if(cod == "defMy") //Giro en torno al eje y.
      retval= getSectionDeformation(SECTION_RESPONSE_MY);
    else if(cod == "defVz")
      retval= getSectionDeformation(SECTION_RESPONSE_VZ);
    else if(cod == "defVy")
      retval= getSectionDeformation(SECTION_RESPONSE_VY);
    else if(cod == "defT")
      retval= getSectionDeformation(SECTION_RESPONSE_T);
    else
      std::cerr << "Deformation code: '" << cod 
                << "' unknown." << std::endl;
    return retval;
  }

//! Devuelve los esfuerzos de la placa.
any_const_ptr XC::SectionForceDeformation::GetPropShellResponse(const std::string &cod) const
  {
    std::cerr << "Deprecated. Use python." << std::endl;
    return any_const_ptr();
  }

//! @brief Envía los datos a través del canal que se pasa como parámetro.
int XC::SectionForceDeformation::sendData(CommParameters &cp)
  {
    setDbTagDataPos(0,getTag());
    int res= cp.sendMatrixPtr(fDefault,getDbTagData(),MatrixCommMetaData(1,2,3,4));
    return res;
  }

//! @brief Recibe los datos a través del canal que se pasa como parámetro.
int XC::SectionForceDeformation::recvData(const CommParameters &cp)
  {
    setTag(getDbTagDataPos(0));
    fDefault= cp.receiveMatrixPtr(fDefault,getDbTagData(),MatrixCommMetaData(1,2,3,4));
    return 0;
  }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
any_const_ptr XC::SectionForceDeformation::GetProp(const std::string &cod) const
  {
    if(verborrea>4)
      std::clog << "SectionForceDeformation::GetProp (" << nombre_clase() << "::GetProp) Buscando propiedad: " << cod << std::endl;

    any_const_ptr tmp= GetPropShellResponse(cod);
    if(!tmp.empty())
      return tmp;
    else if(cod == "getTangentStiffness") //Matriz de rigidez tangente.
      return any_const_ptr(getSectionTangent());
    else if(cod == "getInitialStiffness") //Matriz de rigidez noval.
      return any_const_ptr(getInitialTangent());
    else if(cod == "Flexibility") //Matriz de flexibilidad noval.
      return any_const_ptr(getSectionFlexibility());
    else if(cod == "initialFlexibility") //Matriz de flexibilidad noval.
      return any_const_ptr(getInitialFlexibility());
    else if(cod == "getTypeString")
      {
        tmp_gp_str= getTypeString();
        return any_const_ptr(tmp_gp_str);
      }
    else if(cod == "epsilon")
      {
        static double y; y= 0.0;
        static double z; z= 0.0;
        if(InterpreteRPN::Pila().size()>1)
          {
            z= convert_to_double(InterpreteRPN::Pila().Pop());
            y= convert_to_double(InterpreteRPN::Pila().Pop());
          }
        else if(InterpreteRPN::Pila().size()>0)
          y= convert_to_double(InterpreteRPN::Pila().Pop());
        else
          err_num_argumentos(std::cerr,1,"GetProp",cod);
        tmp_gp_dbl= getStrain(y,z);
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod == "getStressResultant")
      {
        return get_prop_vector(getStressResultant());
      }
    else if(cod == "getSectionDeformation")
      {
        return get_prop_vector(getSectionDeformation());
      }
    else
      return Material::GetProp(cod);
  }


// AddingSensitivity:BEGIN ////////////////////////////////////////
int XC::SectionForceDeformation::setParameter(const std::vector<std::string> &argv, Parameter &param)
  { return -1; }

int XC::SectionForceDeformation::updateParameter(int responseID, Information &eleInformation)
  { return -1; }

int XC::SectionForceDeformation::activateParameter(int parameterID)
  { return -1; }

const XC::Vector &XC::SectionForceDeformation::getStressResultantSensitivity(int gradNumber, bool conditional)
  {
    static Vector dummy(1);
    return dummy;
  }

const XC::Vector &XC::SectionForceDeformation::getSectionDeformationSensitivity(int gradNumber)
  {
    static Vector dummy(1);
    return dummy;
  }

const XC::Matrix &XC::SectionForceDeformation::getSectionTangentSensitivity(int gradNumber)
  {
    static XC::Matrix dummy(1,1);
    return dummy;
  }

double XC::SectionForceDeformation::getRhoSensitivity(int gradNumber)
  { return 0.0; }

int XC::SectionForceDeformation::commitSensitivity(const XC::Vector& defSens, int gradNumber, int numGrads)
  { return -1; }
// AddingSensitivity:END ///////////////////////////////////////////
