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
//ElementBase.h

#ifndef ElementBase_h
#define ElementBase_h

#include "Element.h"
#include "NodePtrsWithIDs.h"
#include "material/Material.h"
#include "domain/domain/Domain.h"
#include "xc_utils/src/geom/pos_vec/Pos3d.h"

namespace XC {
class Node;

//! \ingroup Elem
//
//! @brief Elemento con punteros a nodos
template <int NNODOS>
class ElementBase: public Element
  {
  protected:
    NodePtrsWithIDs theNodes;  //!< Punteros a nodo.

    template <class TIPOMAT>
    TIPOMAT *cast_material(const Material *ptr_mat);

    int sendData(CommParameters &cp);
    int recvData(const CommParameters &cp);

  public:
    ElementBase(int tag, int classTag);
    ElementBase(const ElementBase &otro);
    ElementBase &operator=(const ElementBase &otro);

    static inline const int numNodos(void)
      { return NNODOS; }
    // public methods to obtain inforrmation about dof & connectivity    
    int getNumExternalNodes(void) const;
    NodePtrsWithIDs &getNodePtrs(void);
    const NodePtrsWithIDs &getNodePtrs(void) const;
    const Pos3d &getPosCdg(bool initialGeometry= true) const;
  };



//! @brief Constructor por defecto.
template <int NNODOS>
XC::ElementBase<NNODOS>::ElementBase(int tag, int classTag)
  : Element(tag,classTag), theNodes(this,NNODOS) {}

//! @brief Constructor.
template <int NNODOS>
XC::ElementBase<NNODOS>::ElementBase(const ElementBase<NNODOS> &otro)
  : Element(otro), theNodes(otro.theNodes) 
  { theNodes.set_owner(this); }

//! @brief Operador asignación
template <int NNODOS>
XC::ElementBase<NNODOS> &XC::ElementBase<NNODOS>::operator=(const ElementBase &otro)
  {
    Element::operator=(otro);
    theNodes= otro.theNodes;
    theNodes.set_owner(this);
    return *this;
  }

//! @brief Return the number of external nodes.
template <int NNODOS>
int XC::ElementBase<NNODOS>::getNumExternalNodes(void) const
  { return theNodes.size(); }

//! @brief Devuelve un puntero al vector de nodos.
template <int NNODOS>
const XC::NodePtrsWithIDs &XC::ElementBase<NNODOS>::getNodePtrs(void) const
  { return theNodes; }

//! @brief Devuelve un puntero al vector de nodos.
template <int NNODOS>
XC::NodePtrsWithIDs &XC::ElementBase<NNODOS>::getNodePtrs(void)
  { return theNodes; }

//! @brief Asigna el material al elemento a partir del puntero being passed as parameter.
template <int NNODOS> template <class TIPOMAT>
TIPOMAT *ElementBase<NNODOS>::cast_material(const Material *ptr_mat)
  {
    TIPOMAT *retval= NULL;
    const TIPOMAT *tmp = dynamic_cast<const TIPOMAT *>(ptr_mat);
    if(tmp)
      retval= tmp->getCopy();
    else
      {
        std::cerr << "FATAL XC::ElementBase::cast_material - elemento: " << getTag() 
                  << " el material " << ptr_mat->getTag()
                  << " es de tipo inadecuado." << std::endl;
        abort();
      }
    if(!retval)
      {
	std::cerr << "FATAL XC::ElementBase::cast_material - elemento:" << getTag() <<
	  "no se pudo obtener una copia del material con identificador: " << ptr_mat->getTag() << std::endl;
        abort();
      }
    return retval;
  }

//! @brief Send members through the channel being passed as parameter.
template <int NNODOS>
int XC::ElementBase<NNODOS>::sendData(CommParameters &cp)
  {
    int res= Element::sendData(cp);
    res+= cp.sendMovable(theNodes,getDbTagData(),CommMetaData(6));
    return res;
  }

//! @brief Receives members through the channel being passed as parameter.
template <int NNODOS>
int XC::ElementBase<NNODOS>::recvData(const CommParameters &cp)
  {
    int res= Element::recvData(cp);
    res+= cp.receiveMovable(theNodes,getDbTagData(),CommMetaData(6));
    return res;
  }

//! @brief Devuelve la posición del centroide del elemento.
template <int NNODOS>
const Pos3d &XC::ElementBase<NNODOS>::getPosCdg(bool initialGeometry) const
  {
    static Pos3d retval;
    retval= theNodes.getPosCdg(initialGeometry);
    return retval;
  }

} //end of XC namespace
#endif
