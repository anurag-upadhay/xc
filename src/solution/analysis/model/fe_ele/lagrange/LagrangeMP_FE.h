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
                                                                        
// $Revision: 1.3 $
// $Date: 2005/11/28 21:38:40 $
// $Source: /usr/local/cvs/OpenSees/SRC/analysis/model/fe_ele/lagrange/LagrangeMP_FE.h,v $
                                                                        
                                                                        
// File: ~/analysis/model/fe_ele/lagrange/LagrangeMP_FE.h
// 
// Written: fmk 
// Created: 02/99
// Revision: A
//
// Description: This file contains the class definition for LagrangeMP_FE.
// LagrangeMP_FE is a subclass of FE_Element which handles MP_Constraints
// using the Lagrange method.
//
// What: "@(#) LagrangeMP_FE.h, revA"


#ifndef LagrangeMP_FE_h
#define LagrangeMP_FE_h

#include "solution/analysis/model/fe_ele/MP_FE.h"
#include "utility/matrix/Matrix.h"
#include "utility/matrix/Vector.h"
#include "Lagrange_FE.h"

namespace XC {
class Element;
class Integrator;
class AnalysisModel;
class Domain;
class DOF_Group;

//! @ingroup AnalisisFE
//
//! @brief LagrangeMP_FE is a subclass of FE_Element
//! which handles MP_Constraints using the Lagrange method.
 class LagrangeMP_FE: public MP_FE, public Lagrange_FE
  {
  private:
    void determineTangent(void);
    
    friend class AnalysisModel;
    LagrangeMP_FE(int tag, Domain &theDomain, MP_Constraint &theMP, 
		  DOF_Group &theDofGrp, double alpha = 1.0);
  public:

    // public methods
    virtual int setID(void);
    virtual const Matrix &getTangent(Integrator *theIntegrator);    
    virtual const Vector &getResidual(Integrator *theIntegrator);    
    virtual const Vector &getTangForce(const Vector &x, double fact = 1.0);

    virtual const Vector &getK_Force(const Vector &x, double fact = 1.0);
    virtual const Vector &getC_Force(const Vector &x, double fact = 1.0);
    virtual const Vector &getM_Force(const Vector &x, double fact = 1.0);
  };
} // fin namespace XC

#endif

