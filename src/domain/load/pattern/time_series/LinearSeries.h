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
                                                                        
// $Revision: 1.2 $
// $Date: 2003/02/14 23:01:00 $
// $Source: /usr/local/cvs/OpenSees/SRC/domain/load/pattern/LinearSeries.h,v $
                                                                        
                                                                        
#ifndef LinearSeries_h
#define LinearSeries_h

// File: ~/domain/load/pattern/LinearSeries.h
//
// Written: fmk 
// Created: 07/99
// Revision: A
//
// Purpose: This file contains the class definition for LinearSeries.
// LinearSeries is a concrete class. A LinearSeries object provides
// a linear time series. the factor is given by the pseudoTime and 
// a constant factor provided in the constructor.
//
// What: "@(#) LinearSeries.h, revA"

#include <domain/load/pattern/time_series/CFactorSeries.h>

namespace XC {
//! @ingroup TSeries
//
//! @brief Función lineal en el tiempo.
class LinearSeries : public CFactorSeries
  {
  protected:
    bool procesa_comando(CmdStatus &status);
  public:
    LinearSeries(double cFactor = 1.0);

    TimeSeries *getCopy(void) const
      { return new LinearSeries(*this); }

    // method to get load factor
    double getFactor(double pseudoTime) const;

    // None of the following functions should be invoked on this type
    // of object
    double getDuration(void) const {return 0.0;} // dummy function
    double getPeakFactor(void) const {return cFactor;} // dummy function
    double getTimeIncr(double pseudoTime) const {return 1.0;} // dummy function
    
    void Print(std::ostream &s, int flag =0) const;    
  };
} // fin namespace XC

#endif