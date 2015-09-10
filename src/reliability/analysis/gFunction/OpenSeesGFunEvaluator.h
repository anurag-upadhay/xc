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
** (C) Copyright 2001, The Regents of the University of California    **
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
** Reliability module developed by:                                   **
**   Terje Haukaas (haukaas@ce.berkeley.edu)                          **
**   Armen Der Kiureghian (adk@ce.berkeley.edu)                       **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.7 $
// $Date: 2003/10/27 23:45:43 $
// $Source: /usr/local/cvs/OpenSees/SRC/reliability/analysis/gFunction/OpenSeesGFunEvaluator.h,v $


//
// Written by Terje Haukaas (haukaas@ce.berkeley.edu)
//

#ifndef OpenSeesGFunEvaluator_h
#define OpenSeesGFunEvaluator_h

#include "GFunEvaluator.h"
#include <utility/matrix/Vector.h>
#include <reliability/domain/components/ReliabilityDomain.h>
#include <tcl.h>

#include <fstream>
using std::ofstream;


namespace XC {
class OpenSeesGFunEvaluator : public GFunEvaluator
{
private:
	int createRecorders();
	int removeRecorders();
	char *rec_node_occurrence(char tempchar[100], bool createRecorders, int &line, int &column);
	char *rec_element_occurrence(char tempchar[100], bool createRecorders, int &line, int &column);
	std::string fileName;
	int nsteps;
	double dt;

public:
	OpenSeesGFunEvaluator(Tcl_Interp *passedTclInterp,
						ReliabilityDomain *passedReliabilityDomain,
						const std::string &fileName);
	OpenSeesGFunEvaluator(Tcl_Interp *passedTclInterp,
						ReliabilityDomain *passedReliabilityDomain,
						int nsteps, double dt);
	int		runGFunAnalysis(Vector x);
	int		tokenizeSpecials(const std::string &theExpression);

	void    setNsteps(int nsteps);
	double  getDt();
};
} // fin namespace XC

#endif