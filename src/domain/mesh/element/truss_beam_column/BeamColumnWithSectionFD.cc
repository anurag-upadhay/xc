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
//BeamColumnWithSectionFD.cc

#include "BeamColumnWithSectionFD.h"


#include "utility/matrix/Vector.h"
#include "utility/matrix/Matrix.h"
#include <material/section/SeccionBarraPrismatica.h>
#include <utility/recorder/response/ElementResponse.h>


XC::BeamColumnWithSectionFD::BeamColumnWithSectionFD(int tag, int classTag,const size_t &numSecc)
  : Element1D(tag,classTag), theSections(numSecc)
  {}

XC::BeamColumnWithSectionFD::BeamColumnWithSectionFD(int tag, int classTag,const size_t &numSecc= 1,const Material *m= nullptr)
  : Element1D(tag,classTag), theSections(numSecc,m)
  {}

XC::BeamColumnWithSectionFD::BeamColumnWithSectionFD(int tag, int classTag,const size_t &numSecc,const SeccionBarraPrismatica *matModel)
  : Element1D(tag,classTag), theSections(numSecc,matModel)
  {}

XC::BeamColumnWithSectionFD::BeamColumnWithSectionFD(int tag, int classTag,const size_t &numSecc,const SeccionBarraPrismatica *sccModel,int Nd1,int Nd2)
  : Element1D(tag,classTag,Nd1,Nd2), theSections(numSecc,sccModel)
  {}


int XC::BeamColumnWithSectionFD::commitState(void)
  {
    int retVal = 0;

    if((retVal= Element1D::commitState()) != 0)
      {
        std::cerr << "EightNodeBrick-XC::u_p_U::commitState () - failed in base class";
        return (-1);
      }
    retVal+= theSections.commitState();
    return retVal;
  }

int XC::BeamColumnWithSectionFD::revertToLastCommit(void)
  {
    int retval= theSections.revertToLastCommit();
    return retval;
  }

int XC::BeamColumnWithSectionFD::revertToStart(void)
  {
    int retval= Element1D::revertToStart();
    retval+= theSections.revertToStart();
    return retval;
  }

void XC::BeamColumnWithSectionFD::setSection(const SeccionBarraPrismatica *matModel)
  { theSections.setSection(matModel); }

bool XC::BeamColumnWithSectionFD::setSections(const std::vector<SeccionBarraPrismatica *> &sectionPtrs)
  { return theSections.setSections(sectionPtrs); }
void XC::BeamColumnWithSectionFD::setTrialSectionDeformations(const std::vector<Vector> &vs)
  { return theSections.setTrialSectionDeformations(vs); }

//! @brief Devuelve un puntero a la sección del elemento cuyo índice se pasa como parámetro.
const XC::SeccionBarraPrismatica *XC::BeamColumnWithSectionFD::getSectionPtr(const size_t &i) const
  { return theSections[i]; }

XC::Response* XC::BeamColumnWithSectionFD::setSectionResponse(SeccionBarraPrismatica *theSection,const std::vector<std::string> &argv,const size_t &offset, Information &info)
  {
    Response *retval= nullptr;
    if(theSection)
      {
        std::vector<std::string> argvOffset(argv);
        argvOffset.erase(argvOffset.begin(),argvOffset.begin()+offset);
        retval= theSection->setResponse(argvOffset,info);
      }
    return retval;
  }

int XC::BeamColumnWithSectionFD::setSectionParameter(SeccionBarraPrismatica *theSection,const std::vector<std::string> &argv,const size_t &offset, Parameter &param)
  {
    int retval= -1;
    if(theSection)
      {
        std::vector<std::string> argvOffset(argv);
        argvOffset.erase(argvOffset.begin(),argvOffset.begin()+offset);
        retval= theSection->setParameter(argvOffset,param);
      }
    return retval;
  }

//! @brief Send members through the channel being passed as parameter.
int XC::BeamColumnWithSectionFD::sendData(CommParameters &cp)
  {
    int res= Element1D::sendData(cp);
    res+= cp.sendMovable(theSections,getDbTagData(),CommMetaData(7));
    res+= cp.sendMovable(section_matrices,getDbTagData(),CommMetaData(8));
    return res;
  }

//! @brief Receives members through the channel being passed as parameter.
int XC::BeamColumnWithSectionFD::recvData(const CommParameters &cp)
  {
    int res= Element1D::recvData(cp);
    res+= cp.receiveMovable(theSections,getDbTagData(),CommMetaData(7));
    res+= cp.receiveMovable(section_matrices,getDbTagData(),CommMetaData(8));
    return res;
  }


