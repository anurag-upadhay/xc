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
// $Date: 2003/02/25 23:32:40 $
// $Source: domain/load/groundMotion/GroundMotionRecord.h,v $
                                                                        
                                                                        
// File: ~/earthquake/GroundMotionRecord.h
// 
// Written: MHS
// Created: 09/99
// Revision: A
//
// Description: This file contains the class definition for 
// a GroundMotionRecord, which creates a GroundMotion from
// user supplied data contained in an input file.
//
// What: "@(#) GroundMotionRecord.h, revA"

#ifndef GroundMotionRecord_h
#define GroundMotionRecord_h

#include "GroundMotion.h"
#include "MotionHistory.h"

namespace XC {
//! @ingroup GMotion
//
//! @brief Datos que definen un terremoto real.
class GroundMotionRecord : public GroundMotion
  {
  private:
    MotionHistory history;
  protected:
    bool procesa_comando(CmdStatus &status);
  public:
    GroundMotionRecord(void);
    GroundMotionRecord(const std::string &fileNameAccel, double timeStep, double fact = 1.0, double dTintegration = 0.01);
    GroundMotionRecord(const std::string &fileNameAccel, const std::string &fileNameTime, double fact = 1.0, double dTintegration = 0.01);
    // ADD CONSTRUCTORS FOR OTHER INPUT FORMATS
    virtual GroundMotion *getCopy(void) const;

    virtual double getDuration(void) const;

    virtual double getPeakAccel(void) const;
    virtual double getPeakVel(void) const;
    virtual double getPeakDisp(void) const;

    virtual double getAccel(double time) const;    
    virtual double getVel(double time) const;    
    virtual double getDisp(double time) const;

    inline MotionHistory &getHistory(void)
      { return history; }        
    
    int sendSelf(CommParameters &);
    int recvSelf(const CommParameters &);
  
    // AddingSensitivity:BEGIN //////////////////////////////////////////
    virtual double getAccelSensitivity(double time);
    virtual int setParameter(const std::vector<std::string> &argv, Parameter &param);
    virtual int updateParameter(int parameterID, Information &info);
    virtual int activateParameter(int parameterID);
    // AddingSensitivity:END ///////////////////////////////////////////
  };
} // fin namespace XC

#endif