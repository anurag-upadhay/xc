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
//NodePtrs.cpp

#include "NodePtrs.h"
#include "Element.h"
#include "utility/matrix/ID.h"
#include "utility/matrix/Matrix.h"
#include "domain/mesh/node/Node.h"
#include "domain/domain/Domain.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/geom/pos_vec/Pos3d.h"
#include "xc_utils/src/geom/pos_vec/Vector3d.h"
#include "xc_utils/src/base/utils_any.h"
#include "xc_utils/src/geom/d2/Plano3d.h"
#include "xc_utils/src/geom/d3/SemiEspacio3d.h"


//! @brief Constructor.
XC::NodePtrs::NodePtrs(Element *owr,const size_t &sz)
  : EntCmd(owr), theNodes(sz,nullptr) {}

//! @brief Constructor de copia.
XC::NodePtrs::NodePtrs(const NodePtrs &otro)
  : EntCmd(otro), theNodes(otro.theNodes.size(),nullptr) 
  {
    const size_t sz= theNodes.size();
    if(sz>0)
      {
        const ContinuaReprComponent *owr= dynamic_cast<const ContinuaReprComponent *>(Owner());
        assert(owr);
        Domain *dom= owr->getDomain();
        if(dom)
          setPtrs(dom,ID(otro.getTags()));
      }
  }

XC::NodePtrs &XC::NodePtrs::operator=(const NodePtrs &otro)
  {
    inic();
    EntCmd::operator=(otro);
    theNodes= vector_ptr_nodes(otro.theNodes.size(),nullptr);
    const size_t sz= theNodes.size();
    if(sz>0)
      {
        const ContinuaReprComponent *owr= dynamic_cast<const ContinuaReprComponent *>(Owner());
        assert(owr);
        Domain *dom= owr->getDomain();
        if(dom)
          setPtrs(dom,ID(otro.getTags()));
      }
    return *this;
  }

//! @brief Destructor.
XC::NodePtrs::~NodePtrs(void)
  {
    inic();
    theNodes.clear();
  }

void XC::NodePtrs::desconecta(void)
  {
    ContinuaReprComponent *owr= dynamic_cast<ContinuaReprComponent *>(Owner());
    assert(owr);
    for(iterator i= begin();i!= end();i++)
      {
        Node *tmp= *i;
        if(tmp)
          tmp->desconecta(owr);
      }
  }

//! @brief Pone a cero los punteros a nodo.
void XC::NodePtrs::inic(void)
  {
    desconecta();
    for(iterator i= begin();i!=end();i++)
      (*i)= nullptr;
  }

//! @brief Lee un objeto NodePtrs desde archivo
bool XC::NodePtrs::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(NodePtrs) Procesando comando: " << cmd << std::endl;
    if(cmd == "for_each")
      {
        const std::string nmbBlq= nombre_clase()+":for_each";
        const std::string &bloque= status.GetBloque();
	iterator i= begin();
	for(;i!= end();i++)
          (*i)->EjecutaBloque(status,bloque,nmbBlq);
        return true;
      }
    else if(cmd == "nodo")
      {
        const std::deque<boost::any> fnc_indices= status.Parser().SeparaIndices(this);
        const size_t sz= fnc_indices.size();
        const std::string posLectura= get_ptr_status()->GetEntradaComandos().getPosicionLecturaActual();
        if(sz>0)
          {
            const size_t i= convert_to_size_t(fnc_indices[0]); //Tag del nodo.
            if(i<size())
              {
                if(theNodes[i])
                  theNodes[i]->LeeCmd(status);
                else
                  {
                    status.GetBloque(); //Ignoramos entrada.
	            std::cerr << "El nodo de ídice " << i << " no está asignado."
                              << posLectura << std::endl;
                  }
              }
            else
              {
                status.GetBloque(); //Ignoramos entrada.
	        std::cerr << "Índice de nodo " << i << " fuera de rango."
                          << posLectura << std::endl;
              }
          }
        else
          {
            status.GetBloque(); //Ignoramos entrada.
	    std::cerr << "Se esperaba un índice de nodo."
                          << posLectura << std::endl;
          }
        return true;
      }
    else
      return EntCmd::procesa_comando(status);
  }

//! @brief Devuelve verdadero si alguno de los punteros
//! es nulo.
bool XC::NodePtrs::hasNull(void) const
  {
    bool retval= false;
    const size_t sz= theNodes.size();
    for(size_t i=0; i<sz; i++)
      {
        if(!theNodes[i])
          {
            retval= true;
            break;
          }
      }
    return retval;
  }

//! @brief Asigna los punteros a partir de los identificadores de elemento.
void XC::NodePtrs::setPtrs(Domain *theDomain, const ID &theNodeTags)
  {
    inic();
    const size_t sz= theNodeTags.Size();
    theNodes.resize(sz,nullptr);
    ContinuaReprComponent *owr= dynamic_cast<ContinuaReprComponent *>(Owner());
    assert(owr);
    for(size_t i=0; i<sz; i++)
      {
        theNodes[i]= theDomain->getNode(theNodeTags(i));
        if(theNodes[i])
          theNodes[i]->conecta(owr);
        else
          {
            std::cerr << "WARNING - NodePtrs::setDomain - node with tag ";
	    std::cerr << theNodeTags(i) << " does not exist in the domain\n";
          }
      }
  }

//! @brief Devuelve un iterador al elemento cuyo tag se pasa como parámetro.
XC::NodePtrs::iterator XC::NodePtrs::find(const int &tag)
  {
    iterator retval= end();
    for(iterator i= begin();i!=end();i++)
      if((*i)->getTag() == tag)
        retval= i;
    return retval;
  }

//! @brief Devuelve un iterador al elemento cuyo tag se pasa como parámetro.
XC::NodePtrs::const_iterator XC::NodePtrs::find(const int &tag) const
  {
    const_iterator retval= end();
    for(const_iterator i= begin();i!=end();i++)
      if((*i)->getTag() == tag)
        retval= i;
    return retval;
  }

//! @brief Devuelve el índice del nodo cuyo apuntador se pasa como parámetro.
int XC::NodePtrs::find(const Node *nPtr) const
  {
    int retval= -1, conta=0;
    for(const_iterator i= begin();i!=end();i++,conta++)
      if((*i) == nPtr)
        {
          retval= conta;
          break;
        }
    return retval;
  }

//! @brief Devuelve un puntero al nodo i.
XC::Node *XC::NodePtrs::getNodePtr(const size_t &i)
  { return theNodes[i]; }

//! @brief Devuelve una referencia al nodo i.
XC::NodePtrs::const_reference XC::NodePtrs::operator()(const size_t &i) const
  { return theNodes[i]; }

//! @brief Devuelve una referencia al nodo i.
XC::NodePtrs::const_reference XC::NodePtrs::operator[](const size_t &i) const
  { return theNodes[i]; }

//!@brief Asigna el puntero al nodo i.
void XC::NodePtrs::set_node(const size_t &i,Node *n)
  {
    ContinuaReprComponent *owr= dynamic_cast<ContinuaReprComponent *>(Owner());
    if(theNodes[i])
      {
        if(theNodes[i]!=n)
          {
            theNodes[i]->desconecta(owr);
            theNodes[i]= n;
            if(n)
              theNodes[i]->conecta(owr);
          }
      }
    else
      {
        theNodes[i]= n;
        if(n)
          theNodes[i]->conecta(owr);
      }
  }

//! @brief Devuelve un vector que contiene los grados de libertad de cada nodo.
XC::ID XC::NodePtrs::getNumDOFs(void) const
  {
    const int numNodes= size();
    ID retval(numNodes);
    for(int i=0; i<numNodes; i++)
      retval(i)= theNodes[i]->getNumberDOF();
    return retval;
  }

//! @brief Devuelve el total de grados de libertad de los nodos.
int XC::NodePtrs::getTotalDOFs(void) const
  {
    const int numNodes= size();
    int retval= 0;
    for(int i=0; i<numNodes; i++)
      retval+= theNodes[i]->getNumberDOF();
    return retval;
  }

//! @brief Devuelve el valor máximo de la coordenada i de los nodos.
double XC::NodePtrs::MaxCooNod(int icoo) const
  {
    assert(!theNodes.empty());
    const_iterator i= begin();
    double retval= (*i)->getCrds()[icoo];
    i++;
    for(;i!=end();i++)
      retval= std::max(retval,(*i)->getCrds()[icoo]);
    return retval;
  }

//! @brief Devuelve el valor mínimo de la coordenada i de los nodos.
double XC::NodePtrs::MinCooNod(int icoo) const
  {
    assert(!theNodes.empty());
    const_iterator i= begin();
    double retval= (*i)->getCrds()[icoo];
    i++;
    for(;i!=end();i++)
      retval= std::min(retval,(*i)->getCrds()[icoo]);
    return retval;
  }

//! @brief Devuelve un vector con los tags de los nodos.
const std::vector<int> &XC::NodePtrs::getTags(void) const
  {
    static std::vector<int> retval;
    const size_t sz= theNodes.size();
    retval.resize(sz);
    for(size_t i=0; i<sz; i++)
      if(theNodes[i]) retval[i]= theNodes[i]->getTag();
    return retval;    
  }

std::vector<int> XC::NodePtrs::getIdx(void) const
  {
    const size_t sz= theNodes.size();
    std::vector<int> retval(sz,-1);
    for(size_t i=0; i<sz; i++)
      if(theNodes[i])
        retval[i]= theNodes[i]->getIdx();
    return retval;     
  }

//! @brief Devuelve una matriz con las coordenadas de los nodos.
const XC::Matrix &XC::NodePtrs::getCoordinates(void) const
  {
    static Matrix retval;
    const size_t sz= theNodes.size();
    retval= Matrix(sz,3);
    for(size_t i=0; i<sz; i++)
      {
        if(theNodes[i])
          {
            const Vector &coo= theNodes[i]->getCrds();
            retval(i,0)= coo[0];
            retval(i,1)= coo[1];
            retval(i,2)= coo[2];
          }
      }
    return retval;
  }

//! @brief Devuelve la posición del nodo cuyo índice se
//! pasa como parámetro.
Pos3d XC::NodePtrs::getPosNodo(const size_t &i,bool geomInicial) const
  {
    if(geomInicial)
      return theNodes[i]->getPosInicial3d();
    else
      return theNodes[i]->getPosFinal3d();
  }

//! @brief Devuelve una matriz con las posiciones de los nodos.
std::list<Pos3d> XC::NodePtrs::getPosiciones(bool geomInicial) const
  {
    std::list<Pos3d> retval;
    const size_t sz= theNodes.size();
    for(size_t i=0;i<sz;i++)
      retval.push_back(getPosNodo(i,geomInicial));
    return retval;
  }

//! @brief Devuelve el centro de gravedad de las posiciones de los nodos.
Pos3d XC::NodePtrs::getPosCdg(bool geomInicial) const
  {
    Pos3d retval;
    const size_t sz= theNodes.size();
    if(sz>0)
      {
        Vector3d tmp= getPosNodo(0,geomInicial).VectorPos();;
        for(size_t i=1;i<sz;i++)
          tmp+= getPosNodo(i,geomInicial).VectorPos();
        tmp= tmp * 1.0/sz;
        retval= Pos3d()+tmp;
      }
    return retval;
  }

//! @brief Devuelve verdadero si los nodos están contenidos en el semiespacio.
bool XC::NodePtrs::In(const SemiEspacio3d &semiEsp,const double &tol,bool geomInicial) const
  {
    bool retval= true;
    std::list<Pos3d> posiciones= getPosiciones(geomInicial);
    for(std::list<Pos3d>::const_iterator i= posiciones.begin();i!=posiciones.end();i++)
      if(!semiEsp.In(*i))
        { retval= false; break; }
    return retval;
  }

//! @brief Devuelve verdadero si los nodos están fuera en el semiespacio.
bool XC::NodePtrs::Out(const SemiEspacio3d &semiEsp,const double &tol,bool geomInicial) const
  {
    SemiEspacio3d complementario(semiEsp);
    complementario.Swap();
    return In(complementario,tol,geomInicial);
  }

//! @brief Devuelve verdadero si los nodos están a uno y otro lado del plano.
bool XC::NodePtrs::Corta(const Plano3d &plano,bool geomInicial) const
  {
    bool in= In(plano,0.0,geomInicial);
    bool out= Out(plano,0.0,geomInicial);
    return (!in && !out);
  }

//! @brief Devuelve el nodo más próximo al punto que se pasa como parámetro.
XC::Node *XC::NodePtrs::getNearestNode(const Pos3d &p,bool geomInicial)
  {
    Node *retval= nullptr;
    double d= DBL_MAX;
    const size_t sz= theNodes.size();
    if(!theNodes.empty() && !hasNull())
      {
        d= theNodes[0]->getDist2(p,geomInicial);
        retval= theNodes[0];
        double tmp;
        for(size_t i=1;i<sz;i++)
          {
            tmp= theNodes[i]->getDist2(p,geomInicial);
            if(tmp<d)
              {
                d= tmp;
                retval= theNodes[i];
              }
          }
      }
    return retval;
  }

//! @brief Devuelve el nodo más próximo al punto que se pasa como parámetro.
const XC::Node *XC::NodePtrs::getNearestNode(const Pos3d &p,bool geomInicial) const
  {
    NodePtrs *this_no_const= const_cast<NodePtrs *>(this);
    return this_no_const->getNearestNode(p,geomInicial);
  }

//! @brief Devuelve la posición del puntero a nodo en el array.
int XC::NodePtrs::getIndiceNodo(const Node *ptrNod) const
  {
    int retval= -1;
    const size_t sz= theNodes.size();
    for(size_t i=0;i<sz;i++)
      if(theNodes[i]==ptrNod)
        {
          retval= i;
          break;
        }
    return retval;
  }


//! @brief Anula la longitud, área o volumen de los
//! nodos conectados.
void XC::NodePtrs::resetTributarias(void) const
  {
    const size_t sz= theNodes.size();
    for(size_t i=0;i<sz;i++)
      theNodes[i]->resetTributaria();    
  }

//! @brief Agrega al la magnitud tributaria de cada nodo i
//! la componente i del vector que se pasa como parámetro.
void XC::NodePtrs::vuelcaTributarias(const std::vector<double> &t) const
  {
    const size_t sz= theNodes.size();
    assert(sz== t.size());
    for(size_t i=0;i<sz;i++)
      theNodes[i]->addTributaria(t[i]);
  }

//! @brief Devuelve un vector que contiene los factores de distribución
//! de cada nodo que corresponden al modo que se pasa como parámetro.
XC::Vector XC::NodePtrs::getDistributionFactor(int mode) const
  {
    const int nrows= getTotalDOFs();
    Vector retval(nrows);
    // Calculamos el factor de distribución
    const int numNodes= size();
    int loc = 0;
    for(int i=0; i<numNodes; i++)
      {
        const Vector df= theNodes[i]->getDistributionFactor(mode);
        for(int j=0; j<df.Size(); j++)
          { retval(loc++)= df[j]; }
      }
    return retval;
  }

//! @brief Devuelve los vectores correspondientes a cada nodo colocados por filas.
XC::Matrix XC::NodePtrs::getNodeVectors(const Vector &v) const
  {
    const int numNodes= size();
    const ID dofs= getNumDOFs();
    const int ndof_max= dofs.max();
    Matrix retval(numNodes,ndof_max);
    int loc = 0;
    for(int i=0; i<numNodes; i++)
      {
        const int nc= theNodes[i]->getNumberDOF();
        for(int j=0; j<nc; j++)
          { retval(i,j)= v[loc++]; }
      }
    return retval;
    
  }

//! Devuelve la propiedad del objeto cuyo código se pasa
//! como parámetro.
any_const_ptr XC::NodePtrs::GetProp(const std::string &cod) const
  {
    if(cod == "num_nod")
      {
        tmp_gp_szt= theNodes.size();
        return any_const_ptr(tmp_gp_szt);
      }
    else if(cod=="getTagNode")
      {
        const size_t i= popSize_t(cod);
        const Node *tmp= theNodes[i];
        if(tmp)
          tmp_gp_int= tmp->getTag();
        else
	  std::cerr << "El nodo de ídice " << i << " no está asignado." << std::endl;
        return any_const_ptr(tmp_gp_int);
      }
    else if(cod=="getTagNearestNode")
      {
        const Pos3d p= popPos3d(cod);
        const Node *tmp= getNearestNode(p);
        tmp_gp_int= tmp->getTag();
        return any_const_ptr(tmp_gp_int);
      }
    else
      return EntCmd::GetProp(cod);
  }