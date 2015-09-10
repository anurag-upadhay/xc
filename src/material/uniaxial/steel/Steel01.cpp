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

// $Revision: 1.14 $
// $Date: 2005/01/25 21:55:36 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/uniaxial/Steel01.cpp,v $

// Written: MHS
// Created: 06/99
// Revision: A
//
// Description: This file contains the class implementation for
// Steel01.
//
// What: "@(#) Steel01.C, revA"


#include <material/uniaxial/steel/Steel01.h>
#include "domain/component/Parameter.h"
#include <utility/matrix/Vector.h>
#include <utility/matrix/Matrix.h>

#include <domain/mesh/element/Information.h>
#include <cmath>
#include <cfloat>
#include "xc_utils/src/base/CmdStatus.h"
#include "utility/actor/actor/MovableVector.h"
#include "utility/actor/actor/MovableMatrix.h"
#include "utility/actor/actor/MatrixCommMetaData.h"

//! @brief Sets all history and state variables to initial values
int XC::Steel01::setup_parameters(void)
  {
    // History variables
    CminStrain= 0.0;
    CmaxStrain= 0.0;
    CshiftP= 1.0;
    CshiftN= 1.0;
    Cloading= 0;

    TminStrain= 0.0;
    TmaxStrain= 0.0;
    TshiftP= 1.0;
    TshiftN= 1.0;
    Tloading= 0;

    // State variables
    Cstrain= 0.0;
    Cstress= 0.0;
    Ctangent= E0;

    Tstrain= 0.0;
    Tstress= 0.0;
    Ttangent= E0;
    return 0;
  }

//! @brief Libera la memoria reservada para SHVs.
void XC::Steel01::libera(void)
  {
    if(SHVs)
      {
        delete SHVs;
        SHVs= nullptr;
      }
  }

//! @brief Asigna SHVs.
void XC::Steel01::alloc(const Matrix &m)
  {
    libera();
    SHVs= new Matrix(m);
  }

//! @brief Constructor.
XC::Steel01::Steel01(int tag, double FY, double E, double B,double A1, double A2, double A3, double A4)
  : SteelBase0103(tag,MAT_TAG_Steel01,FY,E,B,A1,A2,A3,A4), parameterID(0), SHVs(nullptr)
  {
    // Sets all history and state variables to initial values
    setup_parameters();
  }

//! @brief Constructor.
XC::Steel01::Steel01(int tag)
  :SteelBase0103(tag,MAT_TAG_Steel01), parameterID(0), SHVs(nullptr) {}

//! @brief Constructor.
XC::Steel01::Steel01(void)
  :SteelBase0103(MAT_TAG_Steel01), parameterID(0), SHVs(nullptr) {}

//! @brief Constructor de copia.
XC::Steel01::Steel01(const Steel01 &otro)
  :SteelBase0103(otro), parameterID(otro.parameterID), SHVs(nullptr) 
  {
    if(otro.SHVs)
      alloc(*otro.SHVs);
  }

//! @brief Operador asignación.
XC::Steel01 &XC::Steel01::operator=(const Steel01 &otro)
  {
    SteelBase0103::operator=(otro);
    parameterID= otro.parameterID;
    if(otro.SHVs)
      alloc(*otro.SHVs);
    return *this;
  }

//! @brief Lee un objeto Steel01 desde archivo
bool XC::Steel01::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(Steel01) Procesando comando: " << cmd << std::endl;
    return SteelBase0103::procesa_comando(status);
  }

XC::Steel01::~Steel01(void)
  { libera(); }

//! @brief Calculates the trial state variables based on the trial strain
void XC::Steel01::determineTrialState(double dStrain)
  {
    const double fyOneMinusB= fy * (1.0 - b);
    const double Esh= getEsh();
    const double epsy= getEpsy();

    const double c1= Esh*Tstrain;
    const double c2= TshiftN*fyOneMinusB;
    const double c3= TshiftP*fyOneMinusB;
    const double c= Cstress + E0*dStrain;

//     /**********************************************************
//        removal of the following lines due to problems with
//        optimization may be required (e.g. on gnucc compiler
//        with optimization turned on & -ffloat-store option not
//        used) .. replace them with line that follows but which
//        now requires 2 function calls to achieve same result !!
//     ************************************************************/

//     const double c1c3= c1 + c3;

//     if(c1c3<c)
//       Tstress = c1c3;
//     else
//       Tstress = c;

//     const double c1c2= c1-c2;

//     if(c1c2 > Tstress)
//       Tstress = c1c2;

//     /* ***********************************************************
//     and replace them with:
//     Tstress = fmax((c1-c2), fmin((c1+c3),c));
//     **************************************************************/
    Tstress= std::max((c1-c2), std::min((c1+c3),c));

    if(fabs(Tstress-c)<DBL_EPSILON)
      Ttangent = E0;
    else
      Ttangent = Esh;

    //
    // Determine if a load reversal has occurred due to the trial strain
    //

    // Determine initial loading condition:  1 = loading (positive strain increment)
                                         // -1 = unloading (negative strain increment)
                                         // 0 initially
    if(Tloading == 0 && dStrain != 0.0)
      {
        if(dStrain > 0.0)
          Tloading = 1;
        else
          Tloading = -1;
      }

    // Transition from loading to unloading, i.e. positive strain increment
    // to negative strain increment
    if(Tloading == 1 && dStrain < 0.0)
      {
        Tloading = -1;
        if(Cstrain > TmaxStrain)
          TmaxStrain = Cstrain;
        TshiftN= 1 + a1*pow((TmaxStrain-TminStrain)/(2.0*a2*epsy),0.8);
      }

    // Transition from unloading to loading, i.e. negative strain increment
    // to positive strain increment
    if(Tloading == -1 && dStrain > 0.0)
      {
        Tloading = 1;
        if(Cstrain < TminStrain)
          TminStrain = Cstrain;
        TshiftP = 1 + a3*pow((TmaxStrain-TminStrain)/(2.0*a4*epsy),0.8);
      }
  }

//! @brief Determines if a load reversal has occurred based on the trial strain
void XC::Steel01::detectLoadReversal(double dStrain)
  {
    // Determine initial loading condition
    if(Tloading == 0 && dStrain != 0.0)
      {
        if(dStrain > 0.0)
          Tloading = 1;
        else
          Tloading = -1;
      }

   const double epsy= getEpsy();

   // Transition from loading to unloading, i.e. positive strain increment
   // to negative strain increment
   if(Tloading == 1 && dStrain < 0.0)
     {
       Tloading = -1;
       if(Cstrain > TmaxStrain)
         TmaxStrain = Cstrain;
       TshiftN= 1 + a1*pow((TmaxStrain-TminStrain)/(2.0*a2*epsy),0.8);
     }

   // Transition from unloading to loading, i.e. negative strain increment
   // to positive strain increment
   if(Tloading == -1 && dStrain > 0.0)
     {
       Tloading = 1;
       if(Cstrain < TminStrain)
         TminStrain = Cstrain;
       TshiftP = 1 + a3*pow((TmaxStrain-TminStrain)/(2.0*a4*epsy),0.8);
     }
  }

int XC::Steel01::revertToStart(void)
  {
    SteelBase0103::revertToStart();
// AddingSensitivity:BEGIN /////////////////////////////////
    if(SHVs) SHVs->Zero();
// AddingSensitivity:END //////////////////////////////////
    return 0;
  }

//! @brief Constructor virtual.
XC::UniaxialMaterial* XC::Steel01::getCopy(void) const
  { return new Steel01(*this); }

//! @brief Envía los miembros del objeto a través del canal que se pasa como parámetro.
int XC::Steel01::sendData(CommParameters &cp)
  {
    int res= SteelBase0103::sendData(cp);
    const double pid= parameterID;
    res+= cp.sendDouble(pid,getDbTagData(),CommMetaData(8));
    res+= cp.sendMatrixPtr(SHVs,getDbTagData(),MatrixCommMetaData(9,10,11,12));
    return res;
  }

//! @brief Recibe los miembros del objeto a través del canal que se pasa como parámetro.
int XC::Steel01::recvData(const CommParameters &cp)
  {
    int res= SteelBase0103::recvData(cp);
    double pid;
    res+= cp.receiveDouble(pid,getDbTagData(),CommMetaData(7));
    parameterID= pid;
    libera();
    SHVs= cp.receiveMatrixPtr(SHVs,getDbTagData(),MatrixCommMetaData(9,10,11,12));
    return res;
  }

//! @brief Envía el objeto a través del canal que se pasa como parámetro.
int XC::Steel01::sendSelf(CommParameters &cp)
  {
    setDbTag(cp);
    const int dataTag= getDbTag();
    inicComm(13);
    int res= sendData(cp);

    res+= cp.sendIdData(getDbTagData(),dataTag);
    if(res < 0)
      std::cerr << nombre_clase() << "sendSelf() - failed to send data\n";
    return res;
  }

//! @brief Recibe el objeto a través del canal que se pasa como parámetro.
int XC::Steel01::recvSelf(const CommParameters &cp)
  {
    inicComm(13);
    const int dataTag= getDbTag();
    int res= cp.receiveIdData(getDbTagData(),dataTag);

    if(res<0)
      std::cerr << nombre_clase() << "::recvSelf - failed to receive ids.\n";
    else
      {
        //setTag(getDbTagDataPos(0));
        res+= recvData(cp);
        if(res<0)
          std::cerr << nombre_clase() << "::recvSelf - failed to receive data.\n";
      }
    return res;
  }

//! @brief Imprime el objeto.
void XC::Steel01::Print(std::ostream& s, int flag)
  {
    s << "Steel01 tag: " << this->getTag() << std::endl;
    s << "  fy: " << fy << " ";
    s << "  E0: " << E0 << " ";
    s << "  b:  " << b << " ";
    s << "  a1: " << a1 << " ";
    s << "  a2: " << a2 << " ";
    s << "  a3: " << a3 << " ";
    s << "  a4: " << a4 << " ";
  }

// AddingSensitivity:BEGIN ///////////////////////////////////
int XC::Steel01::setParameter(const std::vector<std::string> &argv, Parameter &param)
  {
    const size_t argc= argv.size();
    if(argc < 1) return -1;
    if(argv[0] == "sigmaY" || argv[0] == "fy")
      return param.addObject(1, this);
    if(argv[0] == "E")
      return param.addObject(2, this);
    if(argv[0] == "b")
      return param.addObject(3, this);
    if(argv[0] == "a1")
      return param.addObject(4, this);
    if(argv[0] == "a2")
      return param.addObject(5, this);
    if(argv[0] == "a3")
      return param.addObject(6, this);
    if(argv[0] == "a4")
      return param.addObject(7, this);
    else
      std::cerr << "WARNING: Could not set parameter in XC::Steel01. " << std::endl;
    return -1;
  }


int XC::Steel01::updateParameter(int parameterID, Information &info)
  {
    switch (parameterID)
      {
      case -1:
        return -1;
      case 1:
        this->fy= info.theDouble;
        break;
      case 2:
        this->E0 = info.theDouble;
        break;
      case 3:
        this->b = info.theDouble;
        break;
      case 4:
        this->a1 = info.theDouble;
        break;
      case 5:
        this->a2 = info.theDouble;
        break;
      case 6:
        this->a3 = info.theDouble;
        break;
      case 7:
        this->a4 = info.theDouble;
        break;
      default:
        return -1;
      }
    Ttangent = E0;          // Initial stiffness
    return 0;
  }




int XC::Steel01::activateParameter(int passedParameterID)
  {
    parameterID = passedParameterID;
    return 0;
  }



double XC::Steel01::getStressSensitivity(int gradNumber, bool conditional)
  {
    // Initialize return value
    double gradient = 0.0;

    // Pick up sensitivity history variables
    double CstrainSensitivity = 0.0;
    double CstressSensitivity = 0.0;
    if(SHVs)
      {
        CstrainSensitivity= (*SHVs)(0,(gradNumber-1));
        CstressSensitivity= (*SHVs)(1,(gradNumber-1));
      }

    // Assign values to parameter derivatives (depending on what's random)
    double fySensitivity = 0.0;
    double E0Sensitivity = 0.0;
    double bSensitivity = 0.0;
    if(parameterID == 1)
      { fySensitivity = 1.0; }
    else if(parameterID == 2)
      { E0Sensitivity = 1.0; }
    else if(parameterID == 3)
      { bSensitivity = 1.0; }

    // Compute min and max stress
    double Tstress;
    const double dStrain = Tstrain-Cstrain;
    const double sigmaElastic = Cstress + E0*dStrain;
    const double fyOneMinusB = fy * (1.0 - b);
    const double Esh = b*E0;
    const double c1 = Esh*Tstrain;
    const double c2 = TshiftN*fyOneMinusB;
    const double c3 = TshiftP*fyOneMinusB;
    const double sigmaMax = c1+c3;
    const double sigmaMin = c1-c2;

    // Evaluate stress sensitivity
    if( (sigmaMax < sigmaElastic) && (fabs(sigmaMax-sigmaElastic)>1e-5) )
      {
        Tstress = sigmaMax;
        gradient = E0Sensitivity*b*Tstrain
                   + E0*bSensitivity*Tstrain
                   + TshiftP*(fySensitivity*(1-b)-fy*bSensitivity);
      }
    else
      {
        Tstress = sigmaElastic;
        gradient = CstressSensitivity
                   + E0Sensitivity*(Tstrain-Cstrain)
                   - E0*CstrainSensitivity;
      }
    if(sigmaMin > Tstress)
      {
        gradient = E0Sensitivity*b*Tstrain
                   + E0*bSensitivity*Tstrain
                   - TshiftN*(fySensitivity*(1-b)-fy*bSensitivity);
      }
    return gradient;
  }


double XC::Steel01::getInitialTangentSensitivity(int gradNumber)
  {
    // For now, assume that this is only called for initial stiffness
    if(parameterID == 2)
      { return 1.0; }
    else
      { return 0.0; }
  }


int XC::Steel01::commitSensitivity(double TstrainSensitivity, int gradNumber, int numGrads)
  {
    if(!SHVs)
      alloc(Matrix(2,numGrads));

    // Initialize unconditaional stress sensitivity
    double gradient= 0.0;

    // Pick up sensitivity history variables
    double CstrainSensitivity = 0.0;
    double CstressSensitivity         = 0.0;
    if(SHVs)
      {
        CstrainSensitivity= (*SHVs)(0,(gradNumber-1));
        CstressSensitivity= (*SHVs)(1,(gradNumber-1));
      }

    // Assign values to parameter derivatives (depending on what's random)
    double fySensitivity = 0.0;
    double E0Sensitivity = 0.0;
    double bSensitivity = 0.0;
    if(parameterID == 1)
      { fySensitivity = 1.0; }
    else if(parameterID == 2)
      { E0Sensitivity = 1.0; }
    else if(parameterID == 3)
      { bSensitivity = 1.0; }

    // Compute min and max stress
    double Tstress;
    const double dStrain = Tstrain-Cstrain;
    const double sigmaElastic = Cstress + E0*dStrain;
    const double fyOneMinusB = fy * (1.0 - b);
    const double Esh = b*E0;
    const double c1 = Esh*Tstrain;
    const double c2 = TshiftN*fyOneMinusB;
    const double c3 = TshiftP*fyOneMinusB;
    const double sigmaMax = c1+c3;
    const double sigmaMin = c1-c2;

    // Evaluate stress sensitivity ('gradient')
    if( (sigmaMax < sigmaElastic) && (fabs(sigmaMax-sigmaElastic)>1e-5) )
      {
        Tstress = sigmaMax;
        gradient = E0Sensitivity*b*Tstrain
                   + E0*bSensitivity*Tstrain
                   + E0*b*TstrainSensitivity
                   + TshiftP*(fySensitivity*(1-b)-fy*bSensitivity);
      }
    else
      {
        Tstress = sigmaElastic;
        gradient = CstressSensitivity
                   + E0Sensitivity*(Tstrain-Cstrain)
                   + E0*(TstrainSensitivity-CstrainSensitivity);
      }
    if(sigmaMin > Tstress)
      {
        gradient = E0Sensitivity*b*Tstrain
                   + E0*bSensitivity*Tstrain
                   + E0*b*TstrainSensitivity
                   - TshiftN*(fySensitivity*(1-b)-fy*bSensitivity);
      }

    // Commit history variables
    (*SHVs)(0,(gradNumber-1)) = TstrainSensitivity;
    (*SHVs)(1,(gradNumber-1)) = gradient;
    return 0;
  }

// AddingSensitivity:END /////////////////////////////////////////////
