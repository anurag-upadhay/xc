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
// $Date: 2005/12/06 22:11:37 $
// $Source: /usr/local/cvs/OpenSees/SRC/system_of_eqn/linearSOE/sparseGEN/SparseGenColLinSOE.h,v $
                                                                        
                                                                        
#ifndef SparseGenColLinSOE_h
#define SparseGenColLinSOE_h

// Written: fmk 
// Created: 04/98
// Revision: A
//
// Description: This file contains the class definition for SparseGenColLinSOE
// SparseGenColLinSOE is a subclass of LinearSOE. It stores the matrix equation
// Ax=b using the sparse column-compacted storage scheme for storing the 
// matrix A. 
//
// What: "@(#) SparseGenColLinSOE.h, revA"

#include <solution/system_of_eqn/linearSOE/sparseGEN/SparseGenSOEBase.h>

namespace XC {
class SparseGenColLinSolver;

//! @ingroup SOE
//
//! @brief Clase base para los sistemas de ecuaciones
//! con matriz dispersa por columnas.
class SparseGenColLinSOE : public SparseGenSOEBase
  {
  protected:
    ID rowA;
    ID colStartA;//!< int arrays containing info about coeficientss in A
  protected:
    virtual bool setSolver(LinearSOESolver *);

    friend class SoluMethod;
    friend class FEM_ObjectBroker;
    SparseGenColLinSOE(SoluMethod *);        
    SparseGenColLinSOE(SoluMethod *,int classTag);
    SystemOfEqn *getCopy(void) const;
  public:
    virtual int setSize(Graph &theGraph);
    virtual int addA(const Matrix &, const ID &, double fact = 1.0);
    
    virtual int sendSelf(CommParameters &);
    virtual int recvSelf(const CommParameters &);
#ifdef _PARALLEL_PROCESSING
    friend class SuperLU;    
    friend class ThreadedSuperLU;        
    friend class DistributedSuperLU;        
#else
    friend class SuperLU;    
#endif

  };
inline SystemOfEqn *SparseGenColLinSOE::getCopy(void) const
  { return new SparseGenColLinSOE(*this); }
} // fin namespace XC


#endif
