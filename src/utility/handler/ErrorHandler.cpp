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
                                                                        
// $Revision: 1.4 $
// $Date: 2003/02/14 23:01:24 $
// $Source: /usr/local/cvs/OpenSees/SRC/handler/ErrorHandler.cpp,v $
                                                                        
                                                                        
// File: ~/handler/ErrorHandler.C
//
// Written: fmk 
// Created: 11/99
// Revision: A
//
// Description: This file contains the class implementation for
// ErrorHandler. 
//
// What: "@(#) ErrorHandler.C, revA"

#include "utility/handler/ErrorHandler.h"

XC::ErrorHandler::ErrorHandler(void)
  {}

void XC::ErrorHandler::outputMessage(ostream &theStream, const char *msg, va_list args) 
  {
    int dataInt;
    double dataDouble;
    char *dataString;
    char dataChar;

  int pos =0;
  int done =0;
  
  // parse the msg string until end of string '\0' is
  // encounterd, send to the output stream any character,
  // if we encounter a %d or a %f get the integer or double 
  // from the next arg on the va_list and send it to the stream

  while (done != 1) {

    // if reach string end then we are done
    if (msg[pos] == '\0')
      break;

    // otherwise parse string , looking for special %d and %f
    if (msg[pos] != '%') {
      dataChar = msg[pos];
      theStream << dataChar;
    } else {
      pos++;
      switch(msg[pos]) {
      case 'd':
	dataInt = va_arg(args,int);
	theStream << dataInt;
	break;
	
      case 'f':
	dataDouble = va_arg(args,double);
	theStream << dataDouble;
	break;	

      case 's':
	dataString = va_arg(args,char *);
	theStream << dataString;
	break;	

      default:
	;
      }
     }
    pos++;
  }
  theStream << std::endl;
}