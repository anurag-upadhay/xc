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
                                                                        
// $Revision: 1.1.1.1 $
// $Date: 2000/09/15 08:23:16 $
// $Source: /usr/local/cvs/OpenSees/SRC/analysis/algorithm/domainDecompAlgo/DomainDecompAlgo.h,v $
                                                                        
                                                                        
// File: ~/OOP/analysis/algorithm/DomainDecompAlgo.h 
// 
// Written: fmk 
// Created: 10/96 
// Revision: A 
//

// Description: This file contains the class definition for 
// DomainDecompAlgo. DomainDecompAlgo is an abstract base class, 
// i.e. no objects of it's type can be created.  Its subclasses deifine
// the sequence of operations to be performed in the analysis by static
// equilibrium of a finite element model.  
// 
// What: "@(#)DomainDecompAlgo.h, revA"

#ifndef DomainDecompAlgo_h
#define DomainDecompAlgo_h

#include <solution/analysis/algorithm/SolutionAlgorithm.h>

namespace XC {
class AnalysisModel;
class IncrementalIntegrator;
class LinearSOE;
class DomainSolver;
class DomainDecompAnalysis;
class Subdomain;

//! @ingroup AnalAlgo
//
//! @brief Algoritmo de solución por descomposición del dominio.
class DomainDecompAlgo: public SolutionAlgorithm
  {
  protected:
    friend class FEM_ObjectBroker;
    friend class SoluMethod;
    DomainDecompAlgo(SoluMethod *);
    SolutionAlgorithm *getCopy(void) const;
  public:
    // public functions defined for subclasses
    int solveCurrentStep(void);
    int sendSelf(CommParameters &);
    int recvSelf(const CommParameters &);
  };

inline SolutionAlgorithm *DomainDecompAlgo::getCopy(void) const
  { return new DomainDecompAlgo(*this); }

} // fin namespace XC

#endif

