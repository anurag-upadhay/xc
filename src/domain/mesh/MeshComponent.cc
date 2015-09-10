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
//MeshComponent.cc

#include "MeshComponent.h"
#include "utility/matrix/Matrix.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/base/utils_any.h"
#include "utility/actor/actor/MovableID.h"

void XC::MeshComponent::check_matrices(const std::deque<Matrix> &matrices,const int &ndof) const
  {
    const size_t numMatrices= matrices.size();
    for(size_t i=0; i<numMatrices; i++)
      if(matrices[i].noRows() == ndof)
        {
          index = i;
          i = numMatrices;
        }
  }

void XC::MeshComponent::nueva_matriz(std::deque<Matrix> &matrices,const int &ndof) const
  {
    const size_t numMatrices= matrices.size();
    matrices.push_back(Matrix(ndof,ndof));
    index = numMatrices;
  }

void XC::MeshComponent::setup_matrices(std::deque<Matrix> &matrices,const int &ndof) const
  {
    check_matrices(matrices,ndof);
    if(index == -1)
      nueva_matriz(matrices,ndof);
  }

XC::MeshComponent::MeshComponent(int classTag)
  : ContinuaReprComponent(0,classTag), index(-1){}

XC::MeshComponent::MeshComponent(int tag, int classTag)
  : ContinuaReprComponent(tag,classTag), index(-1){}

//! @brief Envía las etiquetas por el canal que se pasa como parámetro.
int XC::MeshComponent::sendIdsEtiquetas(int posDbTag,CommParameters &cp)
  {
    int res= 0;
    static ID etiqIds;
    const std::set<int> ids= etiquetas.getIdsEtiquetas();
    const size_t sz= ids.size();
    if(sz>0)
      {
        etiqIds.resize(sz);
        int conta= 0;
        for(std::set<int>::const_iterator i= ids.begin();i!=ids.end();i++,conta++)
          etiqIds[conta]= *i;
        res+= cp.sendID(etiqIds,getDbTagData(),CommMetaData(posDbTag));
      }
    else
      setDbTagDataPos(posDbTag,0);
    return res;
  }

//! @brief Recibe las etiquetas por el canal que se pasa como parámetro.
int XC::MeshComponent::recvIdsEtiquetas(int posDbTag,const CommParameters &cp)
  {
    int res= 0;
    static ID etiqIds;
    if(getDbTagDataPos(posDbTag)!= 0)
      {
        const DiccionarioEtiquetas &dic= etiquetas.getDiccionario();
        res= cp.receiveID(etiqIds,getDbTagData(),CommMetaData(posDbTag));
    
        const size_t sz= etiqIds.Size();
        for(size_t i=0;i<sz;i++)
          etiquetas.addEtiqueta(dic(etiqIds[i]));
      }
    return res;
  }

//! @brief Envía los miembros a través del canal que se pasa como parámetro.
int XC::MeshComponent::sendData(CommParameters &cp)
  {
    int res= ContinuaReprComponent::sendData(cp);
    cp.sendInt(index,getDbTagData(),CommMetaData(2));
    res+= sendIdsEtiquetas(3,cp);
    return res;
  }

//! @brief Recibe los miembros a través del canal que se pasa como parámetro.
int XC::MeshComponent::recvData(const CommParameters &cp)
  {
    int res= ContinuaReprComponent::recvData(cp);
    cp.receiveInt(index,getDbTagData(),CommMetaData(2));
    res+= recvIdsEtiquetas(3,cp);
    return res;
  }

//! @brief Lee un objeto XC::MeshComponent desde archivo
bool XC::MeshComponent::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(MeshComponent) Procesando comando: " << cmd << std::endl;
    
    if(cmd == "add_labels") //Agrega una etiqueta al objeto.
      {
        const std::vector<boost::any> &labels= interpretaVectorAny(status.GetBloque());
        for(std::vector<boost::any>::const_iterator i= labels.begin();i!=labels.end();i++)
          etiquetas.addEtiqueta(convert_to_string(*i));
        return true;
      }
    else if(cmd == "remove_labels") //Elimina una etiqueta del objeto.
      {
        const std::vector<boost::any> &labels= interpretaVectorAny(status.GetBloque());
        for(std::vector<boost::any>::const_iterator i= labels.begin();i!=labels.end();i++)
          etiquetas.removeEtiqueta(convert_to_string(*i));
        return true;
      }
    else
      return ContinuaReprComponent::procesa_comando(status);
  }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
any_const_ptr XC::MeshComponent::GetProp(const std::string &cod) const
  {
    if(verborrea>4)
      std::clog << "MeshComponent::GetProp (" << nombre_clase() 
                << "::GetProp) Buscando propiedad: " << cod << std::endl;
    if(cod=="index")
      return any_const_ptr(index);
    else if(cod=="hasLabel")
      {
        const std::string label= popString(cod);
        tmp_gp_bool= etiquetas.hasEtiqueta(label);
        return any_const_ptr(tmp_gp_bool);
      }
    else
      return ContinuaReprComponent::GetProp(cod);
  }