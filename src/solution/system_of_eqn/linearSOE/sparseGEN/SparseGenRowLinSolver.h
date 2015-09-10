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
                                                                        
#ifndef SparseGenRowLinSolver_h
#define SparseGenRowLinSolver_h

// $Revision: 1.1 $
// $Date: 2005/04/08 02:38:18 $
// $Source: /usr/local/cvs/OpenSees/SRC/system_of_eqn/linearSOE/sparseGEN/SparseGenRowLinSolver.h,v $

// Written: fmk 
// Created: 04/05
// Revision: A
//
// Description: This file contains the class definition for SparseGenRowLinSolver.
// SparseGenRowLinSolver is an abstract base class and thus no objects of it's type
// can be instantiated. It has pure virtual functions which must be
// implemented in it's derived classes.  Instances of SparseGenRowLinSolver 
// are used to solve a system of equations of type SparseGenRowLinSOE.
//
// What: "@(#) SparseGenRowLinSolver.h, revA"

#include <solution/system_of_eqn/linearSOE/LinearSOESolver.h>
namespace XC {
class SparseGenRowLinSOE;

//! @ingroup LinearSolver
//
//! @brief Objeto encargado de obtener la solución
//! de un sistemas de ecuaciones lineal con matriz dispersa no simétrica.
class SparseGenRowLinSolver : public LinearSOESolver
  {
  protected:
    SparseGenRowLinSOE *theSOE;
    SparseGenRowLinSolver(int classTag);    
    virtual bool setLinearSOE(LinearSOE *theSOE);
  public:

    virtual bool setLinearSOE(SparseGenRowLinSOE &theSOE);
  };
} // fin namespace XC

#endif
