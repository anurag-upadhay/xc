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
//PulseBaseSeries.cpp

#include "PulseBaseSeries.h"
#include <utility/matrix/Vector.h>
#include <classTags.h>
#include <cmath>
#include "utility/actor/actor/MovableVector.h"
#include "utility/matrix/ID.h"



//! @brief Constructor.
XC::PulseBaseSeries::PulseBaseSeries(int classTag,const double &startTime,const double &finishTime,const double &factor)
  : CFactorSeries(classTag,factor), tStart(startTime),tFinish(finishTime) {}

//! @brief Send members del objeto through the channel being passed as parameter.
int XC::PulseBaseSeries::sendData(CommParameters &cp)
  {
    int res= CFactorSeries::sendData(cp);
    res+= cp.sendDoubles(tStart,tFinish,getDbTagData(),CommMetaData(1));
    return res;
  }

//! @brief Receives members del objeto through the channel being passed as parameter.
int XC::PulseBaseSeries::recvData(const CommParameters &cp)
  {
    int res= CFactorSeries::recvData(cp);
    res+= cp.receiveDoubles(tStart,tFinish,getDbTagData(),CommMetaData(1));
    return res;
  }


//! @brief Sends object through the channel being passed as parameter.
int XC::PulseBaseSeries::sendSelf(CommParameters &cp)
  {
    inicComm(2);
    int result= sendData(cp);

    const int dataTag= getDbTag();
    result+= cp.sendIdData(getDbTagData(),dataTag);
    if(result < 0)
      std::cerr << "PulseBaseSeries::sendSelf() - ch failed to send data\n";
    return result;
  }

//! @brief Receives object through the channel being passed as parameter.
int XC::PulseBaseSeries::recvSelf(const CommParameters &cp)
  {
    inicComm(2);

    const int dataTag = this->getDbTag();  
    int result = cp.receiveIdData(getDbTagData(),dataTag);
    if(result<0)
      std::cerr << "PulseBaseSeries::sendSelf() - ch failed to receive data\n";
    else
      result+= recvData(cp);
    return result;    
  }

//! @brief Imprime el objeto.
void XC::PulseBaseSeries::Print(std::ostream &s, int flag) const
  {
    s << "Pulse Series" << std::endl;
    s << "\tFactor: " << cFactor << std::endl;
    s << "\ttStart: " << tStart << std::endl;
    s << "\ttFinish: " << tFinish << std::endl;
  }
