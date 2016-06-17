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

#include "SteelBase0103.h"
#include "domain/component/Parameter.h"
#include <utility/matrix/Vector.h>
#include <utility/matrix/Matrix.h>

#include <domain/mesh/element/Information.h>
#include <cmath>
#include <cfloat>
#include "utility/actor/actor/MovableVector.h"

//! @brief Sets all history and state variables to initial values
int XC::SteelBase0103::setup_parameters(void)
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

//! @brief Constructor.
XC::SteelBase0103::SteelBase0103(int tag, int classTag, double FY, double E, double B,double A1, double A2, double A3, double A4)
  : SteelBase(tag,classTag,FY,E,B,A1,A2,A3,A4) {}

//! @brief Constructor.
XC::SteelBase0103::SteelBase0103(int tag, int classTag)
  :SteelBase(tag,classTag,0.0,0.0,0.0,STEEL_0103_DEFAULT_A1,STEEL_0103_DEFAULT_A2,STEEL_0103_DEFAULT_A3,STEEL_0103_DEFAULT_A4) {}

//! @brief Constructor.
XC::SteelBase0103::SteelBase0103(int classTag)
  :SteelBase(0,classTag,0.0,0.0,0.0,STEEL_0103_DEFAULT_A1,STEEL_0103_DEFAULT_A2,STEEL_0103_DEFAULT_A3,STEEL_0103_DEFAULT_A4) {}

int XC::SteelBase0103::setTrialStrain(double strain, double strainRate)
  {
    if(fabs(strain)>fabs(10.0*getEpsy()))
      std::clog << "¡Ojo!; la deformación del material SteelBase0103 es muy grande: "
                << strain << std::endl;
    // Reset history variables to last converged state
    TminStrain= CminStrain;
    TmaxStrain= CmaxStrain;
    TshiftP= CshiftP;
    TshiftN= CshiftN;
    Tloading= Cloading;

    Tstrain= Cstrain;
    Tstress= Cstress;
    Ttangent= Ctangent;

    // Determine change in strain from last converged state
    const double dStrain= strain - Cstrain;

    if(fabs(dStrain) > DBL_EPSILON)
      {
        // Set trial strain
        Tstrain = strain;
        // Calculate the trial state given the trial strain
        determineTrialState(dStrain);
      }
    return 0;
  }

int XC::SteelBase0103::setTrial(double strain, double &stress, double &tangent, double strainRate)
  {
    setTrialStrain(strain,strainRate);
    stress= Tstress;
    tangent= Ttangent;
    return 0;
  }

double XC::SteelBase0103::getStrain(void) const
  { return Tstrain; }

double XC::SteelBase0103::getStress(void) const
  { return Tstress; }

double XC::SteelBase0103::getTangent(void) const
  { return Ttangent; }

int XC::SteelBase0103::commitState(void)
  {
    // History variables
    CminStrain= TminStrain;
    CmaxStrain= TmaxStrain;
    CshiftP= TshiftP;
    CshiftN= TshiftN;
    Cloading= Tloading;

    // State variables
    Cstrain= Tstrain;
    Cstress= Tstress;
    Ctangent= Ttangent;
    return 0;
  }

//! @brief Reset material to last committed state
int XC::SteelBase0103::revertToLastCommit(void)
  {
    // Reset trial history variables to last committed state
    TminStrain= CminStrain;
    TmaxStrain= CmaxStrain;
    TshiftP= CshiftP;
    TshiftN= CshiftN;
    Tloading= Cloading;

    // Reset trial state variables to last committed state
    Tstrain = Cstrain;
    Tstress = Cstress;
    Ttangent = Ctangent;
    return 0;
  }

int XC::SteelBase0103::revertToStart(void)
  {
    // History variables
    setup_parameters();
    return 0;
  }

//! @brief Send members del objeto through the channel being passed as parameter.
int XC::SteelBase0103::sendData(CommParameters &cp)
  {
    int res= SteelBase::sendData(cp);
    res+= cp.sendDoubles(Cstrain,Cstress,Ctangent,Tstrain,Tstress,Ttangent,getDbTagData(),CommMetaData(4));
    res+= cp.sendDoubles(CminStrain,CmaxStrain,CshiftP,CshiftN,getDbTagData(),CommMetaData(5));
    res+= cp.sendInts(Cloading,Tloading,getDbTagData(),CommMetaData(6));
    res+= cp.sendDoubles(TminStrain,TmaxStrain,TshiftP,TshiftN,getDbTagData(),CommMetaData(7));
    return res;
  }

//! @brief Receives members del objeto through the channel being passed as parameter.
int XC::SteelBase0103::recvData(const CommParameters &cp)
  {
    int res= SteelBase::recvData(cp);
    res+= cp.receiveDoubles(Cstrain,Cstress,Ctangent,Tstrain,Tstress,Ttangent,getDbTagData(),CommMetaData(4));
    res+= cp.receiveDoubles(CminStrain,CmaxStrain,CshiftP,CshiftN,getDbTagData(),CommMetaData(5));
    res+= cp.receiveInts(Cloading,Tloading,getDbTagData(),CommMetaData(6));
    res+= cp.receiveDoubles(TminStrain,TmaxStrain,TshiftP,TshiftN,getDbTagData(),CommMetaData(7));
    return res;
  }

//! @brief Imprime el objeto.
void XC::SteelBase0103::Print(std::ostream& s, int flag)
  {
    s << "SteelBase0103 tag: " << this->getTag() << std::endl;
    s << "  fy: " << fy << " ";
    s << "  E0: " << E0 << " ";
    s << "  b:  " << b << " ";
    s << "  a1: " << a1 << " ";
    s << "  a2: " << a2 << " ";
    s << "  a3: " << a3 << " ";
    s << "  a4: " << a4 << " ";
  }
