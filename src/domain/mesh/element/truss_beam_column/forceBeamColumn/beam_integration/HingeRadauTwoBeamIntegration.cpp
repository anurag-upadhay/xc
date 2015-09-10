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

// $Revision: 1.1 $
// $Date: 2006/01/17 21:32:35 $
// $Source: /usr/local/cvs/OpenSees/SRC/element/forceBeamColumn/HingeRadauTwoBeamIntegration.cpp,v $

#include <domain/mesh/element/truss_beam_column/forceBeamColumn/beam_integration/HingeRadauTwoBeamIntegration.h>

#include <utility/matrix/Matrix.h>
#include <utility/matrix/Vector.h>
#include <domain/mesh/element/Information.h>
#include "domain/component/Parameter.h"

XC::HingeRadauTwoBeamIntegration::HingeRadauTwoBeamIntegration(double lpi,double lpj)
  : PlasticLengthsBeamIntegration(BEAM_INTEGRATION_TAG_HingeRadauTwo,lpi,lpj) {}

XC::HingeRadauTwoBeamIntegration::HingeRadauTwoBeamIntegration()
  : PlasticLengthsBeamIntegration(BEAM_INTEGRATION_TAG_HingeRadauTwo) {}

void XC::HingeRadauTwoBeamIntegration::getSectionLocations(int numSections, double L, double *xi) const
{
  double oneOverL = 1.0/L;

  xi[0] = 0.0;
  xi[1] = 2.0/3*lpI*oneOverL;
  xi[4] = 1.0-2.0/3*lpJ*oneOverL;
  xi[5] = 1.0;

  double alpha = 0.5-0.5*(lpI+lpJ)*oneOverL;
  double beta  = 0.5+0.5*(lpI-lpJ)*oneOverL;
  xi[2] = alpha*(-1/sqrt(3.0)) + beta;
  xi[3] = alpha*(1/sqrt(3.0)) + beta;

  for (int i = 6; i < numSections; i++)
    xi[i] = 0.0;
}

void XC::HingeRadauTwoBeamIntegration::getSectionWeights(int numSections, double L,double *wt) const
{
  double oneOverL = 1.0/L;

  wt[0] = 0.25*lpI*oneOverL;
  wt[1] = 0.75*lpI*oneOverL;
  wt[4] = 0.75*lpJ*oneOverL;
  wt[5] = 0.25*lpJ*oneOverL;

  wt[2] = 0.5-0.5*(lpI+lpJ)*oneOverL;
  wt[3] = 0.5-0.5*(lpI+lpJ)*oneOverL;

  for (int i = 6; i < numSections; i++)
    wt[i] = 1.0;
}

XC::BeamIntegration *XC::HingeRadauTwoBeamIntegration::getCopy(void) const
  { return new HingeRadauTwoBeamIntegration(*this); }

int XC::HingeRadauTwoBeamIntegration::setParameter(const std::vector<std::string> &argv, Parameter &param)
  {
    if(argv[0] == "lpI")
      return param.addObject(1, this);
    else if(argv[0] == "lpJ")
      return param.addObject(2, this);
    else if(argv[0] == "lp")
      return param.addObject(3, this);
    else 
      return -1;
  }

int XC::HingeRadauTwoBeamIntegration::updateParameter(int parameterID, Information &info)
{
  switch (parameterID) {
  case 1:
    lpI = info.theDouble;
    return 0;
  case 2:
    lpJ = info.theDouble;
    return 0;
  case 3:
    lpI = lpJ = info.theDouble;
    return 0;
  default:
    return -1;
  }
}

int XC::HingeRadauTwoBeamIntegration::activateParameter(int paramID)
  {
    parameterID = paramID;

    // For Terje to do
    return 0;
  }

void XC::HingeRadauTwoBeamIntegration::Print(std::ostream &s, int flag)
{
  s << "HingeRadauTwo" << std::endl;
  s << " lpI = " << lpI;
  s << " lpJ = " << lpJ << std::endl;

  return;
}

void XC::HingeRadauTwoBeamIntegration::getLocationsDeriv(int numSections,double L, double dLdh,double *dptsdh)
{
  double oneOverL = 1.0/L;

  for (int i = 0; i < numSections; i++)
    dptsdh[i] = 0.0;

  if (parameterID == 1) { // lpI
    dptsdh[1] = 2.0/3*oneOverL;
    dptsdh[2] = -0.5*(1.0-1/sqrt(3.0))*oneOverL + oneOverL;
    dptsdh[3] = -0.5*(1.0+1/sqrt(3.0))*oneOverL + oneOverL;
  }

  if (parameterID == 2) { // lpJ
    dptsdh[2] = -0.5*(1.0-1/sqrt(3.0))*oneOverL;
    dptsdh[3] = -0.5*(1.0+1/sqrt(3.0))*oneOverL;
    dptsdh[4] = -2.0/3*oneOverL;
  }

  if (parameterID == 3) { // lpI and lpJ
    dptsdh[1] = 2.0/3*oneOverL;
    dptsdh[2] = -(1.0-1/sqrt(3.0))*oneOverL + oneOverL;
    dptsdh[3] = -(1.0+1/sqrt(3.0))*oneOverL + oneOverL;
    dptsdh[4] = -2.0/3*oneOverL;
  }

  if (dLdh != 0.0) {
    // STILL TO DO
    std::cerr << "getPointsDeriv -- to do" << std::endl;
  }

  return;
}

void XC::HingeRadauTwoBeamIntegration::getWeightsDeriv(int numSections, double L, double dLdh, double *dwtsdh)
{
  double oneOverL = 1.0/L;

  for (int i = 0; i < numSections; i++)
    dwtsdh[i] = 0.0;

  if (parameterID == 1) { // lpI
    dwtsdh[0] = 0.25*oneOverL;
    dwtsdh[1] = 0.75*oneOverL;
    dwtsdh[2] = -0.5*oneOverL;
    dwtsdh[3] = -0.5*oneOverL;
  }

  if (parameterID == 2) { // lpJ
    dwtsdh[2] = -0.5*oneOverL;
    dwtsdh[3] = -0.5*oneOverL;
    dwtsdh[4] = 0.75*oneOverL;
    dwtsdh[5] = 0.25*oneOverL;
  }

  if (parameterID == 3) { // lpI and lpJ
    dwtsdh[0] = 0.25*oneOverL;
    dwtsdh[1] = 0.75*oneOverL;
    dwtsdh[2] = -oneOverL;
    dwtsdh[3] = -oneOverL;
    dwtsdh[4] = 0.75*oneOverL;
    dwtsdh[5] = 0.25*oneOverL;
  }

  if (dLdh != 0.0) {
    dwtsdh[0] = -lpI*dLdh/(L*L);
    dwtsdh[5] = -lpJ*dLdh/(L*L);
    // STILL TO DO
    std::cerr << "getWeightsDeriv -- to do" << std::endl;
  }

  return;
}